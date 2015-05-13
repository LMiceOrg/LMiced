TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
timer/timer_system_time.c \
resource/resouce_manage.c \
    trust/trust_manage.c \
    schedule/action_schedule.c \
    net/net_beatheart.c
#    net/net_manage.c
#    main.cpp \
#    resource/resource_shm.cpp \
#    resource/resource_meta_data.c \
#    system/config.cpp \
#    system/system_environment.cpp \
#    system/system_signal.cpp \
#    system/system_worker.cpp \
#    system/system_master.cpp \
#    resource/resouce_manage.cpp \
#    timer/timer_system_time.cpp \
#    timer/timer_oneshot.cpp \
#    resource/resource.cpp

OTHER_FILES += \
    README.txt

HEADERS += \
    rtspace.h \
timer/timer_system_time.h   \
resource/resource_manage.h \
trust/trust_manage.h \
    schedule/action_schedule.h \
    net/net_manage.h \
    net/net_manage_win.h \
    net/net_beatheart.h
#    system/system_environment.h \
#    system/system_signal.h \
#    system/system_environment_internal.h \
#    system/system_worker.h      system/config.h \
#    resource/resourec_shm.h \
#    resource/resource_shm_internal.h \
#    resource/resource_meta_data.h \
#    resource/resource_meta_data_internal.h \
#state/state_action.h		state/state_serialize_engine.h \
#state/state_event.h		state/state_serialize_json.h \
#state/state_guard.h		state/state_transition.h \
#state/state_machine.h		state/state_type.h \
#    system/system_master.h \
#    eal/lmice_eal_thread_pthread.h \
#    eal/lmice_eal_common.h \
#    resource/resource_manage.h \
#

INCLUDEPATH += eal .

mingw* {
message("LMiced - Windows MinGW32")

#QMAKE_CFLAGS += -std=c89
QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG -D_CRT_SECURE_NO_WARNINGS


CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../build/debug
}

CONFIG(release, debug|release) {
LIBS += -L../build/release
}

LIBS += -lWinmm -lsrc
LIBS += -lws2_32 -lKernel32

}

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

INCLUDEPATH += ../lib/jansson-2.7/build/include ../lib/sglib-1.0.4

CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../build/debug
}

CONFIG(release, debug|release) {
LIBS += -L../build/release
}

LIBS += -lWinmm -lsrc -L../lib/jansson-2.7/build/lib/Release -ljansson
LIBS += -lws2_32 -lKernel32
}

macx-clang*{
message("LMiced - MacX Darwin")

QMAKE_CFLAGS += -std=c89 -Wall -Weverything -Wno-long-long
QMAKE_CFLAGS += -funit-at-a-time -Wno-unused-function
QMAKE_CFLAGS += -Dinline=__inline

QMAKE_CXXFLAGS += -std=c++0x -Wshadow


QMAKE_LFLAGS_DEBUG      += -L../debug_build -lsrc
QMAKE_LFLAGS_RELEASE    += -L../release_build -lsrc

INCLUDEPATH += ../lib/boost_1_55_0 \

#    ../lib/libxml2/include

LIBS += -L../lib/boost_1_55_0/stage/lib \

#    -L../lib/libxml2/.libs
LIBS += -lxml2
LIBS += -lboost_program_options

LIBS += -lpthread -liconv

INCLUDEPATH +=/Users/hehao/work/lib/mongo-cxx-driver-v2.4/src \
    /Users/hehao/work/lib/boost_1_55_0

LIBS += -L/Users/hehao/work/lib/mongo-cxx-driver-v2.4 \
    -L/Users/hehao/work/lib/boost_1_55_0/stage/lib

LIBS += -lmongoclient -lboost_filesystem -lboost_system -lboost_thread

CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../build/debug
}

CONFIG(release, debug|release) {
LIBS += -L../build/release
}

#for debug
QMAKE_CXXFLAGS_DEBUG += -D_DEBUG -DDEBUG
QMAKE_CFLAGS_DEBUG += -D_DEBUG -DDEBUG

#macosx min version
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
}

