#-------------------------------------------------
#
# Project created by QtCreator 2020-08-18T20:04:19
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CapTool
TEMPLATE = app

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libswscale.so \
        /usr/local/ffmpeg/lib/libavutil.so


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
