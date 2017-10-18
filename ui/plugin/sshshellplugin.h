#ifndef SSHSHELLPLUGIN_H
#define SSHSHELLPLUGIN_H

#include <QObject>
#include <QWidget>

#include "guiinterface.h"

class SshShellPlugin : public QObject, public GuiInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lmice.designer.plugin.SshShellPlugin" FILE "sshshellplugin.json")
    Q_INTERFACES(PluginInterface GuiInterface)
public:
    explicit SshShellPlugin(QObject *parent = nullptr);
    ~SshShellPlugin();


    QWidget* create(const QString& act) override;

};

#endif // SSHSHELLPLUGIN_H
