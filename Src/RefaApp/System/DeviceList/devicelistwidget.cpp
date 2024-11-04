#include "devicelistwidget.h"
#include "ui_devicelistwidget.h"

#include <QStandardItem>
#include <QTime>
#include <QDateTime>
#include <QScroller>

#include <QFileInfo>
#include <QFileDialog>
#include "Common/ErrorCodeUtils/errorcodeutils.h"
#include "device/device.h"
#include "Device/devicetreeview.h"
#include "System/DeviceList/devicesearchwidget.h"
#include "devicealterip.h"
#include "deviceeditname.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/AgLinkButtonItemDelegate/aglinkbuttonitemdelegate.h"
#include "System/SystemSettings/systemsettingsmanager.h"

DeviceListWidget::DeviceListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceListWidget)
{
    ui->setupUi(this);
	qRegisterMetaType<DeviceState>();//QueuedConnection时需要注册

    initUi();
}

DeviceListWidget::~DeviceListWidget()
{
    delete ui;
}

void DeviceListWidget::updateSearchedDeviceList()
{
    searched_model_ptr_->removeRows(0,searched_model_ptr_->rowCount());
    QList<QSharedPointer<Device> > devices;
	QList<QSharedPointer<Device> > added_devices;
    dev_manager_ptr_->getDevices(devices);
	dev_manager_ptr_->getAddedDevices(added_devices);
    int id =1;
    for(auto dev : devices)
    {
        //只显示未添加的设备
        if(added_devices.contains(dev))
        {
            continue;
        }

        QList<QStandardItem *> dev_info;//id 相机名 型号名 ip 状态 操作

        QStandardItem * pid = new QStandardItem();
		pid->setText(QString("%1").arg(id));
		pid->setTextAlignment(Qt::AlignCenter);
        id++;

		QStandardItem * name = new QStandardItem();
		name->setText(dev->getProperty(Device::PropName).toString());
		name->setTextAlignment(Qt::AlignCenter);

		QStandardItem * model_name = new QStandardItem();
		model_name->setText(dev->getModelName());
		model_name->setTextAlignment(Qt::AlignCenter);

        QStandardItem * ip = new QStandardItem();
		ip->setText(dev->getIp());
		ip->setTextAlignment(Qt::AlignCenter);

        QStandardItem * state = new QStandardItem();
		state->setText(/*dev->getStateStr()*/Device::toStateStr(DeviceState::Unconnected));
		state->setTextAlignment(Qt::AlignCenter);

        QStandardItem * action = new QStandardItem();
        action->setText(tr("connect"));
		action->setData(dev->getIp(), Qt::UserRole);
		action->setTextAlignment(Qt::AlignCenter);

        dev_info<<pid<<name<<model_name<<ip<<state<<action;
        searched_model_ptr_->appendRow(dev_info);


    }
}

