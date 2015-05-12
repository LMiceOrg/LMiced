TEMPLATE = lib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += spi/main.cpp \
    spi/lmspi_python.cpp

HEADERS += \
    spi/lmspi_c.h \
    spi/lmspi_cxx.h \
    spi/lmspi_impl.h

INCLUDEPATH += .
DEFINES += BOOST_PYTHON_STATIC_LIB

CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../build/debug
}

CONFIG(release, debug|release) {
LIBS += -L../build/release
}

LIBS += -lsrc

DEFINES += LMSPI_PROJECT

win32-msvc* {
message("Windows Visual C Compiler")
QMAKE_CFLAGS_DEBUG = -MTd -O2
QMAKE_CFLAGS_RELEASE = -MT -O2

QMAKE_CXXFLAGS_DEBUG = -MTd -O2
QMAKE_CXXFLAGS_RELEASE = -MT -O2

INCLUDEPATH += C:/Python27/include \
../lib/boost_1_57_0
LIBS += -L../lib/boost_1_57_0/stage/x64 \
-LC:/Python27/libs

}

macx-clang* {
message("MacOS LLVM Compiler")
INCLUDEPATH += ../lib/boost_1_55_0 \
/opt/local/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7

QMAKE_CXXFLAGS += -F/opt/local/Library/Frameworks/Python.framework/Versions/2.7

LIBS += -L../lib/boost_1_55_0/stage/lib \
-L/opt/local/lib
LIBS += -stdlib=libc++ -lboost_python -lpython2.7
QMAKE_CXXFLAGS +=  -std=c++11 -stdlib=libc++
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
#QMAKE_CXXFLAGS += -std=gnu++0x
QMAKE_CFLAGS += -std=c89 -Weverything
QMAKE_CFLAGS += -funit-at-a-time -Wno-unused-function -Wall
QMAKE_CFLAGS += -Dinline=__inline__

QMAKE_CFLAGS_DEBUG += -DDEBUG -D_DEBUG

QMAKE_POST_LINK = cp liblmspi.dylib lmspi.so
}
