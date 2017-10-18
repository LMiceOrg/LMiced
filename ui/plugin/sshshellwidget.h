#ifndef SSHSHELLWIDGET_H
#define SSHSHELLWIDGET_H

#include <QWidget>

#include "sshshell.h"

namespace Ui {
class SshShellWidget;
}

class SshShellWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SshShellWidget(QWidget *parent = 0);
    ~SshShellWidget();

public slots:
    void on_error_message(const QString&);
    void on_output_message(const QString&);

    void cmdEdit_keyboardEvent(int k, unsigned int m);
private slots:
    void on_cmdEdit_returnPressed();



private:
    Ui::SshShellWidget *ui;
    SshShell* shell;
    int lastPos;
};

#endif // SSHSHELLWIDGET_H