void DeviceListWidget::updateConnectedDeviceTree()
{
	connected_model_ptr_->removeRows(0, connected_model_ptr_->rowCount());
	QList<QSharedPointer<Device> > devices;
	dev_manager_ptr_->getAddedDevices(devices);

	QList<QList<QStandardItem*> > dev_info_list;//设备信息表
	for (auto dev : devices)
	{
		QList<QStandardItem *> dev_info;// 相机名 型号名 ip 授权信息 状态 操作


		QStandardItem * name = new QStandardItem();
		name->setText(dev->getProperty(Device::PropName).toString());
        name->setIcon(QIcon(":/images/video_camera.png"));

		QStandardItem * model_name = new QStandardItem();
		model_name->setText(dev->getModelName());
		model_name->setTextAlignment(Qt::AlignCenter);

		QStandardItem * ip = new QStandardItem();
		ip->setText(dev->getIp());
		ip->setData(dev->getProperty(Device::PropParentIp), Qt::UserRole);//存放父设备IP 以便界面构建拓扑
		ip->setTextAlignment(Qt::AlignCenter);

		QStandardItem * auth_info = new QStandardItem();
		if (dev->getState() != DeviceState::Connecting)
		{
			DeviceLicense license;
			dev->getLicense(license);
			auth_info->setText(getLicenseDesc(license));
			auth_info->setTextAlignment(Qt::AlignCenter);
		}


		QStandardItem * state = new QStandardItem();
		state->setText(dev->getStateStr());
		state->setTextAlignment(Qt::AlignCenter);

		QStandardItem * action = new QStandardItem();
        action->setText(tr("disconnect"));
		action->setData(dev->getIp(), Qt::UserRole);//存放ip 以便获取设备指针
		action->setTextAlignment(Qt::AlignCenter);

		dev_info  << name << model_name << ip << auth_info << state << action;
		//connected_model_ptr_->appendRow(dev_info);
		dev_info_list << dev_info;
	}

	if (dev_info_list.count()== 1)
	{
		connected_model_ptr_->appendRow(dev_info_list.first());
	}
	else if (dev_info_list.count()== 2)
	{
		//判断是否形成拓扑结构
		QString left_dev_ip = dev_info_list.first().at(2)->text();
		QString left_parent_ip = dev_info_list.first().at(2)->data(Qt::UserRole).toString();
		QString right_dev_ip = dev_info_list.last().at(2)->text();
		QString right_parent_ip = dev_info_list.last().at(2)->data(Qt::UserRole).toString();

		if (left_parent_ip == right_dev_ip)//左相机是右相机的子设备
		{
			connected_model_ptr_->appendRow(dev_info_list.last());
			connected_model_ptr_->item(0)->appendRow(dev_info_list.first());
		}
		else if (right_parent_ip == left_dev_ip)//右相机是左相机的子设备
		{
			connected_model_ptr_->appendRow(dev_info_list.first());
			connected_model_ptr_->item(0)->appendRow(dev_info_list.last());
		}
		else//没有形成拓扑
		{
			connected_model_ptr_->appendRow(dev_info_list.first());
			connected_model_ptr_->appendRow(dev_info_list.last());
		}
		ui->treeViewConnected->expandAll();
	}
	slotCurrentSelectChanged(QModelIndex(), QModelIndex());//取消选中设备
}

void DeviceListWidget::SearchDevices()
{
	on_pushButtonSearch_clicked();
	autoConnectDevices();
}

void DeviceListWidget::DisconnectAllDevice()
{
	QList< QSharedPointer < Device > > connected_devices;
	dev_manager_ptr_->getConnectedDevices(connected_devices);
	for (auto device_ptr : connected_devices)
	{
		if (device_ptr)
		{
			device_ptr->Device::disconnect();
			//移出已添加设备列表
			dev_manager_ptr_->removeAddedDevice(device_ptr);
		}
	}
}

void DeviceListWidget::DisconnectDeviceByIndex(const QModelIndex &index)
{
	QSharedPointer<Device> device_ptr = dev_manager_ptr_->getDevice(index.data(Qt::UserRole).toString());
	if (device_ptr)
	{
		disconnect(device_ptr.data(),0,this,0);

		device_ptr->Device::disconnect();
		//移出已添加设备列表
		dev_manager_ptr_->removeAddedDevice(device_ptr);

		//已连接设备中移除该行
		connected_model_ptr_->removeRow(index.row());
		//刷新搜索到的设备列表
		updateSearchedDeviceList();
	}
}

void DeviceListWidget::slotSearchFinished(int device_count)
{
    //ui->pushButtonSearch->setEnabled(true);
    ui->labelSeachedDeviceCount->setText(QString(tr("<font color = #ff0000>*</font>%1 Devices Searched.")).arg(device_count));

	updateDeviceList();
}


