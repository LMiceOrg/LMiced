TEMPLATE = lib
CONFIG += staticlib
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    lmice_core.c    \
    eal/lmice_ring.c \
    eal/lmice_trace.c \
    eal/lmice_eal_endian.c \
    eal/lmice_eal_shm.c \
    eal/lmice_eal_hash.c \
    eal/lmice_eal_spinlock.c \
    eal/lmice_eal_malloc.c

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
    eal/lmice_eal_common.h

#Common config
INCLUDEPATH += eal

macx-clang*{
message("MacX Darwin")

INCLUDEPATH += ../lib/jansson/bin/include
LIBS += -L../lib/jansson/bin/lib
LIBS += -ljansson

QMAKE_CFLAGS += -std=c89 -funit-at-a-time -Wno-unused-function
DEFINES += inline=__inline__
QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG
}

win32-msvc* {

message("Windows MSVC")
QMAKE_CFLAGS += -std=c89
DEFINES += inline=__inline
QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG
}
