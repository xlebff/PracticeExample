#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::initCombo()
{
    m_PlotComboBox = new QComboBox();

    /* filling combo box with options according ro info from .h */
    QStandardItemModel *model = new QStandardItemModel(m_PlotComboBox);
    for (int i = 0; i < CHARTS_QUANTITY; ++i) {
        QStandardItem *item = new QStandardItem(QString(CHARTS_NAMES[i]));
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        model->appendRow(item);
    }

    m_PlotComboBox->setModel(model);

    ui->gridLayout->addWidget(m_PlotComboBox);

    /* adding methods to the vector for quick switching between charts */
    for (std::function<void()> item : { std::function<void()>([this]() { drawRI(); }),
                                        std::function<void()>([this]() { drawFFTSpectrum(); }),
                                        std::function<void()>([this]() { drawSTFT(); }),
                                        std::function<void()>([this]() { drawIQPlane(); }),
                                        std::function<void()>([this]() { drawPhase(); }),
                                        std::function<void()>([this]() { drawPhase(); }),
                                        std::function<void()>([this]() { drawACF(); }),
                                        std::function<void()>([this]() { drawACF(); }),
                                        std::function<void()>([this]() { drawPDF(); }) } )
    {
        drawChart.push_back(item);
    }

    /* connecting combo box */
    connect(m_PlotComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboSwitched);
}
