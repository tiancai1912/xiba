QT += core
QT -= gui

CONFIG += c++11

TARGET = testThreadPool
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    condition.cpp \
    threadpool.cpp \
    condition1.cpp \
    threadpool1.cpp \
    testcondition.cpp \
    testthreadpool.cpp \
    mythread.cpp

HEADERS += \
    condition.h \
    threadpool.h \
    condition1.h \
    threadpool1.h \
    testcondition.h \
    testthreadpool.h \
    mythread.h
