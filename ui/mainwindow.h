#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void loadPlugins();

signals:
    void widgetClose();
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionFile_triggered();

private:
    Ui::MainWindow *ui;
    std::map<QString, QMenu*> m_mmap;
    std::map<QString, QMenu*> m_lv1mmap;
};

#endif // MAINWINDOW_H
