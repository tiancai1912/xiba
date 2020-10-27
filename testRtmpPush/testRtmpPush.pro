QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtmpPush
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    rtmppush.cpp

HEADERS += \
    rtmppush.h

INCLUDEPATH += /usr/local/ffmpeg/include
INCLUDEPATH += /usr/include/librtmp

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so

LIBS += /usr/lib/x86_64-linux-gnu/librtmp.so
