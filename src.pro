TEMPLATE = lib
CONFIG += staticlib
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    lmice_core.c    \
    lmice_ring.c \
    lmice_trace.c \
    lmice_eal_endian.c \
    lmice_eal_shm.c \
    lmice_eal_hash.c \
    lmice_eal_spinlock.c \
    lmice_eal_malloc.c

OTHER_FILES += \
    ../doc/About_cn.txt \
    ../doc/Console_Color.txt \
    ../test/eal_endian_test.py \
    ../test/eal_shm_test.py \
    ../test/eal_hash_test.py \
    ../test/global_defines.py

HEADERS += \
    lmice_core.h \
    lmice_ring.h \
    lmice_trace.h \
    lmice_json_util.h \
    lmice_eal_endian.h \
    lmice_eal_shm.h \
    lmice_eal_hash.h \
    lmice_eal_align.h \
    lmice_eal_atomic.h \
    lmice_eal_spinlock.h \
    lmice_eal_malloc.h

macx-clang*{
message("MacX Darwin")

INCLUDEPATH += ../lib/jansson/bin/include
LIBS += -L../lib/jansson/bin/lib
LIBS += -ljansson

QMAKE_CFLAGS += -std=c89 -funit-at-a-time -Wno-unused-function
DEFINES += inline=__inline

QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG
}
