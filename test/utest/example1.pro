TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += example1.cpp

INCLUDEPATH += ../..   ../../../lib/sglib-1.0.4

LIBS += -L../../../build/debug

LIBS += -lsrc
