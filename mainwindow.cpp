#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <fftw3.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand(time(0)); /* for generating info bits */

    /* from interactive measurements */
    initPlot();
    initLinesAndText();

    /* ui control */
    initCombo();

    /* working with params:
     * loading settings,
     * generate signal.
     * after adding other signals, must be updated to "initSignal" or sth */
    params.loadFromSettings();
    params.sanitize(this);
    signal = params.generateFSKSignal();

    /* init chart */
    drawChart[0]();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::drawRI()
{
    const int N = params.N;
    const int f = params.f;
    const int A = params.A;
    const int fd = params.fd;
    const int n1 = params.n1;
    const int n2 = params.n2;

    QVector<double> x(N), re(N), im(N);

    for (int i = 0; i < N; ++i) {
        int X = n1 + i;
        x[i] = X;
        re[i] = A * (cos(2 * PI * f * X / fd));
        im[i] = A * (sin(2 * PI * f * X / fd));
    }

    double tempMinY, tempMaxY;
    minMax(tempMinY, tempMaxY, re, N);
    minMax(minY, maxY, im, N);
    if (tempMinY < minY) minY = tempMinY;
    if (tempMaxY > maxY) maxY = tempMaxY;
    minX = n1;
    maxX = n2;

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->xAxis->setLabel("Time (s)");
    m_plot->yAxis->setLabel("Re, Im");

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->addGraph();
    m_plot->graph(0)->setData(x, re);
    m_plot->graph(0)->setPen(QPen(Qt::blue));
    m_plot->graph(0)->setName("Real");

    m_plot->addGraph();
    m_plot->graph(1)->setData(x, im);
    m_plot->graph(1)->setPen(QPen(Qt::red));
    m_plot->graph(1)->setName("Imaginary");

    /* legend, `cause there is two distinct lines */
    m_plot->legend->setVisible(true);

    m_plot->replot();
}

void MainWindow::drawFFTSpectrum()
{
    const int N = params.N;
    const int f = params.f;
    const int fd = params.fd;
    const int n1 = params.n1;
    const int A = params.A;

    /* getting closest power of 2 to N for fftw */
    int N_fft = 1;
    while (N_fft < N) N_fft <<= 1;

    QVector<double> out_fftw_graf(N_fft), x(N_fft);
    fftw_complex *in_fftw, *out_fftw;
    fftw_plan plan;

    /* allocating memory for fftw */
    in_fftw = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N_fft);
    out_fftw = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N_fft);

    /* if memory allocation failed */
    if (!in_fftw || !out_fftw) {
        QMessageBox::critical(this, "Error", "Error during memory allocation");
        qDebug() << "Error during memory allocation: " << Qt::endl <<
            "In FFTW: " << in_fftw << ";" << Qt::endl <<
            "Out FFTW: " << out_fftw << Qt::endl;
        return;
    }

    /* getting values for our chart
     * and annulate Im part to see both positive and negative picks */
    for (int i = 0; i < N_fft; ++i) {
        double arg = 2 * PI * f * (i + n1) / fd;
        in_fftw[i][0] = A * (cos(arg));
        in_fftw[i][1] = 0.0;
    }

    /* creating plan and checking if it`s ok */
    plan = fftw_plan_dft_1d(N_fft, in_fftw, out_fftw, FFTW_FORWARD, FFTW_ESTIMATE);
    if (!plan) {
        QMessageBox::critical(this, "Error", "FFTW plan creation failed");
        fftw_free(in_fftw);
        fftw_free(out_fftw);
        return;
    }
    fftw_execute(plan);

    /* filling up graf vector using formula "I*I + Q*Q" */
    for (int i = 0; i < N_fft; ++i) {
        out_fftw_graf[i] = (out_fftw[i][0])*(out_fftw[i][0]) + (out_fftw[i][1])*(out_fftw[i][1]);
        out_fftw_graf[i] = (out_fftw_graf[i]) / (N_fft * N_fft);

        double df = static_cast<double>(fd) / N_fft;
        x[i] = (i < N_fft/2) ? i * df : (i - N_fft) * df;
    }

    /* cleaning up! */
    fftw_destroy_plan(plan);
    fftw_free(in_fftw);
    fftw_free(out_fftw);


    /* DRAWING */

    clearPlot();

    minX = -fd / 2;
    maxX = fd / 2;

    minMax(minY, maxY, out_fftw_graf, N_fft);

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->xAxis->setLabel("Amplitude");
    m_plot->yAxis->setLabel("Frequency");

    m_plot->addGraph();
    m_plot->graph(0)->setData(x, out_fftw_graf);
    m_plot->graph(0)->setPen(QPen(Qt::red));

    m_plot->replot();
}

