#-------------------------------------------------
#
# Project created by QtCreator 2015-11-30T11:30:03
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = QCustomPlotExample
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    measurement.cpp \
    qcustomplot.cpp \
    signalparams.cpp \
    uisetter.cpp

HEADERS += mainwindow.h \
    qcustomplot.h \
    signalparams.h

FORMS  += mainwindow.ui

LIBS   += -lfftw3

OTHER_FILES += \
    params.ini
