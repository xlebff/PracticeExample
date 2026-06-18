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
    initComboBoxes();
    initButton();

    /* working with params:
     * loading settings,
     * generate signal.
     * after adding other signals, must be updated to "initSignal" or sth */
    params.loadFromSettings();
    params.sanitize(this);
    signal = params.generateASignal(infBit);

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

    double arg = 2 * PI * f / fd;

    for (int i = 0; i < N; ++i) {
        int X = n1 + i;
        x[i] = X;

        re[i] = A * (cos(arg * X));
        im[i] = A * (sin(arg * X));

        //re[i] = signal[0][i];
        //im[i] = signal[1][i];
    }


    /* DRAWING */

    clearPlot();

    minMax(minY, maxY, im, N);
    minX = n1;
    maxX = n2 * MARGIN;

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
    const int fd = params.fd;
    const int df = params.df;

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
        qDebug() << "Error during memory allocation: " << endl <<
            "In FFTW: " << in_fftw << ";" << endl <<
            "Out FFTW: " << out_fftw << endl;
        return;
    }

    /* getting values for our chart
     * and annulate Im part to see both positive and negative picks */

    for (int i = 0; i < N; ++i) {
        in_fftw[i][0] = signal[0][i];
        in_fftw[i][1] = signal[1][i];
    }

    for (int i = N; i < N_fft; ++i) {
        in_fftw[i][0] = 0.0;
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
        // out_fftw_graf[i] = (out_fftw_graf[i]) / (N_fft * N_fft);
        out_fftw_graf[i] = (out_fftw_graf[i]) / (N * N * df);

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
    maxX = fd / 2 * MARGIN;

    minMax(minY, maxY, out_fftw_graf, N_fft);

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->yAxis->setLabel("Amplitude");
    m_plot->xAxis->setLabel("Frequency");

    m_plot->addGraph();
    m_plot->graph(0)->setData(x, out_fftw_graf);
    m_plot->graph(0)->setPen(QPen(Qt::red));

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

    double firstI = signal[0][0];
    double firstQ = signal[1][0];

    clearPlot();

    QRect screenRect = geometry();
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

    phaseVector = new QCPItemLine(m_plot);
    phaseVector->setProperty("tempItem", true);
    phaseVector->start->setCoords(0, 0);
    phaseVector->end->setCoords(firstI, firstQ);
    phaseVector->setHead(QCPLineEnding::esSpikeArrow);
    phaseVector->setPen(QPen(Qt::red, 2));

    double angleRad = std::atan2(firstQ, firstI);
    double angleDeg = angleRad * 180.0 / PI;

    angleText = new QCPItemText(m_plot);
    angleText->setProperty("tempItem", true);
    angleText->position->setCoords(0, 0);
    angleText->position->setCoords(firstI * 0.7, firstQ * 0.7);
    angleText->setText(QString("%1°").arg(angleDeg, 0, 'f', 1));
    angleText->setColor(Qt::red);
    angleText->setFont(QFont(font().family(), 11, QFont::Bold));

    m_plot->xAxis->setLabel("I (синфазная)");
    m_plot->yAxis->setLabel("Q (квадратурная)");
    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->replot();
}

