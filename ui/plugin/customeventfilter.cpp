#include "customeventfilter.h"

#include <QKeyEvent>

CustomEventFilter::CustomEventFilter(QObject *parent) : QObject(parent)
{

}

bool CustomEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ev= static_cast<QKeyEvent*>(e);
        if(ev->key() == Qt::Key_Return ||
                ev->key() == Qt::Key_Tab ||
                ev->key() == Qt::Key_Escape)
        {
            emit keyboardEvent(ev->key(),ev->modifiers());
            return true;
        }

    }
    return QObject::eventFilter(obj, e);
}
