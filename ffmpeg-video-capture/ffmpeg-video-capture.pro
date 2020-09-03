QT += core
QT -= gui

CONFIG += c++11

TARGET = ffmpeg-video-capture
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += /usr/local/ffmpeg/include

SOURCES += main.cpp
