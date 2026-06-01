#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    w.resize(screenRect.width(), screenRect.height());
    w.show();

    return a.exec();
}
