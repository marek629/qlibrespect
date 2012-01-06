#-------------------------------------------------
#
# Project created by QtCreator 2011-07-23T17:29:15
#
#-------------------------------------------------

QT       += core gui
CONFIG += debug

TARGET = qLibReSpect
TEMPLATE = app
LIBS += -LC:\Users\Art\Documents\PROJEKTY\C++\marek629-librespect\release \
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

INCLUDEPATH += C:\Users\Art\Documents\PROJEKTY\C++\marek629-librespect
DEPENDPATH += C:\Users\Art\Documents\PROJEKTY\C++\marek629-librespect\release

OTHER_FILES += \
    TODO

