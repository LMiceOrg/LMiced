TEMPLATE = lib
CONFIG += staticlib
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += eal/lmice_trace.c \
    eal/lmice_eal_shm.c \
    eal/lmice_eal_hash.c    \
    eal/lmice_eal_spinlock.c    \
    eal/lmice_eal_malloc.c  \
    eal/lmice_eal_event.c   \
    eal/lmice_eal_time.c    \
    eal/lmice_eal_aio.c \
    eal/lmice_eal_thread.c

INCLUDEPATH += eal

macx-clang*{
message("MacX Darwin")

INCLUDEPATH += ../lib/jansson/bin/include
#LIBS += -L../lib/jansson/bin/lib
#LIBS += -ljansson

QMAKE_CFLAGS += -std=c89 -Wno-long-long -Weverything -Wall
QMAKE_CFLAGS += -funit-at-a-time -Wno-unused-function
QMAKE_CFLAGS += -Dinline=__inline__

QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
}

SUBDIRS += \
    portal.pro


