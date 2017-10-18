#ifndef WEBSHELLPLUGIN_H
#define WEBSHELLPLUGIN_H

#include <QObject>
#include <QWidget>

#include "guiinterface.h"

class WebShellPlugin: public QObject, public GuiInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lmice.designer.plugin.WebShellPlugin" FILE "webshellplugin.json")
    Q_INTERFACES(PluginInterface GuiInterface)
public:
    explicit WebShellPlugin(QObject *parent = nullptr);
    ~WebShellPlugin();


    QWidget* create(const QString& act) override;

};

#endif // WEBSHELLPLUGIN_H
