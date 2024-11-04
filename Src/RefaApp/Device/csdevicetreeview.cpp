#include "csdevicetreeview.h"

CSDeviceTreeView::CSDeviceTreeView(QWidget *parent):QTreeWidget(parent)
{

}

CSDeviceTreeView::~CSDeviceTreeView()
{

}

void CSDeviceTreeView::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)//空格触发
	{
		QWidget::keyPressEvent(event);
		return;
	}
	QTreeView::keyPressEvent(event);
}

void CSDeviceTreeView::dropEvent(QDropEvent *event)
{
	emit signalDragFinished();
	QTreeWidget::dropEvent(event);
}