void MainWindow::drawSTFT()
{
    const int windowSize = params.window;
    const int hopSize = params.hop;
    const int windowsQuantity = params.windowsQuantity;

    /* allocating memory for fftw
     * windowSize is a power of 2 by default,
     * but we sholud check it on sanitize, i guess */
    fftw_complex *in_fftw = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * windowSize);
    fftw_complex *out_fftw = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * windowSize);
    fftw_plan plan = fftw_plan_dft_1d(windowSize, in_fftw, out_fftw, FFTW_FORWARD, FFTW_ESTIMATE);

    QVector<QVector<double>> magnitudeSpectrogram(windowsQuantity);

    for (int frame = 0; frame < windowsQuantity; ++frame) {
        int start = frame * hopSize;
        for (int j = 0; j < windowSize; ++j) {
            double I = signal[0][start + j];
            double Q = signal[1][start + j];
            double hann = 0.5 * (1.0 - cos((2.0 * PI * j) / (windowSize - 1)));
            in_fftw[j][0] = I * hann;
            in_fftw[j][1] = Q * hann;
        }
        fftw_execute(plan);

        QVector<double> magFrame(windowSize);
        for (int k = 0; k < windowSize; ++k) {
            double re = out_fftw[k][0];
            double im = out_fftw[k][1];
            double linearMag = sqrt(re*re + im*im);
            magFrame[k] = 20.0 * log10(linearMag + 1e-10);
        }

        magnitudeSpectrogram[frame] = magFrame;
    }

    /* cleaning up! */
    fftw_destroy_plan(plan);
    fftw_free(in_fftw);
    fftw_free(out_fftw);

    /* DRAWING */

    clearPlot();

    minX = 0;
    maxX = windowsQuantity - 1;
    minY = 0;
    maxY = windowSize - 1;

    colorMap = new QCPColorMap(m_plot->xAxis, m_plot->yAxis);

    int numFrames = windowsQuantity;
    int numBins = windowSize;

    colorMap->data()->setSize(numFrames, numBins);
    colorMap->data()->setRange(QCPRange(0, maxX), QCPRange(0, maxY));

    for (int i = 0; i < numFrames; ++i) {
        for (int j = 0; j < numBins; ++j) {
            colorMap->data()->setCell(i, j, magnitudeSpectrogram[i][j]);
        }
    }

    colorMap->setGradient(gradient);

    colorMap->rescaleDataRange();

    /* gradient at the right */
    colorScale = new QCPColorScale(m_plot);
    m_plot->plotLayout()->addElement(0, 1, colorScale);
    colorScale->setType(QCPAxis::atRight);
    colorMap->setColorScale(colorScale);
    colorScale->axis()->setLabel("Magnitude (dB)");

    m_plot->xAxis->setLabel("Time Frame");
    m_plot->yAxis->setLabel("Frequency Bin");

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->replot();
}

void MainWindow::drawIQPlane()
{
    const int N = params.N;
    const int A = params.A;

    QVector<double> iData(N), qData(N);
    for (int i = 0; i < N; ++i) {
        iData[i] = signal[0][i];
        qData[i] = signal[1][i];
    }

    /* DRAWING */

    clearPlot();

    QRect screenRect = MainWindow::window()->geometry();
    float coef = (float)screenRect.width() / (float)screenRect.height();

    minX = -A * MARGIN * coef;
    maxX = A * MARGIN * coef;
    minY = -A * MARGIN;
    maxY = A * MARGIN;

    m_plot->addGraph();

    m_plot->graph(0)->setData(iData, qData);

    m_plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    m_plot->graph(0)->setScatterStyle(QCPScatterStyle(
        QCPScatterStyle::ssCircle, QColor("blue"), 6));

    m_plot->graph(0)->rescaleAxes();
    m_plot->xAxis->setLabel("I (синфазная)");
    m_plot->yAxis->setLabel("Q (квадратурная)");
    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->replot();
}

