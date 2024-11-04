#include "csdevicelistwidget.h"
#include "ui_csdevicelistwidget.h"
#include "Main/rccapp/csrccapp.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "Device/csdlgcameraproperty.h"
#include "Device/csdlgtriggerproperty.h"
#include "Device/csdlgtriggerpulsecontrol.h"
#include "System/DeviceList/devicesearchwidget.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/ErrorCodeUtils/errorcodeutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/AgErrorCode/agerrorcode.h"
#include "Common/LogUtils/logutils.h"
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QMessageBox>
#include "Device/csdlgprogressformat.h"

CSDeviceListWidget::CSDeviceListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSDeviceListWidget)
{
    ui->setupUi(this);
	qRegisterMetaType<DeviceState>();//QueuedConnection时需要注册
	qRegisterMetaType<Device::PropType>();
 	qRegisterMetaType<QVariant>();
	m_device_magager_ptr  = &DeviceManager::instance();
	m_external_sync_timer = new QTimer();
	if (!m_external_sync_timer.isNull())
	{
		m_external_sync_timer->setInterval(250);
		QObject::connect(m_external_sync_timer.data(), &QTimer::timeout, this, &CSDeviceListWidget::UpdateDeviceIcon);
		m_external_sync_timer->start();
	}
	InitUi();
	initMenus();
	bind();
}

CSDeviceListWidget::~CSDeviceListWidget()
{
	if (m_external_sync_timer != nullptr)
	{
		delete m_external_sync_timer;
		m_external_sync_timer = nullptr;
	}
    delete ui;
}

void CSDeviceListWidget::InitUi()
{
	//设置表头
	QStringList header_lables;
	header_lables << tr("Device Name") << tr("Model") << tr("IP/SN")  << tr("State");

	ui->deviceTreeView->setHeaderLabels(header_lables);
	ui->deviceTreeView->setColumnCount(4);
	ui->deviceTreeView->setSelectionBehavior(QTreeWidget::SelectRows);     // 选择行
	ui->deviceTreeView->setDragEnabled(true);              // 启用拖动
	ui->deviceTreeView->viewport()->setAcceptDrops(true); // viewport 接受放下动作，默认是复制操作
	ui->deviceTreeView->showDropIndicator();               // 设置放下指示
	ui->deviceTreeView->setDragDropMode(QTreeWidget::InternalMove);// 内部 移动

	ui->deviceTreeView->header()->setFrameShape(QFrame::StyledPanel);
	ui->deviceTreeView->header()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->deviceTreeView->header()->setDefaultAlignment(Qt::AlignCenter);

	//激活菜单关联
	ui->deviceTreeView->setContextMenuPolicy(Qt::CustomContextMenu);//自定义菜单
}

void CSDeviceListWidget::initMenus()
{
	//初始化相机右键菜单
	m_menu_device = new QMenu(ui->deviceTreeView);
	m_action_connect = new QAction(tr("Connect"));
	m_action_disconnect = new QAction(tr("Disconnect"));
	m_action_copy = new QAction(tr("Copy Parameters"));
	m_action_paste = new QAction(tr("Paste Parameters"));
	//m_action_device_info = new QAction(tr("Device Properties"));
	m_action_device_info = new QAction(tr("About Device"));
	m_action_connect_all = new QAction(tr("Connect All"));
	m_action_disconnect_all = new QAction(tr("Disconnect All"));
	m_action_preview_all = new QAction(tr("Preview All"));
	m_action_acquire_all = new QAction(tr("Acquire All"));
	m_action_stop_all = new QAction(tr("Stop All"));
	m_action_pulse_control = new QAction(tr("Pulse Control"));
	m_action_video_mosaic_export = new QAction(tr("Video Mosaic Export"));
	m_action_cf18_info = new QAction(tr("About Synchronous Controller"));
	m_action_delete_current_node = new QAction(tr("Delete"));

	if (!m_menu_device) return;
	if (!m_action_connect) return;
	if (!m_action_disconnect) return;
	if (!m_action_copy) return;
	if (!m_action_paste) return;
	if (!m_action_device_info) return;
	if (!m_action_connect_all) return;
	if (!m_action_disconnect_all) return;
	if (!m_action_preview_all) return;
	if (!m_action_acquire_all) return;
	if (!m_action_stop_all) return;
	if (!m_action_pulse_control) return;
	if (!m_action_video_mosaic_export) return;
	if (!m_action_cf18_info) return;
	if (!m_action_delete_current_node) return;

	//TODO: 禁用暂未实现的功能菜单
	m_action_connect_all->setEnabled(false);
	m_action_disconnect_all->setEnabled(false);
	m_action_acquire_all->setEnabled(false);
	m_action_preview_all->setEnabled(false);
	m_action_stop_all->setEnabled(false);
	m_action_video_mosaic_export->setEnabled(false);

	m_menu_device->addAction(m_action_connect);
	m_menu_device->addSeparator();
	m_menu_device->addAction(m_action_copy);
	m_menu_device->addAction(m_action_paste);
	m_menu_device->addSeparator();
	m_menu_device->addAction(m_action_delete_current_node);
	m_menu_device->addSeparator();
	m_menu_device->addAction(m_action_device_info);

	//刷新操作按钮
	updateCopyAndPasteCtrl();

	updateDeviceActionsCtrl();

	//连接操作槽函数
	connect(m_action_connect.data(), &QAction::triggered, this, &CSDeviceListWidget::slotConnectCurrentDevice);
	connect(m_action_disconnect.data(), &QAction::triggered, this, &CSDeviceListWidget::slotDisconnectCurrentDevice);
	connect(m_action_copy.data(), &QAction::triggered, this, &CSDeviceListWidget::slotToolButtonCopyParam);
	connect(m_action_paste.data(), &QAction::triggered, this, &CSDeviceListWidget::slotToolButtonPasteParam);
	connect(m_action_device_info.data(), &QAction::triggered, this, &CSDeviceListWidget::slotCurrentDeviceInfo);
	connect(m_action_pulse_control.data(), &QAction::triggered, this, &CSDeviceListWidget::slotCurrentTriggerPulseControl);
	connect(m_action_cf18_info.data(), &QAction::triggered, this, &CSDeviceListWidget::slotCurrentDeviceInfo);
	connect(m_action_delete_current_node.data(), &QAction::triggered, this, &CSDeviceListWidget::slotDeleteCurrentNode);
}


QMainWindow * CSDeviceListWidget::getMainWindow()
{
	for (QWidget* widget : qApp->topLevelWidgets())
	{
		if (QMainWindow * mainWin = qobject_cast<QMainWindow*>(widget))
		{
			return mainWin;
		}
	}
	return nullptr;
}

void CSDeviceListWidget::bind()
{
	//TODO:连接操作信号
	//切换选择关联
	bool ok = connect(ui->deviceTreeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CSDeviceListWidget::slotCurrentSelectChanged);
	ok = connect(ui->deviceTreeView, &QTreeWidget::itemPressed, this, &CSDeviceListWidget::slotTreeItemClicked);
	//设备切换关联
	ok = connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSDeviceListWidget::slotCurrentDeviceChanged);
	ok = connect(&DeviceManager::instance(), &DeviceManager::signalAddDevice, this, &CSDeviceListWidget::slotAddDevice);
	ok = connect(ui->refresh_btn, &QPushButton::clicked, this, [=] {
		autoConnectDevices();
		m_countdown_timer->start(1000);
		m_countdown = m_const_count_down;
		setRefreshBtnEnable(false);
		updateRefreshLabel(m_countdown,false);
		
	});
	ok = connect(ui->deviceTreeView, &CSDeviceTreeView::signalDragFinished, this, &CSDeviceListWidget::slotDragFinished, Qt::QueuedConnection);
	ok = connect(ui->deviceTreeView, &CSDeviceTreeView::itemClicked, this, &CSDeviceListWidget::slotItemClicked);
	ok = connect(ui->add_btn, &QPushButton::clicked, this, &CSDeviceListWidget::slotAddBtnClicked);
	ok = connect(ui->edit_btn, &QPushButton::clicked, this, &CSDeviceListWidget::slotEditBtnClicked);
	
	m_countdown_timer = new QTimer();
	ok = connect(m_countdown_timer, &QTimer::timeout, this, &CSDeviceListWidget::slotCountdownTimer);
	
}

