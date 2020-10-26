#-------------------------------------------------
#
# Project created by QtCreator 2020-10-20T01:45:26
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpsPPsParser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videoparser.cpp

HEADERS  += mainwindow.h \
    bs.h \
    videoparser.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so
