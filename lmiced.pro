TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    resource/resource_shm.cpp \
    resource/resource_meta_data.c \
    system/config.cpp \
    system/system_environment.cpp \
    system/system_signal.cpp \
    system/system_worker.cpp \
    system/system_master.cpp

OTHER_FILES += \
    README.txt

HEADERS += \
    system/system_environment.h \
    system/system_signal.h \
    system/system_environment_internal.h \
    system/system_worker.h      system/config.h \
    resource/resourec_shm.h \
    resource/resource_shm_internal.h \
    resource/resource_meta_data.h \
    resource/resource_meta_data_internal.h \
state/state_action.h		state/state_serialize_engine.h \
state/state_event.h		state/state_serialize_json.h \
state/state_guard.h		state/state_transition.h \
state/state_machine.h		state/state_type.h \
    system/system_master.h \
    eal/lmice_eal_thread_pthread.h \
    eal/lmice_eal_common.h

QMAKE_CFLAGS += -std=c89 -funit-at-a-time -Wno-unused-function
QMAKE_CFLAGS += -Dinline=__inline

QMAKE_CXXFLAGS += -std=c++0x -Wshadow

INCLUDEPATH += ../src
QMAKE_LFLAGS_DEBUG      += -L../debug_build -lsrc
QMAKE_LFLAGS_RELEASE    += -L../release_build -lsrc

macx-clang*{
message("MacX Darwin")

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

#CONFIG(debug, debug|release) {
#message("debug mode")
#LIBS += -L../debug_build
#}

#CONFIG(release, debug|release) {
#LIBS += -L../release_build
#}

#LIBS += -lsrc

#for debug
QMAKE_CXXFLAGS_DEBUG += -D_DEBUG -DDEBUG
QMAKE_CFLAGS_DEBUG += -D_DEBUG -DDEBUG


}

