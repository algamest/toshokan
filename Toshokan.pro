#-------------------------------------------------
#
# Project created by QtCreator 2014-01-13T13:37:46
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Toshokan
TEMPLATE = app


SOURCES += main.cpp\
    saddwidget.cpp \
    maddwidget.cpp \
    toshview.cpp \
    toshadd.cpp \
    bookshow.cpp

HEADERS  += \
    saddwidget.h \
    maddwidget.h \
    toshview.h \
    toshadd.h \
    bookshow.h

FORMS    += \
    saddwidget.ui \
    maddwidget.ui \
    toshview.ui \
    bookshow.ui

RESOURCES += \
    toshores.qrc
