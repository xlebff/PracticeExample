#include "signalparams.h"
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <cmath>

void SignalParams::loadFromSettings(const QString &path)
{
    QSettings sett(path, QSettings::IniFormat);

    qDebug() << "Looking for file:" << sett.fileName();
    qDebug() << "File exists:" << QFile::exists(path) << endl;

    sett.beginGroup(PARAMS);
    A = sett.value("A", A_DEFAULT).toInt();
    f = sett.value("f", F_DEFAULT).toInt();
    fd = sett.value("fd", FD_DEFAULT).toInt();
    n1 = sett.value("n1", N1_DEFAULT).toInt();
    n2 = sett.value("n2", N2_DEFAULT).toInt();
    df = sett.value("df", DF_DEFAULT).toInt();
    rate = sett.value("rate", RATE_DEFAULT).toInt();
    bitGenerateMode = sett.value("mode", GENERATING_MODE_DEFAULT).toInt();
    sett.endGroup();
}

void SignalParams::recalcN()
{
    N = n2 - n1 + 1;
    qDebug() << "N is" << N;
}

void SignalParams::recalcSamples()
{
    samplesPerSymbol = (1 / (double)rate) / (1 / (double)fd);
    qDebug() << samplesPerSymbol << "samples per symbol" << endl;
}

void SignalParams::sanitize(QWidget *window)
{
    recalcN();
    recalcSamples();

    if (N <= 0) {
        QMessageBox::critical(window, "Error", "Quantity of point is negative!");
        qDebug() << "Error during calculating N!" << endl;
    }
}

QVector<QVector<double>> SignalParams::generateClearSignal() {
    QVector<QVector<double>> signal(2, QVector<double>(N, 0));

    double arg = 2 * PI * f / fd;
    for (int i = 0; i < N; ++i) {
        double re = A * cos(arg * (i + n1));
        double im = A * sin(arg * (i + n1));

        signal[0][i] = re;
        signal[1][i] = im;
    }

    return signal;
}

QVector<QVector<double>> SignalParams::generateFSKSignal(QVector<short> &infBit) {
    QVector<short> inf_bit(N);
    QVector<QVector<double>> signal(2, QVector<double>(N, 0));

    const bool isRandom = bitGenerateMode;

    inf_bit[0] = 0;
    for (int i = 1; i < N; ++i) {
        if (std::fmod(i, samplesPerSymbol) == 0) {
            if (!isRandom) inf_bit[i] = inf_bit[i - 1] ^ 1;
            else inf_bit[i] = rand() % 2;
        } else {
            inf_bit[i] = inf_bit[i - 1];
        }
    }

    double arg = 2 * PI / fd;
    for (int i = 0; i < N; ++i) {
        double freq = inf_bit[i] == 0 ?
                          f - df :
                          f + df;

        double re = A * cos(arg * freq * (i + n1));
        double im = A * sin(arg * freq * (i + n1));

        signal[0][i] = re;
        signal[1][i] = im;
    }

    infBit = inf_bit;
    return signal;
}

QVector<QVector<double>> SignalParams::generatePhaseSignal(QVector<short> &infBit) {
    QVector<short> inf_bit(N);
    QVector<QVector<double>> signal(2, QVector<double>(N, 0));

    const bool isRandom = bitGenerateMode;

    inf_bit[0] = 0;
    for (int i = 1; i < N; ++i) {
        if (std::fmod(i, samplesPerSymbol) == 0) {
            if (!isRandom) inf_bit[i] = (inf_bit[i - 1] + 1) % 8;
            else inf_bit[i] = rand() % 8;
        } else {
            inf_bit[i] = inf_bit[i - 1];
        }
    }

    for (int i = 0; i < N; ++i) {
        double re = A * cos(PHASES[inf_bit[i] % PHASES_COUNT]);
        double im = A * sin(PHASES[inf_bit[i] % PHASES_COUNT]);

        signal[0][i] = re;
        signal[1][i] = im;
    }

    infBit = inf_bit;
    return signal;
}

QVector<QVector<double>> SignalParams::generateASignal(QVector<short> &infBit) {
    QVector<short> inf_bit(N);
    QVector<QVector<double>> signal(2, QVector<double>(N, 0));

    const bool isRandom = bitGenerateMode;

    inf_bit[0] = 0;
    for (int i = 1; i < N; ++i) {
        if (std::fmod(i, samplesPerSymbol) == 0) {
            if (!isRandom) inf_bit[i] = inf_bit[i - 1] ^ 1;
            else inf_bit[i] = rand() % 2;
        } else {
            inf_bit[i] = inf_bit[i - 1];
        }
    }

    double arg = 2 * PI * f / fd;
    for (int i = 0; i < N; ++i) {
        double re = inf_bit[i] * A * cos(arg * (i + n1));
        double im = inf_bit[i] * A * sin(arg * (i + n1));

        signal[0][i] = re;
        signal[1][i] = im;
    }

    infBit = inf_bit;
    return signal;
}