void DeviceListWidget::slotDevStateChanged(DeviceState state)
{
	if (state == DeviceState::Connected)
	{ 
		//有设备状态变化，刷新表格和树
		updateDeviceList();
	}
	else
	{
		//仅更新状态
		auto device_ptr = dynamic_cast<Device*>(sender());
		if(device_ptr)
		{
			for (int i = 0; i < connected_model_ptr_->rowCount(); ++i)
			{
				auto ip_item_ptr = connected_model_ptr_->item(i, (int)ConnectedDeviceTreeColumn::kConnectedDeviceOperate);
				if (ip_item_ptr)
				{
					if (ip_item_ptr->data(Qt::UserRole).toString() == device_ptr->getIp())
					{
						auto state_item_ptr = connected_model_ptr_->item(i, (int)ConnectedDeviceTreeColumn::kConnectedDeviceState);
						if (state_item_ptr)
						{
							state_item_ptr->setText(device_ptr->getStateStr());
						}
						break;
					}
				}
			}
		}
	}
}

void DeviceListWidget::slotDevErrorOccurred(quint64 err_code)
{
	ErrorCodeUtils::handle(this, err_code);
}

void DeviceListWidget::slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
    //刷新IP 授权 编辑按钮状态
	bool selected = false;
	if (current.isValid())
	{
		selected = true;
	}
	//ui->pushButtonAlterIp->setEnabled(selected);//TODO:相机暂不支持IP更改,暂时禁用按钮
	ui->pushButtonDeviceLicense->setEnabled(selected);
	ui->pushButtonEditDeviceName->setEnabled(selected);
}

void DeviceListWidget::slotDeviceTreeItemDropped(QModelIndex parent_index)
{
	//刷新级联

	//先获取已连接的相机
	if (connected_model_ptr_->rowCount()!=2)
	{
		return;
	}
	QSharedPointer<Device> dev_ptr_1;
	QSharedPointer<Device> dev_ptr_2;
	QString dev_ip_1 = connected_model_ptr_->item(0, 2)->text();
	QString dev_ip_2 = connected_model_ptr_->item(1, 2)->text();
	dev_ptr_1 = dev_manager_ptr_->getDevice(dev_ip_1);
	dev_ptr_2 = dev_manager_ptr_->getDevice(dev_ip_2);

	//没有成功获取两个已连接相机
	if (dev_ptr_1.isNull()||dev_ptr_2.isNull())
	{
		return;
	}

 	QModelIndex child_index = parent_index.child(0,0);
	if (!parent_index.isValid()||!child_index.isValid())//没有级联关系,置空两个parent
	{
		dev_ptr_1->setParent(QSharedPointer<Device>());
		dev_ptr_2->setParent(QSharedPointer<Device>());
		return;
	}

	//有级联关系 获取两个index的相机ip
 	QString parent_ip = connected_model_ptr_->item(parent_index.row(),2)->text();
// 	QString child_ip = connected_model_ptr_->item(child_index.row(), 2)->text();

	//验证ip是否一致
	QSharedPointer<Device> parent_dev_ptr;
	QSharedPointer<Device> child_dev_ptr;
	if (dev_ptr_1->getIp() == parent_ip /*&& dev_ptr_2->getIp() == child_ip*/)
	{
		parent_dev_ptr = dev_ptr_1;
		child_dev_ptr = dev_ptr_2;
	}
	else if (dev_ptr_2->getIp() == parent_ip /*&& dev_ptr_1->getIp() == child_ip*/)
	{
		parent_dev_ptr = dev_ptr_2;
		child_dev_ptr = dev_ptr_1;
	}
	else
	{
		//ip不一致,异常情况
		return;
	}

	//判断相机型号是否一致
#if 1
	if (parent_dev_ptr->getModelName() != child_dev_ptr->getModelName())
	{
		//不一致 取消级联并提示 
		UIUtils::showInfoMsgBox(this, QString(tr("The models of devices are different, link failed.")));

		auto row_0 = connected_model_ptr_->takeRow(0);
		auto row_1 = connected_model_ptr_->takeRow(0);
		connected_model_ptr_->appendRow(row_0);
		connected_model_ptr_->appendRow(row_1);
		connected_model_ptr_->item(0)->takeRow(0);
		connected_model_ptr_->item(1)->takeRow(0);
		return;
	}
#endif

	//符合级联条件
	child_dev_ptr->setParent(parent_dev_ptr);
}