void CSDeviceListWidget::autoConnectDevices()
{
	QList<QSharedPointer<Device>> device_list;
	DeviceManager::instance().getManageDevices(device_list);
	for (auto device_ptr : device_list)
	{
		if (device_ptr.isNull())
		{
			continue;
		}
		if (device_ptr->getState() != DeviceState::Unconnected && 
			device_ptr->getState() != DeviceState::Disconnected)
		{
			continue;
		}

		auto connect_type = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
		auto connect_type_auto = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);
		connect(device_ptr.data(), &Device::errorOccurred, this, &CSDeviceListWidget::slotDeviceErrorOccurred, connect_type);
		connect(device_ptr.data(), &Device::propertyChanged, this, &CSDeviceListWidget::slotDevicePropertyChanged, connect_type_auto);
		connect(device_ptr.data(), &Device::stateChanged, this, &CSDeviceListWidget::slotDeviceStateChangedUpdateRotate, connect_type);
		if (device_ptr->IsCF18()) {
			connect(device_ptr.data(), &Device::cf18StateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
		}
		else {
			connect(device_ptr.data(), &Device::stateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
		}
		connect(device_ptr.data(), &Device::orgStateIsAcquring, this, &CSDeviceListWidget::slotOrgStateIsAcquring, connect_type);
		m_device_magager_ptr->appendAddedDevice(device_ptr);

		device_ptr->setShowTip(false);
		if (device_ptr->IsCF18()) 
		{
			device_ptr->connect();
		}
		else
		{
			device_ptr->preview();
		}
	}
}

void CSDeviceListWidget::updateMenus(DeviceMenuType menu_type)
{
	if (!m_menu_device)
	{
		return;
	}
	//根据菜单类型刷新菜单选项,
	switch (menu_type)
	{
	case CSDeviceListWidget::kMenuTypeCameraConnect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_connect);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_copy);
		m_menu_device->addAction(m_action_paste);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_device_info);

		break;
	case CSDeviceListWidget::kMenuTypeCameraDisconnect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_disconnect);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_copy);
		m_menu_device->addAction(m_action_paste);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_device_info);
		break;
	case CSDeviceListWidget::kMenuTypeTriggerConnect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_connect);
		m_menu_device->addAction(m_action_connect_all);
		m_menu_device->addAction(m_action_disconnect_all);
		m_menu_device->addAction(m_action_preview_all);
		m_menu_device->addAction(m_action_acquire_all);
		m_menu_device->addAction(m_action_stop_all);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_pulse_control);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_device_info);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_video_mosaic_export);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);
		break;
	case CSDeviceListWidget::kMenuTypeTriggerDisconnect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_disconnect);
		m_menu_device->addAction(m_action_connect_all);
		m_menu_device->addAction(m_action_disconnect_all);
		m_menu_device->addAction(m_action_preview_all);
		m_menu_device->addAction(m_action_acquire_all);
		m_menu_device->addAction(m_action_stop_all);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_pulse_control);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_device_info);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_video_mosaic_export);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);

		break;
	case CSDeviceListWidget::kMenuTypeCF18Connect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_connect);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_cf18_info);
		break;
	case CSDeviceListWidget::kMenuTypeCF18Disconnect:
		m_menu_device->clear();
		m_menu_device->addAction(m_action_disconnect);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_delete_current_node);
		m_menu_device->addSeparator();
		m_menu_device->addAction(m_action_cf18_info);
		break;
	case kMenuTypeGroup: {
		m_menu_device->clear();
		m_menu_device->addAction(m_action_delete_current_node);
		break;
	}
	default:
		break;
	}
}

void CSDeviceListWidget::updateCopyAndPasteCtrl()
{
	if (!m_action_copy)return;
	if (!m_action_paste)return;
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())//无设备直接禁用操作
	{
		m_action_copy->setEnabled(false);
		m_action_paste->setEnabled(false);
		return;
	}
	if (device_ptr->IsCF18()) {
		return;
	}
	//更新拷贝操作使能
	if (device_ptr->AllowsSetCameraSettings())//当前状态是否支持设备拷贝
	{
		m_action_copy->setEnabled(true);
	}
	else
	{
		m_action_copy->setEnabled(false);
	}
	

	//更新粘贴操作使能
	//同一个设备,设备型号不同,设备不允许设置参数时都不允许执行粘贴操作
	if (m_device_to_copy.isNull()||
		m_device_to_copy == device_ptr||
		m_device_to_copy->getModelName() != device_ptr->getModelName()||
		!m_device_to_copy->AllowsSetCameraSettings()||
		!device_ptr->AllowsSetCameraSettings())
	{
		m_action_paste->setEnabled(false);
	}
	else
	{
		m_action_paste->setEnabled(true);
	}
}

void CSDeviceListWidget::updateDeviceActionsCtrl()
{
	if (!m_action_disconnect) return;
	if (!m_action_device_info) return;
	if (!m_action_pulse_control) return;
	if (!m_action_cf18_info) return;
	if (!m_action_delete_current_node) return;

	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())//无设备直接禁用操作
	{
		m_action_device_info->setEnabled(false);
		m_action_disconnect->setEnabled(false);
		m_action_pulse_control->setEnabled(false);
		m_action_delete_current_node->setDisabled(false);
		return;
	}

	if (device_ptr->IsCF18()) {
		if (device_ptr->AllowsShowDeviceProperties())//当前状态是否支持
		{
			m_action_cf18_info->setEnabled(true);
		}
		else
		{
			m_action_cf18_info->setEnabled(false);
		}
		m_action_disconnect->setEnabled(device_ptr->allowsDisconnect());
		return;
	}
	//更新操作使能
	if (device_ptr->AllowsShowDeviceProperties())//当前状态是否支持
	{
		m_action_device_info->setEnabled(true);
	}
	else
	{
		m_action_device_info->setEnabled(false);
	}


	if (device_ptr->IsRootTrigger() && device_ptr->getState() == Connected)//TODO:级联支持
	{
		m_action_pulse_control->setEnabled(true);
	}
	else
	{
		m_action_pulse_control->setEnabled(false);
	}

	//更新断连操作使能
	m_action_disconnect->setEnabled(device_ptr->allowsDisconnect());
	m_action_delete_current_node->setDisabled(Acquiring == device_ptr->getState());
}

QString CSDeviceListWidget::getCurrentDeviceIP()
{
	QTreeWidgetItem* current_item = ui->deviceTreeView->currentItem();
	QString current_ip = current_item->text(2);
	return current_ip;
}

void CSDeviceListWidget::UpdateDeviceIcon()
{   //获取所有的设备
	QList<QSharedPointer<Device>> device_list;
	m_device_magager_ptr->getManageDevices(device_list);
	for (auto device : device_list)
	{
		int icon_index = device->GetIconIndex();

		QIcon icon;
		switch (icon_index)
		{
		case kIUnconnectedCamera: 
			switch (getDeviceType(device->getModel()))
			{
			case Syn_Controller: {
				icon = QIcon(":/images/syn_normal.png");
				break;
			}
			case M_Series: {
				icon = QIcon(":/images/MX_normal.png");
				break;
			}
			case GR_Series: {
				icon = QIcon(":/images/GR_normal.png");
				break;
			}
			default:
				break;
			}
			break;
		case kIAbnormalCamera:
			switch (getDeviceType(device->getModel()))
			{
			case Syn_Controller: {
				icon = QIcon(":/images/syn_red.png");
				break;
			}
			case M_Series: {
				icon = QIcon(":/images/MX_red.png");
				break;
			}
			case GR_Series: {
				icon = QIcon(":/images/GR_red.png");
				break;
			}
			default:
				break;
			}
			break;
		case kINormalCamera:
			switch (getDeviceType(device->getModel()))
			{
			case Syn_Controller: {
				icon = QIcon(":/images/syn_green.png");
				break;
			}
			case M_Series: {
				icon = QIcon(":/images/MX_green.png");
				break;
			}
			case GR_Series: {
				icon = QIcon(":/images/GR_green.png");
				break;
			}
			default:
				break;
			}
			break;
		case kIAbnormalExternalSyncCamera:
			switch (getDeviceType(device->getModel()))
			{
			case Syn_Controller: {
				icon = QIcon(":/images/syn_blue.png");
				break;
			}
			case M_Series: {
				icon = QIcon(":/images/MX_blue.png");
				break;
			}
			case GR_Series: {
				icon = QIcon(":/images/GR_blue.png");
				break;
			}
			default:
				break;
			}
			break;
		default:
			switch (getDeviceType(device->getModel()))
			{
			case Syn_Controller: {
				icon = QIcon(":/images/syn_normal.png");
				break;
			}
			case M_Series: {
				icon = QIcon(":/images/MX_normal.png");
				break;
			}
			case GR_Series: {
				icon = QIcon(":/images/GR_normal.png");
				break;
			}
			default:
				break;
			}
			break;
		}

		QString device_ip = device->getIpOrSn();
		QTreeWidgetItemIterator node(ui->deviceTreeView);
		while (*node)
		{
			int conlum = (*node)->columnCount();
			QString node_ip = (*node)->text(2);
			if (device_ip == node_ip) {
				(*node)->setIcon(0, icon);
			}
			++node;
		}
	}
}

void CSDeviceListWidget::UpdateDeviceModeName()
{
	QList<QSharedPointer<Device>> device_list;
	m_device_magager_ptr->getManageDevices(device_list);
	for (auto device : device_list)
	{
		//if (device->getState() == DeviceState::Unconnected)
		{
			QString device_ip = device->getIpOrSn();
			QTreeWidgetItemIterator node(ui->deviceTreeView);
			while (*node)
			{
				int conlum = (*node)->columnCount();
				QString node_ip = (*node)->text(2);
				if (device_ip == node_ip) {
					QString node_text = (*node)->text(1);
					if (device->getModelName() != node_text)
					{
						QString strModelTypeName = device->getModelName();
						if (strModelTypeName == m_const_unknown)
						{
							TreeWidgetItemInfo item_info{};
							item_info = getTreeWidgetItemInfo(device_ip);
							strModelTypeName = item_info.device_model_type;
							if (strModelTypeName == m_const_unknown)
							{
								strModelTypeName = "";
							}
						}
						(*node)->setText(1, strModelTypeName);
						(*node)->setToolTip(1, strModelTypeName);
					}
					break;
				}
				++node;
			}
		}
	}
}

