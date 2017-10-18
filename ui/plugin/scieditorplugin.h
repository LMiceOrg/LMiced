#ifndef SCIEDITORPLUGIN_H
#define SCIEDITORPLUGIN_H

#include <QObject>
#include <QWidget>

#include <guiinterface.h>

class LMSciEditorPlugin : public QObject, public GuiInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lmice.designer.plugin.SciEditorPlugin" FILE "scieditorplugin.json")
    Q_INTERFACES(PluginInterface GuiInterface)

public:
    explicit LMSciEditorPlugin(QObject *parent = nullptr);
    ~LMSciEditorPlugin();

    QWidget* create(const QString& act) override;

public slots:

private:

};

#endif // SCIEDITORPLUGIN_H
