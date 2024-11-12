#include "systemsettingsmanager.h"

#include <QSettings>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QStandardPaths>
#include <QDir>
//    #include "FallPointMeasure/FallPointMeasure.h"
#include "Device/devicemanager.h"
#include "Device/device.h"

static const QString kSystemGroup("System");//系统信息组
static const QString kLocalIpKey("System/LocalIp");//本机ip
static const QString kWorkingDirectoryKey("System/WorkingDirectory");//工作目录
static const QString kCalibrationDirectoryKey("System/CalibrationDirectory");//标定目录
static const QString kAutoPlaybackEnabled("System/AutoPlaybackEnabled");//自动回放
static const QString kAutoConnectEnabled("System/AutoConnectEnabled");//自动重连全部设备
static const QString kRecordingStopEnabled("System/RecordingStopEnabled");//即录即停是否开启
static const QString kActionTypesAfterRecording("System/ActionTypesAfterRecording");//录制后动作
static const QString kLanguageKey("System/Language");//语言
static const QString kEnabledFullScreen("System/EnabledFullScreen");//是否全屏
static const QString kAutoConnectDevices("System/AutoConnectDevices");//自动重连设备列表
//视频导出相关
static const QString kVideoExportFormat("System/VideoExport/VideoFormat");//导出保存格式
static const QString kVideoExportCorlor("System/VideoExport/VideoCorlor");//导出色彩模式端口
static const QString kVideoExportRuleName("System/VideoExport/VideoRuleName");//导出命名规则
static const QString kVideoExportDisplayFPS("System/VideoExport/DisplayFPS");//播放速率
static const QString kVideoExportSkipFrame("System/VideoExport/SkipFrame");//抽帧值
static const QString kVideoExportSkipUnit("System/VideoExport/SkipUnit");//抽帧单位
static const QString kAviCompressEnabled("System/VideoExport/AviCompressEnabled");//avi压缩使能
static const QString kWatermarkEnabled("System/VideoExport/WatermarkEnabled");//水印信息使能
static const QString kVideoWatermarkParam("System/VideoExport/WatermarkParam");//水印操作参数信息

//报靶相关
static const QString kCalibrationFiles("System/CalibrationFiles");//标定参数文件目录(文件路径@相机IP)

static const QString kWindow_CenterlineKey("System/Window_Centerline");//窗口中心线
static const QString kWindowState("System/WindowState");//窗口状态
static const QString kWindowGeometry("System/WindowGeometry");//窗口坐标大小

QString kDeviceSplitSymbol(",");



SystemSettingsManager &SystemSettingsManager::instance()
{
	static SystemSettingsManager inst;
	return inst;
}

void SystemSettingsManager::initialize()
{
	QSettings settings;
	//关闭fallback机制,提升读取效率
	settings.setFallbacksEnabled(false);

	// 查询本地所有Ipv4地址，去除0.0.0.0，127.0.0.1和255.255.255.255
	local_ip_addresses_.clear();
	QList<QHostAddress> all_addresses = QNetworkInterface::allAddresses();
	for (auto & addr : all_addresses)
	{
		if (addr.protocol() != QAbstractSocket::IPv4Protocol)
		{
			continue;
		}

		if (addr.isNull() || addr.isLoopback() || addr.isMulticast())
		{
			continue;
		}

		local_ip_addresses_.append(addr.toString());
	}

	// 创建缺省工作目录
	default_working_directory_ = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AgileDevice/Revealer_Camera_Control";
	QDir dir;
	dir.mkpath(default_working_directory_);

	// 创建默认项目目录
	default_current_exp_directory_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Revealer_Camera_Control.ini";
	m_device_param_save_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Revealer_Camera_Param.ini";
	dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

	//创建工作目录
	dir.mkpath(settings.value(kWorkingDirectoryKey, default_working_directory_).toString());



}

void SystemSettingsManager::setSystemSleepState(bool sleeping)
{
	m_system_sleeping = sleeping;
}

bool SystemSettingsManager::isSystemSleeping()
{
	return m_system_sleeping;
}

void SystemSettingsManager::setLocalIp(const QString &addr)
{
	QSettings settings;
	settings.setValue(kLocalIpKey, addr);
}

QString SystemSettingsManager::getLocalIp() const
{
	if (local_ip_addresses_.empty())
	{
		return "";
	}

	QSettings settings;
	QString addr = settings.value(kLocalIpKey, "").toString();

	// 记忆的本地IP不存在时，自动取本地IP列表中的第一个
	if (!local_ip_addresses_.contains(addr))
	{
		return local_ip_addresses_.first();
	}

	return addr;
}



QStringList SystemSettingsManager::getAllLocalIp() const
{
	return local_ip_addresses_;
}

