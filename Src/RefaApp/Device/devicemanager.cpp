#include "devicemanager.h"

#include <QMutexLocker>
#include <QVariant>
#include "HscDeviceAPI.h"

#include "System/SystemSettings/systemsettingsmanager.h"
#include "Device/device.h"
#include "Video/VideoUtils/videoutils.h"
#include "Common/LogUtils/logutils.h"

DeviceManager &DeviceManager::instance()
{
    static DeviceManager inst;
    return inst;
}

void DeviceManager::search()
{
	CSLOG_INFO("Search Start");
    emit searchProcessChanged(50);
	search_res = false;
	device_count_ = kSupportedMaxDeviceCount;//HscSearchDevice的count参数输入时代表最大搜索数,输出时代表搜索到的数量
    HscResult res = HscSearchDevice(SystemSettingsManager::instance().getLocalIp().toLocal8Bit().data(), device_infos_, &device_count_);

	QList<QString> device_ip{};

	for (int i = 0; i < device_count_; ++i) {
		for (int j = i + 1; j < device_count_; ++j) {
			if (QString::fromLatin1(device_infos_[i].ip) == QString::fromLatin1(device_infos_[j].ip)) {
				device_ip.append(QString::fromLatin1(device_infos_[j].ip));
				break;
			}
		}
	}

	for (auto ip_item : device_ip) {
		for (auto &device_info_item : device_infos_) {
			if (ip_item == QString::fromLatin1(device_info_item.ip) && (HScDevDiscoveryType::DT_BROADCAST == device_info_item.discovery_type)) {
				memset(device_info_item.ip, 0, sizeof(device_info_item.ip));
				device_info_item.discovery_type = HScDevDiscoveryType::DT_GATEWAY;
				break;
			}
		}
	}
	
    if (res == HSC_OK)
    {
        emit searchProcessChanged(80);
		search_res = true;
       
    }

	CSLOG_INFO("Search finish,count:{}",device_count_);
    emit searchProcessChanged(100);
    emit searchFinished(device_count_);
}

