#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "signalparams.h"

QComboBox *initCombo(const char *names[],
                     const int size)
{
    QComboBox *combo = new QComboBox();

    /* filling combo box with options according to info from .h */
    QStandardItemModel *model = new QStandardItemModel(combo);
    for (int i = 0; i < size; ++i) {
        QStandardItem *item = new QStandardItem(QString(names[i]));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        model->appendRow(item);
    }

    combo->setModel(model);

    return combo;
}

void MainWindow::initPlot()
{
    /* plot! main settings */
    m_plot = new QCustomPlot();
    ui->gridLayout->addWidget(m_plot, 1, 0, 1, 1);

    /* setting up the gradient */
    gradient.clearColorStops();
    gradient.setColorStopAt(0.0, QColor("darkblue"));
    gradient.setColorStopAt(0.3, QColor("cyan"));
    gradient.setColorStopAt(0.6, QColor("yellow"));
    gradient.setColorStopAt(1.0, QColor("red"));

    /* connecting mouse events for measurements */
    connect(m_plot, &QCustomPlot::mousePress,   this, &MainWindow::onMousePress);
    connect(m_plot, &QCustomPlot::mouseMove,    this, &MainWindow::onMouseMove);
    connect(m_plot, &QCustomPlot::mouseRelease, this, &MainWindow::onMouseRelease);
}

void MainWindow::initComboBoxes()
{
    m_ChartComboBox = initCombo(CHARTS_NAMES, CHARTS_QUANTITY);
    /* adding methods to the vector for quick switching between charts */
    for (std::function<void()> item : { std::function<void()>([this]() { drawRI(); }),
                                        std::function<void()>([this]() { drawFFTSpectrum(); }),
                                        std::function<void()>([this]() { drawSTFT(); }),
                                        std::function<void()>([this]() { drawIQPlane(); }),
                                        std::function<void()>([this]() { drawPhase(); }),
                                        std::function<void()>([this]() { drawACF(); }),
                                        std::function<void()>([this]() { drawWrappedPhase(); }) } )
    {
        drawChart.push_back(item);
    }

    /* connecting combo box */
    connect(m_ChartComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onChartComboSwitched);


    m_SignalComboBox = initCombo(SIGNALS_NAMES, SIGNALS_QUANTITY);
    /*generateSignal.push_back({
        [this](const bool) -> QVector<QVector<double>> {
            return params.generateASignal(mode);
        }
    });
    generateSignal.push_back({
        [this](const bool) -> QVector<QVector<double>> {
            return params.generateFSKSignal(mode);
        }
    });
    generateSignal.push_back({
        [this](const bool) -> QVector<QVector<double>> {
            return params.generatePhaseSignal(mode);
        }
    });*/

    /* connecting combo box */
    connect(m_SignalComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSignalComboSwitched);


    m_ModeComboBox = initCombo(MODES_NAMES, MODES_QUANTITY);
    /* connecting combo box */
    connect(m_ModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeComboSwitched);


    ui->gridLayout->addWidget(m_ChartComboBox);
    ui->gridLayout->addWidget(m_SignalComboBox);
    ui->gridLayout->addWidget(m_ModeComboBox);
}

void MainWindow::initButton()
{
    m_WriteFileButton = new QPushButton("Write", this);
    ui->gridLayout->addWidget(m_WriteFileButton);
    connect(m_WriteFileButton, SIGNAL(clicked()), this, SLOT(writeFile()));
}
