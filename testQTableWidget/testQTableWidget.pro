#-------------------------------------------------
#
# Project created by QtCreator 2020-10-18T19:57:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testQTableWidget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videoparser.cpp

HEADERS  += mainwindow.h \
    videoparser.h

FORMS    += mainwindow.ui


INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so
