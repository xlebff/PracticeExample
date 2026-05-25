#pragma once

#include <QMainWindow>
#include <qcustomplot.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMousePress(QMouseEvent *event);
    void onMouseMove(QMouseEvent *event);
    void onMouseRelease(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    QCustomPlot *m_plot;

    static constexpr double X_MIN = -60.0;
    static constexpr double X_MAX =  60.0;
    static constexpr double Y_MIN = -60.0;
    static constexpr double Y_MAX =  60.0;
    static constexpr double PADDING = 2.0;
    static constexpr double DELTA_PADDING = -12.0;

    bool m_isDrawing;
    QPointF m_startPoint;
    QPointF m_currentPoint;

    /* start crosshairs (gray, full range) */
    QCPCurve *m_startVertLine;
    QCPCurve *m_startHorizLine;

    /* current crosshairs (gray, full range) */
    QCPCurve *m_currentVertLine;
    QCPCurve *m_currentHorizLine;

    /* highlight segments (red, between start and current) */
    QCPCurve *m_highlightVertAtStartX;
    QCPCurve *m_highlightVertAtCurrentX;
    QCPCurve *m_highlightHorizAtStartY;
    QCPCurve *m_highlightHorizAtCurrentY;

    /* diagonal line (red, from start to current) */
    QCPCurve *m_diagonalLine;

    QCPItemText *m_startText;   /* shows coordinates of start point */
    QCPItemText *m_deltaText;   /* shows ΔX and ΔY near current point */

    void initPlot();
    void initLinesAndText();

    void updateStartCrosshairs(const QPointF &pos);
    void updateCurrentCrosshairs(const QPointF &pos);
    void updateHighlightSegments(const QPointF &start, const QPointF &current);
    void updateDiagonal(const QPointF &start, const QPointF &current);
    void updateStartText(const QPointF &pos);
    void updateDeltaText(const QPointF &start, const QPointF &current);

    void hideDragVisuals();

    void setLineData(QCPCurve *curve, double x1, double y1, double x2, double y2);
};