void CSDeviceListWidget::addTreeWidgetItem(CSDeviceTreeView* parent)
{

}

void CSDeviceListWidget::renameGroup(QString& groupname)
{
	QString groupname_temp = groupname;
	int i = 1;
	while (m_group_list.contains(groupname)) {
		groupname = groupname_temp + QString::number(i++);
	}
	m_group_list.append(groupname);
}

bool CSDeviceListWidget::loadJson(const QString &filepath, bool init) {
	if (filepath.isEmpty())
		return false;
	QFile file(filepath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	const QByteArray raw_data = file.readAll();
	file.close();

	QJsonParseError json_error;
	m_json_doc = QJsonDocument::fromJson(raw_data, &json_error);
	bool z = (json_error.error != QJsonParseError::NoError);
	if (m_json_doc.isNull() || m_json_doc.isEmpty() || json_error.error != QJsonParseError::NoError)
		return false;

	paraseJson(m_json_doc,init);
	return true;
}

void CSDeviceListWidget::paraseJson(const QJsonDocument& json_doc, bool init)
{
	if (json_doc.isObject()) {
		QJsonObject json_doc_object = json_doc.object();
		QJsonObject::const_iterator iter;
		for (iter = json_doc_object.constBegin(); iter != json_doc_object.constEnd(); ++iter) {
			if (json_doc_object.isEmpty()) {
				break;
			}

			if(init){
				if (iter->isObject()) {
					QJsonObject obj = iter->toObject();
					initRecurseParaseJsonObj(obj);
				}
			}
			else {
				QTreeWidgetItem* item = new QTreeWidgetItem(ui->deviceTreeView);
				const QJsonValue iter_val = iter.value();
				switch (iter_val.type()) {
				case QJsonValue::Object: {
					recurseParaseJsonObj(iter_val.toObject(), item);
					break;
				}
				case QJsonValue::Null: break;
				case QJsonValue::Undefined: break;
				default: break;
				}
			}
			
		}
	}
}

void CSDeviceListWidget::recurseParaseJsonObj(const QJsonObject& json_obj, QTreeWidgetItem* item)
{
	if (json_obj.contains(m_const_node_type) && json_obj[m_const_node_type].isDouble()) {
		switch (NodeType(json_obj[m_const_node_type].toInt()))
		{
		case CSDeviceListWidget::nGroup: {
			if (json_obj.contains(m_const_node_name) && json_obj[m_const_node_name].isString()) {
				QString group_name = json_obj[m_const_node_name].toString();
				item->setFlags(item->flags());
				item->setText(0, group_name);
				item->setToolTip(0, group_name);
				item->setExpanded(true);
			}
			if (json_obj.contains(m_const_child_node) && json_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = json_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject iter_obj = iter->toObject();
							QTreeWidgetItem* child_item = new QTreeWidgetItem(item);
							recurseParaseJsonObj(iter_obj, child_item);
						}
					}
				}
			}
			break;
		}
		case CSDeviceListWidget::nSynController: 
		case CSDeviceListWidget::nDevice: {
			if (json_obj.contains(m_const_node_name) && json_obj[m_const_node_name].isString()) {
				QString device_ip = json_obj[m_const_node_name].toString();
				TreeWidgetItemInfo item_info{};
				item_info = getTreeWidgetItemInfo(device_ip);
				if (item_info.p_device) {
					item->setFlags(item->flags());
					item->setText(0, item_info.p_device->getProperty(Device::PropName).toString());
					item->setToolTip(0, item_info.p_device->getProperty(Device::PropName).toString());
					item->setTextAlignment(0, Qt::AlignCenter);
					QString strModelTypeName = item_info.p_device->getModelName();
					if (strModelTypeName == m_const_unknown)
					{
						strModelTypeName = item_info.device_model_type;
						if (strModelTypeName == m_const_unknown)
						{
							strModelTypeName = "";
						}
					}
					item->setText(1, strModelTypeName);
					item->setToolTip(1, strModelTypeName);
					item->setTextAlignment(1, Qt::AlignCenter);
					item->setText(2, item_info.p_device->getIpOrSn());
					item->setToolTip(2, item_info.p_device->getIpOrSn());
					item->setTextAlignment(2, Qt::AlignCenter);
					item->setText(3, item_info.p_device->getStateStr());
					item->setToolTip(3, item_info.p_device->getStateStr());
					item->setTextAlignment(3, Qt::AlignCenter);
					m_device_tree_item_list.append(item);
				}
			}
			if (json_obj.contains(m_const_child_node) && json_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = json_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject iter_obj = iter->toObject();
							QTreeWidgetItem* child_item = new QTreeWidgetItem(item);
							recurseParaseJsonObj(iter_obj, child_item);
						}
					}
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void CSDeviceListWidget::initRecurseParaseJsonObj(const QJsonObject& json_obj)
{
	if (json_obj.contains(m_const_node_type) && json_obj[m_const_node_type].isDouble()) {
		switch (NodeType(json_obj[m_const_node_type].toInt()))
		{
		case CSDeviceListWidget::nGroup: {
			if (json_obj.contains(m_const_child_node) && json_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = json_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject iter_obj = iter->toObject();
							initRecurseParaseJsonObj(iter_obj);
						}
					}
				}
			}
			break;
		}
		case CSDeviceListWidget::nSynController:
		case CSDeviceListWidget::nDevice: {
			QString device_ip{};
			DeviceModel dm{ DEVICE_UNKNOWN };
			QString device_model_type{};

			if (json_obj.contains(m_const_node_name) && json_obj[m_const_node_name].isString()) {
				device_ip = json_obj[m_const_node_name].toString();
			}

			if (json_obj.contains(m_const_model_info) && json_obj[m_const_model_info].isDouble()) {
				dm = DeviceModel(json_obj[m_const_model_info].toInt());
			}

			if (json_obj.contains(m_const_model_type_info) && json_obj[m_const_model_type_info].isString()) {
				device_model_type = json_obj[m_const_model_type_info].toString();
			}

			TreeWidgetItemInfo  iteminfo{};
			if (CSDeviceListWidget::nSynController == NodeType(json_obj[m_const_node_type].toInt())) {
				iteminfo.item_type = TreeWidgetItemType::SYN_CONTROLLER;
			}
			else {
				iteminfo.item_type = TreeWidgetItemType::DEVICE;
			}
			
			if (DeviceManager::instance().getDevice(device_ip)) {
				iteminfo.p_device = DeviceManager::instance().getDevice(device_ip);
			}
			else {
				iteminfo.p_device = DeviceManager::instance().getNewDevice(device_ip, dm);
				if (!device_model_type.isEmpty() && device_model_type.compare(m_const_unknown, Qt::CaseInsensitive) != 0)
				{
					iteminfo.p_device->setModelName(device_model_type);
					iteminfo.device_model_type = device_model_type;
				}
			}
			DeviceManager::instance().appendManageItem(iteminfo);


			if (json_obj.contains(m_const_child_node) && json_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = json_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject iter_obj = iter->toObject();
							initRecurseParaseJsonObj(iter_obj);
						}
					}
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void CSDeviceListWidget::recurseTreeWidgetNode(QTreeWidgetItem* item, QJsonObject& obj)
{
	if (!item)
	{
		return;
	}
	if (0 < item->childCount()) {
		QJsonObject child_obj{};
		for (int i = 0; i < item->childCount(); ++i) {
			QTreeWidgetItem* child_item = item->child(i);
			if (child_item)
			{
				child_item->setExpanded(true);
			}
			QJsonObject json_obj;
			recurseTreeWidgetNode(child_item, json_obj);
			child_obj.insert(getNodeKeyName(), json_obj);
			
		}
		if (item) {
			if (1 < item->columnCount()) {//to do:mpp 设备
				obj.insert(m_const_node_name, item->text(2));
				if (m_device_magager_ptr) {
					auto device_ = m_device_magager_ptr->getDevice(item->text(2));
					if (device_) {
						if (DeviceModel::DEVICE_TRIGGER_CF18 == device_->getModel()) {
							obj.insert(m_const_node_type, nSynController);
						}
						else {
							obj.insert(m_const_node_type, nDevice);
						}
						obj.insert(m_const_model_type_info, device_->getModelName());
						obj.insert(m_const_model_info, (int)device_->getModel());
					}
				}
			}
			else {
				obj.insert(m_const_node_name, item->text(0));
				obj.insert(m_const_node_type, nGroup);
			}
			obj.insert(m_const_child_node, child_obj);
		}
	}
	else {
		if (1 < item->columnCount()) {//to do:mpp 设备
			obj.insert(m_const_node_name, item->text(2));
			if (m_device_magager_ptr) {
				auto device_ = m_device_magager_ptr->getDevice(item->text(2));
				if (device_) {
					if (DeviceModel::DEVICE_TRIGGER_CF18 == device_->getModel()) {
						obj.insert(m_const_node_type, nSynController);
					}
					else {
						obj.insert(m_const_node_type, nDevice);
					}
				}
			}
		}
		else {
			obj.insert(m_const_node_name, item->text(0));
			obj.insert(m_const_node_type, nGroup);
		}
		obj.insert(m_const_child_node, QJsonObject{});
	}	
}

TreeWidgetItemInfo CSDeviceListWidget::getTreeWidgetItemInfo(const QString& ip)
{
	QList<TreeWidgetItemInfo> items(m_device_magager_ptr->getTreeWidgetItemInfo());
	for (auto item : items) {
		if (item.p_device && (ip == item.p_device->getIpOrSn())) {
			return item;
		}
	}
	return TreeWidgetItemInfo{};
}

QString CSDeviceListWidget::getNodeKeyName()
{
	return QString("key%1").arg(m_key_index++);
}

void CSDeviceListWidget::updateTreeWidgetInfo()
{
	QJsonObject root_obj;
	int top_level_item_count = ui->deviceTreeView->topLevelItemCount();
	for (int i = 0; i < top_level_item_count; ++i) {
		QTreeWidgetItem* tree_widget_item = ui->deviceTreeView->topLevelItem(i);
		QJsonObject item_obj;
		recurseTreeWidgetNode(tree_widget_item, item_obj);
		if (1 < tree_widget_item->columnCount()) {//to do:mpp 设备
			item_obj.insert(m_const_node_name, tree_widget_item->text(2));
			if (m_device_magager_ptr) {
				auto device_ptr = m_device_magager_ptr->getDevice(tree_widget_item->text(2));
				if (device_ptr) {
					if (DeviceModel::DEVICE_TRIGGER_CF18 == device_ptr->getModel()) {
						item_obj.insert(m_const_node_type, nSynController);
					}
					else {
						item_obj.insert(m_const_node_type, nDevice);
					}
					item_obj.insert(m_const_model_type_info, device_ptr->getModelName());
					item_obj.insert(m_const_model_info, (int)device_ptr->getModel());
				}
			}
		}
		else {
			item_obj.insert(m_const_node_name, tree_widget_item->text(0));
			item_obj.insert(m_const_node_type, nGroup);
		}
		root_obj.insert(getNodeKeyName(), item_obj);
	}

	m_json_doc.setObject(root_obj);
	QByteArray json_data = m_json_doc.toJson(QJsonDocument::Indented);
	QFile file(m_const_json_file);
	if (!file.open(QIODevice::WriteOnly))
		return;
	file.write(json_data);
	file.close();
}

bool CSDeviceListWidget::deleteJsonNode(QJsonObject& cur_obj, QJsonObject& par_obj, const QString& key, const QString& delete_name)
{
	bool bRet = false;
	if (cur_obj.contains(m_const_node_name) && cur_obj[m_const_node_name].isString()) {
		if (delete_name != cur_obj[m_const_node_name].toString()) {
			if (cur_obj.contains(m_const_child_node) && cur_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = cur_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject obj = iter->toObject();
							if (deleteJsonNode(obj, child_obj, iter.key(), delete_name)) {
								cur_obj.insert(m_const_child_node, child_obj);
								par_obj.insert(key, cur_obj);
								bRet = true;
								break;
							}
						}
					}
				}
				
			}
		}
		else {
			if (cur_obj.contains(m_const_child_node) && cur_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = cur_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							par_obj.insert(getNodeKeyName(), iter->toObject());
						}
					}
				}
			}
			par_obj.remove(key);
			bRet = true;
		}
	}
	return bRet;
}

