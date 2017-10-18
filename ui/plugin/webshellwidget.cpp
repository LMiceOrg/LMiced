#include "webshellwidget.h"
#include "ui_webshellwidget.h"

#include <QWebEngineSettings>

#include <QToolBar>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

#include <QKeyEvent>

WebShellWidget::WebShellWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WebShellWidget)
{
    ui->setupUi(this);
    QVBoxLayout *vb = new QVBoxLayout;

    view = new QWebEngineView(this);
    view->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);
    view->setFocusPolicy(Qt::StrongFocus);


    QAction* act = new QAction(
                style()->standardIcon(QStyle::SP_DriveNetIcon),
                tr("Open"),
                this);
    QObject::connect(act, &QAction::triggered, [this](){
        QDialog dlg(this);
        QFormLayout *form = new QFormLayout;

        QLineEdit* editHost = new QLineEdit("192.168.56.101");
        QLineEdit* editPort = new QLineEdit("8080");
//        QLineEdit* editUser = new QLineEdit("hehao");
//        QLineEdit* editPass = new QLineEdit("123");
//        editPass->setEchoMode(QLineEdit::Password);

        QDialogButtonBox* btn = new QDialogButtonBox();
        btn->addButton(QDialogButtonBox::Ok);
        btn->addButton(QDialogButtonBox::Cancel);
        connect(btn, SIGNAL(accepted()), &dlg, SLOT(accept()));
            connect(btn, SIGNAL(rejected()), &dlg, SLOT(reject()));

            form->addRow(new QLabel(tr("Host IP")), editHost);
            form->addRow(new QLabel(tr("Port")), editPort);
//            form->addRow(new QLabel(tr("Name")), editUser);
//            form->addRow(new QLabel(tr("Password")), editPass);
            form->addWidget(btn);

            dlg.setLayout(form);

            int ret = dlg.exec();
            if(ret == 0)
                return;

            QUrl url;
            url.setHost(editHost->text());
            url.setPort(editPort->text().toInt());
            url.setScheme("http");
            view->setUrl(url);
            qDebug()<<url.port()<<url.scheme()<<url.host();
    });

    QToolBar *bar = new QToolBar(tr("ToolBar"), this);
    bar->setIconSize(QSize(16,16) );
    bar->setFloatable(true);
    bar->setMovable(true);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    vb->insertWidget(0,bar);
    bar->addAction(act);
    this->addAction(act);
    view->addAction(act);

    act = new QAction(style()->standardIcon(QStyle::SP_LineEditClearButton),
                      tr("Clear"),
                      this);
          QObject::connect(act, &QAction::triggered, [this](){

              view->page()->runJavaScript("shellinabox.sendKeys('0d636c6561720d');");

          });

          bar->addAction(act);

    // Show custom actions in popup menu
    this->setContextMenuPolicy(Qt::ActionsContextMenu);

//    QUrl url("http://192.168.56.101:6175/");

//    qDebug()<<url.port()<<url.scheme()<<url.host();
//    view->setUrl(url);
    connect(view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
    connect(view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

    vb->addWidget(view);
    this->setLayout(vb);

}

WebShellWidget::~WebShellWidget()
{
    delete ui;
}

void WebShellWidget::adjustLocation()
{

}

void WebShellWidget::changeLocation()
{

}

void WebShellWidget::adjustTitle()
{
    setWindowTitle(view->title());
}

void WebShellWidget::setProgress(int p)
{

}

void WebShellWidget::finishLoading(bool)
{

}