bool deviceIpLessThan(QSharedPointer<Device> dev_1, QSharedPointer<Device>dev_2)
{
	QString ip_1 = dev_1->getIpOrSn();
	QString ip_2 = dev_2->getIpOrSn();
	if (ip_1.split(".").last().toLong() < ip_2.split(".").last().toLong())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DeviceManager::createDevices()
{
	if (search_res)
	{
		{
			// 构建设备列表
			QMutexLocker locker(&mutex_);
	
			//移除所有设备 (除了已添加设备)
			devices_.clear();
			devices_ << added_devices_;
			
			//管理中的设备先过滤掉added_devices_存在的，然后添加到devices_中
			QMap<QString, int> add_device_ip_map;
			for (auto added_device : added_devices_) {
				QString strIP = added_device->getIpOrSn();
				if (add_device_ip_map.contains(strIP)){
					add_device_ip_map[strIP]++;
				}
				else {
					add_device_ip_map[strIP] = 1;
				}
			}

			QMap<QString, int> manage_device_ip_map;
			for (auto manage_device : m_manage_devices) {
				QString strIP = manage_device->getIpOrSn();
				if (add_device_ip_map.contains(strIP)){
					// 在added_devices_的已经添加过了
				}
				else {
					devices_ << manage_device;
				}
				manage_device_ip_map[strIP] = 1;
			}
	
			//添加新设备(除了已添加设备)
			for (int i = 0; i < device_count_; i++)
			{
				QString strIP = QString::fromLatin1(device_infos_[i].ip);
				if (strIP.isEmpty()) {
					continue;
				}
				if (strIP.compare("0.0.0.0") == 0)
				{
					continue;
				}

				bool is_added_device = false;
				//跳过管理中的设备
				if (manage_device_ip_map.contains(strIP))
				{
					for (auto manage_device : m_manage_devices)
					{
						if (manage_device->getIpOrSn() == strIP)
						{
							// 不进行设备信息更新了，不同型号的相机可能会出现问题。
// 							if (device_infos_[i].model == manage_device->getModel() || manage_device->getState() == DeviceState::Unconnected)
// 							{
// 								manage_device->setDeviceInfo(device_infos_[i]);
// 							}
							//更新连接方法
							auto temp = manage_device->getDeviceInfo();
							temp.dev_connect_method = device_infos_[i].dev_connect_method;
							manage_device->setDeviceInfo(temp);
							is_added_device = true;
							break;
						}
					}
				}
				if (is_added_device)
				{
					continue;
				}

				//跳过已添加设备
// 				for (auto added_device : added_devices_)
// 				{
// 					if (added_device->getIpOrSn() == QString::fromLatin1(device_infos_[i].ip))
// 					{
// 						is_added_device = true;
// 						break;
// 					}
// 				}
				if (add_device_ip_map.contains(strIP))
				{
					is_added_device = true;
				}
				if (is_added_device)
				{
					continue;
				}
	
				CSLOG_INFO("create device ,ip:{}", device_infos_[i].ip);
				QSharedPointer<Device> device_ptr(new Device(device_infos_[i]));
				// 绑定已连接和已断开信号
				connect(device_ptr.data(), &Device::deviceConnected, this, &DeviceManager::deviceConnected, Qt::DirectConnection);
				connect(device_ptr.data(), &Device::deviceDisconnected, this, &DeviceManager::deviceDisconnected);
	
				// 绑定录制完成信号
				connect(device_ptr.data(), &Device::recordingFinished, this, &DeviceManager::onRecordingFinished);
	
				devices_ << device_ptr;
			}
		}
	
		{
			// 构建拓扑关系
			QMutexLocker locker(&mutex_);
	
			for (auto device_ptr : devices_)
			{
				QString parent_ip = device_ptr->getProperty(Device::PropParentIp).toString();
				if (parent_ip.isEmpty())
				{
					continue;
				}
	
				auto parent_device_ptr = getDeviceByIp(parent_ip);
				if (!parent_device_ptr)
				{
					continue;
				}
	
				device_ptr->setParent(parent_device_ptr);
			}
		}
		qSort(devices_.begin(),devices_.end(),deviceIpLessThan);
	}
}

void DeviceManager::createDevice(const HscDeviceInfo& info)
{
	QSharedPointer<Device> device_ptr(new Device(info));
	// 绑定已连接和已断开信号
	connect(device_ptr.data(), &Device::deviceConnected, this, &DeviceManager::deviceConnected, Qt::DirectConnection);
	connect(device_ptr.data(), &Device::deviceDisconnected, this, &DeviceManager::deviceDisconnected);

	// 绑定录制完成信号
	connect(device_ptr.data(), &Device::recordingFinished, this, &DeviceManager::onRecordingFinished);

	devices_ << device_ptr;
	TreeWidgetItemInfo  iteminfo{};
	if (DeviceModel::DEVICE_TRIGGER_CF18 == info.model) {
		iteminfo.item_type = TreeWidgetItemType::SYN_CONTROLLER;
	}
	else {
		iteminfo.item_type = TreeWidgetItemType::DEVICE;
	}
	iteminfo.p_device = device_ptr;
	addManageItem(iteminfo);
}

QSharedPointer<Device> DeviceManager::getDevice(const QString &ip) const
{
    QMutexLocker locker(&mutex_);

    return getDeviceByIp(ip);
}

QSharedPointer<Device> DeviceManager::getNewDevice(const QString & ip, const DeviceModel& dm)
{
	HscDeviceInfo dev_info{};
	memcpy(dev_info.ip, ip.toLocal8Bit().data(), sizeof(dev_info.ip));
	dev_info.model = dm;
	if (DeviceModel::DEVICE_GRABBER_100T <= dm && dm <= DeviceModel::DEVICE_GRABBER_SIMULATE)
	{
		memcpy(dev_info.serial_num, ip.toLocal8Bit().data(), sizeof(dev_info.serial_num));
	}
	QSharedPointer<Device> device_ptr(new Device(dev_info));
	// 绑定已连接和已断开信号
	connect(device_ptr.data(), &Device::deviceConnected, this, &DeviceManager::deviceConnected, Qt::DirectConnection);
	connect(device_ptr.data(), &Device::deviceDisconnected, this, &DeviceManager::deviceDisconnected);

	// 绑定录制完成信号
	connect(device_ptr.data(), &Device::recordingFinished, this, &DeviceManager::onRecordingFinished);

	devices_ << device_ptr;
	return device_ptr;
}

void DeviceManager::getDevices(QList<QSharedPointer<Device> > &devices) const
{
    devices = devices_;
}

bool DeviceManager::hasDevices() const
{
	return !devices_.isEmpty();
}

int DeviceManager::getConnectedDeviceCount() const
{
    QMutexLocker locker(&mutex_);

    int count = 0;
    for (auto device_ptr : added_devices_)
    {
        DeviceState state = device_ptr->getState();
        if (state != DeviceState::Unconnected &&
			state != DeviceState::Connecting &&
			state != DeviceState::Disconnecting &&
			state != DeviceState::Disconnected)
        {
            count++;
        }
    }

    return count;
}

void DeviceManager::getConnectedDevices(QList<QSharedPointer<Device>> & devices) const
{
	for (auto device : added_devices_)
	{
		if (device->getState() != DeviceState::Unconnected &&
			device->getState() != DeviceState::Connecting &&
			device->getState() != DeviceState::Disconnecting &&
			device->getState() != DeviceState::Disconnected)
		{
			devices << device;
		}
    }
}

void DeviceManager::appendAddedDevice(QSharedPointer<Device> device)
{
	if (added_devices_.contains(device))
		return;
	if (added_devices_.count()<= kSupportedMaxAddedDeviceCount)
	{
		added_devices_ << device;
	}
	
}

void DeviceManager::removeAddedDevice(QSharedPointer<Device> device)
{
	for (auto device_to_remove : added_devices_)
	{
		if (device && device_to_remove && (device_to_remove->getIpOrSn() == device->getIpOrSn()))
		{
			added_devices_.removeOne(device_to_remove);
			break;
		}
    }
}

void DeviceManager::SetCurrentDevice(QSharedPointer<Device> device)
{

	if (device.isNull())//传入空指针直接刷新界面
	{
		current_device_ = device;
		emit currentDeviceChanged("");
	}
	else//传入非空指针判断是否相同
	{
		if (current_device_ != device)
		{
			current_device_ = device;
			emit currentDeviceChanged(current_device_->getIpOrSn());
		}
	}
}

QSharedPointer<Device> DeviceManager::getCurrentDevice() const
{
	return current_device_ ;
}

void DeviceManager::getAddedDevices(QList<QSharedPointer<Device> > &devices) const
{
	devices << added_devices_;
}

void DeviceManager::disconnectAll()
{
    QMutexLocker locker(&mutex_);

    for (auto device_ptr : devices_)
    {
        if (device_ptr->getState() != DeviceState::Unconnected)
        {
            device_ptr->Device::disconnect(true);
        }
    }
}

void DeviceManager::ReloadAllDeviceConfigFromLocal()
{
	//获取全部已连接设备
	QList<QSharedPointer<Device>> devices;
	getConnectedDevices(devices);
	for (auto device : devices)
	{
		if (!device)
		{
			continue;
		}
		//重新加载本地属性
		device->reloadPropertiesFromLocal();
	}
}

bool DeviceManager::isAnyCameraHighMode() const
{
	QList<QSharedPointer<Device>> devices;
	getConnectedDevices(devices);
	for (auto device : devices)
	{
		if (!device)
		{
			continue;
		}
		if (device->getState() == Acquiring || device->getState() == Recording)
		{
			return true;
		}
	}

	return false;
}

bool DeviceManager::AllowSoftTriggerAllDevice()
{
	QList<QSharedPointer<Device>> devices;
	getConnectedDevices(devices);
	for (auto device : devices)
	{
		if (!device)
		{
			continue;
		}
		if (device->getState() == Acquiring )
		{
			return true;
		}
	}

	return false;//只要有相机处于高采模式则可以触发
}

void DeviceManager::SoftTriggerAllDevice()
{
	QList<QSharedPointer<Device>> devices;
	getConnectedDevices(devices);
	for (auto device : devices)
	{
		if (!device)
		{
			continue;
		}
		if (device->getState() == Acquiring )
		{
			device->setShowTip(true);
			device->trigger();
		}
	}
}

void DeviceManager::addManageItem(const TreeWidgetItemInfo&  iteminfo)
{
	if (iteminfo.p_device) {
		m_manage_devices << iteminfo.p_device;
	}
	
	m_manage_devices_item_info.append(iteminfo);
	emit signalAddDevice(iteminfo);
}

void DeviceManager::appendManageItem(const TreeWidgetItemInfo& iteminfo)
{
	m_manage_devices_item_info.append(iteminfo);
	if (iteminfo.p_device) {
		m_manage_devices << iteminfo.p_device;
		emit signalUpdateTree(iteminfo.p_device->getIpOrSn());
	}	
}

void DeviceManager::removeManageDevice(const QString& cur_ip)
{
	for (int i = 0; i < m_manage_devices_item_info.size(); ++i) {
		QString manage_device_ip{};
		if (m_manage_devices_item_info[i].p_device) {
			manage_device_ip = m_manage_devices_item_info[i].p_device->getIpOrSn();
		}

		if (0 == cur_ip.compare(manage_device_ip)) {
			m_manage_devices.removeOne(m_manage_devices_item_info[i].p_device);
			m_manage_devices_item_info.removeAt(i);
			break;
		}
	}
	int i = 0;
	for (auto device_ptr : devices_)
	{
		if (0 == cur_ip.compare(device_ptr->getIpOrSn()))
		{
			devices_.removeAt(i);
			break;
		}
		i++;
	}
}

bool DeviceManager::findManageDevice(QSharedPointer<Device> device)
{
	for (auto item : m_manage_devices_item_info) {
		if (device && item.p_device && (item.p_device->getIpOrSn() == device->getIpOrSn()))
		{
			return true;
		}
	}
	return false;
}

void DeviceManager::removeManageGroup(const QString& group_name)
{
	for (int i = 0; i < m_manage_devices_item_info.size(); ++i) {

		QString manage_device_group = m_manage_devices_item_info[i].group_name;
		if (0 == group_name.compare(manage_device_group)) {
			m_manage_devices_item_info.removeAt(i);
			break;
		}
	}
}

void DeviceManager::reviseManageGroup(const QString& group_name, const QString& group_new_name)
{
	for (auto& item : m_manage_devices_item_info) {
		if (group_name == item.group_name)
		{
			item.group_name = group_new_name;
			break;
		}
	}
}

bool DeviceManager::findManageGroup(const QString& group_name)
{
	bool bExit = false;
	for (auto& item : m_manage_devices_item_info) {
		if (group_name == item.group_name)
		{
			bExit = true;
			break;
		}
	}
	return bExit;
}

void DeviceManager::getManageDevices(QList<QSharedPointer<Device>> & devices) const
{
	for (auto item : m_manage_devices_item_info) {
		if (item.p_device)
		{
			devices << item.p_device;
		}
	}
}

QList<TreeWidgetItemInfo> DeviceManager::getTreeWidgetItemInfo()
{
	return m_manage_devices_item_info;
}

void DeviceManager::onRecordingFinished(const QVariant & video_id, bool berforeStopRecing, uint8_t state)
{
	// 根据设置判断需要执行的操作
	QString ip = VideoUtils::parseDeviceIp(video_id);
	auto device_ptr = getDevice(ip);
	if (!device_ptr)
	{
		return;
	}
	if (!state && device_ptr->IsAutoExportEnabled())//自动导出
	{
		emit autoExportVideo(video_id);
	}
	else if(!state && device_ptr->AllowsAutoPlayback())//自动回放
	{
		emit autoPlaybackNeeded(video_id);
	}
	else if(!state && device_ptr->IsAutoExportAndTriggerAfterRecordingEnabled())//自动导出并循环录制
	{
		emit autoExportVideoAndTrigger(video_id);
	}
	else //直接进入高采
	{
		if (berforeStopRecing == 1)return;
		if (device_ptr->isNextAcquiringEnabled())
		{
			device_ptr->setShowTip(true);
			device_ptr->acquire();
		}
	}
	
}

DeviceManager::DeviceManager(QObject *parent) : QObject(parent)
{

}

QSharedPointer<Device> DeviceManager::getDeviceByIp(const QString &ip) const
{
    for (auto device_ptr : devices_)
    {
        if (device_ptr->getIpOrSn() == ip)
        {
            return device_ptr;
        }
    }

    return QSharedPointer<Device>();
}