void SystemSettingsManager::setWorkingDirectory(const QString &dir)
{
	QDir qdir;
	qdir.mkpath(dir);
	QSettings settings;
	settings.setValue(kWorkingDirectoryKey, dir);
}

QString SystemSettingsManager::getWorkingDirectory() const
{
	QSettings settings;
	return settings.value(kWorkingDirectoryKey, default_working_directory_).toString();
}

void SystemSettingsManager::setCalibrationDirectory(const QString & dir)
{
	QSettings settings;
	settings.setValue(kCalibrationDirectoryKey, dir);
}

QString SystemSettingsManager::getCalibrationDirectory() const
{
	QSettings settings;
	return settings.value(kCalibrationDirectoryKey, default_working_directory_).toString();
}

void SystemSettingsManager::loadCalibrationParamFiles()
{
	//获取已经记录到的标定文件路径,逐个导入
	//获取全部文件目录
	QSettings settings;
	auto text = settings.value(kCalibrationFiles).toString();
	QStringList file_list = text.split(kDeviceSplitSymbol, QString::SplitBehavior::SkipEmptyParts);

	//分析文件路径
	for (auto current_name_and_ip : file_list)
	{
		QStringList current_name_and_ip_list = current_name_and_ip.split("@");
		Q_ASSERT(current_name_and_ip_list.size() == 2);
		//导入至标定库
//		std::string cameraType, cameraIp;
//		FallPointMeasure::GetInstance().ImportCalibrationParams(current_name_and_ip_list.first().toLocal8Bit().data(), cameraIp, cameraType);
	}
}


void SystemSettingsManager::addCalibrationFile(QString dir, QString camera_id)
{
	//获取全部文件目录
	QSettings settings;
	auto text = settings.value(kCalibrationFiles).toString();
	QStringList file_list = text.split(kDeviceSplitSymbol,QString::SplitBehavior::SkipEmptyParts);

	QString path_name_and_ip = dir + '@' + camera_id;
	//分析文件名和ip是否存在同名
	bool b_replace = false;//是否需要替换
	int index = 0;
	for (auto current_name_and_ip : file_list)
	{
		QStringList current_name_and_ip_list = current_name_and_ip.split("@");
		Q_ASSERT(current_name_and_ip_list.size() == 2);
		if (dir == current_name_and_ip_list.first())
		{
			//文件名相同
			b_replace = true;
		}
		else
		{
			if (camera_id == current_name_and_ip_list.last())
			{
				//ip相同
				b_replace = true;
			}
		}

		if (b_replace)
		{
			file_list[index] = path_name_and_ip;
			break;
		}
		index++;
	}
	if (!b_replace)
	{
		file_list << path_name_and_ip;
	}

	QString set_text = file_list.join(kDeviceSplitSymbol);
	settings.setValue(kCalibrationFiles, set_text);

}

void SystemSettingsManager::removeCalibrationFile(QString camera_id)
{
	//获取全部文件目录
	QSettings settings;
	auto text = settings.value(kCalibrationFiles).toString();
	QStringList file_list = text.split(kDeviceSplitSymbol, QString::SplitBehavior::SkipEmptyParts);

	//分析ip是否存在同名
	bool b_delete = false;//是否需要删除
	int index = 0;
	for (auto current_name_and_ip : file_list)
	{
		QStringList current_name_and_ip_list = current_name_and_ip.split("@");
		Q_ASSERT(current_name_and_ip_list.size() == 2);

		if (camera_id == current_name_and_ip_list.last())
		{
			//ip相同
			b_delete = true;
		}

		if (b_delete)
		{
			file_list.removeAt(index);
			break;
		}
		index++;
	}

	QString set_text = file_list.join(kDeviceSplitSymbol);
	settings.setValue(kCalibrationFiles, set_text);
}

void SystemSettingsManager::setAutoPlaybackEnabled(bool enable)
{
	QSettings settings;
	settings.setValue(kAutoPlaybackEnabled, enable);
}

bool SystemSettingsManager::isAutoPlaybackEnabled() const
{
	QSettings settings;
	return settings.value(kAutoPlaybackEnabled, false).toBool();
}

void SystemSettingsManager::setAutoConnectEnabled(bool enable)
{
	QSettings settings;
	settings.setValue(kAutoConnectEnabled, enable);
}

bool SystemSettingsManager::isAutoConnectEnabled() const
{
	QSettings settings;
	return settings.value(kAutoConnectEnabled, false).toBool();
}

void SystemSettingsManager::setRecordingStopEnabled(bool enable)
{
	QSettings settings;
	settings.setValue(kRecordingStopEnabled, enable);
}

