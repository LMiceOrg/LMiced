#include <QApplication>
#include "mainwindow.h"
#include <QtPlugin>

Q_IMPORT_PLUGIN(LMSciEditorPlugin);

Q_IMPORT_PLUGIN(SshShellPlugin);

Q_IMPORT_PLUGIN(WebShellPlugin);

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    app.exec();
}
