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
    qDebug() << "File exists:" << QFile::exists(path) << Qt::endl;

    sett.beginGroup(PARAMS);
    A = sett.value("A", A_DEFAULT).toInt();
    f = sett.value("f", F_DEFAULT).toInt();
    fd = sett.value("fd", FD_DEFAULT).toInt();
    n1 = sett.value("n1", N1_DEFAULT).toInt();
    n2 = sett.value("n2", N2_DEFAULT).toInt();
    df = sett.value("df", DF_DEFAULT).toInt();
    rate = sett.value("rate", RATE_DEFAULT).toInt();
    window = sett.value("window", WINDOW_DEFAULT).toInt();
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
    samplesPerSymbol = fd / rate;

    qDebug() << "Rate is" << rate;
    qDebug() << samplesPerSymbol << "samples per symbol" << Qt::endl;
}

void SignalParams::recalcSTFT()
{
    hop = window / HOP_COEF;
    windowsQuantity = 1 + ((N - window) / hop);
}

void SignalParams::sanitize(QWidget *window)
{
    recalcN();
    recalcSamples();
    recalcSTFT();

    if (N <= 0) {
        QMessageBox::critical(window, "Error", "Quantity of point is negative!");
        qDebug() << "Error during calculating N!" << Qt::endl;
    }

    if (samplesPerSymbol < 1) {
        samplesPerSymbol = 1;
    }
}

QVector<QVector<double>> SignalParams::generateFSKSignal() {
    QVector<short> inf_bit(N);
    QVector<QVector<double>> signal(2, QVector<double>(N, 0));

    inf_bit[0] = 0;
    for (int i = 1; i < N; ++i) {
        if (i % samplesPerSymbol == 0) {
            inf_bit[i] = inf_bit[i - 1] ^ 1;
        } else {
            inf_bit[i] = inf_bit[i - 1];
        }
    }

    for (int i = 0; i < N; ++i) {
        double freq = inf_bit[i] == 0 ?
                          f - df :
                          f + df;
        double arg = 2 * PI * freq * (i + n1) / fd;

        double re = A * cos(arg);
        double im = A * sin(arg);

        signal[0][i] = re;
        signal[1][i] = im;
    }

    return signal;
}