bool SystemSettingsManager::isRecordingStopEnabled() const
{
	QSettings settings;
	return settings.value(kRecordingStopEnabled, true).toBool();
}

void SystemSettingsManager::setActionTypesAfterRecording(int type)
{
	QSettings settings;
	settings.setValue(kActionTypesAfterRecording, type);
}

int SystemSettingsManager::getActionTypesAfterRecording() const
{
	QSettings settings;
	return settings.value(kActionTypesAfterRecording, 0).toInt();
}

void SystemSettingsManager::setLanguage(QLocale::Language lang)
{
	QSettings settings;
	settings.setValue(kLanguageKey, (int)lang);
}

QLocale::Language SystemSettingsManager::getLanguage() const
{
	QSettings settings;
	return QLocale::Language(settings.value(kLanguageKey, (int)QLocale::Chinese).toInt());
}

void SystemSettingsManager::setFullScreenEnabled(bool benabled)
{
	QSettings settings;
	settings.setValue(kEnabledFullScreen, benabled);
}

bool SystemSettingsManager::isFullScreenEnabled() const
{
	QSettings settings;
	return settings.value(kEnabledFullScreen, false).toBool();
}

void SystemSettingsManager::saveWindowState(QByteArray data)
{
	QSettings settings;
	settings.setValue(kWindowState, data);
}

QByteArray SystemSettingsManager::getWindowState() const
{
	QSettings settings;
	return settings.value(kWindowState, QByteArray()).toByteArray();
}

void SystemSettingsManager::setAutoConnectedDevice(const QStringList & devices_ip) const
{
	QSettings settings;
	if (devices_ip.isEmpty())
	{
		settings.remove(kAutoConnectDevices);
	}
	else
	{
		QString text = devices_ip.join(kDeviceSplitSymbol);
		settings.setValue(kAutoConnectDevices, text);
	}
}

QStringList SystemSettingsManager::getAutoConnectedDevice() const
{
	QSettings settings;
	auto text = settings.value(kAutoConnectDevices).toString();
	return text.split(kDeviceSplitSymbol);
}

QString SystemSettingsManager::getCurrentExperimentDir() const
{
	return default_current_exp_directory_;
}

QString SystemSettingsManager::getCurrentDeviceParamDir() const
{
	return m_device_param_save_path;
}

void SystemSettingsManager::saveCurrentExperimentInfo()
{
	QSettings settings;
	settings.setValue(kExperimentNameKey, m_current_exp.name);
	settings.setValue(kExperimentCodeKey, m_current_exp.code);
	settings.setValue(kExperimentDescKey, m_current_exp.info);
	QSettings setting_file(getCurrentExperimentDir(), QSettings::Format::IniFormat);
	
	setting_file.setValue(kExperimentNameKey, m_current_exp.name);
	setting_file.setValue(kExperimentCodeKey, m_current_exp.code);
	setting_file.setValue(kExperimentDescKey, m_current_exp.info);

	//获取已连接设备列表
	QList<QSharedPointer<Device>> connected_devices;
	DeviceManager::instance().getConnectedDevices(connected_devices);
	QStringList connected_devices_ip;
	for (auto device : connected_devices)
	{
		if (device)
		{
			connected_devices_ip << device->getIpOrSn();
		}
	}

	//拷贝已连接设备的参数和系统设置相关参数到本地
	for (auto key:settings.allKeys())
	{
		auto current_item = key.split('/').at(0);
		
		if (!connected_devices_ip.contains(current_item) &&
			current_item != kSystemGroup)
		{
			continue;
		}
		if (key == kLanguageKey)
		{
			continue;
		}
		setting_file.setValue(key, settings.value(key));
	}
}

void SystemSettingsManager::saveCurrentDeviceParamInfo()
{
	QSettings settings;
	QSettings setting_file(getCurrentDeviceParamDir(), QSettings::Format::IniFormat);
	setting_file.clear();
	setting_file.sync();

	//全部拷贝到本地文件中
	for (auto key : settings.allKeys())
	{
		if (key.contains("/")) {
			QStringList ip_list = key.split("/");
			if (ip_list.size() > 0) {
				if (0 == ip_list[0].compare(m_current_device_ip)) {
					if (settings.value(key).isValid()) {
						setting_file.setValue(key, settings.value(key));
					}
					}
			}
		}
	}
}

void SystemSettingsManager::loadCurrentExperimentInfo()
{
	QSettings setting_file(getCurrentExperimentDir(), QSettings::Format::IniFormat);

	QSettings settings;
	//全部拷贝到注册表中
	for (auto key : setting_file.allKeys())
	{
		settings.setValue(key, setting_file.value(key));
	}
	m_current_exp.code = settings.value(kExperimentCodeKey).toString();
	m_current_exp.name = settings.value(kExperimentNameKey).toString();
	m_current_exp.info = settings.value(kExperimentDescKey).toString();

}

