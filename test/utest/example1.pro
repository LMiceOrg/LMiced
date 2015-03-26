TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += example1.cpp

INCLUDEPATH += ../..  ../../../rtspace

LIBS += -L../../../build/debug

LIBS += -lsrc
