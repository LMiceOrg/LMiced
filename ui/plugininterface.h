#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

#include <QString>
#include <QStringList>

class PluginInterface
{
public:
    enum PluginType { Custom_Type, Command_Type, Gui_Type};

    virtual ~PluginInterface(){}

    // Name of plugin
    virtual QString name() const = 0;

    // Copyright info
    virtual QStringList copyright() const = 0;

    // Version of plugin
    virtual QString version() const = 0;

    //Type of plugin
    virtual PluginType type() const = 0;
};

QT_BEGIN_NAMESPACE

Q_DECLARE_INTERFACE(PluginInterface, "org.lmice.designer.plugin.PluginInterface")

QT_END_NAMESPACE

#endif // PLUGININTERFACE_H
