#include "devicetreeview.h"

#include <QDropEvent>

DeviceTreeView::DeviceTreeView(QWidget *parent):QTreeView(parent)
{
	
}

void DeviceTreeView::dropEvent(QDropEvent *event)
{

	QTreeView::dropEvent(event);
	emit sigItemDropped(indexAt(event->pos()));
	expandAll();
}

void DeviceTreeView::dragMoveEvent(QDragMoveEvent *event)
{
	if (indexAt(event->pos()).column() > 0)
	{
		event->setAccepted(false);
		return;
	}
	QTreeView::dragMoveEvent(event);
}
