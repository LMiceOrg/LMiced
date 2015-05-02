TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += \
    web/json_request.h \
    web/lmprofile.h \
    web/user.h \
    web\sha1.h

SOURCES += web\main.cpp \
    web/json_request.cpp \
    web/lmprofile.cpp \
    web/user.cpp \
    web/sha1.c

DEFINES += NDEBUG _WEBSOCKETPP_CPP11_STL_ _WEBSOCKETPP_NO_CPP11_REGEX_
DEFINES += _WEBSOCKETPP_NOEXCEPT_TOKEN_= _WEBSOCKETPP_CONSTEXPR_TOKEN_=

INCLUDEPATH += .




#Boost 1.57 library
INCLUDEPATH += ../lib/boost_1_57_0
#Jansson lib
INCLUDEPATH += ../lib/jansson-2.7/bin/include
#websocketpp lib
INCLUDEPATH += ../lib/websocketpp

win32-msvc* {
message("Windows Visual C Compiler")
QMAKE_CFLAGS_DEBUG = -MTd -O2
QMAKE_CFLAGS_RELEASE = -MT -O2
QMAKE_CXXFLAGS_DEBUG = -MTd -O2
QMAKE_CXXFLAGS_RELEASE = -MT -O2

LIBS += -L../lib/boost_1_57_0/stage/x64


CONFIG(debug, debug|release) {
message("debug mode")
LIBS += -L../build/debug
LIBS += -L../lib/jansson-2.7/bin/x64/Debug
LIBS += -ljansson_d
LIBS += -lsrc
}

CONFIG(release, debug|release) {
message("release mode")
LIBS += -L../build/release
LIBS += -L../lib/jansson-2.7/bin/x64/Release
LIBS += -ljansson
LIBS += -lsrc
}

}




