#ifndef CSDEVICETREEVIEW_H
#define CSDEVICETREEVIEW_H

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QKeyEvent>
#include <QDropEvent>
/**
 * @brief 设备树QTreeView控件
 */
class CSDeviceTreeView : public QTreeWidget
{
    Q_OBJECT
public:
    CSDeviceTreeView(QWidget *parent = nullptr);
    virtual ~CSDeviceTreeView();

signals:
	void signalDragFinished();
protected:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;
private:
};

#endif // CSDEVICETREEVIEW_H
