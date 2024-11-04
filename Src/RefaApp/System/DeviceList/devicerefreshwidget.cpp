#include "devicerefreshwidget.h"

#include <QCloseEvent>
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"

DeviceRefreshWidget::DeviceRefreshWidget(QWidget *parent) :
	QWidget(parent)
{
	initUI();
}

DeviceRefreshWidget::~DeviceRefreshWidget()
{
}

void DeviceRefreshWidget::startSearchDevices()
{
	dev_search_thread_ptr_->start();
}

void DeviceRefreshWidget::closeEvent(QCloseEvent *event)
{
	event->accept();
}

void DeviceRefreshWidget::onSearchFinished(int device_count)
{
	dev_manager_ptr_->createDevices();
	emit signalRefreshSearchFinished(device_count);
	close();
}

void DeviceRefreshWidget::initUI()
{
	setHidden(true);
	dev_search_thread_ptr_ = new DeviceSearchThread();
	dev_manager_ptr_ = &DeviceManager::instance();
	connect(dev_manager_ptr_.data(), &DeviceManager::searchFinished, this, &DeviceRefreshWidget::onSearchFinished);
}
