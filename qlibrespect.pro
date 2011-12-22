#-------------------------------------------------
#
# Project created by QtCreator 2011-07-23T17:29:15
#
#-------------------------------------------------

QT       += core gui
CONFIG += debug

TARGET = qLibReSpect
TEMPLATE = app
LIBS += -L../respect-build-desktop-Qt_in_PATH_Release\
       # -R../respect-build-desktop\
        -lrespect

SOURCES += main.cpp\
    formrespect.cpp \
    graphicsview.cpp \
    markerpoint.cpp \
    mainwindow.cpp \
    wavefile.cpp

HEADERS  += formrespect.h \
    graphicsview.h \
    markerpoint.h \
    mainwindow.h \
    wavefile.h

FORMS += \
    formrespect.ui \
    mainwindow.ui

INCLUDEPATH += $$PWD/../respect
DEPENDPATH += $$PWD/../respect-build-desktop-Qt_in_PATH_Release

