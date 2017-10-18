#include "sshshellplugin.h"

#include <QPluginLoader>
#include <QStaticPlugin>
#include <QStringList>

#include <QDebug>

#include "sshshellwidget.h"

SshShellPlugin::SshShellPlugin(QObject *parent)
    :QObject(parent)
{

}

SshShellPlugin::~SshShellPlugin()
{

}


QWidget *SshShellPlugin::create(const QString &act)
{
    (void)act;
    SshShellWidget* widget = new SshShellWidget();
    return widget;
}
