QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtmp
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += /usr/include/librtmp
LIBS += /usr/lib/x86_64-linux-gnu/librtmp.so

SOURCES += main.cpp