bool CSDeviceListWidget::changeJsonNode(QJsonObject& cur_obj, const QString& org_name, const QString& change_name)
{
	bool bRet = false;
	if (cur_obj.contains(m_const_node_name) && cur_obj[m_const_node_name].isString()) {
		if (org_name != cur_obj[m_const_node_name].toString()) {
			if (cur_obj.contains(m_const_child_node) && cur_obj[m_const_child_node].isObject()) {
				QJsonObject child_obj = cur_obj[m_const_child_node].toObject();
				if (!child_obj.empty()) {
					QJsonObject::const_iterator iter;
					for (iter = child_obj.constBegin(); iter != child_obj.constEnd(); ++iter) {
						if (child_obj.isEmpty()) {
							break;
						}
						if (iter->isObject()) {
							QJsonObject obj = iter->toObject();
							QString obj_key = iter.key();
							if (changeJsonNode(obj, org_name, change_name)) {
								child_obj.insert(obj_key, obj);
								cur_obj.insert(m_const_child_node, child_obj);
								bRet = true;
								break;
							}
						}
					}
				}

			}
		}
		else {
			cur_obj[m_const_node_name] = change_name;
			bRet = true;
		}
	}
	return bRet;
}

void CSDeviceListWidget::calDeiveNumInSelectedGroup(QTreeWidgetItem* item)
{
	if (nullptr == item) {
		return;
	}
	if (0 < item->childCount()) {
		for (int i = 0; i < item->childCount(); ++i) {
			QTreeWidgetItem* child_item = item->child(i);
			calDeiveNumInSelectedGroup(child_item);
		}
		if (1 < item->columnCount()) {//to do:mpp 设备
			++m_deive_num_in_selected_group;
		}
	}
	else {
		if (1 < item->columnCount()) {//to do:mpp 设备
			++m_deive_num_in_selected_group;
		}
	}
}

void CSDeviceListWidget::changeGroupName(const QString& org_name, const QString& new_name)
{
	if (m_const_json_file.isEmpty())
		return;
	QFile file(m_const_json_file);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	const QByteArray raw_data = file.readAll();
	file.close();

	QJsonParseError json_error;
	m_json_doc = QJsonDocument::fromJson(raw_data, &json_error);
	bool z = (json_error.error != QJsonParseError::NoError);
	if (m_json_doc.isNull() || m_json_doc.isEmpty() || json_error.error != QJsonParseError::NoError)
		return;

	if (m_json_doc.isObject()) {
		QJsonObject json_doc_object = m_json_doc.object();
		QJsonObject::const_iterator iter;
		for (iter = json_doc_object.constBegin(); iter != json_doc_object.constEnd(); ++iter) {
			if (json_doc_object.isEmpty()) {
				break;
			}
			if (iter->isObject()) {
				QJsonObject obj = iter->toObject();
				QString obj_key = iter.key();
				if (changeJsonNode(obj, org_name, new_name)) {
					json_doc_object.insert(obj_key, obj);
					break;
				}
			}
		}

		m_json_doc.setObject(json_doc_object);
		QByteArray json_data = m_json_doc.toJson(QJsonDocument::Indented);
		QFile file(m_const_json_file);
		if (!file.open(QIODevice::WriteOnly))
			return;
		file.write(json_data);
		file.close();

		ui->deviceTreeView->clear();
		m_device_tree_item_list.clear();
		loadJson(m_const_json_file);
	}
}

void CSDeviceListWidget::updateTreeNodeVessel()
{
	updateGroupInfo();
	updateDeviceVessel();
}

void CSDeviceListWidget::updateGroupInfo()
{
	int top_level_item_count = ui->deviceTreeView->topLevelItemCount();
	for (int i = 0; i < top_level_item_count; ++i) {
		QTreeWidgetItem* tree_widget_item = ui->deviceTreeView->topLevelItem(i);
		updateGroupVessel(tree_widget_item);
	}
}

void CSDeviceListWidget::updateGroupVessel(QTreeWidgetItem* item)
{
	if (nullptr == item) {
		return;
	}
	if (0 < item->childCount()) {
		for (int i = 0; i < item->childCount(); ++i) {
			QTreeWidgetItem* child_item = item->child(i);
			updateGroupVessel(child_item);
		}
		if (1 == item->columnCount()) {//to do:mpp 组
			m_group_set.insert(item->text(0));
		}
	}
	else {
		if (1 == item->columnCount()) {//to do:mpp 组
			m_group_set.insert(item->text(0));
		}
	}
}

void CSDeviceListWidget::updateDeviceVessel()
{
	if (m_device_magager_ptr) {
		QList<QSharedPointer<Device>> devices_{};
		m_device_magager_ptr->getDevices(devices_);
		for (auto item : devices_) {
			if (m_const_cf18 == item->getModelName()) {
				m_syn_controller_set.insert(item->getIpOrSn());
			}
			else {
				m_all_ip_set.insert(item->getIpOrSn());
			}
		}

		QList<QSharedPointer<Device>> manage_devices_{};
		m_device_magager_ptr->getManageDevices(manage_devices_);
		for (auto item : manage_devices_) {
			if (m_const_cf18 == item->getModelName()) {
				m_syn_controller_set.insert(item->getIpOrSn());
			}
			else {
				m_manage_ip_set.insert(item->getIpOrSn());
				m_all_ip_set.insert(item->getIpOrSn());
			}
		}
	}
}

