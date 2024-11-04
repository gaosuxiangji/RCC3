#ifndef DEVICETREEVIEW_H
#define DEVICETREEVIEW_H
#include <QTreeView>
#include <QWidget>
#include <QModelIndex>

/**
 * @brief 设备树视图类
 */
class DeviceTreeView : public QTreeView
{

	Q_OBJECT
public:
    explicit DeviceTreeView(QWidget *parent = nullptr);

signals:
	void sigItemDropped(QModelIndex index);
protected:
	
	void dropEvent(QDropEvent *event)override;
	void dragMoveEvent(QDragMoveEvent *event)override;


};

#endif // DEVICETREEVIEW_H
