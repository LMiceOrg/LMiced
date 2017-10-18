TEMPLATE = app
CONFIG += qt

greaterThan(QT_MAJOR_VERSION, 4) {
        QT += widgets printsupport

    greaterThan(QT_MINOR_VERSION, 1) {
            macx:QT += macextras
    }

    # Work around QTBUG-39300.
    CONFIG -= android_install
}

QT += network widgets webenginewidgets

LIBS += -L$$OUT_PWD/ui/plugin -lscieditor -lsshclient -lwebshell

PRE_TARGETDEPS = $$OUT_PWD/ui/plugin/libscieditor.a
PRE_TARGETDEPS = $$OUT_PWD/ui/plugin/libsshclient.a
PRE_TARGETDEPS = $$OUT_PWD/ui/plugin/libwebshell.a

OTHER_FILES += doc/designer.html

SOURCES += \
    ui/main.cpp \
    ui/mainwindow.cpp

FORMS += \
    ui/mainwindow.ui

HEADERS += \
    ui/mainwindow.h \
    ui/plugininterface.h \
    ui/guiinterface.h

SUBDIRS += \
    ui/plugin/webshell.pro



