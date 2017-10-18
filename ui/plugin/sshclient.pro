TEMPLATE      = lib
CONFIG       += plugin static
QT           += widgets
TARGET        = $$qtLibraryTarget(sshclient)

INCLUDEPATH += ..

include(../../../lib/QSsh.pri)

HEADERS += sshshell.h \
    sshshellplugin.h \
    sshshellwidget.h \
    customeventfilter.h

SOURCES += sshshell.cpp \
    sshshellplugin.cpp \
    sshshellwidget.cpp \
    customeventfilter.cpp

DISTFILES += \
    sshshellplugin.json

FORMS += \
    sshshellwidget.ui
