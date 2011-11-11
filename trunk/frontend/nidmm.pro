#-------------------------------------------------
#
# Project created by QtCreator 2011-10-22T11:31:13
#
#-------------------------------------------------

QT       += core gui



TARGET = nidmm
TEMPLATE = app

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
DESTDIR=bin


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    ../../../Dokumentumok/NI/DOC/NI-4050A/module/ni4050.h

FORMS    += mainwindow.ui

LIBS += -lqwt

