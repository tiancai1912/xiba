QT += core
QT -= gui

CONFIG += c++11

TARGET = testRtspServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
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
    3rd/eventscheduler.cpp \
    3rd/ioevent.cpp \
    3rd/poller.cpp \
    3rd/pollpoller.cpp \
    3rd/timer2.cpp \
    server.cpp \
    net/tcpserver.cpp \
    net/socketconnection.cpp

HEADERS += \
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
    3rd/eventscheduler.h \
    3rd/ioevent.h \
    3rd/poller.h \
    3rd/pollpoller.h \
    3rd/timer2.h \
    server.h \
    net/tcpserver.h \
    net/socketconnection.h

INCLUDEPATH += /usr/local/ffmpeg/include

LIBS += /usr/local/ffmpeg/lib/libavformat.so \
        /usr/local/ffmpeg/lib/libavcodec.so \
        /usr/local/ffmpeg/lib/libavutil.so
