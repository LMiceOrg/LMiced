TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += example1.cpp

INCLUDEPATH += ../..


LIBS += -L../../../build-src-Desktop_Qt_5_4_0_MSVC2013_OpenGL_64bit/debug

LIBS += -lsrc
