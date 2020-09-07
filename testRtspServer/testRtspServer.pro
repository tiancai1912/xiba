QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtspServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    rtp.cpp

HEADERS += \
    rtp.h