EditDeviceInfo::DeviceType CSDeviceListWidget::getCurrentDeviceType(QTreeWidgetItem* item)
{
	EditDeviceInfo::DeviceType dt = EditDeviceInfo::M_Series;
	if (item->columnCount() > 2)
	{
		QString strDeviceIP = item->text(2);
		if (!strDeviceIP.isEmpty())
		{
			auto device_ptr = m_device_magager_ptr->getDevice(strDeviceIP);
			if (device_ptr)
			{
				auto model = device_ptr->getModel();
				switch (model)
				{
				case DEVICE_TRIGGER:
				case DEVICE_TRIGGER_CF18:
				case DEVICE_TRIGGER_ASG8x00:
					dt = EditDeviceInfo::Syn_Controller;
					break;
				case DEVICE_GRABBER_100T:
				case DEVICE_GRABBER_220:
				case DEVICE_GRABBER_210:
				case DEVICE_GRABBER_120:
				case DEVICE_GRABBER_110:
				case DEVICE_GRABBER_2102:
				case DEVICE_GRABBER_FVW:
				case DEVICE_GRABBER_SIMULATE:
					dt = EditDeviceInfo::GR_Series;
					break;
				default:
					break;
				}
				return dt;
			}
		}
	}
	QString current_device_type{};
	current_device_type = item->text(1);
	if (current_device_type.contains("M") || current_device_type.contains("X") || current_device_type.contains("G")) {
		dt = EditDeviceInfo::M_Series;
	}
	else if (current_device_type.contains("GR")) {
		dt = EditDeviceInfo::GR_Series;
	}
	else if (current_device_type.contains("CF")) {
		dt = EditDeviceInfo::Syn_Controller;
	}
	return dt;
}

void CSDeviceListWidget::delTreeNode(const QString& node_name)
{
	if (m_const_json_file.isEmpty())
		return;
	QFile file(m_const_json_file);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	const QByteArray raw_data = file.readAll();
	file.close();

	QJsonParseError json_error;
	m_json_doc = QJsonDocument::fromJson(raw_data, &json_error);
	bool z = (json_error.error != QJsonParseError::NoError);
	if (m_json_doc.isNull() || m_json_doc.isEmpty() || json_error.error != QJsonParseError::NoError)
		return;

	if (m_json_doc.isObject()) {
		QJsonObject json_doc_object = m_json_doc.object();

		QJsonObject::const_iterator iter;
		for (iter = json_doc_object.constBegin(); iter != json_doc_object.constEnd(); ++iter) {
			if (json_doc_object.isEmpty()) {
				break;
			}
			if (iter->isObject()) {
				QJsonObject obj = iter->toObject();
				QString obj_key = iter.key();
				if (deleteJsonNode(obj, json_doc_object, obj_key, node_name)) {
					if (json_doc_object.contains(obj_key)) {
						json_doc_object.insert(obj_key, obj);
					}
					break;
				}
			}
		}

		m_json_doc.setObject(json_doc_object);
		QByteArray json_data = m_json_doc.toJson(QJsonDocument::Indented);
		QFile file(m_const_json_file);
		if (!file.open(QIODevice::WriteOnly))
			return;
		file.write(json_data);
		file.close();

		ui->deviceTreeView->clear();
		m_device_tree_item_list.clear();
		loadJson(m_const_json_file);
	}
}

DeviceModel CSDeviceListWidget::getDeviceModel(DeviceType dt)
{
	switch (dt)
	{
	case CSDeviceListWidget::M_Series:
		return DEVICE_M120;
		break;
	case CSDeviceListWidget::GR_Series:
		return DEVICE_GRABBER_220;
		break;
	case CSDeviceListWidget::Syn_Controller:
		return DEVICE_TRIGGER_CF18;
		break;
	default:
		break;
	}
	return DEVICE_M120;
}

void CSDeviceListWidget::updateRefreshLabel(const uint8_t count, bool normal)
{
	if (normal) {
		ui->refresh_btn->setText(tr("Refresh"));
	}
	else {
		ui->refresh_btn->setText(tr("Refresh(%1s)").arg(count));
	}
}

void CSDeviceListWidget::setRefreshBtnEnable(bool enable) {
	ui->refresh_btn->setEnabled(enable);
}

CSDeviceListWidget::DeviceType CSDeviceListWidget::getDeviceType(const DeviceModel dm)
{
	if (DeviceModel::DEVICE_TRIGGER_CF18 == dm) {
		return Syn_Controller;
	}
	else if ((DeviceModel::DEVICE_GRABBER_100T <=dm && dm <= DeviceModel::DEVICE_GRABBER_SIMULATE)) {
		return GR_Series;
	}
	else {
		return M_Series;
	}
	return M_Series;
}

void CSDeviceListWidget::slotSearchDeviceFinished()
{
	if (m_is_first_search) {
		m_is_first_search = false;

		loadJson(m_const_json_file,true);
		loadJson(m_const_json_file);
		if (SystemSettingsManager::instance().isAutoConnectEnabled())
		{
			QTimer::singleShot(10, this, [this]() {
				autoConnectDevices();
			});
		}
		//autoConnectDevices();
	}
	else
	{
		UpdateDeviceModeName();
	}
}

void CSDeviceListWidget::slotToolButtonCopyParam()
{
	//复制设备操作
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		UIUtils::showErrorMsgBox(this, tr("Parameter copying failed!"));
		return;
	}
	//判断设备是否可复制
	if (device_ptr->AllowsSetCameraSettings())
	{
		m_device_to_copy = device_ptr;
		UIUtils::showInfoMsgBox(this, tr("The parameter is copied successfully!"));

		SystemSettingsManager::instance().saveCurrentDeviceParamInfo();
	}
	else
	{
		UIUtils::showErrorMsgBox(this, tr("Parameter copying failed!"));
	}
	updateCopyAndPasteCtrl();
}

void CSDeviceListWidget::slotToolButtonPasteParam()
{
	//粘贴设备操作
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull() || !device_ptr->AllowsSetCameraSettings())
	{
		UIUtils::showErrorMsgBox(this, tr("Parameter pasting failed!"));
		return;
	}
	bool need_warning_and_delete = false;
	//判断参数是否和当前一致 一致则跳过
	if (device_ptr->IsPixelDepthDifferent())
	{
		need_warning_and_delete = true;
	}
	//添加弹窗确认是否需要执行操作 告知需要格式化视频列表
	//询问是否确定
	if (need_warning_and_delete )
	{
		if (!UIUtils::showQuestionMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_CHANGE_DEPTH_TIP")))
		{
			return;
		} 
	}

	HscResult res = device_ptr->refreshCurrentDeviceSettings();
	if (res == HSC_OK)
	{
		UIUtils::showInfoMsgBox(this, tr("The parameter is pasted successfully!"));
	}
	else
	{
		UIUtils::showErrorMsgBox(this, tr("Parameter pasting failed!"));
	}

	if (need_warning_and_delete)
	{
		//通知相机开始格式化
		HscResult delete_res = device_ptr->DeleteAllVideoClips();
		if (delete_res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_FORMAT_FAIL"));
			return;
		}
		//显示格式化进度对话框
		CSDlgProgressFormat dlg(device_ptr);
		dlg.exec();

		device_ptr->updateVideoSegmentList();

		UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_CHANGE_DEPTH_SUCCESS_TIP"));
	}
}

void CSDeviceListWidget::on_deviceTreeView_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	//获取当前设备
	auto device_ptr = m_device_magager_ptr->getCurrentDevice();
	if (device_ptr.isNull())//触发器不可双击连接
	{
		return ;
	}
	//双击设备直接进入预览或高采模式
	DeviceState state = device_ptr->getState();
	if (state == DeviceState::Disconnected || state == DeviceState::Unconnected)
	{
		//连接槽函数
		auto connect_type = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
		auto connect_type_auto = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);
		connect(device_ptr.data(), &Device::errorOccurred, this, &CSDeviceListWidget::slotDeviceErrorOccurred, connect_type);
		connect(device_ptr.data(), &Device::propertyChanged, this, &CSDeviceListWidget::slotDevicePropertyChanged, connect_type_auto);
		connect(device_ptr.data(), &Device::orgStateIsAcquring, this, &CSDeviceListWidget::slotOrgStateIsAcquring, connect_type);
		if (device_ptr->IsCF18()) {
			connect(device_ptr.data(), &Device::cf18StateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
		}
		else {
			connect(device_ptr.data(), &Device::stateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
		}
		m_device_magager_ptr->appendAddedDevice(device_ptr);

	}

	if (device_ptr->IsCF18()) {
		device_ptr->connect();
		return;
	}
	if (state == DeviceState::Connected ||
		state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::Previewing
		)
	{
		CSRccApp* main_window = dynamic_cast<CSRccApp*>( getMainWindow());
		if (main_window)
		{
			if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
			{
				main_window->on_actionHigh_speed_Acquisition_triggered();
			}
			else
			{
				main_window->on_actionPreview_P_triggered();
			}
		}
	}

}

void CSDeviceListWidget::slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (!m_allow_selection_changed)//设备管理器主动改变界面,不需要再次通知
	{
		return;
	}

	// 当前选择设备变更,切换按钮使能状态
	QString current_ip ;
	if (current.isValid())
	{
		current_ip = getCurrentDeviceIP();
	}

	//设置设备管理器的当前选中设备
	m_device_magager_ptr->SetCurrentDevice(m_device_magager_ptr->getDevice(current_ip));

}

