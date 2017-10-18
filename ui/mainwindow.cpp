#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPluginLoader>

#include "ui/guiinterface.h"

#include <QDebug>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <map>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setCentralWidget(ui->mdiArea);
    loadPlugins();
    ui->mdiArea->tileSubWindows();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadPlugins()
{
    QMenuBar* menubar = this->menuBar();
    auto findMenu=[menubar](const QString& name) {
        QMenu*menu = nullptr;
        foreach(QObject* obj, menubar->children())
        {
            QMenu*menu = qobject_cast<QMenu*>(obj);
            if(menu && menu->title().compare(name) == 0)
            {
                return menu;
            }
        }
        menu = nullptr;
        return menu;
    };

    foreach (QStaticPlugin plugin, QPluginLoader::staticPlugins())
    {
        QJsonObject o = plugin.metaData().value("MetaData").toObject();
        qDebug()<<"plugin ver=" <<o.value("version").toInt()
               <<" name= "<<o.value("name").toString();
        GuiInterface* gui = qobject_cast<GuiInterface*>(plugin.instance());
        if(gui)
        {
            QJsonArray array = o.value("menu").toArray();

            QStringList sl;
            foreach(QJsonValue o, array)
            {
                sl<<o.toString();
            }

            //QStringList sl = gui->menu();
            qDebug()<<"menu"<<sl;
            for(int i=0; i<sl.length(); ++i)
            {
                QString name = sl.at(i);
                QStringList sm = name.split("|");
                if(sm.length() > 1)
                {
                    QMenu* menu = nullptr;
                    for(int j=0; j<sm.length() -1; ++j)
                    {
                        auto iter = m_mmap.find(sm.at(j));
                        if(iter != m_mmap.end())
                        {
                            menu = iter->second;
                        }
                        else if(menu == nullptr)
                        {
                            menu = findMenu( sm.at(j) );
                            if(menu == nullptr)
                            {
                                // New level 1 menu
                                menu = new QMenu(sm.at(j));
                                m_mmap[sm.at(j)] = menu;
                                m_lv1mmap[sm.at(j)]=menu;
                            }
                        }
                        else
                        {
                            menu = menu->addMenu(sm.at(j));
                            m_mmap[sm.at(j)]=menu;
                        }

                    }//end-for

                    QAction* act = new QAction(sm.last());
                    menu->addAction(act);
                    QObject::connect(act, &QAction::triggered,  [gui, name, this]{

                        QWidget* w = gui->create(name);
                        QMdiSubWindow* sw = ui->mdiArea->addSubWindow(w);

                        sw->setAttribute(Qt::WA_DeleteOnClose);
                        sw->setOption(QMdiSubWindow::RubberBandResize, true);
                        sw->setOption(QMdiSubWindow::RubberBandMove, true);
                        sw->show();



                    }
                    ) ;
                }


            }//end-for
        }
    }

    // Append created menu to menubar
    for(auto iter = m_lv1mmap.begin(); iter != m_lv1mmap.end(); ++iter)
    {
        menubar->addMenu(iter->second);
        iter->second->setParent(menubar);
    }

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit widgetClose();
    event->accept();
}

void MainWindow::on_actionFile_triggered()
{
    //ui->mdiArea->tileSubWindows();
}