void MainWindow::drawPhase()
{
    const int N = params.N;
    const int fd = params.fd;

    QVector<double> time(N), envelope(N), phase(N), unwrappedPhase(N);

    for (int i = 0; i < N; ++i) {
        time[i] = (double)i / (double)fd;
    }

    /* calculating envelope and phase */
    for (int i = 0; i < N; ++i) {
        double I = signal[0][i];
        double Q = signal[1][i];
        /* getting module of the amplitude */
        envelope[i] = std::sqrt(I*I + Q*Q);
        phase[i] = std::atan2(Q, I); /* [-PI, PI] */
    }

    /* unwrapping the phase */
    unwrappedPhase[0] = phase[0];
    double cumulativeShift = 0.0;
    for (int i = 1; i < N; ++i) {
        double diff = phase[i] - phase[i-1];
        /* if the leap is greater than PI switching the branch */
        if (diff > M_PI)       cumulativeShift -= 2.0 * M_PI;
        else if (diff < -M_PI) cumulativeShift += 2.0 * M_PI;
        unwrappedPhase[i] = phase[i] + cumulativeShift;
    }

    /* DRAWING */

    clearPlot();

    minMax(minX, maxX, time, N);
    minMax(minY, maxY, envelope, N);

    /* if there is no A-modulation, all envelope values will be the same
     * but there is floating numbers! so it`s just veery-very close to each other,
     * and i didn`t think up anything better than this */
    if (minY > maxY - 10) {
        minY = 0;
    }

    /* additional right axis */
    rightAxis = m_plot->axisRect()->addAxis(QCPAxis::atRight);
    rightAxis->setLabel("Phase (rad)");

    double minPhase = *std::min_element(unwrappedPhase.constBegin(), unwrappedPhase.constEnd());
    double maxPhase = *std::max_element(unwrappedPhase.constBegin(), unwrappedPhase.constEnd());
    double margin = (maxPhase - minPhase) * 0.05 + 0.5;
    rightAxis->setRange(minPhase - margin, maxPhase + margin);

    m_plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignRight);

    /* envelope */
    QCPGraph *envGraph = m_plot->addGraph(m_plot->xAxis, m_plot->yAxis);
    envGraph->setData(time, envelope);
    envGraph->setPen(QPen(Qt::blue, 1));
    envGraph->setName("Envelope");

    /* phase */
    QCPGraph *phaseGraph = m_plot->addGraph(m_plot->xAxis, rightAxis);
    phaseGraph->setData(time, unwrappedPhase);
    phaseGraph->setPen(QPen(Qt::red, 1));
    phaseGraph->setName("Phase");

    m_plot->xAxis->setLabel("Time (s)");
    m_plot->yAxis->setLabel("Amplitude");

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    /* legend, `cause there is two distinct lines */
    m_plot->legend->setVisible(true);

    m_plot->replot();
}

void MainWindow::drawACF()
{}

void MainWindow::drawPDF()
{}

void MainWindow::onComboSwitched(int index)
{
    drawChart[index](); /* simple! */
}

void MainWindow::clearPlot()
{
    m_plot->legend->setVisible(false);

    m_plot->clearPlottables();
    m_plot->clearGraphs();

    /* if there was gradient,
     * clearing it and updating layout */
    if (colorScale) {
        m_plot->plotLayout()->remove(colorScale);
        colorScale = nullptr;

        colorMap->clearData();

        m_plot->plotLayout()->simplify();
        m_plot->plotLayout()->updateLayout();
    }

    /* if there was additional right axis */
    if (rightAxis) {
        rightAxis->setVisible(false);
        rightAxis = nullptr;
    }
}


/* simple helper that setting up min and max values
 * for min and max vars
 * from the "array" with size "size" */
void MainWindow::minMax(double &min, double &max, const QVector<double> &array, const int &size)
{
    min = array[0]; max = array[0];
    for (int i = 1; i < size; ++i) {
        min = array[i] < min ? array[i] : min;
        max = array[i] > max ? array[i] : max;
    }
    max *= MARGIN;
    min *= MARGIN;
}
