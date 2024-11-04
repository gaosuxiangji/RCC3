#ifndef QTGROUPBOXPROPERTYBROWSER_H
#define QTGROUPBOXPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

QT_BEGIN_NAMESPACE

class QtGroupBoxPropertyBrowserPrivate;

class QtGroupBoxPropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
public:

    QtGroupBoxPropertyBrowser(QWidget *parent = 0);
    ~QtGroupBoxPropertyBrowser();

protected:
    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
    virtual void itemRemoved(QtBrowserItem *item);
    virtual void itemChanged(QtBrowserItem *item);

private:

    QScopedPointer<QtGroupBoxPropertyBrowserPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtGroupBoxPropertyBrowser)
    Q_DISABLE_COPY(QtGroupBoxPropertyBrowser)
    Q_PRIVATE_SLOT(d_func(), void slotUpdate())
    Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed())

};

QT_END_NAMESPACE

#endif