void SystemSettingsManager::setWindow_Centerline(bool bEnable)
{
	QSettings settings;
	settings.setValue(kWindow_CenterlineKey, bEnable);
}

bool SystemSettingsManager::isWindow_Centerline() const
{
	QSettings settings;
	return settings.value(kWindow_CenterlineKey, false).toBool();
}

void SystemSettingsManager::setAviCompressEnabled(bool enable)
{
	QSettings settings;
	settings.setValue(kAviCompressEnabled, enable);
}

bool SystemSettingsManager::isAviCompressEnabled() const
{
	QSettings settings;
	return settings.value(kAviCompressEnabled, false).toBool();
}

void SystemSettingsManager::setDefaultVideoWatermarkEnabled(bool enable)
{
	QSettings settings;
	settings.setValue(kWatermarkEnabled, enable);
}

bool SystemSettingsManager::isDefaultVideoWatermarkEnabled() const
{
	QSettings settings;
	return settings.value(kWatermarkEnabled, false).toBool();
}

void SystemSettingsManager::setCurrentDeviceIP(const QString& iporsn)
{
	m_current_device_ip = iporsn;
}


QString SystemSettingsManager::get12PxStyle() const
{
	return m_12px_style;
}

void SystemSettingsManager::setVideoFormat(const QString videoFormat)
{
	QSettings settings;
	settings.setValue(kVideoExportFormat, videoFormat);
}

bool SystemSettingsManager::getVideoFormat(QString& videoFormat)
{
	QSettings settings;
	if (!settings.value(kVideoExportFormat).isValid())
	{
		return false;
	}
	videoFormat = settings.value(kVideoExportFormat).toString();
	return true;
}

void SystemSettingsManager::setVideoCorlor(const QString videoColor)
{
	QSettings settings;
	settings.setValue(kVideoExportCorlor, videoColor);
}

bool SystemSettingsManager::getVideoCorlor(QString& videoColor)
{
	QSettings settings;
	if (!settings.value(kVideoExportCorlor).isValid())
	{
		return false;
	}
	videoColor = settings.value(kVideoExportCorlor).toString();
	return true;
}

void SystemSettingsManager::setVideoRuleName(const QString ruleName)
{
	QSettings settings;
	settings.setValue(kVideoExportRuleName, ruleName);
}

bool SystemSettingsManager::getVideoRuleName(QString& ruleName)
{
	QSettings settings;
	if (!settings.value(kVideoExportRuleName).isValid())
	{
		return false;
	}
	ruleName = settings.value(kVideoExportRuleName).toString();
	return true;
}

void SystemSettingsManager::setDisplayfps(int fps)
{
	QSettings settings;
	settings.setValue(kVideoExportDisplayFPS, fps);
}

int SystemSettingsManager::getDisplayfps()const
{
	QSettings settings; 
	return settings.value(kVideoExportDisplayFPS,25).toInt();
}

void SystemSettingsManager::setSkipFrame(double skipFrame)
{
	QSettings settings;
	settings.setValue(kVideoExportSkipFrame, skipFrame);
}

double SystemSettingsManager::getSkipFrame() const
{
	QSettings settings;
	return settings.value(kVideoExportSkipFrame, 0).toInt();
}

void SystemSettingsManager::setSkipUnit(int skipUnit)
{
	QSettings settings;
	settings.setValue(kVideoExportSkipUnit, skipUnit);
}

bool SystemSettingsManager::getSkipUnit(int &skipUnit) const
{
	QSettings settings;
	if (!settings.value(kVideoExportSkipUnit).isValid())
	{
		return false;
	}
	skipUnit = settings.value(kVideoExportSkipUnit, 0).toInt();
	return true;
}

void SystemSettingsManager::setVideoWatermarkInfo(QMap<QString, QStringList> watermarkMap)
{
	QSettings settings;
	QSettings::SettingsMap settingsMap;
	for (auto iter = watermarkMap.begin();iter!= watermarkMap.end();iter++)
	{
		settingsMap.insert(iter.key(), iter.value());
	}
	settings.setValue(kVideoWatermarkParam, settingsMap);
}

QMap<QString, QVariant> SystemSettingsManager::getVideoWatermarkInfo()const
{
	QSettings settings;
	return settings.value(kVideoWatermarkParam).toMap();
}

QMap<QString, std::pair<int, int>> SystemSettingsManager::getTemperatureThreshold() const
{
	return map_range_;
}

SystemSettingsManager::SystemSettingsManager()
{

}