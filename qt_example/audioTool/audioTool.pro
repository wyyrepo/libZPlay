#-------------------------------------------------
#
# Project created by QtCreator 2017-01-06T15:59:57
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AudioTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    curveplot.cpp

HEADERS  += mainwindow.h \
    curveplot.h \
    libzplay.h

FORMS    += mainwindow.ui

LIBS     += $$PWD/lib/libzplay.lib

RESOURCES += \
    res.qrc

RC_FILE=icon.rc

