QT += core
QT -= gui

CONFIG += c++11

TARGET = testEventScheduler
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ioevent.cpp \
    eventscheduler.cpp \
    poller.cpp \
    pollpoller.cpp

HEADERS += \
    ioevent.h \
    eventscheduler.h \
    poller.h \
    pollpoller.h
