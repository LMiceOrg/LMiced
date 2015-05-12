TEMPLATE = lib
CONFIG += staticlib
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += eal/lmice_ring.c \
    eal/lmice_trace.c \
    eal/lmice_eal_shm.c \
    eal/lmice_eal_hash.c \
    eal/lmice_eal_spinlock.c \
    eal/lmice_eal_malloc.c \
    eal/lmice_eal_event.c \
    eal/lmice_eal_time.c


OTHER_FILES += \
    ../doc/About_cn.txt \
    ../doc/Console_Color.txt \
    test/eal_endian_test.py \
    test/eal_shm_test.py \
    test/eal_hash_test.py \
    test/global_defines.py

HEADERS += \
    lmice_core.h \
    eal/lmice_ring.h \
    eal/lmice_trace.h \
    lmice_json_util.h \
    eal/lmice_eal_endian.h \
    eal/lmice_eal_shm.h \
    eal/lmice_eal_hash.h \
    eal/lmice_eal_align.h \
    eal/lmice_eal_atomic.h \
    eal/lmice_eal_spinlock.h \
    eal/lmice_eal_malloc.h \
    eal/lmice_eal_thread.h \
    eal/lmice_eal_thread_win.h \
    eal/lmice_eal_thread_pthread.h \
    eal/lmice_eal_common.h \
    eal/lmice_eal_time.h \
    eal/lmice_eal_time_win.h \
    eal/lmice_eal_shm_win.h \
    eal/lmice_eal_event.h \
    eal/lmice_eal_socket.h

#Common config
INCLUDEPATH += eal

CONFIG(debug, debug|release) {
    DESTDIR = "$$OUT_PWD/../build/debug"
} else {
    DESTDIR = "$$OUT_PWD/../build/release"
}

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

win32-msvc* {

message("Windows MSVC")
QMAKE_CFLAGS_WARN_ON    = -W4
#QMAKE_CFLAGS += -std=c89
DEFINES += inline=__inline
QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG

QMAKE_CFLAGS_DEBUG = -MTd -O2
QMAKE_CFLAGS_RELEASE = -MT -O2

QMAKE_CXXFLAGS_DEBUG = -MTd -O2
QMAKE_CXXFLAGS_RELEASE = -MT -O2

SOURCES += eal/lmice_eal_wsa.c
HEADERS += eal/lmice_eal_iocp.h \
eal/lmice_eal_wsa.h

}

mingw* {

QMAKE_CFLAGS += -Winline

}
