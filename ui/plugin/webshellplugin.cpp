#include "webshellplugin.h"

#include "webshellwidget.h"

WebShellPlugin::WebShellPlugin(QObject *parent)
    :QObject(parent)
{

}

WebShellPlugin::~WebShellPlugin()
{

}

QWidget *WebShellPlugin::create(const QString &act)
{
    (void)act;
    WebShellWidget *widget = new WebShellWidget;
    return widget;

}
