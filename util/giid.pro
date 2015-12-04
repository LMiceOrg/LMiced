TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += giid.c

INCLUDEPATH += ../eal

win32-msvc* {
message("LMiced - Windows MSVC")
#QMAKE_CFLAGS += -std=c89
DEFINES += inline=__inline
QMAKE_CFLAGS_WARN_ON    = /W4
QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG -D_CRT_SECURE_NO_WARNINGS

QMAKE_CFLAGS_DEBUG -= -MDd
QMAKE_CFLAGS_RELEASE -= -MD
QMAKE_CFLAGS_DEBUG += /MTd /O2 -DSTDC89
QMAKE_CFLAGS_RELEASE += /MT /O2 -DSTDC89

INCLUDEPATH += ../../lib/jansson-2.7/build/include ../lib/sglib-1.0.4

CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../../../../build-src/build/debug
}

CONFIG(release, debug|release) {
LIBS += -L../build-src/build/release
}

LIBS += -lWinmm -lsrc -L../../../../lib/jansson-2.7/build/lib/Release -ljansson
LIBS += -lws2_32 -lKernel32 -lIphlpapi
}