void DeviceListWidget::slotDeviceConnected(const QString & ip)
{
	if (bauto_connecting_devices_)
	{
		autoConnectDevicesResult(ip, true);
	}

	//连接后弹窗提示授权信息
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (device_ptr)
	{
		DeviceLicense license;
		device_ptr->getLicense(license);
		licenseMsgBox(license);
	}
}

void DeviceListWidget::slotDeviceDisonnected(const QString & ip)
{
	if (bauto_connecting_devices_)
	{
		autoConnectDevicesResult(ip, false);
	}
	                                                                     
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (device_ptr)
	{
		if (device_ptr->getState() == DeviceState::Unconnected)
		{
			for (int i = 0, count = connected_model_ptr_->rowCount(); i < count; ++i)
			{
				static const int state_col = 5;
				auto index = connected_model_ptr_->index(i, state_col);
				if (index.isValid())
				{
					const QString ip = index.data(Qt::UserRole).toString();
					if (ip == device_ptr->getIp())
					{
						DisconnectDeviceByIndex(index);
						break;
					}
				}
			}
		}
	}
}

void DeviceListWidget::on_pushButtonSearch_clicked()
{
	//开始搜索进度对话框
	DeviceSearchWidget search_widget;
	slotSearchFinished(search_widget.startSearchDevices());
}

void DeviceListWidget::onConnectButtonClicked(const QModelIndex &index)
{
	if (connected_model_ptr_->rowCount() >= kConnectedDevicesCountMax)
	{
		QString msg = QString(tr("Up to %1 device(s) can be connected!")).arg(kConnectedDevicesCountMax);
		UIUtils::showInfoMsgBox(this, msg);
		return;
	}

	QSharedPointer<Device> device_ptr = dev_manager_ptr_->getDevice(index.data(Qt::UserRole).toString());
	if (device_ptr)
	{
		auto connect_type = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
		connect(device_ptr.data(), &Device::deviceConnected, this, &DeviceListWidget::slotDeviceConnected, connect_type);
		connect(device_ptr.data(), &Device::deviceDisconnected, this, &DeviceListWidget::slotDeviceDisonnected, connect_type);
		connect(device_ptr.data(), &Device::stateChanged, this, &DeviceListWidget::slotDevStateChanged,connect_type);
		connect(device_ptr.data(), &Device::errorOccurred, this, &DeviceListWidget::slotDevErrorOccurred,connect_type);
		device_ptr->connect();
		//纳入已添加设备列表
		dev_manager_ptr_->appendAddedDevice(device_ptr);
	}
}

QString DeviceListWidget::getLicenseDesc(DeviceLicense license)
{
	QString str;

	switch (license.state)
	{
	case 0://永久授权
		str = QString(tr("Permanent Auth"));
		break;
	case 1://试用授权
		str = QString(tr("Auth to %1")).arg(QDateTime::currentDateTime().addSecs(license.remainder).toString("yyyy-M-d H:m"));
		break;
	case 2://未授权
		str = QString(tr("No Authorization"));
		break;
	case 3://授权到期
		str = QString(tr("Auth Expired"));
		break;
	default://授权错误
		str = QString(tr("Auth Error"));
		break;
	}
	
	return str;
}

void DeviceListWidget::licenseMsgBox(const DeviceLicense & license)
{
	switch (license.state)
	{
	case 1://试用授权
		if (license.remainder < secs_of_day_ * trial_license_tip_days_)
		{
			UIUtils::showWarnMsgBox(this, QString(tr("The authorization will soon expire. Please authorize in time to make sure the divice works correctly.")));
		}
		break;
	case 3://授权到期
		UIUtils::showErrorMsgBox(this, QString(tr("The authorization has expired, please use after authorization.")));
		break;
	default://授权错误
		break;
	}
}