void CSDeviceListWidget::slotCurrentDeviceChanged(const QString & device_ip)
{
	//刷新当前界面显示,但是不要通知设备管理器
	m_allow_selection_changed = false;
	updateCopyAndPasteCtrl();

	updateDeviceActionsCtrl();

	for (auto it : m_device_tree_item_list)
	{
		if (it && (it->columnCount() > 2)) {
			if (device_ip == it->text(2)) {
				emit signalItemClicked();
				it->setSelected(true);
			}
			else {
				it->setSelected(false);
			}
		}
	}
	m_allow_selection_changed = true;

	for (auto it : m_device_tree_item_list)
	{
		if (it && (it->columnCount() > 2)) {
			if (device_ip == it->text(2)) {
				it->setSelected(true);
			}
			else {
				it->setSelected(false);
			}
		}
	}
}

bool CSDeviceListWidget::slotConnectCurrentDevice()
{
	//获取当前设备
	auto device_ptr = m_device_magager_ptr->getCurrentDevice();
	if (device_ptr.isNull())
	{
		return false;
	}
	if (device_ptr->getState() != DeviceState::Unconnected &&
		device_ptr->getState() != DeviceState::Disconnected)
	{
		return false;
	}

	//连接槽函数
	auto connect_type = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
	auto connect_type_auto = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);
	connect(device_ptr.data(), &Device::errorOccurred, this, &CSDeviceListWidget::slotDeviceErrorOccurred, connect_type);
	connect(device_ptr.data(), &Device::orgStateIsAcquring, this, &CSDeviceListWidget::slotOrgStateIsAcquring, connect_type);
	connect(device_ptr.data(), &Device::propertyChanged, this, &CSDeviceListWidget::slotDevicePropertyChanged, connect_type_auto);
	if (device_ptr->IsCF18()) {
		connect(device_ptr.data(), &Device::cf18StateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
	}
	else {
		connect(device_ptr.data(), &Device::stateChanged, this, &CSDeviceListWidget::slotDeviceStateChanged, connect_type);
	}

	m_device_magager_ptr->appendAddedDevice(device_ptr);

	CSRccApp* main_window = dynamic_cast<CSRccApp*>(getMainWindow());
	if (main_window)
	{
		//连接设备
		if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())//西光所版本直接进入高采
		{
			main_window->on_actionHigh_speed_Acquisition_triggered();
		}
		else
		{
			device_ptr->setShowTip(true);
			device_ptr->connect();
		}
	}

	return true;
}

bool CSDeviceListWidget::slotDisconnectCurrentDevice()
{
	//获取当前设备
	auto device_ptr = m_device_magager_ptr->getCurrentDevice();
	if (device_ptr.isNull())
	{
		return false;
	}

	//断开连接
	device_ptr->Device::disconnect();
	m_device_magager_ptr->removeAddedDevice(device_ptr);
	return true;
	//断开槽函数连接
	//disconnect(device_ptr.data(), 0, this, 0);//断开之后没法获取状态,不可断开
}

void CSDeviceListWidget::slotDeviceStateChanged(const QString &current_ip, DeviceState state)
{
	Q_UNUSED(state);

	//更新设备状态
	auto device_ptr = dynamic_cast<Device*>(sender());//获取发送信号的设备
	if (device_ptr)
	{
		QString device_ip = device_ptr->getIpOrSn();
		QTreeWidgetItemIterator node(ui->deviceTreeView);
		while (*node)
		{
			int conlum = (*node)->columnCount();
			QString node_ip = (*node)->text(2);
			if (device_ip == node_ip) {
				DeviceState ds = device_ptr->getState();

				(*node)->setText(3, device_ptr->getStateStr());
				(*node)->setTextColor(3, Qt::black);
				(*node)->setToolTip(3, device_ptr->getStateStr());
				break;
			}
			++node;
		}
	}

	//更新复制参数相关按钮使能
	updateCopyAndPasteCtrl();

	updateDeviceActionsCtrl();
}

void CSDeviceListWidget::slotDevicePropertyChanged(Device::PropType type, const QVariant & value)
{
	switch (type)
	{
	case Device::PropName:
	{
		//更新设备状态
		auto device_ptr = dynamic_cast<Device*>(sender());//获取发送信号的设备
		if (device_ptr)
		{
			QString device_ip = device_ptr->getIpOrSn();
			QTreeWidgetItemIterator node(ui->deviceTreeView);
			while (*node)
			{
				int conlum = (*node)->columnCount();
				QString node_ip = (*node)->text(2);
				if (device_ip == node_ip) {
					(*node)->setText(0, value.toString());
					(*node)->setToolTip(0, value.toString());
					break;
				}
				++node;
			}
		}
	}
		break;
	default:
		break;
	}
}

void CSDeviceListWidget::slotDeviceErrorOccurred(quint64 err_code, bool bShowTip, QString extTip)
{
	auto device_ptr = dynamic_cast<Device*>(sender());//获取发送信号的设备
	QString device_ip{};
	if (device_ptr) {
		device_ip = device_ptr->getIpOrSn();
	}
	if (err_code == HSC_NO_ENOUGH_MEMORY)
	{
		QString caption = QObject::tr("Error Code");
		QString value = QString("0x%1").arg(err_code, 8, 16, QChar('0'));

		QString error_desc;
		int level = 0;
		AgErrorCodeInfo error_code_info{};
		if (AgErrorCode::instance().get(err_code, error_code_info))
		{
			error_desc = error_code_info.desc;
			level = error_code_info.level;
		}
		QString strSpaceMemory;
		if (device_ptr)
		{
			//计算剩余空间
			RecordType type = RECORD_BY_FRAME;
			uint64_t available = 0;
			device_ptr->GetAvailableFrameStorageSpace(available, type);

			if (type == RECORD_BY_FRAME)
			{
				strSpaceMemory = tr("%1 %2!").arg(available).arg(QObject::tr("frames"));
			}
			else if (type == RECORD_BY_TIME)
			{
				strSpaceMemory = tr("%1 %2!").arg(available).arg(QObject::tr("ms"));
			}
			else if (type == RECORD_BY_TIME_S)
			{
				strSpaceMemory = tr("%1 %2!").arg(available).arg(QObject::tr("s"));
			}
		}

		QString msg_text = QString("%1(%2): %3 %4 %5").arg(caption).arg(value).arg(error_desc).arg(QObject::tr("Free space:")).arg(strSpaceMemory);
		msg_text = msg_text.trimmed();
		if (msg_text.endsWith(":"))
		{
			msg_text.chop(1);
		}

		
		QTreeWidgetItemIterator node(ui->deviceTreeView);
		while (*node)
		{
			int conlum = (*node)->columnCount();
			QString node_ip = (*node)->text(2);
			if (device_ip == node_ip) {
				(*node)->setToolTip(3, msg_text);
				break;
			}
			++node;
		}
		if (bShowTip)
		{
			UIUtils::showInfoMsgBox(getMainWindow(), msg_text);
		}
	}
	else if (err_code == HSC_NETWORK_ERROR || err_code == HSC_DLL_NOT_FOUND || err_code == HSC_DEVICE_ISOPENED || err_code == HSC_IN_UPGRADING)
	{
		QTreeWidgetItemIterator node(ui->deviceTreeView);
		AgErrorCodeInfo error_code_info{};
		AgErrorCode::instance().get(err_code, error_code_info);
		while (*node)
		{
			int conlum = (*node)->columnCount();
			QString node_ip = (*node)->text(2);
			if (device_ip == node_ip) {
				(*node)->setText(3, tr("connect failed"));
				(*node)->setTextColor(3, Qt::red);
				(*node)->setToolTip(3, error_code_info.desc);
				break;
			}
			++node;
		}
		if (bShowTip)
		{
			ErrorCodeUtils::handle(getMainWindow(), err_code);
		}
	}
	else if (err_code == HSC_DEVICE_INIT_ERROR)
	{
		//临时方案: 该错误不做提示,只记录
		CSLOG_WARN("error not shown, err_code:{},ip:{}", err_code, device_ip.toStdString().c_str());

	}
	else
	{
		if (bShowTip)
		{
			ErrorCodeUtils::handle(getMainWindow(), err_code);
		}
		else
		{
			QTreeWidgetItemIterator node(ui->deviceTreeView);
			AgErrorCodeInfo error_code_info{};
			AgErrorCode::instance().get(err_code, error_code_info);
			while (*node)
			{
				int conlum = (*node)->columnCount();
				QString node_ip = (*node)->text(2);
				if (device_ip == node_ip) {
					(*node)->setText(3, tr("Device abnormality"));
					(*node)->setTextColor(3, Qt::red);
					(*node)->setToolTip(3, error_code_info.desc);
					break;
				}
				++node;
			}
		}
	}
}

void CSDeviceListWidget::slotOrgStateIsAcquring(const QString & ip)
{
	if (ip.isEmpty())
	{
		auto device_ptr = m_device_magager_ptr->getCurrentDevice();
		if (!device_ptr.isNull())
		{
			CSRccApp* main_window = dynamic_cast<CSRccApp*>(getMainWindow());
			if (main_window)
			{
				main_window->AddDeviceToView(ip);
			}
		}
	}
	else
	{
		auto device_ptr = m_device_magager_ptr->getDevice(ip);
		if (!device_ptr.isNull())
		{
			CSRccApp* main_window = dynamic_cast<CSRccApp*>(getMainWindow());
			if (main_window)
			{
				main_window->AddDeviceToView(ip);
			}
		}
	}
}

