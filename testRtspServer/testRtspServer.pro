QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtspServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    socketconnection.cpp \
    mp4file.cpp \
    base/blockqueue.cpp \
    base/mytimer.cpp \
    net/rtp.cpp \
    base/testq1andq2.cpp \
    task.cpp \
    net/socket.cpp \
    base/mutex.cpp \
    base/condition.cpp \
    base/thread.cpp \
    base/threadpool.cpp \
    net/tcpsession.cpp

HEADERS += \
    socketconnection.h \
    mp4file.h \
    base/blockqueue.h \
    base/mytimer.h \
    net/rtp.h \
    base/testq1andq2.h \
    task.h \
    net/socket.h \
    base/mutex.h \
    base/condition.h \
    base/thread.h \
    base/threadpool.h \
    net/tcpsession.h

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so
