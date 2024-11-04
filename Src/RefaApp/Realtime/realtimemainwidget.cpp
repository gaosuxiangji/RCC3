#include "realtimemainwidget.h"
#include "ui_realtimemainwidget.h"


#include "Device/devicemanager.h"
#include "Device/device.h"
#include "Video/Export/exportutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/setroiwidget.h"
#include "PlayerStructer.h"
#include "LogRunner.h"

using namespace FHJD_LOG;

RealtimeMainWidget::RealtimeMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealtimeMainWidget)
{
	LOG_INFO("Construction");
    ui->setupUi(this);

	connect(ui->widgetPlayer, &RealtimePlayerWidget::fullScreenTriggered, this, &RealtimeMainWidget::fullScreen);
	connect(ui->widgetPlayer, &RealtimePlayerWidget::focusPlayerChanged, this, &RealtimeMainWidget::onFocusPlayerChanged);
    connect(ui->widgetPlayer, &RealtimePlayerWidget::roiChanged, this, &RealtimeMainWidget::onRoiChanged);
    connect(ui->widgetPlayer, &RealtimePlayerWidget::focusPointChanged, this, &RealtimeMainWidget::onFocusPointChanged);
	connect(ui->widgetPlayer, &RealtimePlayerWidget::focusPointVisible, this, &RealtimeMainWidget::onFocusPointVisible);
	connect(ui->widgetPlayer, &RealtimePlayerWidget::saveSnapshot, this, &RealtimeMainWidget::slotSanpshot);

	connect(ui->widgetCtrl, &RealtimeCtrlWidget::sigSetDisplayInfoVisible, ui->widgetPlayer, &RealtimePlayerWidget::setOSDVisible);

    DeviceManager& device_manager = DeviceManager::instance();
    connect(&device_manager, &DeviceManager::deviceConnected, this, &RealtimeMainWidget::onDeviceConnected);
    connect(&device_manager, &DeviceManager::deviceDisconnected, this, &RealtimeMainWidget::onDeviceDisconnected);

	//connect(this, &RealtimeMainWidget::roiChanged, ui->widgetCtrl, &RealtimeCtrlWidget::slotUpdateDynamicRoi);
	ui->widgetRoi->setCurInterfaceType(CurInterfaceType::RealtimeType);
	ui->stackedWidget->setCurrentWidget(ui->widgetCtrl);

	connect(ui->widgetRoi, &SetRoiWidget::sigDeviceRoiChangeFinished, this, &RealtimeMainWidget::onRoiChangeFinished);
}

RealtimeMainWidget::~RealtimeMainWidget()
{
    delete ui;
}

void RealtimeMainWidget::setFullScreen(bool benable)
{
	ui->widgetPlayer->setFullScreen(benable);
}

void RealtimeMainWidget::setFocusPoint(const QPoint &pt)
{
    ui->widgetPlayer->setFocusPoint(pt);
}

void RealtimeMainWidget::setRoi(const QRect &roi)
{
    ui->widgetPlayer->setRoi(roi);
}

void RealtimeMainWidget::onDeviceConnected(const QString & device_ip)
{
    auto device_ptr = DeviceManager::instance().getDevice(device_ip);
    if (device_ptr)
    {
		//绑定信号和槽
		connect(device_ptr.data(), &Device::propertyChanged, this, &RealtimeMainWidget::onDevicePropertyChanged);
		connect(device_ptr.data(), &Device::stateChanged, this, &RealtimeMainWidget::onDeviceStateChanged);

        // 添加视频到播放器
        ui->widgetPlayer->addVideo(device_ptr->getRealtimePlayerController(), device_ptr->getIp());//添加时自动选中当前添加的播放器

		device_ptr->refreshOSDAfterConnect();//连接后刷新OSD信息

        QString current_ip = ui->widgetPlayer->getCurrentPlayerIp();
        if (current_ip == device_ptr->getIp())
        {
            // 设置设备到播放控制界面
            ui->widgetCtrl->setDevice(device_ptr);

			updateProperty(device_ip);
        }
    }
}

