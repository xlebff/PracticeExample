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
    connect(m_SignalComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSignalComboSwitched);

    m_ModeComboBox = initCombo(MODES_NAMES, MODES_QUANTITY);
    connect(m_ModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeComboSwitched);

    m_WriteFileButton = new QPushButton("Write", this);
    connect(m_WriteFileButton, SIGNAL(clicked()), this, SLOT(writeFile()));

    m_ReloadButton = new QPushButton("Reload", this);
    connect(m_ReloadButton, SIGNAL(clicked()), this, SLOT(onReloadButtonClicked()));

    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(m_ChartComboBox);
    controlLayout->addWidget(m_SignalComboBox);
    controlLayout->addWidget(m_ModeComboBox);
    controlLayout->addWidget(m_WriteFileButton);
    controlLayout->addWidget(m_ReloadButton);

    ui->gridLayout->addLayout(controlLayout, 2, 0, 1, 1);
}

void MainWindow::initButton()
{
}
