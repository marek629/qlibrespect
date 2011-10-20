#-------------------------------------------------
#
# Project created by QtCreator 2011-07-23T17:29:15
#
#-------------------------------------------------

QT       += core gui
CONFIG += debug

TARGET = qLibReSpect
TEMPLATE = app
LIBS += -lsndfile\
        -L../respect-build-desktop\
       # -R../respect-build-desktop\
        -lrespect

SOURCES += main.cpp\
    formrespect.cpp \
    graphicsview.cpp \
    markerpoint.cpp \
    mainwindow.cpp

HEADERS  += formrespect.h \
    graphicsview.h \
    markerpoint.h \
    mainwindow.h

FORMS += \
    formrespect.ui \
    mainwindow.ui

INCLUDEPATH += $$PWD/../respect
DEPENDPATH += $$PWD/../respect-build-desktop