void CSDeviceListWidget::on_deviceTreeView_customContextMenuRequested(const QPoint &pos)
{
	//判断是否是有效位置
	QModelIndex index = ui->deviceTreeView->indexAt(pos);
	if (!index.isValid())
	{
		return;
	}

	do 
	{
		QTreeWidgetItem* current_item = ui->deviceTreeView->currentItem();
		if (current_item) {
			if (1 == current_item->columnCount()) {
				updateMenus(kMenuTypeGroup);
				break;
			}

			if (1 < current_item->columnCount()) {
				QString current_ip = getCurrentDeviceIP();
				QSharedPointer<Device> current_device = m_device_magager_ptr->getDevice(current_ip);
				if (current_device) {
					if (current_device != DeviceManager::instance().getCurrentDevice())
					{
						DeviceManager::instance().SetCurrentDevice(current_device);
					}
	
					//刷新菜单列表
					if (current_device->getState() == DeviceState::Unconnected ||
						current_device->getState() == DeviceState::Disconnected)
					{
						if (current_device->IsCamera()) {
							updateMenus(kMenuTypeCameraConnect);
						}
						else {
							if (current_device->IsCF18()) {
								updateMenus(kMenuTypeCF18Connect);
							}
							else {
								updateMenus(kMenuTypeTriggerConnect);
							}
						}
					}
					else
					{
						if (current_device->IsCamera()) {
							updateMenus(kMenuTypeCameraDisconnect);
						}
						else {
							if (current_device->IsCF18()) {
								updateMenus(kMenuTypeCF18Disconnect);
							}
							else {
								updateMenus(kMenuTypeTriggerDisconnect);
							}
						}
					}
					break;
				}
			}
		}

	
	} while (false);

	//开启菜单
	if (m_menu_device)
	{
		m_menu_device->exec(ui->deviceTreeView->viewport()->mapToGlobal(pos));
	}
}

void CSDeviceListWidget::slotCurrentDeviceInfo()
{	
	//获取当前设备
	auto device_ptr = m_device_magager_ptr->getCurrentDevice();
	if (device_ptr.isNull())
	{
		return ;
	}


	//显示设备信息对话框
	if (device_ptr->IsCamera())
	{
		CSDlgCameraProperty dlg(device_ptr, this);
		dlg.exec();
	}
	else//触发器设备信息
	{
		CSDlgTriggerProperty dlg(device_ptr, this);
		dlg.exec();
	}

}

void CSDeviceListWidget::slotCurrentTriggerPulseControl()
{
	//获取当前设备
	auto device_ptr = m_device_magager_ptr->getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	CSDlgTriggerPulseControl dlg(device_ptr,this);
	dlg.exec();
}

void CSDeviceListWidget::slotDeleteCurrentCF18()
{

}

void CSDeviceListWidget::slotDeviceStateChangedUpdateRotate(const QString & current_ip, DeviceState state)
{
	Q_UNUSED(state);

	//更新设备状态
	auto device_ptr = dynamic_cast<Device*>(sender());//获取发送信号的设备
	if (device_ptr)
	{
		DeviceState curState = device_ptr->getState();
		if (curState == Previewing || curState == Acquiring || curState == Recording)
		{
			disconnect(device_ptr, &Device::stateChanged, this, &CSDeviceListWidget::slotDeviceStateChangedUpdateRotate);
			emit signalUpdateDeviceRotate(device_ptr->getIpOrSn());
		}
	}
}

void CSDeviceListWidget::slotDragFinished()
{
	updateTreeWidgetInfo();
}

void CSDeviceListWidget::slotAddDevice(const TreeWidgetItemInfo&  iteminfo) {
	TreeWidgetItemInfo temp_iteminfo = iteminfo;
	//移除原有设备数据
	ui->deviceTreeView->clear();
	m_device_tree_item_list.clear();

	QTreeWidgetItem* tree_widget_item = new QTreeWidgetItem(ui->deviceTreeView);
	QJsonObject root_obj = m_json_doc.object();

	if (TreeWidgetItemType::GROUP == temp_iteminfo.item_type) {
		QJsonObject json_obj{};
		json_obj.insert(m_const_child_node, QJsonObject{});
		json_obj.insert(m_const_node_name, temp_iteminfo.group_name);
		json_obj.insert(m_const_node_type, nGroup);
		root_obj.insert(temp_iteminfo.group_name, json_obj);
	}
	else if (TreeWidgetItemType::DEVICE == temp_iteminfo.item_type) {
		if (temp_iteminfo.p_device) {
			QString ip_str = temp_iteminfo.p_device->getIpOrSn();
			QJsonObject json_obj{};
			json_obj.insert(m_const_child_node, QJsonObject{});
			json_obj.insert(m_const_node_name, ip_str);
			json_obj.insert(m_const_model_info, temp_iteminfo.p_device->getModel());
			json_obj.insert(m_const_node_type, nDevice);
			json_obj.insert(m_const_model_type_info, temp_iteminfo.p_device->getModelName());
			root_obj.insert(ip_str, json_obj);
		}
	}
	else if (TreeWidgetItemType::SYN_CONTROLLER == temp_iteminfo.item_type) {
		if (temp_iteminfo.p_device) {
			QString ip_str = temp_iteminfo.p_device->getIpOrSn();
			QJsonObject json_obj{};
			json_obj.insert(m_const_child_node, QJsonObject{});
			json_obj.insert(m_const_node_name, ip_str);
			json_obj.insert(m_const_model_info, temp_iteminfo.p_device->getModel());
			json_obj.insert(m_const_model_type_info, temp_iteminfo.p_device->getModelName());
			json_obj.insert(m_const_node_type, nSynController);
			root_obj.insert(ip_str, json_obj);
		}
	}
	m_json_doc.setObject(root_obj);
	QByteArray json_data = m_json_doc.toJson(QJsonDocument::Indented);
	QFile file(m_const_json_file);
	if (!file.open(QIODevice::WriteOnly))
		return;
	file.write(json_data);
	file.close();

	ui->deviceTreeView->clear();
	m_device_tree_item_list.clear();
	loadJson(m_const_json_file);

	auto item = FindTreeItem(iteminfo);
	if (item)
	{
		int nColumn = 2;
		if (iteminfo.item_type == GROUP)
		{
			nColumn = 0;
		}
		slotItemClicked(item, nColumn);
		ui->deviceTreeView->setCurrentItem(item);
		if (nColumn == 2)
		{
			QString current_mode = item->text(1);
			on_deviceTreeView_doubleClicked(ui->deviceTreeView->currentIndex());
		}
	}
}

void CSDeviceListWidget::slotItemClicked(QTreeWidgetItem *item, int column)
{
	emit signalItemClicked();
	if (item && item->columnCount() < 2) {//分组
		m_deive_num_in_selected_group = 0;
		calDeiveNumInSelectedGroup(item);
		ui->deviceTreeView->setCurrentItem(item);
		emit m_device_magager_ptr->signalCurrentGroupChanged(ui->deviceTreeView->currentItem()->text(0), QString::number(m_deive_num_in_selected_group));
		return;
	}
	else {
		if (item){
			QString current_device_ip = item->text(2);
			if (m_device_magager_ptr) {
				m_device_magager_ptr->SetCurrentDevice(m_device_magager_ptr->getDevice(current_device_ip));
			}
		}
	}
}

void CSDeviceListWidget::slotCurrentItemClicked()
{
	ui->deviceTreeView->setCurrentIndex(QModelIndex());
}

void CSDeviceListWidget::slotAddBtnClicked()
{
	if (!m_device_magager_ptr) {
		return;
	}
	TreeNodeOperate dlg;
	m_all_ip_set.clear();
	m_manage_ip_set.clear();
	m_group_set.clear();
	m_syn_controller_set.clear();
	updateTreeNodeVessel();
	dlg.setTabIndex(m_tab_index);
	dlg.setAllDeviceIpVessel(m_all_ip_set);
	dlg.setManageDeviceIpVessel(m_manage_ip_set);
	dlg.setGroupNameVessel(m_group_set);
	dlg.setSynControllerVessel(m_syn_controller_set);
	int dlg_ret = dlg.exec();
	if (0 != dlg_ret) {//=0，直接关闭弹框
		TreeNodeOperate::OperatorData od = dlg.getOperatorData();
		if (TreeNodeOperate::TabType::Group == od.tb) {	
			TreeWidgetItemInfo  iteminfo{};
			iteminfo.item_type = TreeWidgetItemType::GROUP;
			iteminfo.group_name = od.group_name;
			m_device_magager_ptr->addManageItem(iteminfo);
		}
		else if (TreeNodeOperate::TabType::Device == od.tb) {
			TreeWidgetItemInfo  iteminfo{};
			if (TreeNodeOperate::DeviceType::Syn_Controller == od.dt) {
				iteminfo.item_type = TreeWidgetItemType::SYN_CONTROLLER;
			}
			else {
				iteminfo.item_type = TreeWidgetItemType::DEVICE;
			}
			DeviceModel dm = getDeviceModel(DeviceType(od.dt));
			iteminfo.p_device = m_device_magager_ptr->getNewDevice(od.device_ip, dm);
			iteminfo.p_device->setProperty(Device::PropName, od.device_name);
			m_device_magager_ptr->addManageItem(iteminfo);
		}
		m_tab_index = 0;
		if (2 == dlg_ret) {//“保存且添加”
			if (TreeNodeOperate::TabType::Group == od.tb) {
				m_tab_index = 1;
			}
			else if (TreeNodeOperate::TabType::Device == od.tb) {
				m_tab_index = 0;
			}
			slotAddBtnClicked();
		}
	}
}

