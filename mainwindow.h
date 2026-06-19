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
    void onChartComboSwitched(int index);
    void onSignalComboSwitched(int index);

    /* slot for file writing */
    void writeFile();

    /* slot for reloading a graph */
    void onReloadButtonClicked();

private:
    /* VITAL */

    Ui::MainWindow *ui;

    SignalParams params;
    QVector<QVector<double>> signal;
    QVector<short> infBit;

    static inline const double PI = acos(-1.0);



    /* MAIN PLOT INITIALIZE */


    /* VARIABLES */

    /* plot! for all charts */
    QCustomPlot *m_plot = nullptr;

    /* combo box for switching between charts */
    QComboBox *m_ChartComboBox;
    /* vector for do it without huge switch-case */
    std::vector<std::function<void()>> drawChart;
    /* all charts i have now */
    static constexpr int CHARTS_QUANTITY = 4;
    const char* CHARTS_NAMES[CHARTS_QUANTITY] = { "Real and Imaginary Parts",
                                                  "FFT Spectrum",
                                                  "Phase Portrait",
                                                  "Information bit" };
    int currentChart = 0;

    /* combo box for different signal modulations:
     * freq (toggle/random),
     * phase (toggle/random),
     * amplitude (toggle/random) */
    QComboBox *m_SignalComboBox;
    static constexpr int SIGNALS_QUANTITY = 3;
    const char* SIGNALS_NAMES[SIGNALS_QUANTITY] = { "Amplitude modulation",
                                                    "Frequency modulation",
                                                    "Phase modulation" };
    /* indexes contst */
    static constexpr int A_MODULATION = 0;
    static constexpr int FSK_MODULATION = 1;
    static constexpr int PHASE_MODULATION = 2;
    int currentSignal = 0;

    int mode = 0;


    /* METHODS */

    /* method for create that plot
     * must be called on start */
    void initPlot();

    /* as like previous,
     * this method must be called on start to init combo box */
    void initComboBoxes();

    void initButton();



    /* DRAWING CHARTS */


    /* VARIABLES */

    /* Y-margin to see the highest points of the chart */
    static constexpr double MARGIN = 1.01;

    QCPItemLine *phaseVector = nullptr;
    QCPItemText *angleText;

    /* METHODS */

    /* methods for drawing different charts */
    void drawRI();
    void drawFFTSpectrum();
    void drawIQPlane();
    void drawMeandr();

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



    /* FILES */


    /* VARIABLES */

    const char *CARRIER_MODE_RESULT_FILE_PATH = "resultcarrier.dat";
    const char *AMPLITUDE_MODE_RESULT_FILE_PATH = "resultamplitude.dat";
    const char *FREQUENCY_MODE_RESULT_FILE_PATH = "resultfsk.dat";
    const char *PHASE_MODE_RESULT_FILE_PATH = "result8psk.dat";

    QPushButton *m_WriteFileButton;

    QPushButton *m_ReloadButton;   /* new - reload button */
};
