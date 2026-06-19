#pragma once

#include <QString>
#include <QWidget>
#include <cmath>
#include <functional>

struct SignalParams {
private:
    static constexpr int A_DEFAULT = 100;
    static constexpr int F_DEFAULT = 10;
    static constexpr int FD_DEFAULT = 1000;
    static constexpr int DF_DEFAULT = 100;
    static constexpr int RATE_DEFAULT = 500;

    static constexpr int N1_DEFAULT = 0;
    static constexpr int N2_DEFAULT = 400;

    static constexpr int GENERATING_MODE_DEFAULT = 0;

    static constexpr double PSI_DEFAULT_DEG = 0.0;

    static inline const QString PATH = "params.ini";
    static inline const QString PARAMS = "Params";
    static inline const double PI = acos(-1.0);

    QVector<QVector<double>> generateSignal(std::function<double()> gen);

public:
    static constexpr int GENERATING_MODE_TOGGLE = 0;
    static constexpr int GENERATING_MODE_RANDOM = 1;

    static constexpr int PHASES_COUNT = 8;
    const double PHASES[PHASES_COUNT] = {(0), (PI / 4), (PI / 2), (3 * PI / 4),
                                         (PI), (5 * PI / 4), (3 * PI / 2), (7 * PI / 4)};

    int A = A_DEFAULT;
    int f = F_DEFAULT;
    int fd = FD_DEFAULT;
    int n1 = N1_DEFAULT;
    int n2 = N2_DEFAULT;
    int df = DF_DEFAULT;
    int rate = RATE_DEFAULT;
    int bitGenerateMode = GENERATING_MODE_DEFAULT;
    double psi = PSI_DEFAULT_DEG;
    int N = 0;
    double samplesPerSymbol = 0.0;

    void loadFromSettings(const QString &path = PATH);
    void recalcN();
    void recalcSamples();
    void sanitize(QWidget *window);

    QVector<QVector<double>> generateClearSignal();
    QVector<QVector<double>> generateFSKSignal(QVector<short> &infBit);
    QVector<QVector<double>> generatePhaseSignal(QVector<short> &infBit);
    QVector<QVector<double>> generateASignal(QVector<short> &infBit);
};