void DeviceListWidget::autoConnectDevices()
{
	auto auto_connect_devices = SystemSettingsManager::instance().getAutoConnectedDevice();
	if (auto_connect_devices.isEmpty())
	{
		return;
	}
	auto_connecting_devices_state_.clear();
	bauto_connecting_devices_ = true;

	for (const auto e : auto_connect_devices)
	{
		for (int i = 0, count = searched_model_ptr_->rowCount(); i < count; ++i)
		{
			static const int state_col = 5;
			auto index = searched_model_ptr_->index(i, state_col);
			if (index.isValid())
			{
				const QString ip = index.data(Qt::UserRole).toString();
				if (ip == e)
				{
					auto device_ptr = DeviceManager::instance().getDevice(ip);
					if (device_ptr)
					{
						onConnectButtonClicked(index);
					}
					break;
				}
			}
		}
	}
}

void DeviceListWidget::autoConnectDevicesResult(const QString & ip, bool bconnected)
{
	if (bauto_connecting_devices_)
	{
		auto_connecting_devices_state_[ip] = bconnected;

		//TODO:判断是否自动连接成功
		auto auto_connect_devices = SystemSettingsManager::instance().getAutoConnectedDevice();
		if (auto_connect_devices.size() == auto_connecting_devices_state_.size())
		{
			bool bsend_signal = true;
			for (auto e : auto_connecting_devices_state_)
			{
				if (!e)
				{
					bsend_signal = false;
					break;
				}
			}
			if (bsend_signal)
			{
				emit sigAutoConnectDeviceSuccess();
			}
			bauto_connecting_devices_ = false;
		}
	}
}

void DeviceListWidget::updateDeviceList()
{
	updateSearchedDeviceList();
	updateConnectedDeviceTree();
}

void DeviceListWidget::onDisconnectButtonClicked(const QModelIndex &index)
{
	//弹窗提示
	QString msg = QString("%1\r\n%2")
		.arg(tr("Disconnect?"))
		.arg(tr("After clicking OK, the system will disconnect this device."));
	if (UIUtils::showQuestionMsgBox(this, msg))
	{
		DisconnectDeviceByIndex(index);
	}
}

void DeviceListWidget::on_pushButtonAlterIp_clicked()
{
	QSharedPointer<Device> cur_dev_ptr = getCurrentDevicePtr();

	if (!cur_dev_ptr.isNull())
	{
		DeviceAlterIp dlg(getCurrentDevicePtr(), this);
		if (dlg.exec())
	    {
			updateConnectedDeviceTree();
	    }		
	}	
}

void DeviceListWidget::on_pushButtonDeviceLicense_clicked()
{
	QSharedPointer<Device> cur_dev_ptr = getCurrentDevicePtr();
	if (cur_dev_ptr.isNull())
	{
		return;
	}
	QString default_dir_path ;
	do 
	{
		//选择授权文件
		QString license_file_path = QFileDialog::getOpenFileName(this, tr("Select license file"),default_dir_path,tr("DAT files (*.dat)"));
		if (license_file_path.isEmpty())
		{
			break;
		}

		//访问权限
		QFileInfo file_info(license_file_path);
		QFile::Permissions flags = file_info.permissions();
		bool no_access = false;
		if (!file_info.isReadable() || !file_info.isWritable())
		{
			no_access = true;
		}
		else
		{

		}

		if (no_access)//无权限，重新选择
		{
			UIUtils::showInfoMsgBox(this, tr("No Access!"));
			default_dir_path = license_file_path;
			continue;
		}

		//尝试授权
		cur_dev_ptr->setLicensePath(license_file_path);

		break;

	} while (true);
}

void DeviceListWidget::on_pushButtonEditDeviceName_clicked()
{
	QSharedPointer<Device> cur_dev_ptr = getCurrentDevicePtr();

	if (!cur_dev_ptr.isNull())
	{
		DeviceEditName dlg(getCurrentDevicePtr(), this);
		if (dlg.exec())
		{
			updateConnectedDeviceTree();
		}
    }
}