void CSDeviceListWidget::slotEditBtnClicked()
{
	if (ui->deviceTreeView->currentItem()) {
		if (ui->deviceTreeView->currentItem()->columnCount() > 1) {
			m_all_ip_set.clear();
			m_manage_ip_set.clear();
			m_syn_controller_set.clear();
			updateDeviceVessel();
			EditDeviceInfo dlg;
			EditDeviceInfo::OperatorData od;
			dlg.setAllDeviceIpVessel(m_all_ip_set);
			dlg.setManageDeviceIpVessel(m_manage_ip_set);
			dlg.setSynControllerVessel(m_syn_controller_set);
			
			od.old_device_ip = ui->deviceTreeView->currentItem()->text(2);
			od.old_dt = getCurrentDeviceType(ui->deviceTreeView->currentItem());
			od.old_device_name = ui->deviceTreeView->currentItem()->text(0);
			auto dev_ptr = m_device_magager_ptr->getDevice(od.old_device_ip);
			if (m_device_magager_ptr && dev_ptr) {

				DeviceState ds = dev_ptr->getState();
				if ((DeviceState::Recording == ds) || (DeviceState::Acquiring == ds)) {
					dlg.setPartsEnabled(false);
				}
				dlg.setDeviceStatus((DeviceState::Connected == ds) || (DeviceState::Previewing == ds));
			}
	
			dlg.setEditDeviceInfo(od);
			if (QDialog::Accepted == dlg.exec()) {
				EditDeviceInfo::OperatorData od_rt = dlg.getEditDeviceInfo();

				if ((0 == od_rt.device_ip.compare(od_rt.old_device_ip)) && (od_rt.dt == od_rt.old_dt)) {
					if (0 != od_rt.device_name.compare(od_rt.old_device_name)) {
						//to do:mpp 重设设备的名称
						auto device_ptr = m_device_magager_ptr->getDevice(od_rt.device_ip);
						if (device_ptr)
						{
							device_ptr->setProperty(Device::PropName, od_rt.device_name);
						}
						updateDeviceListValue(od_rt.device_ip, DLH_Name, od_rt.device_name);
					}
				}
				else {
					if (m_device_magager_ptr) {
						auto old_device_ptr = m_device_magager_ptr->getDevice(od_rt.old_device_ip);
						if (old_device_ptr)
						{
							DeviceState status = old_device_ptr->getState();
							if (status != Disconnected && status != Unconnected)
							{
								old_device_ptr->disconnect(true);
							}
							m_device_magager_ptr->removeAddedDevice(old_device_ptr);
						}
						m_device_magager_ptr->removeManageDevice(od_rt.old_device_ip);
						delTreeNode(od_rt.old_device_ip);
						HscDeviceInfo dev_info{};
						dev_info.model = getDeviceModel(DeviceType(od_rt.dt));
						memcpy(dev_info.ip, od_rt.device_ip.toLocal8Bit().data(), sizeof(dev_info.ip));
						m_device_magager_ptr->createDevice(dev_info);
						auto device_ptr = m_device_magager_ptr->getDevice(od_rt.device_ip);
						if (device_ptr)
						{
							device_ptr->setProperty(Device::PropName, od_rt.device_name);
						}
						updateDeviceListValue(od_rt.device_ip, DLH_Name, od_rt.device_name);
					}
				}
			}
		}
		else {
			m_group_set.clear();
			updateGroupInfo();
			EditGroupInfo dlg;
			EditGroupInfo::OperatorData od;
			dlg.setGroupNameVessel(m_group_set);

			od.old_group_name = ui->deviceTreeView->currentItem()->text(0);
			dlg.setEditGroupInfo(od);

			if (QDialog::Accepted == dlg.exec()) {
				EditGroupInfo::OperatorData od_rt = dlg.getEditGroupInfo();
				if (0 != od_rt.old_group_name.compare(od_rt.group_name)) {
					changeGroupName(od_rt.old_group_name, od_rt.group_name);
				}
			}
		}
	}
}

void CSDeviceListWidget::slotDeleteCurrentNode()
{
	QString delete_name{};
	if (ui->deviceTreeView->currentItem()) {
		if (ui->deviceTreeView->currentItem()->childCount() > 0) {
			QString message = tr("Delete group?\nAfter clicking OK, the devices in the group will\nbe removed from the group.");
			if (QMessageBox::No == QMessageBox::information(this, QObject::tr("RCC"), message, QMessageBox::Yes, QMessageBox::No)) {
				return;
			}
		}

		if (ui->deviceTreeView->currentItem()->columnCount() > 1) {
			delete_name = ui->deviceTreeView->currentItem()->text(2);
			if (m_device_magager_ptr) {
				QSharedPointer<Device> pDevice = m_device_magager_ptr->getDevice(delete_name);
				if (pDevice && ((DeviceState::Connected == pDevice->getState()) || (DeviceState::Previewing == pDevice->getState()))) {
					QString message = tr("Delete device?\nAfter clicking OK, device will be disconnected and will not appear in the device list.");
					if (QMessageBox::No == QMessageBox::information(this, QObject::tr("RCC"), message, QMessageBox::Yes, QMessageBox::No)) {
						return;
					}
				}
				emit signalRemoveCurrentDevice();
				m_device_magager_ptr->removeManageDevice(delete_name);
			}
		}
		else if(1 == ui->deviceTreeView->currentItem()->columnCount()){
			delete_name = ui->deviceTreeView->currentItem()->text(0);
		}
	}
	delTreeNode(delete_name);
}

void CSDeviceListWidget::slotCountdownTimer()
{
	if (0 == m_countdown) {
		m_countdown_timer->stop();
		updateRefreshLabel(m_countdown);
		setRefreshBtnEnable(true);
		return;
	}
	updateRefreshLabel(--m_countdown,false);
}

void CSDeviceListWidget::slotTreeItemClicked(QTreeWidgetItem *item, int column)
{
	if (item && item->columnCount() < 2) {//分组
		for (auto it : m_device_tree_item_list)
		{
			it->setFlags(it->flags() & ~Qt::ItemIsDropEnabled);
			it->setExpanded(true); // 只有设置了父控件，才能展开
		}
		return;
	}
	else {
		for (auto it : m_device_tree_item_list)
		{
			it->setFlags(it->flags() | Qt::ItemIsDropEnabled);
			it->setExpanded(true); // 只有设置了父控件，才能展开
		}
	}
}

void CSDeviceListWidget::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		ui->deviceTreeView->clear();
		m_device_tree_item_list.clear();
		loadJson(m_const_json_file);
		//设置表头
		QStringList header_lables;
		header_lables << tr("Device Name") << tr("Model") << tr("IP/SN") << tr("State");

		ui->deviceTreeView->setHeaderLabels(header_lables);


		if (!m_action_connect) return;
		if (!m_action_disconnect) return;
		if (!m_action_copy) return;
		if (!m_action_paste) return;
		if (!m_action_device_info) return;
		if (!m_action_cf18_info) return;
		if (!m_action_delete_current_node) return;

		m_action_connect->setText(tr("Connect"));
		m_action_disconnect->setText(tr("Disconnect"));
		m_action_copy->setText(tr("Copy Parameters"));
		m_action_paste->setText(tr("Paste Parameters"));
		m_action_device_info->setText(tr("About Device"));
		m_action_cf18_info->setText(tr("About Synchronous Controller"));
		m_action_delete_current_node->setText(tr("Delete"));
		
	}

	QWidget::changeEvent(event);
}

void CSDeviceListWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)//空格触发
	{
		QWidget::keyPressEvent(event);
	}
}

QTreeWidgetItem* CSDeviceListWidget::FindTreeItem(const TreeWidgetItemInfo& iteminfo)
{
	QTreeWidgetItem *pItem = nullptr;
	QString strText;
	int nColumn = -1;
	if (iteminfo.item_type == GROUP)
	{
		nColumn = 0;
		strText = iteminfo.group_name;
	}
	else if (iteminfo.item_type == DEVICE || iteminfo.item_type == SYN_CONTROLLER)
	{
		nColumn = 2;
		if (iteminfo.p_device)
		{
			strText = iteminfo.p_device->getIpOrSn();
		}
	}
	if (nColumn > -1)
	{
		QList<QTreeWidgetItem *> itmes = ui->deviceTreeView->findItems(strText, Qt::MatchCaseSensitive, nColumn);
		if (itmes.size() == 1)
		{
			pItem = itmes[0];
		}
	}
	return pItem;
}

void CSDeviceListWidget::updateDeviceListValue(const QString& strIP, const EDeviceListHeader colum, QString strText)
{
	QTreeWidgetItemIterator node(ui->deviceTreeView);
	while (*node)
	{
		int conlum = (*node)->columnCount();
		QString node_ip = (*node)->text(EDeviceListHeader::DLH_IP);
		if (strIP == node_ip) {
			(*node)->setText(colum, strText);
			(*node)->setToolTip(colum, strText);
			break;
		}
		++node;
	}
}