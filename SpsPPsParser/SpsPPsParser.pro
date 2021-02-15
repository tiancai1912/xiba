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

INCLUDEPATH += /usr/local/ffmpeg/include \
            /usr/local/include

LIBS += -L /usr/local/lib/ -lavformat \
        -L /usr/local/lib/ -lavcodec \
        -L /usr/local/lib/ -lavutil