void MainWindow::drawMeandr()
{
    /* const int N = params.N;
    const int fd = params.fd;

    QVector<double> time(N), phase(N);

    for (int i = 0; i < N; ++i) {
        time[i] = (double)i / (double)fd;
    }

    for (int i = 0; i < N; ++i) {
        double I = signal[0][i];
        double Q = signal[1][i];
        phase[i] = std::atan2(Q, I);
    }

    QVector<double> X(N), Y(N);
    for (int i = 0; i < N; ++i) {
        Y[i] = (i < infBit.size()) ? infBit[i] : 0;
        X[i] = i;
    }

    clearPlot();

    minMax(minX, maxX, time, N);
    minMax(minY, maxY, phase, N);

    double infoMinY = 0, infoMaxY = 1;
    if (infBit.size() > 0) {
        infoMinY = infBit[0];
        infoMaxY = infBit[0];
        for (int i = 1; i < infBit.size(); ++i) {
            infoMinY = infBit[i] < infoMinY ? infBit[i] : infoMinY;
            infoMaxY = infBit[i] > infoMaxY ? infBit[i] : infoMaxY;
        }
    }
    infoMinY /= MARGIN;
    infoMaxY *= MARGIN;

    if (infoMinY < minY) minY = infoMinY;
    if (infoMaxY > maxY) maxY = infoMaxY;

    m_plot->addGraph();
    m_plot->graph(0)->setData(time, phase);
    m_plot->graph(0)->setPen(QPen(Qt::red));
    m_plot->graph(0)->setName("Phase");

    m_plot->addGraph();
    m_plot->graph(1)->setData(X, Y);
    m_plot->graph(1)->setPen(QPen(Qt::green));
    m_plot->graph(1)->setName("Info bits");

    m_plot->xAxis->setLabel("Time (s)");
    m_plot->yAxis->setLabel("Phase (rad) / Bits");

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->legend->setVisible(true);

    m_plot->replot(); */

    const int N = params.N;

    QVector<double> X(N), Y(N);
    for (int i = 0; i < N; ++i) {
        Y[i] = infBit[i];
        X[i] = i;
    }

    clearPlot();

    minMax(minX, maxX, X, N);
    minMax(minY, maxY, Y, N);

    m_plot->addGraph();
    m_plot->graph(0)->setData(X, Y);
    m_plot->graph(0)->setPen(QPen(Qt::green));
    m_plot->graph(0)->setName("Info bits");

    m_plot->xAxis->setLabel("Time (s)");
    m_plot->yAxis->setLabel("Phase (rad) / Bits");

    m_plot->xAxis->setRange(minX, maxX);
    m_plot->yAxis->setRange(minY, maxY);

    m_plot->replot();
}

void MainWindow::onChartComboSwitched(int index)
{
    currentChart = index;
    drawChart[currentChart](); /* simple! */
}

void MainWindow::onSignalComboSwitched(int index)
{
    currentSignal = index;

    switch (currentSignal)
    {
    case A_MODULATION:
        signal = params.generateASignal(infBit);
        break;
    case FSK_MODULATION:
        signal = params.generateFSKSignal(infBit);
        break;
    case PHASE_MODULATION:
        signal = params.generatePhaseSignal(infBit);
        break;
    default:
        signal = params.generateClearSignal();
        for (int i = 0; i < infBit.capacity(); ++i)
            infBit[i] = 0;
        break;
    }

    onChartComboSwitched(currentChart);
}

void MainWindow::clearPlot()
{
    m_plot->legend->setVisible(false);

    m_plot->clearPlottables();
    m_plot->clearGraphs();

    for (int i = m_plot->itemCount() - 1; i >= 0; --i) {
        QCPAbstractItem *item = m_plot->item(i);
        if (item->property("tempItem").toBool()) {
            m_plot->removeItem(item);
        }
    }

    if (angleText || phaseVector) {
        angleText->setVisible(false);
        phaseVector->setVisible(false);
    }
}

void MainWindow::writeFile()
{
    const int N = params.N;
    short result[2];

    const char *fileName;
    switch (currentSignal)
    {
    case A_MODULATION:
        fileName = AMPLITUDE_MODE_RESULT_FILE_PATH;
        break;
    case FSK_MODULATION:
        fileName = FREQUENCY_MODE_RESULT_FILE_PATH;
        break;
    case PHASE_MODULATION:
        fileName = PHASE_MODE_RESULT_FILE_PATH;
        break;
    default:
        fileName = CARRIER_MODE_RESULT_FILE_PATH;
        break;
    }

    FILE* f = fopen(fileName, "w+b");
    if (!f) {
        QMessageBox::critical(this, "Error", "Error during opening result file");
        qDebug() << "Error during opening " << fileName << endl;
        return;
    }

    for (int i = 0; i < N; ++i) {
        result[0] = static_cast<short>(signal[0][i]);
        result[1] = static_cast<short>(signal[1][i]);

        size_t written = fwrite(result, sizeof(short), 2, f);
        if (written != 2) {
            QMessageBox::warning(this, "Warning", "Error during writting in phase mode");
            qDebug() << "Error during writting in phase mode" << endl;
        }
    }

    fclose(f);
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
}

/* re-reading 'params.ini' after clicking 'Reload' */

void MainWindow::onReloadButtonClicked()
{
    params.loadFromSettings();
    params.sanitize(this);
    signal = params.generateASignal(infBit);
    onSignalComboSwitched(currentSignal);
}
