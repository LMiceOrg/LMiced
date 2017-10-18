#ifndef GUIINTERFACE_H
#define GUIINTERFACE_H
#include "plugininterface.h"

#include <QtWidgets>

class GuiInterface:public PluginInterface
{
public:
    virtual ~GuiInterface()=default;

    // Default implement PluginInterface
    virtual QString name() const override {return "GuiInterface";}
    virtual QStringList copyright() const override {return QStringList()<<"Copyright 2017";}
    virtual QString version() const override {return "1.0";}
    virtual PluginType type() const override {return Gui_Type;}

    // Menu of plugin
    virtual QStringList menu() const {return QStringList();}

    // Menu action to create Widget
    virtual QWidget* create(const QString&) {return nullptr;}
signals:



};
QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(GuiInterface, "org.lmice.designer.plugin.GuiInterface")
QT_END_NAMESPACE



#endif // GUIINTERFACE_H
