#ifndef WEBSHELLWIDGET_H
#define WEBSHELLWIDGET_H

#include <QWidget>

#include <QWebEngineView>

namespace Ui {
class WebShellWidget;
}

class WebShellWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebShellWidget(QWidget *parent = 0);
    ~WebShellWidget();

protected slots:

      void adjustLocation();
      void changeLocation();
      void adjustTitle();
      void setProgress(int p);
      void finishLoading(bool);

private:
    Ui::WebShellWidget *ui;
    QWebEngineView *view;
};

#endif // WEBSHELLWIDGET_H
