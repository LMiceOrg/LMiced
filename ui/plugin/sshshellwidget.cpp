#include "sshshellwidget.h"
#include "ui_sshshellwidget.h"

#include <QFileDialog>
#include <QFile>
#include <QTextDocumentFragment>
#include <QToolBar>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>

#include <ssh/sshconnection.h>
#include "sshshell.h"

#include "customeventfilter.h"

SshShellWidget::SshShellWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SshShellWidget)
{
    ui->setupUi(this);
    shell = nullptr;

    QToolBar *bar = new QToolBar(this);
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->verticalLayout->insertWidget(0,bar);


    // Action
    //QStyle style;
    QIcon ico = style()->standardIcon(QStyle::SP_DialogOpenButton);


    QAction* act = new QAction(ico, tr("Open"), this);
    act->setShortcut(QKeySequence::Open);
    act->setShortcutContext(Qt::WidgetShortcut);
    this->addAction(act);
    bar->addAction(act);
    ui->siteCmbBox->addAction(act);
    QObject::connect(act, &QAction::triggered, [this](){
        QDialog dlg(this);
        QFormLayout *form = new QFormLayout;

        QLineEdit* editHost = new QLineEdit("192.168.56.101");
        QLineEdit* editPort = new QLineEdit("22");
        QLineEdit* editUser = new QLineEdit("hehao");
        QLineEdit* editPass = new QLineEdit("123");
        editPass->setEchoMode(QLineEdit::Password);

        QDialogButtonBox* btn = new QDialogButtonBox();
        btn->addButton(QDialogButtonBox::Ok);
        btn->addButton(QDialogButtonBox::Cancel);
        connect(btn, SIGNAL(accepted()), &dlg, SLOT(accept()));
            connect(btn, SIGNAL(rejected()), &dlg, SLOT(reject()));

        form->addRow(new QLabel(tr("Host IP")), editHost);
        form->addRow(new QLabel(tr("Port")), editPort);
        form->addRow(new QLabel(tr("Name")), editUser);
        form->addRow(new QLabel(tr("Password")), editPass);
        form->addWidget(btn);


        dlg.setLayout(form);

        int ret = dlg.exec();
        if(ret == 0)
            return;
//        QString name = QFileDialog::getOpenFileName(this, tr("Open"));
//        if(name.isEmpty())
//            return;
//        QFile file(name);
//        file.open(QIODevice::ReadOnly);
//        QByteArray data = file.readAll();
//        file.close();
//        ui->textEdit->setText(data.data());
//        this->setWindowTitle(name);
        QSsh::SshConnectionParameters param;
        param.host =editHost->text();// "192.168.56.101";
        param.port = editPort->text().toInt();//22;
        param.userName = editUser->text();// "hehao";
        param.password = editPass->text();//"123";
        param.authenticationType = param.AuthenticationTypePassword;
        param.timeout =120;
        if(shell)
        {
            delete shell;
            shell = nullptr;
        }
        shell = new SshShell(param, this);
        shell->run();
        //ui->textEdit->setText("hello\nwelcome\nmy name\nis hehao\n");
        //ui->textEdit->markerAdd(2, 0);
        connect(shell, SIGNAL(sshStandardError(QString)),  this, SLOT(on_error_message(QString)) );
        connect(shell, SIGNAL(sshStandardOutput(QString)), this, SLOT(on_output_message(QString)) );



    });

    act = new QAction(style()->standardIcon(QStyle::SP_TitleBarCloseButton), tr("Clear"), this);
    QObject::connect(act, &QAction::triggered, [this](){
        ui->textEdit->clear();
        if(shell)
        {
            shell->writeToSsh("\n");
        }
    });
    bar->addAction(act);
    this->addAction(act);


    // Show custom actions in popup menu
    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->siteCmbBox->setContextMenuPolicy(Qt::ActionsContextMenu);

    ui->textEdit->setCursorWidth(2);

    CustomEventFilter* ef = new CustomEventFilter(this);
    ui->textEdit->installEventFilter(ef);
    connect(ef, SIGNAL(keyboardEvent(int,unsigned int)),
            this, SLOT(cmdEdit_keyboardEvent(int,unsigned int)) );

}

SshShellWidget::~SshShellWidget()
{
    delete ui;
}

void SshShellWidget::on_error_message(const QString & msg)
{
    qDebug()<<"error "<<msg;
    //ui->textEdit->append(msg);

    QTextCursor cur = ui->textEdit->textCursor();
    cur.insertHtml(msg);

    cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    ui->textEdit->setTextCursor(cur);
}

void SshShellWidget::on_output_message(const QString & msg)
{
    qDebug()<<"output "<<msg;
    QChar c = 0x0007;
    //QString beep (c);
    //qDebug()<<"b="<<c<<" m="<<msg.size()<<msg.at(0);
    //ui->textEdit->append(msg);
    if(msg.at(0) == c)
    {
        qDebug()<<"beep";
        qApp->beep();
        return;
    }
    QTextCursor cur = ui->textEdit->textCursor();
    cur.insertHtml(msg);

    cur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
    ui->textEdit->setTextCursor(cur);
    lastPos = cur.position();
    //cur.setPosition(cur.position() );

}

void SshShellWidget::cmdEdit_keyboardEvent(int k, unsigned int m)
{
    if(!shell)
        return;
    (void)k;
    (void)m;
    //return;
    //if(m & Qt::ShiftModifier &&
    if(k ==Qt::Key_Return)
    {
          ui->textEdit->setUpdatesEnabled(false);

        QTextCursor cur = ui->textEdit->textCursor();
        //qDebug()<<"pos="<<lastPos<<cur.position();
        cur.setPosition(lastPos, QTextCursor::KeepAnchor);
        QTextDocumentFragment  f = cur.selection();
        QString cmd;
        if(!f.isEmpty())
        {
            cmd = f.toPlainText();
            cur.removeSelectedText();
            //qDebug()<<" cmd="<<cmd;

        }
        cmd += "\n";
        shell->writeToSsh(cmd);

        ui->textEdit->setUpdatesEnabled(true);
    }
    else if(k == Qt::Key_Tab)
    {
        qDebug()<<"send tab";
        shell->writeToSsh("\011");
    }
    else if(k == Qt::Key_Escape)
    {
        qDebug()<<"send esc";
        shell->writeToSsh("\033");
    }
}

void SshShellWidget::on_cmdEdit_returnPressed()
{
    if(!shell)
     return;

    //ui->cmdEdit->insert("\n");
    shell->writeToSsh(ui->cmdEdit->text()+"\n");
    ui->cmdEdit->clear();


}
