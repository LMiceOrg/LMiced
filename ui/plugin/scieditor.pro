TEMPLATE      = lib
CONFIG       += plugin static
QT           += widgets
TARGET        = $$qtLibraryTarget(scieditor)

include(../../../lib/QScintilla.pri)

HEADERS += \
    scieditorplugin.h \
    scieditorwidget.h \
    ../plugininterface.h \
    ../guiinterface.h

SOURCES += \
    scieditorplugin.cpp \
    scieditorwidget.cpp

INCLUDEPATH += ..

FORMS += \
    scieditorwidget.ui

DISTFILES += \
    scieditorplugin.json
