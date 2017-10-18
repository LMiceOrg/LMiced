TEMPLATE      = lib
CONFIG       += plugin static
QT           += widgets webenginewidgets
TARGET        = $$qtLibraryTarget(webshell)

INCLUDEPATH += ..

HEADERS += \
    webshellplugin.h \
    webshellwidget.h

SOURCES += \
    webshellplugin.cpp \
    webshellwidget.cpp

DISTFILES += \
    webshellplugin.json

FORMS += \
    webshellwidget.ui
