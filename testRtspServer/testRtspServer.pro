QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtspServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    mytimer.cpp \
    rtp.cpp \
    aacsend.cpp \
    socketconnection.cpp \
    mp4file.cpp

HEADERS += \
    mytimer.h \
    rtp.h \
    aacsend.h \
    socketconnection.h \
    mp4file.h

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so