void RealtimeMainWidget::onDeviceDisconnected(const QString & device_ip)
{
    auto device_ptr = DeviceManager::instance().getDevice(device_ip);
    if (device_ptr)
    {
		//解除信号绑定
		disconnect(device_ptr.data(), 0, this, 0);

		QString current_ip = ui->widgetPlayer->getCurrentPlayerIp();
        // 从播放器删除视频
        ui->widgetPlayer->removeVideo(device_ptr->getRealtimePlayerController());


        if (current_ip == device_ptr->getIp())
        {
            // 重置播放控制界面的设备
            ui->widgetCtrl->setDevice(QSharedPointer<Device>());
        }
    }
}

void RealtimeMainWidget::onFocusPlayerChanged(const QString &ip)
{
	if (!ip.isEmpty())
	{
		ui->widgetCtrl->setDevice(DeviceManager::instance().getDevice(ip));
	}
	else
	{
		ui->widgetCtrl->setDevice(QSharedPointer<Device>());
	}
}

void RealtimeMainWidget::slotSanpshot(const QString& ip, const RMAImage& img)
{
	if (img.Empty() || ip.isEmpty())
		return;

	QString filepath = ExportUtils::getSnapshotPath(ip);
	bool ok = ExportUtils::snapshot(filepath, img);
	if (ok)
	{
		UIUtils::showInfoMsgBox(this, tr("Snapshot Saved."));
	}
	else
	{
		UIUtils::showInfoMsgBox(this, tr("Out of disk space."));
	}
}

void RealtimeMainWidget::onRoiChanged(const QString& ip, const QRect& roi)
{
	//切换到roi设置页面
	ui->stackedWidget->setCurrentWidget(ui->widgetRoi);
	ui->widgetRoi->DeviceRoiChanged(ip, roi);
	
}

void RealtimeMainWidget::onRoiChangeFinished(bool b_applyed, const QString &ip, const QRect &roi)
{
	if (b_applyed)
	{
		ui->widgetCtrl->slotUpdateDynamicRoi(ip, roi);
		auto device_ptr = DeviceManager::instance().getDevice(ip);
		if (!device_ptr)
			return;
		ui->widgetPlayer->setRoi(device_ptr->getProperty(Device::PropRoi).toRect());
	}
	else
	{
		ui->widgetPlayer->setRoi(roi);
	}

	//播放器设置为空闲模式
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);

	//切换回到控制面板
	ui->stackedWidget->setCurrentWidget(ui->widgetCtrl);
}

void RealtimeMainWidget::onFocusPointChanged(const QString& ip, const QPoint& pt)
{
	if (ip != ui->widgetPlayer->getCurrentPlayerIp())
		return;
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
		return;

	device_ptr->setProperty(Device::PropType::PropFocusPoint, pt);
}

void RealtimeMainWidget::onFocusPointVisible(const QString &ip, bool bvisible)
{
	if (ip != ui->widgetPlayer->getCurrentPlayerIp())
		return;
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
		return;
	device_ptr->setProperty(Device::PropType::PropFocusPointVisible, bvisible);
}

void RealtimeMainWidget::onDevicePropertyChanged(Device::PropType type, const QVariant & value)
{
	auto device_ptr = dynamic_cast<Device*>(sender());
	if (!device_ptr)
		return;
	if (device_ptr->getIp() != ui->widgetPlayer->getCurrentPlayerIp())
		return;
	if (Device::PropType::PropRoi == type)
	{
		setRoi(value.toRect());
	}
}

void RealtimeMainWidget::onDeviceStateChanged(DeviceState state)
{
	auto device_ptr = dynamic_cast<Device*>(sender());
	if (!device_ptr)
		return;
	ui->widgetPlayer->setRoiSelectionEnabled(DeviceState::Previewing == state, device_ptr->getIp());
}

void RealtimeMainWidget::updateProperty(const QString& ip)
{
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
		return;
	QVariant var = device_ptr->getProperty(Device::PropRoi);
	if (var.isValid())
		ui->widgetPlayer->setRoi(var.toRect());
	var = device_ptr->getProperty(Device::PropFocusPoint);
	if (var.isValid())
		ui->widgetPlayer->setFocusPoint(var.toPoint());
	ui->widgetPlayer->setFocusPointVisible(device_ptr->getProperty(Device::PropFocusPointVisible).toBool());
}
