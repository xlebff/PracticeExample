#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initPlot();
    initLinesAndText();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initPlot()
{
    m_plot = new QCustomPlot();
    ui->gridLayout->addWidget(m_plot, 1, 0, 1, 1);

    QVector<double> x{ -45, 46, -25, -12, 25 };
    QVector<double> y{ -43, 42, -24, 10, 26 };
    m_plot->addGraph();
    m_plot->graph(0)->setData(x, y);

    m_plot->xAxis->setLabel("x");
    m_plot->yAxis->setLabel("y");
    m_plot->xAxis->setRange(X_MIN, X_MAX);
    m_plot->yAxis->setRange(Y_MIN, Y_MAX);
\
    connect(m_plot, &QCustomPlot::mousePress, this, &MainWindow::onMousePress);
    connect(m_plot, &QCustomPlot::mouseMove,  this, &MainWindow::onMouseMove);
    connect(m_plot, &QCustomPlot::mouseRelease, this, &MainWindow::onMouseRelease);
}

void MainWindow::initLinesAndText()
{
    m_startVertLine = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_startHorizLine = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_currentVertLine = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_currentHorizLine = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    for (QCPCurve *line : {m_currentHorizLine, m_currentVertLine,
                           m_startHorizLine, m_startVertLine}) {
        m_plot->addPlottable(line);
        line->setPen(QPen(Qt::gray));
        line->setVisible(false);
    }

    m_highlightVertAtStartX = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_highlightVertAtCurrentX = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_highlightHorizAtStartY = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_highlightHorizAtCurrentY = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    m_diagonalLine = new QCPCurve(m_plot->xAxis, m_plot->yAxis);
    for (QCPCurve *line : {m_highlightVertAtStartX, m_highlightVertAtCurrentX,
                           m_highlightHorizAtStartY, m_highlightHorizAtCurrentY,
                           m_diagonalLine}) {
        m_plot->addPlottable(line);
        line->setPen(QPen(Qt::red));
        line->setVisible(false);
    }

    m_startText = new QCPItemText(m_plot);
    m_deltaText = new QCPItemText(m_plot);
    m_startText->setPositionAlignment(Qt::AlignLeft);
    m_deltaText->setPositionAlignment(Qt::AlignLeft);
    m_startText->setText("");
    m_deltaText->setText("");

    m_plot->replot();
}

void MainWindow::onMousePress(QMouseEvent *event)
{
    double x = m_plot->xAxis->pixelToCoord(event->pos().x());
    double y = m_plot->yAxis->pixelToCoord(event->pos().y());
    m_startPoint = QPointF(x, y);

    hideDragVisuals();

    updateStartCrosshairs(m_startPoint);
    updateStartText(m_startPoint);
    m_startVertLine->setVisible(true);
    m_startHorizLine->setVisible(true);

    m_isDrawing = true;
    m_plot->replot();
}

void MainWindow::onMouseMove(QMouseEvent *event)
{
    if (!(QApplication::mouseButtons() & Qt::LeftButton))
        return; /* if it`s not left mouse button -> return */

    if (!m_isDrawing)
        return; /* if there is no measurement -> return */

    double x = m_plot->xAxis->pixelToCoord(event->pos().x());
    double y = m_plot->yAxis->pixelToCoord(event->pos().y());
    m_currentPoint = QPointF(x, y);

    updateCurrentCrosshairs(m_currentPoint);
    updateHighlightSegments(m_startPoint, m_currentPoint);
    updateDiagonal(m_startPoint, m_currentPoint);
    updateDeltaText(m_startPoint, m_currentPoint);

    m_currentVertLine->setVisible(true);
    m_currentHorizLine->setVisible(true);
    m_highlightVertAtStartX->setVisible(true);
    m_highlightVertAtCurrentX->setVisible(true);
    m_highlightHorizAtStartY->setVisible(true);
    m_highlightHorizAtCurrentY->setVisible(true);
    m_diagonalLine->setVisible(true);
    m_deltaText->setVisible(true);

    m_plot->replot();
}

void MainWindow::onMouseRelease(QMouseEvent */*event*/)
{
    m_isDrawing = false;
}

void MainWindow::updateStartCrosshairs(const QPointF &pos)
{
    setLineData(m_startVertLine, pos.x(), Y_MIN, pos.x(), Y_MAX);
    setLineData(m_startHorizLine, X_MIN, pos.y(), X_MAX, pos.y());
}

void MainWindow::updateCurrentCrosshairs(const QPointF &pos)
{
    setLineData(m_currentVertLine, pos.x(), Y_MIN, pos.x(), Y_MAX);
    setLineData(m_currentHorizLine, X_MIN, pos.y(), X_MAX, pos.y());
}

void MainWindow::updateHighlightSegments(const QPointF &start, const QPointF &current)
{
    /* vertical segment at start X (from start.y to current.y) */
    setLineData(m_highlightVertAtStartX,
                start.x(), start.y(),
                start.x(), current.y());

    /* vertical segment at current X (from start.y to current.y) */
    setLineData(m_highlightVertAtCurrentX,
                current.x(), start.y(),
                current.x(), current.y());

    /* horizontal segment at start Y (from start.x to current.x) */
    setLineData(m_highlightHorizAtStartY,
                start.x(), start.y(),
                current.x(), start.y());

    /* horizontal segment at current Y (from start.x to current.x) */
    setLineData(m_highlightHorizAtCurrentY,
                start.x(), current.y(),
                current.x(), current.y());
}

void MainWindow::updateDiagonal(const QPointF &start, const QPointF &current)
{
    setLineData(m_diagonalLine, start.x(), start.y(), current.x(), current.y());
}

void MainWindow::updateStartText(const QPointF &pos)
{
    m_startText->position->setCoords(pos.x() + PADDING, pos.y());
    QString text = QString("X: %1; Y: %2")
                       .arg(pos.x(), 0, 'f', 2)
                       .arg(pos.y(), 0, 'f', 2);
    m_startText->setText(text);
}

void MainWindow::updateDeltaText(const QPointF &start, const QPointF &current)
{
    double dx = current.x() - start.x();
    double dy = current.y() - start.y();
    m_deltaText->position->setCoords(current.x() + DELTA_PADDING, current.y());
    QString text = QString("ΔX: %1; ΔY: %2")
                       .arg(dx, 0, 'f', 2)
                       .arg(dy, 0, 'f', 2);
    m_deltaText->setText(text);
}

void MainWindow::hideDragVisuals()
{
    m_currentVertLine->setVisible(false);
    m_currentHorizLine->setVisible(false);
    m_highlightVertAtStartX->setVisible(false);
    m_highlightVertAtCurrentX->setVisible(false);
    m_highlightHorizAtStartY->setVisible(false);
    m_highlightHorizAtCurrentY->setVisible(false);
    m_diagonalLine->setVisible(false);
    m_deltaText->setVisible(false);
}

void MainWindow::setLineData(QCPCurve *curve, double x1, double y1, double x2, double y2)
{
    QVector<double> x = { x1, x2 };
    QVector<double> y = { y1, y2 };
    curve->setData(x, y);
}