void DeviceListWidget::initUi()
{
    initSearchedUi();
    initConnectedUi();
}

void DeviceListWidget::initSearchedUi()
{
    // 设置模型
    searched_model_ptr_ = new QStandardItemModel();
    ui->tableViewSearched->setModel(searched_model_ptr_);
	QScroller::grabGesture(ui->tableViewSearched, QScroller::TouchGesture);

    // 水平表头设置
    QStringList labels;
    labels << tr("ID") << tr("Device Name") << tr("Model Name") << tr("IP/SN") << tr("State") << tr("Action");
    searched_model_ptr_->setHorizontalHeaderLabels(labels);

    // 垂直表头隐藏
    ui->tableViewSearched->verticalHeader()->setVisible(false);

    // 列布局
    ui->tableViewSearched->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 操作列：链接按钮样式
    AgLinkButtonItemDelegate* connect_item_delegate_ptr = new AgLinkButtonItemDelegate(this);
    const int kActionColumn = labels.size() - 1;
    ui->tableViewSearched->setItemDelegateForColumn(kActionColumn, connect_item_delegate_ptr);

    // 连接按钮单击关联
    connect(connect_item_delegate_ptr, &AgLinkButtonItemDelegate::linkButtonClicked, this, &DeviceListWidget::onConnectButtonClicked);

    
    dev_manager_ptr_ = &DeviceManager::instance();
	//connect(dev_manager_ptr_.data(), &DeviceManager::searchFinished, this,  & DeviceListWidget::slotSearchFinished);
}

void DeviceListWidget::initConnectedUi()
{
    // 设置模型
    connected_model_ptr_ = new QStandardItemModel();
    ui->treeViewConnected->setModel(connected_model_ptr_);

    // 设置表头
    QStringList labels;
    labels << tr("Device Name") << tr("Model Name") << tr("IP/SN") << tr("License") << tr("State") << tr("Action");
    connected_model_ptr_->setHorizontalHeaderLabels(labels);
    ui->treeViewConnected->header()->setDefaultAlignment(Qt::AlignCenter);

    // 列布局
    ui->treeViewConnected->header()->setSectionResizeMode(QHeaderView::Stretch);

    // 操作列：链接按钮样式
    AgLinkButtonItemDelegate* disconnect_item_delegate_ptr = new AgLinkButtonItemDelegate(this);
    const int kActionColumn = labels.size() - 1;
    ui->treeViewConnected->setItemDelegateForColumn(kActionColumn, disconnect_item_delegate_ptr);

	// 更改IP、相机授权、编辑相机名初始禁用
	ui->pushButtonAlterIp->setEnabled(false);
	ui->pushButtonDeviceLicense->setEnabled(false);
	ui->pushButtonEditDeviceName->setEnabled(false);

    // 断开按钮单击关联
    connect(disconnect_item_delegate_ptr, &AgLinkButtonItemDelegate::linkButtonClicked, this, &DeviceListWidget::onDisconnectButtonClicked);

    // 选择切换关联
    connect(ui->treeViewConnected->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &DeviceListWidget::slotCurrentSelectChanged);

	// 拖放事件关联
	connect(ui->treeViewConnected, &DeviceTreeView::sigItemDropped, this, &DeviceListWidget::slotDeviceTreeItemDropped);
}

QSharedPointer<Device> DeviceListWidget::getCurrentDevicePtr()
{
	QModelIndex cur_idx = ui->treeViewConnected->selectionModel()->currentIndex();
	if (cur_idx.isValid())
	{
		QString cur_ip = connected_model_ptr_->item(cur_idx.row(), 2)->text();
		if (!cur_ip.isEmpty())
		{
			return dev_manager_ptr_->getDevice(cur_ip);
		}
	}

	return QSharedPointer<Device>();
}
