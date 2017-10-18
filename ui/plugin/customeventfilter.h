#ifndef CUSTOMEVENTFILTER_H
#define CUSTOMEVENTFILTER_H

#include <QObject>
#include <QEvent>

class CustomEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit CustomEventFilter(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* e);
signals:
    void keyboardEvent(int key, unsigned int mod);
public slots:
};

#endif // CUSTOMEVENTFILTER_H
