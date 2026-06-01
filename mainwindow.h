#pragma once

#include "signalparams.h"

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
    /* slots for dynamic measurement */
    void onMousePress(QMouseEvent *event);
    void onMouseMove(QMouseEvent *event);
    void onMouseRelease(QMouseEvent *event);

    /* slot for switching current chart */
    void onComboSwitched(int index);

private:
    /* VITAL */

    Ui::MainWindow *ui;

    SignalParams params;
    QVector<QVector<double>> signal;

    static inline const double PI = acos(-1.0);



    /* MAIN PLOT INITIALIZE */


    /* VARIABLES */

    /* plot! for all charts */
    QCustomPlot *m_plot = nullptr;

    /* combo box for switching between charts */
    QComboBox *m_PlotComboBox;
    /* vector for do it without huge switch-case */
    std::vector<std::function<void()>> drawChart;

    /* all charts i have now */
    static constexpr int CHARTS_QUANTITY = 7;
    const char* CHARTS_NAMES[CHARTS_QUANTITY] = { "Real and Imaginary Parts",
                                                  "FFT Spectrum",
                                                  "Spectrogram",
                                                  "Phase Portrait",
                                                  "Envelope and Instantaneous Phase",
                                                  "Autocorrelation Function",
                                                  "Amplitude Distribution Histogram" };


    /* METHODS */

    /* method for create that plot
     * must be called on start */
    void initPlot();

    /* as like previous,
     * this method must be called on start to init combo box */
    void initCombo();



    /* DRAWING CHARTS */


    /* VARIABLES */

    /* Y-margin to see the highest points of the chart */
    static constexpr double MARGIN = 1.01;

    /* gradient for spectrum */
    QCPColorScale *colorScale = nullptr;
    QCPColorMap *colorMap = nullptr;
    QCPColorGradient gradient;

    /* additional right axis */
    QCPAxis *rightAxis = nullptr;


    /* METHODS */

    /* methods for drawing different charts */
    void drawRI();
    void drawFFTSpectrum();
    void drawSTFT();
    void drawIQPlane();
    void drawPhase();
    void drawACF();
    void drawPDF();

    /* helpers */
    void clearPlot();
    void minMax(double &min, double &max, const QVector<double> &array, const int &size);



    /* MEASUREMENT FEACHES */


    /* VARIABLES */

    /* values for draw big vertical and horizontal lines,
     * also using for setting up the ranges of the axises */
    double minY = 0, maxY = 0, minX = 0, maxX = 0;

    /* uniq value for every scale, coordinates text coef
     * sould be multiple on maxX */
    static constexpr double PADDING = 0.005;

    /* flag indicades visability of drag-lines */
    bool m_isDrawing = false;

    /* points of user`s mouse */
    QPointF m_startPoint = QPointF(0, 0);
    QPointF m_currentPoint = QPointF(0, 0);

    /* gray crosshair lines */
    QCPItemLine *m_startVertLine = nullptr;
    QCPItemLine *m_startHorizLine = nullptr;
    QCPItemLine *m_currentVertLine = nullptr;
    QCPItemLine *m_currentHorizLine = nullptr;

    /* diagonal line */
    QCPItemLine *m_diagonalLine;

    /* highlight rectangle */
    QCPItemRect *m_highlightRect;

    /* text annotations */
    QCPItemText *m_startText;
    QCPItemText *m_deltaText;


    /* METHODS */

    /* initializer that created lines and text fields */
    void initLinesAndText();

    /* updaters for crosshairs, highlighted rect, text and etc. */
    void updateCrosshair(QCPItemLine *vertLine,
                         QCPItemLine *horizLine,
                         const QPointF &pos);

    void updateHighlightSegments(const QPointF &start,
                                 const QPointF &current);

    void updateDiagonal(const QPointF &start,
                        const QPointF &current);

    void updateStartText(const QPointF &pos);

    void updateDeltaText(const QPointF &start,
                         const QPointF &current);

    /* just simple helpers */
    void hideDragVisuals();
    void showDragVisuals();
};
