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
    static constexpr int RATE_DEFAULT = 200;

    static constexpr int N1_DEFAULT = 0;
    static constexpr int N2_DEFAULT = 400;

    static constexpr int WINDOW_DEFAULT = 256;
    static constexpr int HOP_DEFAULT = 0;
    static constexpr int HOP_COEF = 2;
    static constexpr int WINDOWS_QUANTITY_DEFAULT = 0;

    // static constexpr int GENERATING_MODE_DEFAULT = 0;

    static inline const QString PATH = "params.ini";
    static inline const QString PARAMS = "Params";
    static inline const double PI = acos(-1.0);

    QVector<QVector<double>> generateSignal(std::function<double()> gen);

public:
    static constexpr int GENERATING_MODE_RANDOM = 0;
    static constexpr int GENERATING_MODE_TOGGLE = 1;

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
    int window = WINDOW_DEFAULT;
    int hop = HOP_DEFAULT;
    int windowsQuantity = WINDOWS_QUANTITY_DEFAULT;

    // int bitGenerateMode = GENERATING_MODE_DEFAULT;
    int N = 0;
    int samplesPerSymbol = 0;

    void loadFromSettings(const QString &path = PATH);
    void recalcN();
    void recalcSamples();
    void recalcSTFT();
    void sanitize(QWidget *window);

    QVector<QVector<double>> generateClearSignal();
    QVector<QVector<double>> generateFSKSignal(const bool isRandom, QVector<short> &infBit);
    QVector<QVector<double>> generatePhaseSignal(const bool isRandom, QVector<short> &infBit);
    QVector<QVector<double>> generateASignal(const bool isRandom, QVector<short> &infBit);
};
