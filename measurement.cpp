#include "mainwindow.h"

void MainWindow::initLinesAndText()
{
    /* gray lines for start and current position */
    m_startVertLine    = new QCPItemLine(m_plot);
    m_startHorizLine   = new QCPItemLine(m_plot);
    m_currentVertLine  = new QCPItemLine(m_plot);
    m_currentHorizLine = new QCPItemLine(m_plot);
    m_diagonalLine     = new QCPItemLine(m_plot);

    /* setting it up */
    for (QCPItemLine *line : { m_startVertLine, m_startHorizLine,
                              m_currentVertLine, m_currentHorizLine,
                              m_diagonalLine}) {
        m_plot->addItem(line);
        line->setPen(QPen(Qt::gray));
        /* invisible by default */
        line->setVisible(false);
    }

    /* and red color for diagonal line */
    m_diagonalLine->setPen(QPen(Qt::red));

    /* red rectangle */
    m_highlightRect = new QCPItemRect(m_plot);
    m_plot->addItem(m_highlightRect);
    m_highlightRect->setPen(QPen(Qt::red));
    m_highlightRect->setBrush(Qt::NoBrush);
    /* invisible by default */
    m_highlightRect->setVisible(false);

    /* text for positions */
    m_startText = new QCPItemText(m_plot);
    m_deltaText = new QCPItemText(m_plot);
    m_plot->addItem(m_startText);
    m_plot->addItem(m_deltaText);
    m_startText->setPositionAlignment(Qt::AlignLeft);
    m_deltaText->setPositionAlignment(Qt::AlignLeft);
    /* invisible by default */
    m_startText->setText("");
    m_deltaText->setText("");

    /* angle feaches */
    angleText = new QCPItemText(m_plot);
    m_plot->addItem(angleText);
    angleText->setText("");
    angleText->setColor(Qt::black);
    angleText->setFont(QFont(font().family(), 11));
    angleText->setVisible(false);

    phaseVector = new QCPItemLine(m_plot);
    phaseVector->start->setCoords(0, 0);
    phaseVector->setHead(QCPLineEnding::esSpikeArrow);
    phaseVector->setPen(QPen(Qt::red, 2));
    phaseVector->setVisible(false);
}

void MainWindow::onMousePress(QMouseEvent *event)
{
    /* getting pressed position and converting it to a point */
    double x = m_plot->xAxis->pixelToCoord(event->pos().x());
    double y = m_plot->yAxis->pixelToCoord(event->pos().y());
    m_startPoint = QPointF(x, y);

    /* hiding everything excluding start crosshair and text */
    hideDragVisuals();

    /* updating start crosshair and start text */
    updateCrosshair(m_startVertLine, m_startHorizLine, m_startPoint);
    updateStartText(m_startPoint);

    /* making start crosshair visible */
    m_startVertLine->setVisible(true);
    m_startHorizLine->setVisible(true);

    /* setting drawing flag, `cause we`re starting drawing! */
    m_isDrawing = true;

    m_plot->replot();
}

void MainWindow::onMouseMove(QMouseEvent *event)
{
    /* if it`s not a left button */
    if (!(QApplication::mouseButtons() & Qt::LeftButton))
        return;
    /* if it`s just a move without left button */
    if (!m_isDrawing)
        return;

    /* getting pressed position and converting it to a point */
    double x = m_plot->xAxis->pixelToCoord(event->pos().x());
    double y = m_plot->yAxis->pixelToCoord(event->pos().y());
    m_currentPoint = QPointF(x, y);

    /* updating everything except start crosshair */
    updateCrosshair(m_currentVertLine, m_currentHorizLine, m_currentPoint);
    updateHighlightSegments(m_startPoint, m_currentPoint);
    updateDiagonal(m_startPoint, m_currentPoint);
    updateDeltaText(m_startPoint, m_currentPoint);

    /* showing everything up */
    showDragVisuals();

    m_plot->replot();
}

void MainWindow::onMouseRelease(QMouseEvent *)
{
    /* when we stop pressing the left button,
     * unsetting the drawing flag */
    m_isDrawing = false;
}

void MainWindow::updateCrosshair(QCPItemLine *vertLine,
                                 QCPItemLine *horizLine,
                                 const QPointF &pos)
{
    /* setting up x and y coordinates for
     * vertical and horizontal lines respectively */
    vertLine->start->setCoords(pos.x(), minY);
    vertLine->end->setCoords(pos.x(), maxY);
    horizLine->start->setCoords(minX, pos.y());
    horizLine->end->setCoords(maxX, pos.y());
}

void MainWindow::updateHighlightSegments(const QPointF &start, const QPointF &current)
{
    /* rectangle defined by two opposite corners */
    m_highlightRect->topLeft->setCoords(qMin(start.x(), current.x()),
                                        qMax(start.y(), current.y()));
    m_highlightRect->bottomRight->setCoords(qMax(start.x(), current.x()),
                                            qMin(start.y(), current.y()));
}

void MainWindow::updateDiagonal(const QPointF &start, const QPointF &current)
{
    /* setting opposite coordinates for diagonal */
    m_diagonalLine->start->setCoords(start.x(), start.y());
    m_diagonalLine->end->setCoords(current.x(), current.y());
}

void MainWindow::updateStartText(const QPointF &pos)
{
    /* setting position for start text,
     * * creating and setting text for text field */
    m_startText->position->setCoords(pos.x() + (maxX * PADDING), pos.y());

    QString text = QString("X: %1; Y: %2")
                       .arg(pos.x(), 0, 'f', 2)
                       .arg(pos.y(), 0, 'f', 2);
    m_startText->setText(text);
}

void MainWindow::updateDeltaText(const QPointF &start, const QPointF &current)
{
    /* finding delta-values,
     * setting position for delta-text (with padding),
     * creating and setting text for text field */

    double dx = current.x() - start.x();
    double dy = current.y() - start.y();

    m_deltaText->position->setCoords(current.x() + (maxX * PADDING), current.y());

    QString text = QString("ΔX: %1; ΔY: %2")
                       .arg(dx, 0, 'f', 2)
                       .arg(dy, 0, 'f', 2);
    m_deltaText->setText(text);
}

void MainWindow::hideDragVisuals()
{
    /* just turning everything off (excluse start crosshair and text) */
    m_currentVertLine->setVisible(false);
    m_currentHorizLine->setVisible(false);
    m_highlightRect->setVisible(false);
    m_diagonalLine->setVisible(false);
    m_deltaText->setVisible(false);
}

void MainWindow::showDragVisuals()
{
    /* just turning everything on (excluse start crosshair and text) */
    m_currentVertLine->setVisible(true);
    m_currentHorizLine->setVisible(true);
    m_highlightRect->setVisible(true);
    m_diagonalLine->setVisible(true);
    m_deltaText->setVisible(true);
}
