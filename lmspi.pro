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

INCLUDEPATH += . \
    ../lib/boost_1_57_0
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

INCLUDEPATH += C:/Python27/include
LIBS += -L../lib/boost_1_57_0/stage/x64 \
-LC:/Python27/libs

}
