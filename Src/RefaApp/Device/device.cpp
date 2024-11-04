#include "device.h"

#include <QSettings>
#include <QMutexLocker>
#include <QHostAddress>
#include <QDateTime>
#include <QMetaEnum>
#include <QtMath>
#include <QTimer>
#include <thread>
#include <QElapsedTimer>
#include <QImage>
#include <QCoreApplication>
#include <QMessageBox>
#include <iostream>
#include "HscAPI.h"
#include "HscAPIHeader.h"
#include "AgCapability/acquisition_period_range_capability.h"
#include "AgCapability/exposure_time_range_capability.h"
#include "AgCapability/roi_range_capability.h"
#include "deviceutils.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include <HscCommandAPI.h>
#include <HscDeviceAPI.h>
#include <Device/csframeinfo.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <QImageReader>
#include <QBuffer>
#include <iostream>
using namespace cv;
#ifdef CSRCCAPP

#else

#include "realtimeplayercontroller.h"
#include "RMAImage.h"
#endif //CSRCCAPP

#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoUtils/videoutils.h"
#include "Common/LogUtils/logutils.h"
#include "Common/PathUtils/pathutils.h"
#include "Common/UIExplorer/uiexplorer.h"
//#include "FallPointMeasure/FallPointMeasure.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "System/cshealthmanager.h"
#include "imageprocessor.h"
#include "previewthread.h"
#include "acquirethread.h"
#include "msglistener.h"
#include "Common/LogUtils/logutils.h"
#include <fstream>
#include <iostream>
#include <QNetworkInterface>
#include "AGSDK.h"
using namespace std;
//向上对齐
#define ALIGNUP(size,alignSize)    ((size)%(alignSize)==0?(size):(size+alignSize-(size)%(alignSize)))
//向下对齐
#define ALIGNDOWN(size,alignSize)  ((size)%(alignSize)==0?(size):(size-(size)%(alignSize)))

#define TEST_THREAD_PROCESS_FRAME 1

//单位转换
template<agile_device::capability::Units>
struct RadioDxTrait
{

};


template<>
struct RadioDxTrait<agile_device::capability::kUnitNone>
{
};

template<>
struct RadioDxTrait<agile_device::capability::kUnitSecs>
{
	static constexpr std::intmax_t value = 1;
};

template<>
struct RadioDxTrait<agile_device::capability::kUnitMs>
{
	static constexpr std::intmax_t value = 1000;
};

template<>
struct RadioDxTrait<agile_device::capability::kUnitUs>
{
	static constexpr std::intmax_t value = 1000000;
};

template<>
struct RadioDxTrait<agile_device::capability::kUnit100ns>
{
	static constexpr std::intmax_t value = 10000000;
};

template<>
struct RadioDxTrait<agile_device::capability::kUnit10ns>
{
	static constexpr std::intmax_t value = 100000000;
};

template<>
struct RadioDxTrait<agile_device::capability::kUnitNs>
{
	static constexpr std::intmax_t value = 1000000000;
};
//std::chrono::milliseconds
typedef std::chrono::duration<long long, std::ratio<1, RadioDxTrait<agile_device::capability::kUnit100ns>::value>> nanoseconds_100;//100ns
typedef std::chrono::duration<long long, std::ratio<1, RadioDxTrait<agile_device::capability::kUnit10ns>::value>> nanoseconds_10;//10ns


/**
*@brief 自定义MutexLock，参照QMutexLock编写，增加信息输出，便于排查死锁问题
**/
class MyMutexLock
{
public:
	inline explicit MyMutexLock(QBasicMutex *m, const QString & func_name, bool bout_put_info = false) QT_MUTEX_LOCK_NOEXCEPT
	{
		func_name_ = func_name;
		bout_put_info_ = bout_put_info;

		Q_ASSERT_X((reinterpret_cast<quintptr>(m) & quintptr(1u)) == quintptr(0),
			"QMutexLocker", "QMutex pointer is misaligned");
		val = quintptr(m);
		if (Q_LIKELY(m)) {
			// call QMutex::lock() instead of QBasicMutex::lock()
			static_cast<QMutex *>(m)->lock();
			val |= 1;

			if (bout_put_info_)
			{
				std::cout<<"MyMutexLock lock : " << func_name_.toStdString().c_str()<<std::endl;
			}
		}
	}
	inline ~MyMutexLock() { unlock(); }

	inline void unlock() Q_DECL_NOTHROW
	{
		if ((val & quintptr(1u)) == quintptr(1u)) {
			val &= ~quintptr(1u);
			mutex()->unlock();

			if (bout_put_info_)
			{
				std::cout<<"MyMutexLock unlock :" << func_name_.toStdString().c_str() << std::endl;
			}
		}
	}

	inline void relock() QT_MUTEX_LOCK_NOEXCEPT
	{
		if (val) {
			if ((val & quintptr(1u)) == quintptr(0u)) {
				mutex()->lock();
				val |= quintptr(1u);
			}
		}
	}

#if defined(Q_CC_MSVC)
#pragma warning( push )
#pragma warning( disable : 4312 ) // ignoring the warning from /Wp64
#endif

	inline QMutex *mutex() const
	{
		return reinterpret_cast<QMutex *>(val & ~quintptr(1u));
	}

#if defined(Q_CC_MSVC)
#pragma warning( pop )
#endif

private:
	Q_DISABLE_COPY(MyMutexLock)

	quintptr val;
	QString func_name_;
	bool bout_put_info_{ false };
};

#define LOCK_TEST(val) MyMutexLock locked(val,__FUNCTION__, false)

int Device::ImageBitsChange(int value, int srcbit, int dstbit)
{
	return std::round(value / (pow(2, srcbit) - 1)*(pow(2, dstbit) - 1));
}

Device::Device(const HscDeviceInfo & info, QObject *parent) : QObject(parent), info_(info), image_processor_ptr_(new ImageProcessor(this))
{
	initComputerIpList();

	using AgUnit = agile_device::capability::Units;
	qRegisterMetaTypeStreamOperators<CameraWindowRect>("CameraWindowRect");
	qRegisterMetaTypeStreamOperators<HscIntelligentTriggerParamV2>("HscIntelligentTriggerParamV2");
	qRegisterMetaTypeStreamOperators<HscAutoExposureParameter>("HscAutoExposureParameter");
	qRegisterMetaTypeStreamOperators<HscImageCtrlParam>("HscImageCtrlParam");
	qRegisterMetaTypeStreamOperators<HscEDRParam>("HscEDRParam");
	qRegisterMetaTypeStreamOperators<HscHisiManualAwb>("HscHisiManualAwb");
	qRegisterMetaTypeStreamOperators<HscLaserParam>("HscLaserParam");
	qRegisterMetaTypeStreamOperators<HscPIVParam>("HscPIVParam");
	qRegisterMetaTypeStreamOperators<HscEDRParamV2>("HscEDRParamV2");
	qRegisterMetaType<RccFrameInfo>("RccFrameInfo");

	time_unit_ratio_map_.insert(AgUnit::kUnitSecs, 1);
	time_unit_ratio_map_.insert(AgUnit::kUnitMs, 1E3);
	time_unit_ratio_map_.insert(AgUnit::kUnitUs, 1E6);
	time_unit_ratio_map_.insert(AgUnit::kUnit100ns, 1E7);
	time_unit_ratio_map_.insert(AgUnit::kUnit10ns, 1E8);
	time_unit_ratio_map_.insert(AgUnit::kUnitNs, 1E9);

	initFunctions();

	msg_listener_ptr_.reset(new MsgListener(this));

#ifdef CSRCCAPP

#else

	rma_image_ptr_.reset(new RMAImage);
#endif //CSRCCAPP

	realtime_frame_timer_ptr_ = new QTimer();
#if TEST_THREAD_PROCESS_FRAME
	realtime_frame_timer_ptr_->setInterval(90);
#else
	realtime_frame_timer_ptr_->setInterval(100);
#endif

	QObject::connect(realtime_frame_timer_ptr_.data(), &QTimer::timeout, this, &Device::updateRealtimeFrame);

#ifdef CSRCCAPP

#else

	realtime_player_controller_ptr_.reset(new RealtimePlayerController(getIp()));
#endif //CSRCCAPP

	QObject::connect(this, &Device::startedUpdateRealtimeFrameTimer, this, &Device::startUpdateRealtimeFrame);
	QObject::connect(this, &Device::propertyChanged, this, &Device::onPropertyChanged);

	// 自动重连
	reconnected_timer_ptr_ = new QTimer();
	reconnected_timer_ptr_->setInterval(1000);//1s刷新一次
	QObject::connect(reconnected_timer_ptr_, &QTimer::timeout, this, &Device::updateReconnectOsd);
	QObject::connect(this, &Device::reconnectStart, this, [this] {
		reconnect_count_down_ = kReconnectTimeout_ / 1000; // 倒计时单位为s
		reconnected_timer_ptr_->start();
	});
	QObject::connect(this, &Device::reconnectFinished, reconnected_timer_ptr_, &QTimer::stop);

	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		//健康管理计时器
		m_health_record_timer_ptr = new QTimer();
		m_health_record_timer_ptr->setInterval(kUpdateHealthRecordInterval);
		QObject::connect(m_health_record_timer_ptr, &QTimer::timeout, this, &Device::doUpdateHealthRecord);
		m_health_record_timer_ptr->start();

		//开机工作记录计时器
		m_boot_and_work_record_timer_ptr = new QTimer();
		m_boot_and_work_record_timer_ptr->setInterval(kUpdateCurBootTimeAndTotalWorkTimeInterval);
		QObject::connect(m_boot_and_work_record_timer_ptr, &QTimer::timeout, this, &Device::doUpdateCurBootTimeAndTotalWorkTime);
	}


	//温度数据
	if (temperature_buf_ptr_ == nullptr)
	{
		temperature_buf_ptr_ = new char[kTemperatureBufLen];
	}
	qRegisterMetaType<uint8_t>("uint8_t");
	qRegisterMetaType<uint32_t>("uint32_t");
}

Device::~Device()
{
	//取消注册
	//HscUnRegister(device_handle_, msg_listener_ptr_.get());

	//温度数据
	if (temperature_buf_ptr_ != nullptr)
	{
		delete[] temperature_buf_ptr_;
	}
	if (m_bits != nullptr)
	{
		delete []m_bits;
		m_bits = nullptr;
	}
	if (m_gain_range != nullptr)
	{
		delete []m_gain_range;
		m_gain_range = nullptr;
	}
	if (m_analog_gain_values != nullptr)
	{
		delete[]m_analog_gain_values;
		m_analog_gain_values = nullptr;
	}
	

	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		m_health_record_timer_ptr->stop();
	}

	exitMsgProcessThread();
	exitCF18HeartBeatThread();
#if TEST_THREAD_PROCESS_FRAME
	exitRccImageProcessThread();
#endif
	m_destory = true;
}

bool Device::allowEditRoi() const
{
	return getState() == Previewing || getState() == Connected;
}

bool Device::connected() const
{
	return (getState() != Unconnected) && (getState() != Disconnected) && (getState() != Connecting) && (getState() != Disconnecting) && (getState() != Reconnecting);
}

void Device::suspend()
{
	previous_state_ = getState();

	if (previous_state_ == DeviceState::Previewing || previous_state_ == DeviceState::Acquiring)
	{
		doStop();
	}
}

void Device::resume()
{
	if (previous_state_ == DeviceState::Previewing)
	{
		doPreview();
		emit startedUpdateRealtimeFrameTimer();
	}
	else if (previous_state_ == DeviceState::Acquiring)
	{
		doAcquire();
		emit startedUpdateRealtimeFrameTimer();
	}
}

void Device::connect()
{
	std::thread(&Device::doConnect, this).detach();
}

void Device::refreshOSDAfterConnect()
{
	if (state_ == DeviceState::Connected)
	{
#ifdef CSRCCAPP
		updateViewOSD();
#else

		// 已连接时，仅刷新OSD
		RMAImage rma_image;
		rma_image.SetOSD(makeRMAImageOSDInfo());

		emit realtime_player_controller_ptr_->sigImageReady(rma_image);
#endif //CSRCCAPP
	}
}

bool Device::allowsPreview() const
{
    DeviceState state = getState();
    return ((state == DeviceState::Connected) || (state == DeviceState::Acquiring) || (state == DeviceState::Recording));
}

void Device::preview(bool bAutoConnect)
{
	m_bAutoConnect = bAutoConnect;
	PreviewThread *thread_ptr = new PreviewThread(this);
	QObject::connect(thread_ptr, &QThread::finished, this, &Device::startUpdateRealtimeFrame);
	QObject::connect(thread_ptr, &QThread::finished, thread_ptr, &QObject::deleteLater);
	thread_ptr->start();
}

bool Device::allowsAcquire() const
{
    DeviceState state = getState();
    return ((state == DeviceState::Connected) || (state == DeviceState::Previewing));
}

void Device::acquire()
{
	AcquireThread *thread_ptr = new AcquireThread(this);
	QObject::connect(thread_ptr, &QThread::finished, this, &Device::startUpdateRealtimeFrame);
	QObject::connect(thread_ptr, &QThread::finished, thread_ptr, &QObject::deleteLater);
	thread_ptr->start();
}

bool Device::allowsTrigger() const
{
    DeviceState state = getState();
    return (state == DeviceState::Acquiring);
}

void Device::trigger()
{
	
	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscResult res = triggerDevice();

    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
    }
}

void Device::stopCapture(bool save_video_clip, bool next_acquiring_enabled)
{
	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	HscResult res = stopCaptureDevice(save_video_clip, next_acquiring_enabled);

	CSLOG_INFO("stopCaptureDevice finishend, ret = {}",res);

	if (res != HSC_OK)
	{
		emit errorOccurred(res, m_bShowTip);
	}
}

void Device::finishRecording(int vid, bool berforeStopRecing)
{
	std::thread(&Device::doFinishRecording, this, vid, berforeStopRecing).detach();
}

bool Device::allowsStop() const
{
    DeviceState state = getState();
    return ((state == DeviceState::Previewing) || (state == DeviceState::Acquiring) || (state == DeviceState::Recording));
}

void Device::stop(uint8_t mode)
{
    std::thread(&Device::doStop, this, mode).detach();
}

void Device::triggerStart()
{
	if (!IsTrigger()) 
	{
		return;
	}
	HscResult res = HscTriggerStart(device_handle_);
	if (res != HSC_OK)
	{
		emit errorOccurred(res, m_bShowTip);
	}
	return;

}

void Device::triggerStop()
{
	if (!IsTrigger())
	{
		return;
	}
	HscResult res = HscTriggerStop(device_handle_);
	if (res != HSC_OK)
	{
		emit errorOccurred(res, m_bShowTip);
	}
	return;

}

void Device::disconnect(bool b_wait)
{	
    std::thread th(&Device::doDisconnect, this);

	if (b_wait)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
	else
	{
		th.detach();
	}
}

bool Device::allowsDisconnect() const
{
	return state_ == Connected || state_ == Previewing || state_ == Acquiring;
}

bool Device::IsCamera() const
{
	return !DeviceUtils::IsTrigger(getModel());

}

bool Device::IsTrigger() const
{
	return DeviceUtils::IsTrigger(getModel());

}

bool Device::IsCF18() const
{
	return FunctionCustomizer::GetInstance().isCF18ControlSupported()&DeviceUtils::IsCF18(getModel());
}

bool Device::isRoot() const
{
	return info_.previous_ip[0] == 0 || info_.dev_connect_method == HScDevConnectMethod::CM_USB;
}

QString Device::getModelName() const
{
    QString model_name = QString::fromLatin1(info_.model_name);
//     if (model_name.isEmpty())
//     {
//         model_name = "unknown";
//     }

    return model_name;
}

void Device::setModelName(const QString &strModelName) 
{
	if (strModelName.compare("unknown", Qt::CaseInsensitive) == 0)
	{
		return;
	}
	if (strModelName.length() > sizeof(info_.model_name))
	{
		return;
	}
	memcpy(info_.model_name, strModelName.toUtf8().data(), strModelName.toUtf8().length());
}

void Device::setIp(const QString &ip)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscDeviceNetParam net_param{};
    HscResult res = HscGetDeviceNetConfig(device_handle_, &net_param);
    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
        return;
    }

    QHostAddress host_addr(ip);
    quint32 ipv4 = host_addr.toIPv4Address();
    net_param.ipAddr[0] = (ipv4 & 0xff000000) >> 24;
    net_param.ipAddr[1] = (ipv4 & 0x00ff0000) >> 16;
    net_param.ipAddr[2] = (ipv4 & 0x0000ff00) >> 8;
    net_param.ipAddr[3] = (ipv4 & 0x000000ff);

    res = HscSetDeviceNetConfig(device_handle_, net_param);
    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
    }
}

QString Device::getIpOrSn() const
{
	if (info_.dev_connect_method == HScDevConnectMethod::CM_USB ||
		info_.dev_connect_method == HScDevConnectMethod::CM_PCI_E) {
		return getSn();
	}
	else {
		return getIp();
	}
}

QString Device::getIp() const
{
    return QString::fromLatin1(info_.ip);
}

QString Device::getSn() const
{
	return QString::fromLatin1(info_.serial_num);
}

QString Device::getDescription() const
{
	QString deviceType = UIExplorer::instance().getStringById(IsTrigger() ? "STRID_DT_TRIGGER" : "STRID_DT_CAMERA");
	QString deviceName = getProperty(PropName).toString();
	QString ipOrSn = getIpOrSn();

	if (deviceName.isEmpty())
	{
		return "[" + ipOrSn + "]";
	}
	else
	{
		return deviceName + "[" + ipOrSn + "]";
	}
}

HscResult Device::setLicensePath(const QString &path)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscResult res = HscSetAuthorization(device_handle_, path.toLocal8Bit().data());
	return res;
}

void Device::getLicense(DeviceLicense & license) const
{
    HscAuthInfo auth_info{};
    HscResult res = HscGetAuthInfo(device_handle_, &auth_info);
    if (res != HSC_OK)
    {
        return;
    }

    license.mode = auth_info.authMode;
    license.state = auth_info.authType;
    license.remainder = auth_info.remainingAuthTime;
}

void Device::setProperty(Device::PropType type, const QVariant &value)
{
	DeviceState cur_state = getState();//切换状态时不可设置属性,避免界面锁死
	if (cur_state == Connecting ||
		cur_state == ToAcquire || 
		cur_state == ToExport || 
		cur_state == ToPreview || 
		cur_state == Disconnecting ||
		cur_state == Reconnecting)
	{
		return;
	}
	HscResult res = HSC_OK;
	{
		LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);	
		res = setProperty2Device(type, value);
	}
    if (res != HSC_OK 
		&& cur_state != DeviceState::Unconnected
		&& cur_state != DeviceState::Disconnected
		&& cur_state != DeviceState::Connecting
		&& cur_state != DeviceState::Reconnecting
		&& cur_state != DeviceState::Disconnecting)
    {
		CSLOG_ERROR("Illegal parameters, device_type:{},return:{}", type,res);
        emit errorOccurred(res, m_bShowTip);
    }
	else
	{
		emit propertyChanged(type, value);
	}
}

QVariant Device::getProperty(PropType type,bool from_local) const
{
	if (IsCF18()) {
		if (Connected == state_){
			auto iter = map_cf18_prop_get_.find(type);
			if (iter == map_cf18_prop_get_.end())
			{
				return QVariant();
			}
			QElapsedTimer timer;//调试:判断获取时间超时的参数
			timer.start();
			
			QVariant value;
			HscResult res = iter->second(value, from_local);
			if (timer.hasExpired(200))
			{
				QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
				CSLOG_ERROR("getProperty time out:{}ms, propKey:{}", timer.elapsed(), QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type)).toStdString());
			}
			
			if (res != HSC_OK)
			{
				return QVariant();
			}
			
			return value;
		}
		return QVariant();
	}

	//非连接状态直接从本地获取
	if (state_ == Connecting || state_ == Disconnected || state_ == Disconnecting || state_ == Unconnected)
	{
		from_local = true;
	}
	
    auto iter = map_prop_get_.find(type);
    if (iter == map_prop_get_.end())
    {
        return QVariant();
    }
	QElapsedTimer timer;//调试:判断获取时间超时的参数
	timer.start();

    QVariant value;
    HscResult res = iter->second(value,from_local);
	if (timer.hasExpired(200))
	{
		QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
		CSLOG_ERROR("getProperty time out:{}ms, propKey:{}",timer.elapsed(), QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type)).toStdString());
	}
	
    if (res != HSC_OK)
    {
        return QVariant();
    }

    return value;
}

void Device::getTypicalProperties(Device::PropType type, QVariantList &values, bool need_costom) const
{
    switch (type)
    {
    case PropRoi:
        getTypicalResolutions(values,need_costom);
        break;
    case PropFrameRate:
        getTypicalFrameRates(values);
        break;
    case PropExposureTime:
        getTypicalExposureTimes(values);
        break;
    }
}

QList<Device::PropType> Device::getSupportedPropertyTypes(Device* device_ptr) const
{
	if (IsCamera())
	{
		QList<PropType> device_prop_types{
			PropStreamType, // 协议格式 ,协议格式影响其他参数,放在第一个设置
			PropRoi, // ROI
			PropRoiVisible, // ROI可见
			PropBlackFieldEnable, // 暗场校正使能
			PropLut,// Lut
			PropDigitalGain,// 数字增益:Lut的第一个y值
			PropWbEnv,//拍摄环境
			PropLuminance,// 亮度
			PropContrast,// 对比度
			PropFrameRate, // 帧率
			PropExposureTimeUnit,//曝光时间单位
			PropExposureTime, // 曝光时间
			PropOverExposureTipEnable, // 过曝提示使能
			PropSyncSource, // 同步方式
			PropTriggerMode, // 触发方式
			PropWatermarkEnable, // 叠加水印
			PropPulseWidth, //同步脉冲宽度
			PropRecordingUnit, // 保存单位
			PropRecordingOffsetMode, // 保存起点方式
			PropRecordingOffset, // 保存起点
			PropRecordingLength, // 保存长度
			PropVideoFormat, // 视频格式
			PropRotateType, // 旋转类型
		};
		//模拟增益
		if (device_capability_.supportAnalogGain && (!device_ptr || device_ptr->IsAnalogGainSupported()))
		{
			device_prop_types << PropAnalogGain;
		}
		//自动曝光
		if (device_capability_.supportAutoExposure && (!device_ptr || device_ptr->IsAutoExposureSupported()))
		{
			device_prop_types << PropAutoExposure;
		}
		//智能触发
		if (device_capability_.imgTrigCapability && (!device_ptr || device_ptr->IsIntelligentTriggerSupported()))
		{
			device_prop_types << PropIntelligentTrigger;
		}
		//色彩模式
		if (device_capability_.colorCapability && (!device_ptr || device_ptr->isColorSupported()))
		{
			device_prop_types << PropDisplayMode;
		}
		//二次曝光
		if (device_capability_.supportEDR && (!device_ptr || device_ptr->IsEdrExposureSupported()))
		{
			if (IsIntelligentTriggerV4Supported()||isSupportHighBitParam()) {
				device_prop_types << PropEdrExposureV2;
			}
			else {
				device_prop_types << PropEdrExposure;
			}
		}
		//外触发方式
		if (device_capability_.supportExternalTriggerMode && (!device_ptr || device_ptr->IsExternalTriggerModeSupported()))
		{
			device_prop_types << PropExTriggerMode;
		}
		// 触发同步,会影响消抖长度取值范围,需要放在其前面
		if (IsTriggerSyncSupported() && (!device_ptr || device_ptr->IsTriggerSyncSupported()))
		{
			device_prop_types << PropTriggerSyncEnable;
		}
		//消抖长度
		if (m_support_jitter_elimination_length && (!device_ptr || device_ptr->IsJitterEliminationLengthSupported()))
		{
			device_prop_types << PropJitterEliminationLength;
		}
		//角度数据矫正
		if (IsDataCorrectionSupported() && (!device_ptr || device_ptr->IsDataCorrectionSupported()))
		{
			device_prop_types << PropAngleDataCorrection;
		}
		//piv
		if (IsPIVSupported() && (!device_ptr || device_ptr->IsPIVSupported()))
		{
			device_prop_types << PropPivParam;
		}
		//白平衡参数
		if (isSupportManualWhiteBalanceMode() && (!device_ptr || device_ptr->isSupportManualWhiteBalanceMode()))
		{
			device_prop_types << PropWhiteBalance;
		}
		// 数字增益
		if (IsSupportCMOSDigitGain() && (!device_ptr || device_ptr->IsSupportCMOSDigitGain()))
		{
			device_prop_types << PropCmosDigitalVal;
		}
		//像素位深
		if (isSupportHighBitParam() && (!device_ptr || device_ptr->isSupportHighBitParam()))
		{
			device_prop_types << PropPixelBitDepth;
			device_prop_types << PropDisplayBitDepth;
		}
		//sdi,固件参数复制存在问题,暂时取消复制
// 		if (IsSdiCtrlSupported() && (!device_ptr || device_ptr->IsSdiCtrlSupported()))
// 		{
// 			device_prop_types << PropSdiFpsResols;
// 		}

		return device_prop_types;
	}
	else
	{
		QList<PropType> device_prop_types{
			PropFrameRate, // 帧率
			PropSyncSource, // 同步方式
			PropTriggerMode, // 触发方式
			PropPulseWidth, //同步脉冲宽度
			PropChn1Delay, //通道1延时
			PropChn2Delay, //通道2延时
			PropChn3Delay, //通道3延时
			PropChn4Delay, //通道4延时
			PropChn5Delay, //通道5延时
			PropChn6Delay, //通道6延时
			PropChn7Delay, //通道7延时
			PropChn8Delay, //通道8延时
			PropRecordingOffsetMode, // 保存起点方式
			PropRecordingOffset, // 保存起点
			PropRecordingLength, // 保存长度
			PropRecordingUnit, // 保存单位
		};
		//外触发方式
		if (device_capability_.supportExternalTriggerMode)
		{
			device_prop_types << PropExTriggerMode;
		}


		return device_prop_types;
	}
}

QList<Device::Device::PropType> Device::getSavingPropertyTypes() const
{
	QList<PropType> device_save_types;
	device_save_types << getSupportedPropertyTypes(nullptr);//包含全部支持的属性种类

	QList<PropType> common_types
	{
		PropParentIp, // 父设备IP
		PropName, // 设备名称
		PropDeviceIndex, // 设备序号
		PropOsdVisible, // OSD可见
		PropFocusPoint, // 焦点
		PropFocusPointVisible, // 焦点可见
		PropImageCtrl,//图像控制，镜像、倒像、翻转
		PropTemperaturePanelEnable,//温控列表使能
	}; 


	device_save_types << common_types;

	return device_save_types;
}

QList<Device::Device::PropType> Device::getAcquirePropertyTypes() const
{

	QList<PropType> device_prop_types{
		PropRoi, // ROI
		PropFrameRate, // 帧率
		PropExposureTime, // 曝光时间
		PropExposureTimeUnit,//曝光时间单位
		PropRecordingOffsetMode, // 保存起点方式
		PropRecordingOffset, // 保存起点
		PropRecordingLength, // 保存长度
		PropRecordingUnit, // 保存单位
	};
	return device_prop_types;
}

HscResult Device::reloadPropertiesFromLocal()
{
	HscResult res = HSC_OK;

	QList<PropType> device_prop_types;

	device_prop_types << Device::PropType::PropName;
	device_prop_types << getSupportedPropertyTypes(this);

	for (auto type : device_prop_types)
	{
		QVariant value = getProperty(type, true);
		res = setProperty2Device(type, value);
		if (res != HSC_OK)
		{
			emit errorOccurred(res, m_bShowTip);
			break;
		}
		emit propertyChanged(type, value);
	}

	return res;
}

void Device::getSupportedProperties(Device::PropType type, QVariantList &values) const
{
    auto iter = map_prop_get_supported_.find(type);
    if (iter == map_prop_get_supported_.end())
    {
        return;
    }

    iter->second(values);
}

void Device::GetSupportedVideoFormats(StreamType stream_type, QList<VideoFormat> & video_formats) 
{
	if (stream_type == TYPE_RAW8)
	{
		video_formats.push_back(VIDEO_RHVD);
	}

	video_formats.push_back(VIDEO_AVI);
	if (stream_type != TYPE_H264)
	{
		video_formats.push_back(VIDEO_MP4);

		video_formats.push_back(VIDEO_BMP);
		video_formats.push_back(VIDEO_JPG);
		video_formats.push_back(VIDEO_TIF);
		video_formats.push_back(VIDEO_PNG);
	}
}

void Device::GetSupportedDisplayModes(StreamType stream_type, QList<HscDisplayMode> & display_modes) 
{
	if (stream_type == TYPE_RAW8 || stream_type == TYPE_YUV420)
	{
		display_modes.push_back(HSC_DISPLAY_MONO);
		if (IsColorSupported())
		{
			display_modes.push_back(HSC_DISPLAY_COLOR);
		}
	}
	else
	{
		if (IsColorSupported())
		{
			display_modes.push_back(HSC_DISPLAY_COLOR);
		}
		else
		{
			display_modes.push_back(HSC_DISPLAY_MONO);
		}
	}
}

void Device::getPropertyRange(Device::PropType type, qint64 &min, qint64 &max, qint64 &inc) const
{
	if (IsCF18()) {
		auto iter = map_cf18_prop_get_range_.find(type);
		if (iter == map_cf18_prop_get_range_.end())
		{
			return;
		}

		iter->second(min, max, inc);
	}
	else {
		auto iter = map_prop_get_range_.find(type);
		if (iter == map_prop_get_range_.end())
		{
			return;
		}

		iter->second(min, max, inc);
	}
}

DeviceState Device::getState() const
{
    return state_;
}

QString Device::getStateStr() const
{
    return toStateStr(getState());
}

QString Device::getFormattedStateStr() const
{
	DeviceState state = getState();
	return getFormattedStateStr(toStateStr(state));
}

QString Device::getFormattedStateStr(const QString & str) const
{
	DeviceState state = getState();
	QString color;
	if (state == DeviceState::Unconnected  ||
		state == DeviceState::Disconnected || 
		state == DeviceState::Connecting || 
		state == DeviceState::Disconnecting ||
		state == DeviceState::Reconnecting)
	{
		color = "#FF0000";
	}
	else
	{
		color = "#00FF00";
	}

	return QString("<font color=%1>%2</font>").arg(color).arg(str);
}

QColor Device::getStateStrColor() const
{
	DeviceState state = getState();
	QColor color;
	if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::Connecting ||
		state == DeviceState::Disconnecting ||
		state == DeviceState::Reconnecting)
	{
		color = QColor(255, 0, 0);
	}
	else
	{
		color = QColor(0, 255, 0);
	}
	return color;
}

QString Device::toStateStr(DeviceState state)
{
	QString str;
    switch (state)
    {
    case Unconnected: // 未连接
        str = tr("Unconnected");
        break;
	case Disconnected: // 已断开
		str = tr("Disconnected");
		break;
    case Connecting: // 正在连接
        str = tr("Connecting");
        break;
    case Connected: // 已连接
        str = tr("Connected");
        break;
    case ToPreview: // 正在切换预览
        str = tr("Preview being switched");
        break;
    case Previewing: // 预览中
        str = tr("Previewing");
        break;
    case ToAcquire: // 正在切换采集
        str = tr("Acquisition being switched");
        break;
    case Acquiring: // 采集中
        str = tr("Acquiring");
        break;
    case Recording: // 录制中
        str = tr("Recording");
        break;
    case ToReplay: // 正在切换回放
        str = tr("Replay being switched");
        break;
    case Replaying: // 回放中
        str = tr("Replaying");
        break;
    case ToExport: // 正在切换导出
        str = tr("Export being switched");
        break;
    case Exporting: // 导出中
        str = tr("Exporting");
        break;
    case Disconnecting: // 正在断开
        str = tr("Disconnecting");
        break;
	case Reconnecting: // 正在重连
		str = tr("Reconnecting");
		break;
	case StandBy: // 待机
		str = tr("StandBy");
		break;
	case RecoveringData: // 正在断电数据恢复
		str = tr("RecoveringData");
		break;
	case ToWakeup: // 唤醒
		str = tr("ToWakeup");
		break;
    }

    return str;
}

void Device::setParent(QSharedPointer<Device> parent)
{
    parent_wptr_ = parent;

    // 保存配置
    QString parent_ip = parent ? parent->getIpOrSn() : "";
    setProperty(PropType::PropParentIp, parent_ip);
}

QSharedPointer<Device> Device::getParent() const
{
    return parent_wptr_.lock();
}

bool Device::allowsImageTraining() const
{
	DeviceState state = getState();
	if (device_capability_.supportManualImageTraining)
	{
		if (state == Previewing || state == Acquiring)
		{
			return true;
		}
	}
	return false;
}

bool Device::isImagingTrainingEnabled() const
{
	return m_enable_image_training;
}

void Device::enableImageTraining(bool enable)
{
	LOCK_TEST(&mutex_);
	HscResult res = HscEnableTestModel(device_handle_, enable);
	if (res == HSC_OK)
	{
		if (m_enable_image_training != enable)
		{
			m_enable_image_training = enable;

			// 退出训练模式时，做一次停机，重新进入预览或高采，目的是规避训练后出现分屏现象。
			if (!enable)
			{
				if (getState() == Previewing)
				{
					res = stopDevice(true);
					if (res != HSC_OK)
					{
						disconnectDevice(false);
						return;
					}

					res = previewDevice();
					emit startedUpdateRealtimeFrameTimer();
				}
				else if (getState() == Acquiring)
				{
					res = stopDevice(true);
					if (res != HSC_OK)
					{
						disconnectDevice(false);
						return ;
					}

					acquireDevice();
					emit startedUpdateRealtimeFrameTimer();

				}
			}
		}
	}

}

void Device::mainBoardPhaseAdjust()
{
	LOCK_TEST(&mutex_);
	HscMainBoardPhaseAdjust(device_handle_);
}

void Device::slvPhaseAdjust()
{
	LOCK_TEST(&mutex_);
	HscSlvPhaseAdjust(device_handle_);
}

int Device::SystemSelfCheck(ErrorInfo error_info[MAX_SYSTEM_SELF_CHECK_COUNT])
{
	LOCK_TEST(&mutex_);

	return HscSystemSelfCheck(device_handle_, error_info);
}

bool Device::isGetParamsFromDevice() const
{
	return settingsNeedsLoadFromDevice();
}

bool Device::AllowsSetCameraSettings() const
{
	if (IsCamera())
	{
		if (getState() == Connected || getState() == Previewing)
		{
			return true;
		}
	}
	return false;
}

HscResult Device::CopyCameraSettingsFrom(QSharedPointer<Device> device_ptr)
{
	//参数拷贝操作
	//判断是否支持
	if (device_ptr.isNull())
	{
		emit errorOccurred(HSC_DEVICE_NOT_FOUND);
		return HSC_DEVICE_NOT_FOUND;
	}

	//获取被拷贝参数,应用到设备
	QList<PropType> prop_list = device_ptr->getSupportedPropertyTypes(this);

	for (auto prop_type:prop_list)
	{
		if (PropDisplayMode == prop_type)
		{
			QList<HscDisplayMode> display_modes;
			GetSupportedDisplayModes(getProperty(Device::PropStreamType).value<StreamType>(), display_modes);
			HscDisplayMode mode = device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>();
			bool bCopyEnable = false;
			for (auto m:display_modes)
			{
				if (m == mode)
				{
					bCopyEnable = true;
					break;
				}
			}
			if (!bCopyEnable)
			{
				continue;
			}
		}
		this->blockSignals(true);
		setProperty(prop_type, device_ptr->getProperty(prop_type));
		this->blockSignals(false);
	}

	// [2022/9/29 rgq]: 添加刷新界面信息
	emit SignalUpdateUIInfo();
	return HSC_OK;
}

HscResult Device::refreshCurrentDeviceSettings()
{
	QSettings setting_file(SystemSettingsManager::instance().getCurrentDeviceParamDir(), QSettings::Format::IniFormat);
	QMap<PropType, QString>file_prop_map{};

	for (auto key : setting_file.allKeys())
	{
		if (key.contains("/")) {
			QStringList ip_list = key.split("/");
			if (ip_list.size() > 1) {
				QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
				Device::PropType prop_type = (Device::PropType)meta_enum.keysToValue(ip_list[1].toStdString().c_str());
				if (PropWhiteBalance == prop_type) {
					if (isColorSupported()) {
						file_prop_map.insert(prop_type, key);
					}
				}
				else {
					file_prop_map.insert(prop_type, key);
				}
			}
		}
	}


	//获取被拷贝参数,应用到设备
	QList<PropType> prop_list = getSupportedPropertyTypes(this);
	for (auto prop_type : prop_list)
	{
		if (!file_prop_map.keys().contains(prop_type)) {
			continue;
		}

		if (PropDisplayMode == prop_type)
		{
			QList<HscDisplayMode> display_modes;
			GetSupportedDisplayModes(getProperty(Device::PropStreamType).value<StreamType>(), display_modes);
			HscDisplayMode mode = (HscDisplayMode)setting_file.value(file_prop_map[prop_type]).toInt();
			bool bCopyEnable = false;
			for (auto m : display_modes)
			{
				if (m == mode)
				{
					bCopyEnable = true;
					break;
				}
			}
			if (!bCopyEnable)
			{
				continue;
			}
		}

		this->blockSignals(true);
		if (PropExposureTime == prop_type) {
			DWORD exposure_time = ConvertExposureTime(QVariant::fromValue(setting_file.value(file_prop_map[prop_type])).toLongLong(), GetRealExposureTimeUnit(),
				getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(),
				getProperty(PropFrameRate).toInt());
			setProperty(prop_type, QVariant::fromValue(exposure_time));
		}
		else if (PropEdrExposure == prop_type) {
			HscEDRParam param = qvariant_cast<HscEDRParam>(QVariant::fromValue(setting_file.value(file_prop_map[PropEdrExposure])));
		
			auto exposure_time = ConvertExposureTime(param.doubleExposureTime, GetRealExposureTimeUnit(), \
				getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), \
				getProperty(PropFrameRate).toInt());
			param.doubleExposureTime = exposure_time;
			setProperty(prop_type, QVariant::fromValue(param));
		}
		else {
			setProperty(prop_type, QVariant::fromValue(setting_file.value(file_prop_map[prop_type])));
		}
		this->blockSignals(false);
	}

	// [2022/9/29 rgq]: 添加刷新界面信息
	emit SignalUpdateUIInfo();
	return HSC_OK;
}

bool Device::IsPixelDepthDifferent()
{
	QSettings setting_file(SystemSettingsManager::instance().getCurrentDeviceParamDir(), QSettings::Format::IniFormat);
	QMap<PropType, QString>file_prop_map{};

	for (auto key : setting_file.allKeys())
	{
		if (key.contains("/")) {
			QStringList ip_list = key.split("/");
			if (ip_list.size() > 1) {
				QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
				Device::PropType prop_type = (Device::PropType)meta_enum.keysToValue(ip_list[1].toStdString().c_str());
				if (PropWhiteBalance == prop_type) {
					if (isColorSupported()) {
						file_prop_map.insert(prop_type, key);
					}
				}
				else {
					file_prop_map.insert(prop_type, key);
				}
			}
		}
	}

	auto local = QVariant::fromValue(setting_file.value(file_prop_map[PropPixelBitDepth]));
	return getProperty(PropPixelBitDepth) != local;

}

bool Device::IsDataCorrectionSupported() const
{
	return FunctionCustomizer::GetInstance().isXiguangsuoVersion();
}

bool Device::AllowsEditDataCorrectionFrameOffset(StreamType stream_type) const
{
	if (!IsDataCorrectionSupported())
	{
		return false;
	}

	if (stream_type != TYPE_RAW8 && stream_type != TYPE_RGB8888)
	{
		return false;
	}

	return true;
}

HscResult Device::setColorCorrectInfo(HscColorCorrectInfo info)
{
	return HscSetColorCorrectInfo(device_handle_, info);
}

HscResult Device::getColorCorrectInfo(HscColorCorrectInfo &info) const
{
	return HscGetColorCorrectInfo(device_handle_, &info);
}

bool Device::allowsEditWbMode() const
{
	if (allowsEditArmWbMode())
	{
		return true;
	}

	if (!isColorSupported())
	{
		return false;
	}

	StreamType stream_type = getProperty(Device::PropStreamType).value<StreamType>();
	if (stream_type == TYPE_YUV420 || stream_type == TYPE_H264)
	{
		return false;
	}

	return getProperty(Device::PropDisplayMode).value<HscDisplayMode>() == HSC_DISPLAY_COLOR;
}

bool Device::allowsEditArmWbMode() const
{
	if (!isArmWbSupported())
	{
		return false;
	}
	//yuv和h264支持arm白平衡
	StreamType stream_type = getProperty(Device::PropStreamType).value<StreamType>();
	return (stream_type == TYPE_YUV420 || stream_type == TYPE_H264);

}

HscWhiteBalanceMode Device::GetWbMode() const
{
	if (isArmWbSupported())
	{
		// 支持ARM白平衡参数设置时，YUV和H264下，返回ARM白平衡模式
		StreamType stream_type = getProperty(Device::PropStreamType).value<StreamType>();
		if (stream_type == TYPE_YUV420 || stream_type == TYPE_H264)
		{
			uint8_t wb_mode = 0;
			GetArmWbMode(wb_mode);
			return (wb_mode == 0) ? HSC_WB_AUTO_GAIN_FROM_SOFTWARE : HSC_WB_MANUAL_GAIN;
		}
	}

	if (isGetParamsFromDevice()) // 白平衡参数存储在设备侧
	{
		HscColorCorrectInfo info;
		getColorCorrectInfo(info);
		return info.awb_mode_;
	}
	else // 白平衡参数存储在软件侧
	{
		std::shared_ptr<ImageProcessor> pBufferProcessor = getProcessor();
		if (pBufferProcessor != nullptr)
		{
			return pBufferProcessor->getWhiteBalanceMode();
		}
	}

	return HSC_WB_NONE;
}

HscResult Device::ApplyWbMode(HscWhiteBalanceMode wb_mode)
{
	if (isArmWbSupported())
	{
		// 支持ARM白平衡参数设置时，YUV和H264下，设置ARM白平衡模式
		StreamType stream_type = getProperty(Device::PropStreamType).value<StreamType>();
		if (stream_type == TYPE_YUV420 || stream_type == TYPE_H264)
		{
			uint8_t mode = 0;
			if (wb_mode == HSC_WB_MANUAL_GAIN)
			{
				mode = 1;
			}

			return HscDevSet(device_handle_, HSC_CFG_ARM_WB_MODE, &mode, sizeof(mode));
		}
	}

	if (isGetParamsFromDevice()) // 白平衡参数存储在设备侧
	{
		HscColorCorrectInfo info;
		getColorCorrectInfo(info);
		info.awb_mode_ = wb_mode;
		return setColorCorrectInfo(info);
	}
	else // 软件侧
	{
		std::shared_ptr<ImageProcessor> pBufferProcessor = getProcessor();
		if (pBufferProcessor != nullptr)
		{
			pBufferProcessor->setWhiteBalanceMode(wb_mode);
			//TODO:saveConfig() 保存数据在本地文件中.
		}
	}

	return HSC_OK;
}

HscResult Device::GetArmWbMode(uint8_t & wb_mode) const
{
	return HscDevGet(device_handle_, HSC_CFG_ARM_WB_MODE, &wb_mode, sizeof(wb_mode));

}

HscResult Device::GetArmMwbGainRange(HscIntRange & range) const
{
	return HscDevGet(device_handle_, HscRangeSelector(HSC_CFG_ARM_R_GAIN), &range, sizeof(range));

}

HscResult Device::GetArmMwbGain(uint16_t & r_gain, uint16_t & gr_gain, uint16_t & gb_gain, uint16_t & b_gain) const
{
	HscResult res = HscDevGet(device_handle_, HSC_CFG_ARM_R_GAIN, &r_gain, sizeof(r_gain));

	if (res == HSC_OK)
	{
		res = HscDevGet(device_handle_, HSC_CFG_ARM_GR_GAIN, &gr_gain, sizeof(gr_gain));
	}

	if (res == HSC_OK)
	{
		res = HscDevGet(device_handle_, HSC_CFG_ARM_GB_GAIN, &gb_gain, sizeof(gb_gain));
	}

	if (res == HSC_OK)
	{
		res = HscDevGet(device_handle_, HSC_CFG_ARM_B_GAIN, &b_gain, sizeof(b_gain));
	}

	return res;
}

HscResult Device::ApplyArmRGain(uint16_t & gain)
{
	return HscDevSet(device_handle_, HSC_CFG_ARM_R_GAIN, &gain, sizeof(gain));

}

HscResult Device::ApplyArmGrGain(uint16_t & gain)
{
	return HscDevSet(device_handle_, HSC_CFG_ARM_GR_GAIN, &gain, sizeof(gain));

}

HscResult Device::ApplyArmGbGain(uint16_t & gain)
{
	return HscDevSet(device_handle_, HSC_CFG_ARM_GB_GAIN, &gain, sizeof(gain));

}

HscResult Device::ApplyArmBGain(uint16_t & gain)
{
	return HscDevSet(device_handle_, HSC_CFG_ARM_B_GAIN, &gain, sizeof(gain));

}

HscResult Device::RestoreFactorySetting()
{
	HscResult res = HscRestoreFactorySetting(device_handle_);
	if (res != HSC_OK)
	{
		return res;
	}

	LoadSettingsFromDevice();

	setProperty2Device(Device::PropType::PropOverExposureTipEnable, false);
	setProperty2Device(Device::PropType::PropName, "");

	emit propertyChanged(Device::PropName, getProperty(PropName));//通过更新一个属性来刷新整个属性列表
	emit updateVideoSegmentList();//刷新视频列表
	return res;
}

HscResult Device::LoadSettingsFromDevice()
{
	HscResult res = HSC_OK;
	QList<PropType> set_prop_type_list = getSupportedPropertyTypes(nullptr);
	for (auto type : set_prop_type_list)
	{
		res = setProperty2Device(type, getProperty(type));
		if (res != HSC_OK)
		{
			break;
		}
	}
	return res;
}

bool Device::IsFactoryResetSupported() const
{
	bool bRet = (1 == device_capability_.supportFactoryReset) ? true : false;
	return bRet && getState() == Connected;
}

bool Device::IsNetConfigSupported() const
{
	bool bRet = (1 == device_capability_.supportNetConfig) ? true : false;
	return bRet&& getState() == Connected;
}

bool Device::isArmWbSupported() const
{
	return m_support_arm_wb;
}

bool Device::isColorSupported() const
{
	return device_capability_.colorCapability == 1;
}

bool Device::AllowsEnableTargetScoring() const
{
	if (FunctionCustomizer::GetInstance().isTargetScoringSupported())
	{
//		if (FallPointMeasure::GetInstance().CameraParamExisted(getIpOrSn().toStdString()))//检查报靶模块是否有标定参数
//		{
//			return true;
//		}
	}
	return false;
}

bool Device::AllowsEnableBlackField() const
{
	if (!IsCamera())
	{
		return false;
	}

	if (getModel() == DEVICE_6F20)
	{
		return false;
	}

	if (!FunctionCustomizer::GetInstance().isIntegratingSphereVersion())
	{
		if (getModel() == DEVICE_XJ520_XG )
		{
			return false;
		}
	}

	if (getProperty(PropStreamType).value<StreamType>() != TYPE_RAW8 && getProperty(PropStreamType).value<StreamType>() != TYPE_RGB8888)
	{
		return false;
	}

	return true;
}

bool Device::AllowsBlackFieldCalibration()
{
	if (!IsCamera())
	{
		return false;
	}

	if (getModel() == DEVICE_6F20)
	{
		return false;
	}

	if (!FunctionCustomizer::GetInstance().isIntegratingSphereVersion())
	{
		if (getModel() == DEVICE_XJ520_XG )
		{
			return false;
		}
	}

	if (getState() != Connected)
	{
		return false;
	}

	if (getProperty(PropStreamType).value<StreamType>() != TYPE_RAW8 && getProperty(PropStreamType).value<StreamType>() != TYPE_RGB8888)
	{
		return false;
	}

	return true;
}

HscResult Device::BlackFieldCalibration()
{
	return HscBlackFieldCalibration(device_handle_);

}

bool Device::AllowsSetGain() const
{
	if (FunctionCustomizer::GetInstance().isUsabilityVersion()) {
		return( getState() == Previewing);
	}
	else {
		return false;
	}
}

bool Device::IsDigitalGainSupported() const
{
	return device_capability_.digitalGainType == 2;
}

bool Device::IsAnalogGainSupported() const
{
	return device_capability_.supportAnalogGain == 1;
}

uint16_t Device::GetLUTValueMax()
{
	if (isSupportHighBitParam())
	{
		return 65535;
	}
	return (device_capability_.digitalGainType == 0 ? 1023 : 255);
}

void Device::GetDefaultLUTCtrlPoints(QList<QVariant> & ctrlPoints)
{
	QList<QVariant> default_lut;
	for (int i = 0; i < 4; i++)
	{
		default_lut.append(QPoint(0, 0));
	}
	if (GetLUTValueMax() == 255)
	{
		default_lut[0] = QPoint(0, 0);
		default_lut[1] = QPoint(64 * 4, 64);
		default_lut[2] = QPoint(192 * 4, 192);
		default_lut[3] = QPoint(1023, 255);
	}
	else if(GetLUTValueMax() == 1023)
	{
		default_lut[0] = QPoint(0, 0);
		default_lut[1] = QPoint(64 * 4, 64 * 4);
		default_lut[2] = QPoint(192 * 4, 192 * 4);
		default_lut[3] = QPoint(1023, 1023);
	}
	else if(GetLUTValueMax() == 65535)
	{
		default_lut[0] = QPoint(0, 0);
		default_lut[1] = QPoint(64 * 256, 64 * 256);
		default_lut[2] = QPoint(192 * 256, 192 * 256);
		default_lut[3] = QPoint(65535, 65535);
	}
	ctrlPoints = default_lut;
}

bool Device::IsColorSupported() const
{
	return device_capability_.colorCapability == 1;
}

bool Device::IsTemperaturePanelSupported() const
{
	return device_capability_.supportShowModulesTemperature == 1;
}

bool Device::AllowsEditDisplayMode() const
{
	return IsColorSupported() && getProperty(PropStreamType).value<StreamType>() != StreamType::TYPE_H264;
}

bool Device::IsSdiCtrlSupported() const
{
	return device_capability_.supportSDICtrl == 1 || device_capability_.supportSDICtrl == 2;
}

bool Device::IsSdiPlaybackCtrlSupported() const
{
	return device_capability_.supportSDICtrl == 1;
}

void Device::GetSupportedSdiPlaySpeeds(QStringList & play_speeds) const
{
	play_speeds.push_back("0.02");
	play_speeds.push_back("0.04");
	play_speeds.push_back("0.1");
	play_speeds.push_back("0.2");
	play_speeds.push_back("0.3");
	for (int i = 1; i <= 50; i++)
	{
		play_speeds.push_back(QString::number(i));
	}
}

static const int kPlaySpeedScaleFactor = 100;

HscResult Device::GetSdiRange(uint32_t & start_frame_no, uint32_t & end_frame_no, double & interval) const
{
	uint32_t interval_int{0};
	HscResult res = HscGetSDIRange(device_handle_, &start_frame_no, &end_frame_no, &interval_int);
	if ((interval_int & 0x80000000) != 0)//最高位非零,特殊处理:最高位转为0,除100
	{
		interval_int &= ~(0x80000000);
		interval = (double)interval_int / (1.0 * kPlaySpeedScaleFactor);
	}
	else
	{
		interval = (double)interval_int;
	}
	return res;

}

HscResult Device::ApplySdiRange(uint32_t start_frame_no, uint32_t end_frame_no, double interval)
{
	LOCK_TEST(&mutex_);
	uint32_t interval_int;
	interval_int = std::round(interval * kPlaySpeedScaleFactor);
	interval_int |= 0x80000000;
	return HscSetSDIRange(device_handle_, start_frame_no, end_frame_no, interval_int);
}

HscResult Device::SetSdiState(HscSDIState state)
{
	LOCK_TEST(&mutex_);
	HscSDIState old_state;
	HscGetSDIState(device_handle_, &old_state);
	if (old_state == HSC_SDI_STOP && state == HSC_SDI_PAUSE)//规避直接从停机进入暂停的调用方式,会导致sdi启用失败
	{
		state = HSC_SDI_STOP;
	}
	return HscSetSDIState(device_handle_, state);
}

HscResult Device::GetSdiState(HscSDIState & state) const
{
	return HscGetSDIState(device_handle_, &state);
}

HscResult Device::SdiBackward()
{
	LOCK_TEST(&mutex_);
	return HscSDISeekToPreFrame(device_handle_);
}

HscResult Device::SdiForward()
{
	LOCK_TEST(&mutex_);
	return HscSDISeekToNextFrame(device_handle_);
}

bool Device::IsWatermarkSupported() const
{
	return IsWatermarkSupported(getProperty(Device::PropVideoFormat).value<VideoFormat>());
}

bool Device::IsWatermarkSupported(VideoFormat video_format) const
{
	if (!IsCamera())
	{
		return false;
	}

	if (video_format == VIDEO_RHVD)
	{
		return false;
	}

	if (FunctionCustomizer::GetInstance().isH150Enabled())
	{
		return true;
	}

	StreamType stream_type = getProperty(Device::PropStreamType).value<StreamType>();
	auto bpp = getProcessor()->GetBitsPerPixel();
	if (video_format == VIDEO_TIF && bpp == 16)//16bit tif水印暂时屏蔽
	{
		return false;
	}

	if (stream_type == TYPE_RAW8 || stream_type == TYPE_RGB888 || stream_type == TYPE_YUV420)
	{
		return true;
	}

	return false;
}

int Device::GettProcessorBitsPerPixel()
{
	auto bpp = getProcessor()->GetBitsPerPixel();
	return bpp;
}

bool Device::AllowsEditWatermark() const
{
	if (!IsWatermarkSupported())
	{
		return false;
	}

	return getState() == Connected || getState() == Previewing;
}

bool Device::IsTriggerSyncSupported() const
{
	return m_support_trigger_sync;
}

bool Device::AllowsEditTriggerSyncEnabled() const
{
	if (!IsTriggerSyncSupported() )
	{
		return false;
	}

	//外同步时不支持设置触发同步
	if (getProperty(Device::PropSyncSource).value<SyncSource>() == SYNC_EXTERNAL)
	{
		return false;
	}

	//内触发不支持触发同步
	if (getProperty(Device::PropTriggerMode).value<TriggerMode>() == TRIGGER_INTERNAL)
	{
		return false;
	}


	if (getState() == Acquiring)
	{
		if (IsSupportAcquireEdit())
		{
			return true;
		}
	}
	return getState() == Connected || getState() == Previewing;
}

bool Device::AllowsEditTriggerMode() const
{
	DeviceState state = getState();
	if (IsCamera())
	{
		if (state == Acquiring)
		{
			if (IsSupportAcquireEdit())
			{
				return true;
			}
		}
		return state == Connected || state == Previewing;
	}
	else if (IsRootTrigger())
	{
		if (state == Connected)
		{
			//TODO:级联支持


			if (!FunctionCustomizer::GetInstance().isH150Enabled())
			{
				return true;
			}

			if (IsPIVSupported())
			{
				return true;
			}
		}
	}

	return false;
}

bool Device::IsExternalTriggerModeSupported() const
{
	return device_capability_.supportExternalTriggerMode == 1;

}

bool Device::AllowsEditExternalTriggerMode() const
{
	if (!IsExternalTriggerModeSupported())
	{
		return false;
	}

	if (IsCamera())
	{
		if (getProperty(Device::PropTriggerMode).value<TriggerMode>() != TRIGGER_EXTERNAL)
		{
			return false;
		}

		if (getState() == Acquiring)
		{
			if (IsSupportAcquireEdit())
			{
				return true;
			}
		}
		return getState() == Connected || getState() == Previewing;
	}
	else if (IsRootTrigger())
	{
		if (getProperty(Device::PropTriggerMode).value<TriggerMode>() != TRIGGER_EXTERNAL)
		{
			return false;
		}

		return getState() == Connected;
	}

	return false;
}

bool Device::IsJitterEliminationLengthSupported() const
{
	return m_support_jitter_elimination_length;

}

bool Device::AllowsEditJitterEliminationLength() const
{
	if (!IsJitterEliminationLengthSupported())
	{
		return false;
	}

	if (!IsCamera())
	{
		return false;
	}

	return AllowsEditExternalTriggerMode();
}

bool Device::AllowsEditLuminanceAndContrast() const
{
	return getState() == Previewing;
}

bool Device::AllowsShowDeviceProperties() const
{
	return getState() == Connected || getState() == Previewing || getState() == Acquiring || getState() == Recording;
}

HscResult Device::ReloadDeviceInfo()
{
	return HscGetDeviceInfoByDeviceHandle(device_handle_, &info_);
}

int Device::GetDeviceTotalMemorySize() const
{
	return HscGetDeviceTotalMemorySize(device_handle_);

}

int Device::GetDeviceUsedMemorySize() const
{
	return HscGetDeviceUsedMemorySize(device_handle_);

}


HscResult Device::GetTemperatures(QString & out) 
{
	LOCK_TEST(&mutex_);
	HscResult res = HSC_OK;
	DeviceState state = getState();
	if (state == Connected || state == Previewing || state == Acquiring || state == Recording)
	{
		memset(temperature_buf_ptr_, 0, kTemperatureBufLen);
		res = HscGetDeviceTemperatures(device_handle_, temperature_buf_ptr_, kTemperatureBufLen);
		if (res == HSC_OK)
		{
			out = QString(temperature_buf_ptr_);
		}
	}

	return res;

}

HscResult Device::GetTemperatures(std::map<TemperatureTypes, double> & map_temperature)
{
	QString temps_str;
	HscResult res = GetTemperatures(temps_str);
	if (res == HSC_OK)
	{
		//分割字符串

		QStringList temp_splits = temps_str.split(';');

		for (auto temp_split : temp_splits)
		{
			QStringList type_temp_splits = temp_split.split(':');
			if (type_temp_splits.size() == 2)
			{
				QString type_str = type_temp_splits.at(0).toUpper();
				QString temp_str = type_temp_splits.at(1);
				float temp = temp_str.toFloat();
				if (type_str == "MAINBOARD")
				{
					map_temperature[kTempMainBoard] = temp;
				}
				else if (type_str == "SLAVEBOARD")
				{
					map_temperature[kTempSlaveBoard] = temp;
				}
				else if (type_str == "ARMCHIP")
				{
					map_temperature[kTempArmChip] = temp;
				}
				else if (type_str == "CHAMBER")
				{
					map_temperature[kTempChamber] = temp;
				}
			}
		}
	}

	return res;
}

HscResult Device::GetRouteDelay(DWORD & dwDelay)
{
	HscResult res = HscGetRouteDelay(device_handle_, &dwDelay);
	return res;
}

HscResult Device::GetAuthInfo(HscAuthInfo *auth_info)
{
	return HscGetAuthInfo(device_handle_, auth_info);
}

bool Device::IsAuthEnabled() const
{
	if (device_capability_.authCapability != 1) // 设备不支持
	{
		return false;
	}

	if (getState() != DeviceState::Connected) // 非停机模式
	{
		return false;
	}

	return true;
}



bool Device::IsOnlineUpdate() const
{
	if (1 != device_capability_.onlineUpgradeCapability)
	{
		return false;
	}

	if (getState() != DeviceState::Connected)
	{
		return false;
	}

	return true;
}

HscResult Device::StartOnlineUpgrade(const UpgradeInfo & info)
{
	LOCK_TEST(&mutex_);
	if (getState() != DeviceState::Connected)
	{
		return HSC_ERROR;
	}

	return HscOnlineUpgrade(device_handle_, const_cast<UpgradeInfo*>(&info));
}

HscResult Device::GetDeviceVersion(std::vector<VersionInfo> & versions)
{
	if (IsCF18())
	{
		std::string arm_ver;
		std::string fpga_ver;
		GetVersionData(arm_ver, fpga_ver);
		VersionInfo arm_info,fpga_info;
		arm_info.type = "ARM";
		arm_info.version = QString::fromStdString(arm_ver);
		fpga_info.type = "FPGA";
		fpga_info.version = QString::fromStdString(fpga_ver);
		versions.push_back(arm_info);
		versions.push_back(fpga_info);
		return HSC_OK;
	}

	// 兼容旧版接口
	DWORD version = 0;
	HscResult res = HscGetDeviceVersion(device_handle_, &version);
	if (res == HSC_OK && version != 0)
	{
		VersionInfo versionInfo;
		versionInfo.type = tr("Hardware");
		//版本号规则:(8位).(8位).(16位)
		versionInfo.version = QString::number((unsigned char)(version >> 24)) + "." + QString::number((unsigned char)(version >> 16)) + tr(" Build") + QString::number((unsigned short)(version));
		versions.push_back(versionInfo);
		return HSC_OK;
	}

	char cversions[MAX_DEVICE_VERSION_LENGTH]{ 0 };
	res = HscGetDeviceVersion(device_handle_, cversions, MAX_DEVICE_VERSION_LENGTH);
	if (res == HSC_OK)
	{
		// 解析版本信息，格式形如：CFPGA:1.0.0;UFGA:1.0.1;IFPGA:1.0.1;ARM:1.1.0
		std::string versionInfo = cversions;
		if (!versionInfo.empty())
		{
			std::vector<std::string> vecTypeVersion = PathUtils::split(versionInfo, ";");
			for (auto typeVersion : vecTypeVersion)
			{
				std::vector<std::string> versionSplits = PathUtils::split(typeVersion, ":");
				if (versionSplits.size() == 2)
				{
					VersionInfo info;
					info.type = QString::fromStdString(versionSplits.at(0));
					info.version = QString::fromStdString(versionSplits.at(1));
					versions.push_back(info);
				}
			}
		}
	}

	return res;
}

HscResult Device::GetSdkVersion(VersionInfo &version_info)
{
	// 兼容旧版接口
	DWORD version = 0;
	HscResult res = HscGetSdkVersion(device_handle_, &version);
	if (res == HSC_OK && version != 0)
	{
		version_info.type = tr("SDK");
		//版本号规则:(8位).(8位).(16位)
		version_info.version = QString::number((unsigned char)(version >> 24)) + "." + QString::number((unsigned char)(version >> 16)) + tr(" Build") + QString::number((unsigned short)(version));
		return HSC_OK;
	}

	char cversions[MAX_SDK_VERSION_LENGTH]{ 0 };
	res = HscGetSdkVersion(cversions, MAX_SDK_VERSION_LENGTH);
	if (res == HSC_OK)
	{
		version_info.type = tr("SDK");
		version_info.version =QString::fromStdString(cversions);
	}
	return res;
}

HscResult Device::GetPluginVersion(VersionInfo & version_info)
{
	char cversions[MAX_PLUGIN_VERSION_LENGTH]{ 0 };
	HscResult res = HscGetPluginVersion(info_.model, cversions, MAX_PLUGIN_VERSION_LENGTH);
	if (res == HSC_OK)
	{
		version_info.type = tr("Plugin");
		version_info.version = QString::fromStdString(cversions);
	}

	return res;
}

bool Device::AllowEditFanCtrl() const
{
	if (!IsFanCtrlSupported())
	{
		return false;
	}

	return (getState() != Unconnected && getState() != DeviceState::Disconnected && getState() != DeviceState::StandBy);
}

bool Device::IsFanCtrlSupported() const
{
	return device_capability_.fanCapability == 1;
}

HscResult Device::SetFanState(uint8_t state)
{
	LOCK_TEST(&mutex_);
	m_fan_state = state;
	return HscSetFanState(device_handle_, state);
}

HscResult Device::GetFanState(uint8_t & state)
{
	if (getState() != Connected  )//非连接状态时直接从本地获取风扇状态,防止频繁获取状态
	{
		state = m_fan_state;
		return HSC_OK;
	}

	{
		LOCK_TEST(&mutex_);
		return HscGetFanState(device_handle_, &state);
	}

}

HscResult Device::SetBinningMode(bool enable)
{
	bool param = enable;
	HscResult res = HscDevSet(device_handle_, HSC_CFG_BINNING_MODE, &param, sizeof(param));

	if (image_processor_ptr_)
	{
		image_processor_ptr_->SetBinningModeEnable(param);
	}

	return res;
}

HscResult Device::GetBinningMode(bool &enable)
{
	HscResult res = HSC_OK;

	{
		bool param{ false };
		res = HscDevGet(device_handle_, HSC_CFG_BINNING_MODE, &param, sizeof(param));
		if (res == HSC_OK)
		{
			enable = param;
		}
	}

	return res;
}

bool Device::IsBinningModeSupported() const
{
	return m_support_binning_mode;
}

bool Device::AllowsSetBinningMode() const
{
	if (!IsBinningModeSupported())
		return false;

	auto state = getState();
	return (state == Previewing || state == Connected);
}

HscResult Device::SetNetConfigByGateway(uint8_t* devMacAddr, const char* newIP, const char* newMask, const char* newGateway)
{
	return HscSetNetConfigByGateway(devMacAddr, newIP, newMask, newGateway);
}

bool Device::IsIntelligentTriggerSupported() const
{
	return (device_capability_.imgTrigCapability == 2 
		|| device_capability_.imgTrigCapability == 5
		|| device_capability_.imgTrigCapability == 4 
		|| device_capability_.imgTrigCapability == 3);
}

bool Device::IsIntelligentTriggerV4Supported() const
{
	return  device_capability_.imgTrigCapability == 4;//支持第四版智能触发
}

bool Device::IsIntelligentTriggerV5Supported() const
{
	return  device_capability_.imgTrigCapability == 5;//支持第五版智能触发

}

bool Device::IsPulseWidthUnit100Ns() const
{
	return  m_pulsewidth_mode == 2;//百纳秒脉冲宽度

}

bool Device::AllowsSetIntelligentTrigger() const
{
	if (!IsIntelligentTriggerSupported())
		return false;
	//PIV模式不可编辑
	if (IsPIVEnabled())
	{
		return false;
	}
	return (getState() == Previewing);
}

bool Device::IsIntelligentTriggerEnabled() const
{
	if (!IsIntelligentTriggerSupported())
	{
		return false;
	}

	//PIV模式使能关闭
	if (IsPIVEnabled())
	{
		return false;
	}
	return getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().enable!=0;
}

bool Device::IsAutoExposureSupported() const
{
	return device_capability_.supportAutoExposure == 1;
	
}

bool Device::AllowSetAutoExposure() const
{
	if (!IsAutoExposureSupported())
	{
		return false;
	}

	//PIV开启时，禁用自动曝光参数设置
	if (IsPIVEnabled())
	{
		return false;
	}

	return getState() == Previewing;
	
}

bool Device::IsAutoExposureEnabled() const
{
	if (!IsAutoExposureSupported())
	{
		return false;
	}

	//PIV开启时，禁用自动曝光参数设置
	if (IsPIVEnabled())
	{
		return false;
	}
	auto val = getProperty(PropAutoExposure);
	if (val.isNull())
		return false;
	return 	val.value<HscAutoExposureParameter>().bEnable;

}

bool Device::IsOverExposureTipSupported() const
{
	return FunctionCustomizer::GetInstance().isUsabilityVersion();
}

bool Device::AllowsEditOverExposureTip() const
{
	if (!IsOverExposureTipSupported())
	{
		return false;
	}

	return getState() == Previewing || getState() == Acquiring || getState() == Recording;
}

bool Device::AllowsEditExposureTime() const
{
// 	if (IsPIVEnabled())
// 	{
// 		return false;
// 	}

	return getState() == Previewing || getState() == Acquiring || getState() == Connected;
}

bool Device::AllowsEditFrameRate() const
{
	return AllowsEditFrameRate(IsCamera() ? getProperty(Device::PropSyncSource).value<SyncSource>() : SYNC_INTERNAL);

}

bool Device::AllowsEditFrameRate(SyncSource sync_source) const
{
	DeviceState state = getState();
	if (IsCamera())
	{
		if (info_.model == DEVICE_HT160A || info_.model == DEVICE_HT120A || info_.model == DEVICE_HT50A || info_.model == DEVICE_HT40A)
		{
			//海图ARM版系列：禁止调节帧率
			return false;
		}

		if ((sync_source == SYNC_INTERNAL) && (state == Connected || state == Previewing || state == Acquiring))
		{
			// PIV开启时，帧率不可编辑
			if (IsPIVEnabled(sync_source))
			{
				return false;
			}
			if (state == Acquiring)
			{
				if (IsSupportAcquireEdit())
				{
					RecordMode mode = getProperty(Device::PropRecordingOffsetMode).value<RecordMode>();
					if (mode == RECORD_BEFORE_SHUTTER) return false;
					return true;
				}
				return false;
			}

			return true;
		}
	}
	else if (IsRootTrigger())
	{
		if (state == Connected)
		{
			//TODO:级联支持
			return true;
		}
	}

	return false;
}

bool Device::AllowsEditRecordingOffsetMode() const
{
	return AllowsEditRecordingSettings()/* && !IsPIVEnabled()*/ && !IsRealtimeExportSupported() && !IsAutoExportAndTriggerAfterRecordingEnabled();

}

bool Device::AllowsEditRecordingUnit() const
{
	if (!AllowsEditRecordingSettings())
	{
		return false;
	}


	if (IsRealtimeExportSupported())
	{
		return false;
	}

	return true;
}

bool Device::AllowsEditRecordingSettings() const
{
	return AllowsEditRecordingSettings(IsCamera() ? getProperty(Device::PropSyncSource).value<SyncSource>() : SYNC_INTERNAL);

}

bool Device::AllowsEditRecordingSettings(SyncSource sync_source) const
{
	Q_UNUSED(sync_source);
	DeviceState state = getState();

	//TODO:级联支持

	if (state == Connected || state == Previewing)
	{
		return true;
	}

	return false;
}

bool Device::IsRealtimeExportSupported() const
{
	return device_capability_.supportRealtimeExport == 1;

}

bool Device::IsAutoExportEnabled() const
{
	return SystemSettingsManager::instance().getActionTypesAfterRecording() == SystemSettingsManager::ActionTypesAfterRecording::kAutoExport;
}

bool Device::IsAutoExportAndTriggerAfterRecordingEnabled() const
{
	return SystemSettingsManager::instance().getActionTypesAfterRecording() == SystemSettingsManager::ActionTypesAfterRecording::kAutoExportAndTrigger;
}

bool Device::AllowsAutoPlayback() const
{
	return SystemSettingsManager::instance().getActionTypesAfterRecording() == SystemSettingsManager::ActionTypesAfterRecording::kPlayback;
}

static std::set<DeviceModel> s_support_auto_del_video_segment_models = {
	DEVICE_5F08, DEVICE_5F10, DEVICE_5F01, DEVICE_ISP504, DEVICE_ISP502, DEVICE_5KF20, DEVICE_5KF10, DEVICE_AE130,
	DEVICE_X150_ISPSeries, DEVICE_X213_ISPSeries, DEVICE_X113_ISPSeries, DEVICE_XJ520_ISPSeries, DEVICE_XJ520_XG, DEVICE_XJ1200, DEVICE_XJ1310
};
bool Device::IsAutoDeleteVideoSegmentSupported() const
{
	return s_support_auto_del_video_segment_models.find(info_.model) != s_support_auto_del_video_segment_models.end();
}


bool Device::IsMemoryManagementSupported() const
{
	return getModel() == DEVICE_XJ1310 || (m_recording_offset_mode_ == 1);
}

bool Device::IsEdrExposureSupported() const
{
	return device_capability_.supportEDR == 1;
}

bool Device::AllowSetEdrExposure() const
{
	if (!IsEdrExposureSupported())
	{
		return false;
	}

	//PIV开启时，禁用二次曝光参数设置
	if (IsPIVEnabled())
	{
		return false;
	}

	return getState() == Previewing;
}

bool Device::IsEdrExposureEnabled() const
{
	if (!IsEdrExposureSupported())
	{
		return false;
	}

	//PIV开启时，禁用二次曝光参数设置
	if (IsPIVEnabled())
	{
		return false;
	}
	if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam())
	{
		auto val = getProperty(PropEdrExposureV2);
		if (val.isNull())
			return false;
		return 	val.value<HscEDRParamV2>().enabled;
	}

	auto val = getProperty(PropEdrExposure);
	if (val.isNull())
		return false;
	return 	val.value<HscEDRParam>().enabled;
}

bool Device::IsDeviceIndexSupported() const
{
	return info_.model == DEVICE_XJ520_XG || info_.model == DEVICE_PCCSeries || info_.model == DEVICE_XJ1310;
}

bool Device::IsStandByModeSupported() const
{
	return device_capability_.supportHardwareStandby;
}

bool Device::AllowEditStandByMode() const
{
	if (IsStandByModeSupported())
	{
		DeviceState state = getState();
		if (state == Connected ||
			state == Previewing ||
			state == Acquiring ||
			state == StandBy)
		{
			return true;
		}
	}
	return false;
}

DeviceState Device::GetWorkStatusFromDevice()
{
	DeviceOperationType mode = DEV_OPT_TYPE_UNKNOW;
	HscResult res = HscGetDeviceMode(device_handle_, &mode);
	if (HSC_OK != res)
	{
		disconnectDevice(false);
	}

	DeviceState state = Unconnected;
	switch (mode)
	{
	case DEV_OPT_TYPE_STOP:
		state = Connected;
		break;
	case DEV_OPT_TYPE_PREVIEW:
		state = Previewing;
		break;
	case DEV_OPT_TYPE_HIGHSPEED:
		state = Acquiring;
		break;
	case DEV_OPT_TYPE_EXPORT_VIDEO_CLIPS:
		state = Connected;
		break;
	case DEV_OPT_TYPE_RECORIDING:
		state = Recording;
		break;
	case DEV_OPT_TYPE_HARDWARE_STANDBY:
		state = StandBy;
		break;
	default:
		break;
	}
	return state;
}

bool Device::IsPIVSupported() const
{
	return device_capability_.supportPIV == 1;
}

bool Device::allowsEditPIVEnable() const
{
	if (!IsPIVSupported())
	{
		return false;
	}

	DeviceState state = getState();
	if (state != Connected && state != Previewing)
	{
		return false;
	}

	if (IsCamera())
	{
		// 非外同步时，PIV不可设
		if (getProperty(Device::PropSyncSource) != SYNC_EXTERNAL)
		{
			return false;
		}



	}


	return true;
}

bool Device::IsPIVEnabled() const
{
	return IsPIVEnabled(getProperty(Device::PropSyncSource).value<SyncSource>());
}

bool Device::IsPIVEnabled(SyncSource sync_source) const
{
	Q_UNUSED(sync_source);
	if (!IsPIVSupported())
	{
		return false;
	}

	HscPIVParam piv_param{ 0 };
	piv_param = getProperty(Device::PropPivParam).value<HscPIVParam>();
	return piv_param.enabled == 1;
}

bool Device::IsImageCtrlSupported() const
{
	return device_capability_.supportImageCtrl == 1;
}

bool Device::AllowsImageCtrl() const
{
	if (!IsImageCtrlSupported())
		return false;
	return getState() == Previewing;
}

#ifdef CSRCCAPP

#else

std::shared_ptr<PlayerControllerInterface> Device::getRealtimePlayerController()
{
    return realtime_player_controller_ptr_;
}
#endif //CSRCCAPP

QList<QSharedPointer<HscVideoClipInfo> > Device::getAllVideoSegments() const
{
    QMutexLocker locker(&video_segment_map_mutex_);

    return video_segment_map_.values();
}

QSharedPointer<HscVideoClipInfo> Device::getVideoSegment(int id) const
{
    QMutexLocker locker(&video_segment_map_mutex_);

    return video_segment_map_.value(id);
}

HscVideoClipDeleteMode Device::GetVideoClipDeleteMode() const
{
	HscVideoClipDeleteMode delteMode = HSC_VCDM_RANDOM;
	HscGetVideoClipDeleteMode(device_handle_, &delteMode);
	return delteMode;
}

quint64 Device::removeVideoSegment(int id)
{
    HscResult res = HSC_OK;
    {
        LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

        res = HscDeleteVideoClip(device_handle_, id);
    }

    if (res == HSC_OK)
    {
        LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

        video_segment_map_.remove(id);

        VideoItemManager::instance().removeVideoItem(VideoUtils::packDeviceVideoId(getIpOrSn(), id));
		m_video_list_thumbnail.remove(VideoUtils::packDeviceVideoId(getIpOrSn(), id));
    }

    return res;
}

HscResult Device::DeleteAllVideoClips()
{
	HscResult res = HSC_OK;
	{
		LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

		res = HscDeleteAllVideoClips(device_handle_);
	}

	if (res == HSC_OK)
	{
		LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

		video_segment_map_.clear();

		//查找全部视频项,移除
		QList<VideoItem> items = VideoItemManager::instance().findVideoItems(VideoItem::Remote, getIpOrSn());
		for (auto item:items)
		{
			VideoItemManager::instance().removeVideoItem(item.getId());
			m_video_list_thumbnail.remove(item.getId());
		}
	}

	return res;
}

void Device::Update(HscEvent *msg, void *parameters)
{
	if (!msg)
	{
		return;
	}

	std::unique_lock<std::mutex> locker(device_cv_mutex_);
	device_msg_vector_.append(EventMsg(msg, parameters));
	device_msg_cv_.notify_one();
}

qint64 Device::toFrameRate(qint64 divisor, qint64 acquisition_period)
{
    if (acquisition_period == 0)
    {
        return 0;
    }
    return (qint64)qFloor(divisor / acquisition_period);
}

qint64 Device::toAcquisitionPeriod(qint64 frame_rate) const
{
    qint64 min_acquisition_period = 1;
    HscIntRange range{};
    if (getAcquisitionPeriodRange(getProperty(PropType::PropRoi).toRect(), range))
    {
        min_acquisition_period = range.min;
    }

    return toAcquisitionPeriod(getCalPeriodDivisor(), frame_rate, min_acquisition_period);
}

qint64 Device::toAcquisitionPeriod(qint64 divisor, qint64 frame_rate, qint64 min_acquisition_period)
{
    if (frame_rate == 0)
    {
        return 0;
    }

    // 帧率换算采集周期时，向下取整，并做最小采集周期约束
    qint64 acquisition_period = (qint64)qFloor(divisor / frame_rate);
    if (acquisition_period < min_acquisition_period)
    {
        acquisition_period = min_acquisition_period;
    }

    return acquisition_period;
}

void Device::startUpdateRealtimeFrame()
{
	realtime_frame_timer_ptr_->start();

#if TEST_THREAD_PROCESS_FRAME
	startRccImageProcessThread();
#endif
}

void Device::onPropertyChanged(PropType type, const QVariant & value)
{
	Q_UNUSED(value);
	switch (type)
	{
	case Device::PropName:
		//当定时器不刷新时主动更新OSD信息，定时器更新时图像实时更新
		if (!realtime_frame_timer_ptr_->isActive())
		{
			refreshOSDAfterConnect();
		}
		break;
	default:
		break;
	}
}

void Device::doConnect()
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	if (IsCF18()) {
		if (CF18Open(getIpOrSn().toStdString(), 8080)) {
			setState(DeviceState::Connected);
			//添加心跳检测进程
			initCF18HeartBeatThread();
		}
		else {
			emit errorOccurred(HSC_NETWORK_ERROR, m_bShowTip);
		}
	}
	else {
		HscResult res = connectDevice(true, true);
		if (res != HSC_OK)
		{
			emit errorOccurred(res, m_bShowTip);
		}
		else
		{
			if (needsDataRecovery())
			{
				setState(DeviceState::Connected);
				emit SignalRecoveringData();
			}
		}
	}
}
    

void Device::doPreview()
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscResult res = previewDevice();
    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
    }
}

void Device::doAcquire()
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscResult res = acquireDevice();
    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
    }
}

void Device::doFinishRecording(int vid, bool berforeStopRecing)
{

	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	CSLOG_INFO("doFinishRecording");
	uint8_t state = 0;
	// 停机
	stopDevice(true, m_stop_mode_);

	//进行视频彩色信息存储(xj1310不执行这一步操作的话可能导致视频无法被保存)
	if (vid >= 0)
	{
		updateColorAlgorithmInfo(vid);
	}

	if (vid == -1)
	{
		QMutexLocker locker(&video_segment_map_mutex_);
		if (video_segment_map_.size()>0 && !video_segment_map_.contains(vid))
		{
			vid = video_segment_map_.lastKey();
		}

		if (vid != -1)
		{
			QSharedPointer<HscVideoClipInfo> oldvalue = video_segment_map_[vid];
			uint64_t oldtime_stamp = oldvalue->time_stamp;
			QMap<int, QSharedPointer<HscVideoClipInfo>>::const_iterator it = video_segment_map_.constBegin();
			while (it != video_segment_map_.end())
			{
				QSharedPointer<HscVideoClipInfo> dainfo = it.value();
				if (dainfo->time_stamp > oldtime_stamp)
				{
					oldtime_stamp = dainfo->time_stamp;
					vid = it.key();
				}
				it++;
			}
		}
	}

	QVariant video_id = VideoUtils::packDeviceVideoId(getIpOrSn(), vid);
	CSLOG_INFO("Device::doFinishRecording: {}, {}", vid, getIpOrSn().toStdString());
	if (IsAutoExportEnabled() || IsAutoExportAndTriggerAfterRecordingEnabled() || AllowsAutoPlayback())
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		bool video_exist = false;
		while (!video_exist) {
			if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() >= 200) {
				CSLOG_WARN("add video item timeout");
				break;
			}
			boost::this_thread::sleep_for(boost::chrono::microseconds(10));
			video_exist = VideoItemManager::instance().containsVideoItem(video_id);
		}
		if (!video_exist)
		{
			CSLOG_WARN("No video information found");
			state = 1;
		}
	}

	if (berforeStopRecing) CSLOG_INFO("berforeStopRecord:{}", vid);
	emit recordingFinished(video_id, berforeStopRecing, state);


	CSLOG_INFO("doFinishRecording finished");

}

void Device::doStop(uint8_t mode)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    HscResult res = stopDevice(true, mode);
    if (res != HSC_OK)
    {
        emit errorOccurred(res, m_bShowTip);
    }
}

void Device::doDisconnect()
{
	m_quit_reconnect = true;
	m_destory = true;
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	if (IsCF18()) {
		CF18Close();
		setState(DeviceState::Unconnected);
		emit deviceDisconnected(getIpOrSn());
	}
	else {
		HscResult res = disconnectDevice(true, true);
		if (res != HSC_OK)
		{
			emit errorOccurred(res, m_bShowTip);
		}
		else
		{
			exitMsgProcessThread();
		}
	}
}

HscResult Device::connectDevice(bool update_state, bool emit_device_connected)
{
	if (!isCurrentDeviceInComputerGateway()) {
		return HSC_GATEWAY_DIFFERENT;
	}
    if (update_state)
    {
        setState(DeviceState::Connecting);
		m_quit_reconnect = false;
    }
	CSLOG_INFO("{} Connecting " ,getIpOrSn().toStdString());
    HscResult res = HSC_OK;
	
    do {
        if (device_handle_ == NULL)
        {
            // 创建设备
            res = HscCreateDevice(&device_handle_, info_.model);
            if (res != HSC_OK)
            {
                break;
            }

            HscRegister(device_handle_, msg_listener_ptr_.get());
        }

        // 打开设备
        res = HscOpenDevice(device_handle_, info_);
        if (res != HSC_OK)
        {
            break;
        }
		
		// 重置温度提示
		m_bTemperatureShowTip = false;

        // 载入设备能力集
        res = HscGetDeviceCapability(device_handle_, &device_capability_);
        if (res != HSC_OK)
        {
            break;
        }

        // 获取设备参数
        res = HscGetDeviceParam(device_handle_, param_);
        if (res != HSC_OK)
        {
            break;
        }

		// 获取roi对称限制
		{
			int32_t value = 0;
			res = HscDevGet(device_handle_, HSC_CFG_ROI_CONSTRAINT, &value, sizeof(value));
			if (res != HSC_OK) break;
			else m_roi_constraint_ = value;
		}

		// 获取是否支持高采编辑属性栏
		{
			uint8_t support = false;
			res = HscDevGet(device_handle_, HSC_CFG_EDIT_PROPERTY, &support, sizeof(support));
			if (res != HSC_OK) break;
			else m_support_edit_property_ = support;
		}
		// 获取是否支持cmos数字增益
		{
			bool support = false;
			res = HscDevGet(device_handle_, HSC_CFG_CMOS_DIGITAL_GAIN, &support, sizeof(support));
			if (res != HSC_OK) break;
			else m_support_cmos_digitalGain_ = support;
		}

		// 是否用G270智能触发界面
		{
			uint8_t type = 0;
			res = HscDevGet(device_handle_, HSC_CFG_INTELLLIGENT_TRIGGER, &type, sizeof(type));
			if (res != HSC_OK) break;
			else m_intelligent_trigger = type;
		}

		// 获取支持bit值
		{
			if (m_bits == nullptr) m_bits = new char[100];
			memset(m_bits, 0, 100);
			res = HscDevGet(device_handle_, HSC_CFG_SUPPORT_BIT_VAL, m_bits, 100);
			if (res != HSC_OK) break;
		}

		//20240104:暂时不需要该逻辑
		//获取像素位深参数,并重新下发,应用参数
// 		current_out_bpp_ = getProperty(PropPixelBitDepth).toInt();
// 		setProperty2Device(PropPixelBitDepth, current_out_bpp_);

		// 获取是否开启像素融合模式
		{
			bool binning_enable = false;

			HscResult res = GetBinningMode(binning_enable);
			if (res == HSC_OK)
			{
				m_support_binning_mode = true;
				SetBinningMode(binning_enable);
			}
			else
			{
				m_support_binning_mode = false;
				SetBinningMode(false);
			}
		}
		// 获取自动曝光值类型
		{
			uint8_t type = 0;
			res = HscDevGet(device_handle_, HSC_CFG_EXPOSURE_VAL_TYPE, &type, sizeof(type));
			if (res != HSC_OK) break;
			else m_exposure_value_type = type;
			//m_exposure_value_type = device_capability_.supportAutoExposure;
		}
	
		// 获取典型分辨率类型
		{
			uint8_t value = 0;
			res = HscDevGet(device_handle_, HSC_CFG_RESOLUTION_RATIO, &value, sizeof(value));
			if (res != HSC_OK) break;
			else m_resolution_ratio_type_ = value;
		}

		// 获取是否支持消抖长度
		{
			uint8_t support = 0;
			res = HscDevGet(device_handle_, HscSupportSelector(HSC_CFG_JITTER_ELIMINATION_LENGTH), &support, sizeof(support));
			if (res == HSC_OK)
			{
				m_support_jitter_elimination_length = (support == 1);
			}
			else
			{
				break;
			}
		}

		// 获取白平衡参数:是否支持ARM白平衡
		{
			uint8_t support = 0;
			res = HscDevGet(device_handle_, HscSupportSelector(HSC_CFG_ARM_WB_MODE), &support, sizeof(support));
			if (res == HSC_OK)
			{
				m_support_arm_wb = (support == 1);
			}
			else
			{
				break;
			}
		}

		// 获取是否支持触发同步
		{
			uint8_t support = 0;
			res = HscDevGet(device_handle_, HSC_CFG_TRIGGER_SYNC_ENABLE, &support, sizeof(support));
			if (res == HSC_OK)
			{
				m_support_trigger_sync = (support >= 1);
			}
			else
			{
				break;
			}
		}
		{
			uint8_t state = m_fan_state;
			auto res = HscGetFanState(device_handle_, &state);
			if (res == HSC_OK)  m_fan_state = state;
		}

        // APP存储参数时，应用参数
        if (!settingsNeedsLoadFromDevice())
        {
			QList<PropType> set_prop_type_list = getSupportedPropertyTypes(nullptr);
            for (auto type : set_prop_type_list)
            {
                res = setProperty2Device(type, getProperty(type));
                if (res != HSC_OK)
                {
                    break;
                }
            }
        }

		 //获取是否停机模式
		{
			bool stop_mode = false;
			res = HscDevGet(device_handle_, HSC_CFG_STOP_MODE, &stop_mode, sizeof(stop_mode));
			if (res != HSC_OK) break;
			else m_stop_mode_ = stop_mode;
		}

		// 获取是否连续导出
		{
			bool export_mode = false;
			res = HscDevGet(device_handle_, HSC_CFG_EXPORT_MODE, &export_mode, sizeof(export_mode));
			if (res != HSC_OK) break;
			else m_export_mode_ = export_mode; 
		}

		// 获取同步脉宽范围取值模式
		{
			int32_t pulsewidth_mode = 0;
			res = HscDevGet(device_handle_, HSC_CFG_PULSEWIDTH_MODE, &pulsewidth_mode, sizeof(pulsewidth_mode));
			if (res != HSC_OK) break;
			else m_pulsewidth_mode = pulsewidth_mode; 
		}

		// 采集周期单位
		{
			int32_t period_unit = 0;
			res = HscDevGet(device_handle_, HSC_CFG_PERIOD_UNIT_MODE, &period_unit, sizeof(period_unit));
			if (res != HSC_OK) break;
			else m_period_unit_ = period_unit;
		}
	
		{
            int32_t recording_offset_mode = 0;
			res = HscDevGet(device_handle_, HSC_CFG_RECORD_OFFSET_MODE, &recording_offset_mode, sizeof(recording_offset_mode));
			if (res != HSC_OK) break;
			else m_recording_offset_mode_ = (int8_t)recording_offset_mode;
		}

		// 获取增益模拟值
		{
			if (m_analog_gain_values == nullptr) m_analog_gain_values = new char[100];
			memset(m_analog_gain_values, 0, 100);
			res = HscDevGet(device_handle_, HSC_CFG_ANALOG_GAIN_VAL, m_analog_gain_values, 100);
			if (res != HSC_OK) break;
		}
		{
			int32_t m_frame_head_type = 0;
			res = HscDevGet(device_handle_, HSC_CFG_FRAME_HEAD_MODE, &m_frame_head_type, sizeof(m_frame_head_type_));
			if (res != HSC_OK) break;
			else m_frame_head_type_ = (int8_t)m_frame_head_type;
			//if (image_processor_ptr_) image_processor_ptr_->setFrameHeadType(m_frame_head_type);
		}
		if (IsCamera())
		{

			// 显示模式
			HscDisplayMode display_mode = HscDisplayMode(0);
			HscDevGet(device_handle_, HSC_CFG_DISPLAY_MODE, &display_mode, sizeof(display_mode));
			if (display_mode == 0)
			{
				// 显示模式无效时，彩色相机默认为彩色，黑白相机默认为单色
				if (device_capability_.colorCapability == 1)
				{
					display_mode = HscDisplayMode::HSC_DISPLAY_COLOR;
				}
				else
				{
					display_mode = HSC_DISPLAY_MONO;
				}
			}

			if (DEVICE_PCCSeries != info_.model)
			{
				//颜色校正参数
				HscColorCorrectInfo info;
				res = getColorCorrectInfo(info);
				if (res != HSC_OK)
				{
					break;
				}
				setColorCorrectInfo(info);


			}

			// 重设显示模式，用于图像处理
			HscDevSet(device_handle_, HSC_CFG_DISPLAY_MODE, &display_mode, sizeof(display_mode));
		}

    } while(0);

    if (res == HSC_OK)
    {
        // 连接成功后，更新视频列表
		if (IsCamera())
		{
			updateVideoSegmentList();
		}

		//启动消息处理线程
		startMsgProcessThread();

		//加载设备信息
		ReloadDeviceInfo();

    }
	CSLOG_INFO("{} Connected,res ={} ", getIpOrSn().toStdString(),res);

	//优先发送连接信号,确保界面正常连接其他信号
	if (emit_device_connected)
	{
		if (res == HSC_OK)
		{
			emit deviceConnected(getIpOrSn());
		}
		else
		{
			emit deviceDisconnected(getIpOrSn());
		}
	}

    if (update_state)
    {
// 		if (!IsStandByModeSupported())
// 		{
// 			setState(res == HSC_OK ? DeviceState::Connected : DeviceState::Unconnected);
// 		}
// 		else
		{
			if (res == HSC_OK)
			{


				DeviceState state = GetWorkStatusFromDevice();
				setState(state);

				if (IsCamera() && (state == Acquiring || state == Previewing || state == Recording))
				{
					if (state == Acquiring)
					{
						m_next_acquiring_enabled = true;
					}
					emit orgStateIsAcquring(getIpOrSn());//正在输出图像,添加到主视图中

					if (!realtime_frame_timer_ptr_->isActive())
					{
						emit startedUpdateRealtimeFrameTimer();
					}
				}
			}
			else
			{
				setState(DeviceState::Unconnected);
			}
		}
    }


	if (res == HSC_OK)
	{

		QElapsedTimer timer;//调试:判断设置超时的参数
		timer.restart();
		//保存设备配置到本地
		saveConfig2Local();
		if (timer.hasExpired(200))
		{
			CSLOG_ERROR("saveAllConfig2Local time out:{}ms", timer.elapsed());
		}
	}

	//西光所版本开启运行计时
	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion() && res == HSC_OK)
	{
		UpdateCurBootTimeAndTotalWorkTime();
		m_boot_and_work_record_timer_ptr->start();
	}


	//授权提示
	if (IsCamera()&& res == HSC_OK &&  (HSC_AUTH_ERROR_NULL == info_.auth_info.authType || HSC_AUTH_ERROR_OVERDUE == info_.auth_info.authType))
	{
		//再次主动获取授权信息
		HscAuthInfo authInfo;
		if (GetAuthInfo(&authInfo) == HSC_OK)
		{

			if (authInfo.authType == HSC_AUTH_ERROR_NULL ||
				authInfo.authType == HSC_AUTH_ERROR_OVERDUE)
			{
				HscResult resOuttime = HSC_AUTHORIZATION_OVERDUE;
				emit errorOccurred(resOuttime);
			}	
		}
		else//再次获取授权信息失败
		{
			HscResult resOuttime = HSC_AUTHORIZATION_UNKNOW_ERR; 
			emit errorOccurred(resOuttime);
		}
	}

    return res;
}

HscResult Device::previewDevice()
{
    DeviceState pre_state = getState();
	if (pre_state == DeviceState::Unconnected || pre_state == DeviceState::Disconnected)
	{
		// 未连接时先连接
		HscResult connect_res = connectDevice(true,true);
		if (connect_res!=HSC_OK)
		{
			if(m_bAutoConnect)
			{
				// [2022/8/17 rgq]: 自动连接不弹出错误
				m_bAutoConnect = false;
				return HSC_OK;//预览时连接失败直接报网络错误,避免自动连接全部设备时频繁弹窗 
			}
			else
			{
				return connect_res;
			}
		}
		else
		{
			if (needsDataRecovery())
			{
				setState(DeviceState::Connected);
				emit SignalRecoveringData();
				return HSC_OK;
			}
		}
		if (getState() != Connected)
		{
			return connect_res;
		}
		pre_state = getState();
	}

	CSLOG_INFO("{} ToPreview ", getIpOrSn().toStdString());

    HscResult res = HSC_OK;
    DeviceState state = DeviceState::Previewing;
    do
    {
        if (pre_state == DeviceState::Previewing)
        {
            break;
		}
		setState(DeviceState::ToPreview);


        // 直接先停机
        stopDevice(false, m_export_mode_);

		//应用bit位
		image_processor_ptr_->SetSignificantBitsPerPixel(current_out_bpp_);
		image_processor_ptr_->SetBitsPerPixel(current_out_bpp_ <= 8 ? 8 : 16);
		image_processor_ptr_->setFilerEnable(mFilterEnable);
		
		//应用像素融合模式
		if (IsBinningModeSupported())
		{
			bool binning_enable = false;
			GetBinningMode(binning_enable);
			image_processor_ptr_->SetBinningModeEnable(binning_enable);
		}

        res = HscStartPreviewMode(device_handle_);
        if (res != HSC_OK)
        {
                stopDevice(false);
                state = DeviceState::Connected;
        }
		else
		{
			if (IsCamera() && (state == Acquiring || state == Previewing || state == Recording))
			{
				if (state == Acquiring)
				{
					m_next_acquiring_enabled = true;
				}
				emit orgStateIsAcquring(getIpOrSn());//正在输出图像,添加到主视图中

				qDebug() << "connectDevice emit orgStateIsAcquring ip = " << getIpOrSn();
				if (!realtime_frame_timer_ptr_->isActive())
				{
					emit startedUpdateRealtimeFrameTimer();
				}
			}
			CSLOG_INFO("{} Preview OK ", getIpOrSn().toStdString());
		}
    } while(0);

    setState(state);

    return res;
}

HscResult Device::acquireDevice()
{
    DeviceState pre_state = getState();

	if (pre_state == DeviceState::Unconnected || pre_state == DeviceState::Disconnected)
	{
		// 未连接时先连接
		HscResult connect_res = connectDevice(true, true);
		if (connect_res != HSC_OK)
		{
			return HSC_NETWORK_ERROR;//预览时连接失败直接报网络错误,避免自动连接全部设备时频繁弹窗
		}
		else
		{
			if (needsDataRecovery())
			{
				setState(DeviceState::Connected);
				emit SignalRecoveringData();
				return HSC_OK;
			}
		}
	}
    setState(DeviceState::ToAcquire);
	CSLOG_INFO("{} to acquire ", getIpOrSn().toStdString());
    HscResult res = HSC_OK;
    DeviceState state = DeviceState::Acquiring;
    do
    {
        if (pre_state == DeviceState::Acquiring)
        {
            break;
        }
		
		if (pre_state != DeviceState::Connected)
        {
            // 未停机时先停机
            stopDevice(false, m_export_mode_);
        }

       // 应用属性
		QList<PropType> device_prop_types = getAcquirePropertyTypes();
		for (auto type : device_prop_types)
		{
			res = setProperty2Device(type, getProperty(type));
			if (res != HSC_OK)
			{
				break;
			}
		}
		if (res != HSC_OK)
		{
			break;
		}

		//应用bit位
		image_processor_ptr_->SetSignificantBitsPerPixel(current_out_bpp_);
		image_processor_ptr_->SetBitsPerPixel(current_out_bpp_ <= 8 ? 8 : 16);
		image_processor_ptr_->setFilerEnable(mFilterEnable);

		//应用像素融合模式
		if (IsBinningModeSupported())
		{
			bool binning_enable = false;
			GetBinningMode(binning_enable);
			image_processor_ptr_->SetBinningModeEnable(binning_enable);
		}

        res = HscStartHighMode(device_handle_);
        if (res != HSC_OK)
		{
			break;
        }
		else
		{
			CSLOG_INFO("{} acquring ", getIpOrSn().toStdString());
		}
		m_next_acquiring_enabled = true;//录制结束继续进入高采模式
    } while(0);

	if (res != HSC_OK)
	{
		stopDevice(false);
		state = DeviceState::Connected;	// 进入高采失败, 此时应该还在连接状态
	}
    setState(state);

    return res;
}

HscResult Device::triggerDevice()
{
    DeviceState state = DeviceState::Recording;

    uint64_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    HscResult res = HscSoftTrig(device_handle_, timestamp);

	CSLOG_INFO("soft trigger, ret ={}", (int)res);
    if (res != HSC_OK)
    {
        disconnectDevice(false);
        state = DeviceState::Unconnected;
    }
    setState(state);
    return res;
}

HscResult Device::stopDevice(bool update_state, uint8_t stop_mode)
{
    if (update_state)
    {
        setState(DeviceState::Connected);
    }
	
	HscResult res = HSC_OK;
	if (stop_mode != eNoStop) res = HscStop(device_handle_);
	CSLOG_INFO("{} stop device ,res = {}", getIpOrSn().toStdString(), res);

    // 停止实时帧刷新
    realtime_frame_timer_ptr_->stop();

#if TEST_THREAD_PROCESS_FRAME
	exitRccImageProcessThread();
#endif
    return res;
}

HscResult Device::stopCaptureDevice(bool save_video_clip, bool next_acquiring_enabled /*= false*/)
{
	m_next_acquiring_enabled = next_acquiring_enabled;

	HscResult res = HscStopCapture(device_handle_, save_video_clip);
	CSLOG_INFO("{} stop capture ,res = {}",getIpOrSn().toStdString(),res);
	// 停止实时帧刷新
	realtime_frame_timer_ptr_->stop();

#if TEST_THREAD_PROCESS_FRAME
	exitRccImageProcessThread();
#endif

	setState(DeviceState::Connected);
	return res;
}

HscResult Device::disconnectDevice(bool update_state, bool emit_device_disconnected)
{
    DeviceState pre_state = getState();

    if (update_state)
    {
        setState(DeviceState::Disconnecting);
    }

    if (pre_state == DeviceState::Previewing || 
		pre_state == DeviceState::Acquiring || 
		pre_state == DeviceState::Recording ||
		pre_state == DeviceState::Reconnecting ||
		pre_state == DeviceState::Disconnected||
		DeviceState::Exporting == pre_state)
    {
        stopDevice(false, 0);
    }


    HscCloseDevice(device_handle_);
	CSLOG_INFO("{} device closed.", getIpOrSn().toStdString());
	//西光所版本关闭运行计时
	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		m_boot_and_work_record_timer_ptr->stop();
	}

    if (update_state)
    {
        setState(DeviceState::Unconnected);
    }

    if (emit_device_disconnected)
    {
		emit videoSegmentListUpdated();
        emit deviceDisconnected(getIpOrSn());
    }

    return HSC_OK;
}


void Device::initCF18HeartBeatThread()
{
	if (!IsCF18())
	{
		return;
	}
	exitCF18HeartBeatThread();
	bcf18_heart_beat_thread_exit_ = false;
	cf18_heart_beat_thread_ = std::move(std::thread(&Device::doCF18HeartBeat, this));
	CSLOG_INFO("{} start CF18 heart beat.", getIpOrSn().toStdString());

}

void Device::exitCF18HeartBeatThread()
{
	bcf18_heart_beat_thread_exit_ = true;
	if (cf18_heart_beat_thread_.joinable())
	{
		cf18_heart_beat_thread_.join();
	}
	//析构时输出日志导致崩溃,暂时去除
	//CSLOG_INFO("{} stop CF18 heart beat.", getIpOrSn().toStdString());

}

void Device::doCF18HeartBeat()
{
	while (!bcf18_heart_beat_thread_exit_)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
		if (IsCF18()) {
			if (CF18Open(getIpOrSn().toStdString(), 8080)) {
			}
			else {
				bcf18_heart_beat_thread_exit_ = true;
				setState(Disconnected);
				emit errorOccurred(HSC_NETWORK_ERROR, false);
				break;
			}
		}

	}

}

void Device::updateRealtimeFrame()
{
	if (!m_bManualWhiteBalance)
	{
#if TEST_THREAD_PROCESS_FRAME
		RccImageFrameInfo rccFrame;
		if (GetOneRccImageFrame(rccFrame))
		{
			if (rccFrame.image.image.isNull())
			{
				CSLOG_INFO("RccImageFrameInfo qImage is empty...");
			}

			emit updateFrame(rccFrame.image);
			if (isGType()|| IsIntelligentTriggerV5Supported()) {
				emit SignalUpdateFrameIntelligentAvgBright(info_.ip, (int)rccFrame.active_pixel_num_);
			}
			else {
				emit SignalUpdateFrameIntelligentAvgBright(info_.ip, rccFrame.image_trigger_avg_avg_lum);
			}
			emit SignalUpdateFrameAutoExposureAvgGray(info_.ip, rccFrame.auto_exposure_area_avg_lum);
		}
		return;
#endif
	}
	//qDebug() << "HscGetLastFrame:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
	DeviceState cur_state = getState();//切换状态时不刷新图像,避免界面锁死
	if (cur_state == Connecting ||
		cur_state == ToAcquire ||
		cur_state == ToExport ||
		cur_state == ToPreview ||
		cur_state == Disconnecting ||
		cur_state == Reconnecting ||
		cur_state == Connected)
	{
		return;
	}
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	DeviceState state = getState();
	if ((state != DeviceState::Previewing) && (state != DeviceState::Acquiring) && (state != DeviceState::Recording))
	{
		return;
	}
	

    CAGBuffer* buffer_ptr = HscGetLastFrame(device_handle_);
    if (buffer_ptr && isLastestFrame(buffer_ptr->frame_head.time_stamp))
	{
		memcpy(last_realtime_timestamp_, buffer_ptr->frame_head.time_stamp, 9);
		if (m_bManualWhiteBalance)
		{
			RccFrameInfo frameInfo{};
			if (m_frame_head_type_ == HEAD_TYPE::eMType) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestamp(last_realtime_timestamp_)));
			}
			else if (m_frame_head_type_ == HEAD_TYPE::eGTypeNs) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampG(last_realtime_timestamp_)));
			}
			else if (m_frame_head_type_ == HEAD_TYPE::eS1315) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampNs(last_realtime_timestamp_)));
			}
			else {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatTimestamp(last_realtime_timestamp_)));
			}
			frameInfo.device_name = getProperty(PropName).toString();
			frameInfo.ip_or_sn = getIpOrSn();
			// 获取视频扩展信息
			getVideoExtendInfo(buffer_ptr->frame_head, frameInfo.extend_info);

			for (int i = 0; i < 9; i++)
			{
				frameInfo.timestamp[i] = last_realtime_timestamp_[i];
			}

			// 进行手动白平衡的时候图像在其内进行处理，这里不做转换了，主界面的图像不再更新
			emit updateWhiteBalanceFrame(buffer_ptr, frameInfo);
			return;
		}

        DeviceState state = getState();
		cv::Mat image;
		if (isGType() && buffer_ptr->frame_head.rect.width *  buffer_ptr->frame_head.rect.height > 3 * 1024 * 1024)
		{
			image = image_processor_ptr_->cv_process_no_copy(buffer_ptr, state == DeviceState::Previewing ? 0 : 1, true);
		}
		else
		{
			image = image_processor_ptr_->cv_process(buffer_ptr, state == DeviceState::Previewing ? 0 : 1, true);
		}

        image = image_processor_ptr_->cv_process(image, getContrast(), getLuminance(), false);
		QImage qt_image;
		//void *ddd = image.data;
		CPlayerViewBase::cvMat2QImage(image, qt_image);
		//void *dddd = qt_image.bits();
	    //qDebug() << "HscGetLastFrame finish process" <<  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");;
		RccFrameInfo frameInfo{};
		if (m_frame_head_type_ == HEAD_TYPE::eMType) {
			frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestamp(last_realtime_timestamp_)));
		}
		else if (m_frame_head_type_ == HEAD_TYPE::eGTypeNs) {
			frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampG(last_realtime_timestamp_)));
		}
		else if (m_frame_head_type_ == HEAD_TYPE::eS1315) {
			frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampNs(last_realtime_timestamp_)));
		}
		else {
			frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatTimestamp(last_realtime_timestamp_)));
		}

		frameInfo.raw_image = image.clone();
		frameInfo.valid_bits = buffer_ptr->frame_head.bpp;

		frameInfo.image = qt_image;
		frameInfo.device_name = getProperty(PropName).toString();
		frameInfo.ip_or_sn = getIpOrSn();
		// 获取视频扩展信息
		getVideoExtendInfo(buffer_ptr->frame_head, frameInfo.extend_info);

		for (int i = 0 ; i < 9 ; i++)
		{
			frameInfo.timestamp[i] = last_realtime_timestamp_[i];
		}

		//西光所xj1310调试信息中输出俯仰角信息
		if (FunctionCustomizer::GetInstance().isXiguangsuoVersion() && getModel() == DEVICE_XJ1310)
		{
			printf("azimuth=%d, pitch=%d, focal_length=%d\n", frameInfo.extend_info.azimuth, frameInfo.extend_info.pitch, frameInfo.extend_info.focal_length);
		}
		int nBits = getProperty(PropPixelBitDepth).toInt();
		int image_trigger_avg_avg_lum = buffer_ptr->frame_head.image_trigger_avg_avg_lum;
		int auto_exposure_area_avg_lum = buffer_ptr->frame_head.auto_exposure_area_avg_lum;
		if ((IsIntelligentTriggerV4Supported() || isSupportHighBitParam())&& nBits >= 8)
		{
			image_trigger_avg_avg_lum = ImageBitsChange(image_trigger_avg_avg_lum, 16, nBits);
			auto_exposure_area_avg_lum = ImageBitsChange(auto_exposure_area_avg_lum, 16, nBits);
		}

		//QString timestamp_str = DeviceUtils::formatTimestamp(buffer_ptr->frame_head.time_stamp);
		//qDebug() << timestamp_str << ","<< QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
		emit updateFrame(frameInfo);
		emit SignalUpdateFrameIntelligentAvgBright(info_.ip, image_trigger_avg_avg_lum);
		emit SignalUpdateFrameAutoExposureAvgGray(info_.ip, auto_exposure_area_avg_lum);
		//qDebug() << "HscGetLastFrame finish updateFrameupdateFrame:" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    }

}

void Device::getVideoExtendInfo(const FrameHead & frame_head, VideoExtendInfo & extend_info)
{
	extend_info.device_model = getModel();
	extend_info.device_index = GetDeviceIndex();
	extend_info.device_state = 0;
	memcpy(&extend_info.azimuth, &frame_head.fvalues[0], sizeof(int32_t));
	memcpy(&extend_info.pitch, &frame_head.fvalues[1], sizeof(int32_t));
	extend_info.distance = (extend_info.device_model == DEVICE_XJ1310) ? 0x7FFFFFFFFFFFFFFFL : 0L;
	memcpy(&extend_info.focal_length, &frame_head.fvalues[2], sizeof(int32_t));
	extend_info.frame_rate = m_fps_backup;
	extend_info.frame_header_rows = GetExtraHeight();
	extend_info.exposure_time = frame_head.exposure_time;
	agile_device::capability::Units exposure_time_unit = GetRealExposureTimeUnit();
	if (exposure_time_unit == agile_device::capability::kUnit100ns)
	{
		extend_info.exposure_time /= 10;
	}
	else if (exposure_time_unit == agile_device::capability::kUnit10ns)
	{
		extend_info.exposure_time /= 100;
	}


}

uint16_t Device::GetExtraHeight() const
{
	if (info_.model == DEVICE_XJ520_XG || info_.model == DEVICE_XJ1310)
	{
		return 4;
	}

	return 0;
}

void Device::setState(DeviceState state)
{
	CSLOG_INFO("setState, {}->{}", state_, state);
    if (state_ != state)
    {
        state_ = state;

		auto ip = getIpOrSn();
		if (IsCF18()) {
			emit cf18StateChanged(ip, state);
			updateViewOSD();
			return;
		}
        emit stateChanged(ip, state);

		DeviceState cur_state = getState();
		if (cur_state != Recording )//录制中不更新空白主界面,避免闪屏
		{
			updateViewOSD();
		}
      
    }

}

HscResult Device::applyProperties2Device(bool from_local)
{
    HscResult res = HSC_OK;

	QList<PropType> device_prop_types = getSupportedPropertyTypes(nullptr);
    for (auto type : device_prop_types)
    {
        res = setProperty2Device(type, getProperty(type,from_local));
        if (res != HSC_OK)
        {
            break;
        }
    }

    return res;
}



HscResult Device::setProperty2Device(Device::PropType type, const QVariant &value)
{
	if (IsCF18()) {
		auto iter = map_cf18_prop_set_.find(type);
		if (iter == map_cf18_prop_set_.end())
		{
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
		QElapsedTimer timer;//调试:判断设置超时的参数
		timer.start();
		HscResult res = iter->second(type, value);
		if (HSC_OK != res) {
			CSLOG_ERROR("paste param fail, device_type:{}, return:{}", type, res);
		}
		if (timer.hasExpired(20))
		{
			QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
			CSLOG_ERROR("setProperty2Device time out:{}ms, propKey:{}", timer.elapsed(), QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type)).toStdString());
		}
		return res;
	}

    auto iter = map_prop_set_.find(type);
    if (iter == map_prop_set_.end())
    {
		CSLOG_ERROR("Illegal parameters, device_type:{}", type);
        return HSC_INVALID_PARAMETER;
    }
	QElapsedTimer timer;//调试:判断设置超时的参数
	timer.start();
	HscResult res = iter->second(type, value);
	if (HSC_OK != res) {
		CSLOG_ERROR("paste param fail, device_type:{}, return:{}", type, res);
	}
	if (timer.hasExpired(20))
	{
		QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
		CSLOG_ERROR("setProperty2Device time out:{}ms, propKey:{}", timer.elapsed(), QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type)).toStdString());
	}
	return res;
}

void Device::saveConfig2Local()
{
	QList<PropType> type_list = getSavingPropertyTypes();
	for (auto type : type_list)
	{
		QElapsedTimer timer;//调试:判断设置超时的参数

		QVariant value = getProperty(type);
		timer.start();
		QSettings settings;
		if (PropType::PropExposureTime == type) {
			DWORD exposure_time = ConvertExposureTime(value.toLongLong(), getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(),
				GetRealExposureTimeUnit(), 1);
			settings.setValue(this->settingsKey(type), QVariant::fromValue(exposure_time));

		}
		else {
			settings.setValue(this->settingsKey(type), value);
		}
		
		if (timer.hasExpired(20))
		{
			QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
			CSLOG_ERROR("saveConfig2Local time out:{}ms, propKey:{}", timer.elapsed(), QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type)).toStdString());
		}
	}
}

QString Device::settingsKey(PropType type)
{
    QMetaEnum meta_enum = QMetaEnum::fromType<PropType>();
    return QString("%1/%2").arg(getIpOrSn()).arg(meta_enum.valueToKey(type));
}

bool Device::settingsNeedsLoadFromDevice() const
{
    return (device_capability_.offlineCapability == 1) || (device_capability_.getParamsFromDevCapability == 1);
}

void Device::applySettingsToDevice()
{

}

void Device::getMaxRoi(QRect &roi, RoiTypes roi_type) const
{
	if (roi_type == kDeviceRoi)
	{
		roi.setX(0);
		roi.setY(0);
		roi.setWidth(image_processor_ptr_->GetDisplayWidthMax());
		roi.setHeight(image_processor_ptr_->GetDisplayHeightMax());
	}
	else {
		QRect device_roi = GetRoi(kDeviceRoi);
		roi =  device_roi;
	}
}

void Device::getTypicalResolutions(QVariantList &resolutions  , bool need_costom) const
{
    HscIntRange x_range{};
    if (!getRoiXRange(x_range))
    {
        return;
    }

    HscIntRange y_range{};
    if (!getRoiYRange(y_range))
    {
        return;
    }

    HscIntRange width_range{};
    if (!getRoiWidthRange(0, width_range))
    {
        return;
    }

    HscIntRange height_range{};
    if (!getRoiHeightRange(0, height_range))
    {
        return;
    }

	bool custom = true;

	// 当前分辨率
	QRect current_resolution = getProperty(Device::PropRoi).toRect();

    // 最大分辨率
	QRect max_resolution = QRect(0, 0, width_range.max, height_range.max);
    resolutions.push_back(max_resolution);
	if (current_resolution == max_resolution)
	{
		custom = false;
	}

    QList<QSize> general_typical_resolution;
    DeviceUtils::getGeneralTypicalResolutions(general_typical_resolution,getModel(), m_resolution_ratio_type_);
    for (auto res : general_typical_resolution)
    {
		//去除已经添加的默认最大分辨率
		if ((res.width() == width_range.max) && (res.height() == height_range.max))
		{
			continue;
		}

        // 宽度有效性校验
        if ((res.width() < width_range.min) || (res.width() > width_range.max) || (width_range.inc!=0 && (res.width() % width_range.inc != 0)))
        {
            continue;
        }

        // 高度有效性校验
        if ((res.height() < height_range.min) || (res.height() > height_range.max) || (height_range.inc != 0 && (res.height() % height_range.inc != 0)))
        {
            continue;
        }

        // 横向居中
        int x = (width_range.max - res.width()) / 2;
        x = ALIGNDOWN(x, x_range.inc);

        // 纵向居中
        int y = (height_range.max - res.height()) / 2;
        y = ALIGNDOWN(y, y_range.inc);

		//if (isGrabberDevice()) {
		//	x = 0;
		//	y = 0;
		//}
		QRect resolution = QRect(x, y, res.width(), res.height());
        resolutions.push_back(resolution);

		if (current_resolution == resolution)
		{
			custom = false;
		}
    }

	if (custom && need_costom)
	{
		resolutions.push_back(current_resolution);
	}
}

void Device::getTypicalFrameRates(QVariantList &frame_rates) const
{
    HscIntRange acquisition_period_range{};
    if (!getAcquisitionPeriodRange(getProperty(PropType::PropRoi).toRect(), acquisition_period_range))
    {
        return;
    }

    qint64 min_frame_rate = toFrameRate(getCalPeriodDivisor(), acquisition_period_range.max);
    qint64 max_frame_rate = toFrameRate(getCalPeriodDivisor(), acquisition_period_range.min);

    QList<qint64> general_typical_frame_rates;
    DeviceUtils::getGeneralTypicalFrameRates(general_typical_frame_rates);
    for (auto frame_rate : general_typical_frame_rates)
    {
        if (frame_rate < min_frame_rate || frame_rate >= max_frame_rate)
        {
            continue;
        }

        frame_rates.push_back(frame_rate);
    }

    // 最大帧率
    frame_rates.push_back(max_frame_rate);
}

void Device::getTypicalExposureTimes(QVariantList &exposure_times) const
{
    qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());

    HscIntRange range{};
    if (!getExposureTimeRange(acquisition_period, getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(), range))
    {
        return;
    }

    QList<qint64> general_typical_exposure_times;
    DeviceUtils::getGeneralTypicalExposureTimes(general_typical_exposure_times);
    for (auto exposure_time : general_typical_exposure_times)
    {
        if (exposure_time < range.min || exposure_time > range.max)
        {
            continue;
        }

        exposure_times.push_back(exposure_time);
    }
}

bool Device::getRoiXRange(HscIntRange &range,RoiTypes roi_type) const
{
	if (roi_type != kDeviceRoi)
	{
		QRect device_roi = GetRoi(kDeviceRoi);
		range.min = device_roi.left();
		range.max = device_roi.left() + device_roi.width() - 1;
		range.inc = 1;
		return true;
	}
    std::string requestStr, responseStr;
    if (getDeviceCapability(HSC_CT_ROI_OFFSET_X_RANGE, requestStr, responseStr) == HSC_OK)
    {
        agile_device::capability::roi_offset_x_range::Response response;
        if (response.parse(responseStr))
        {
            range.min = response.range.min;
            range.max = response.range.max;
            range.inc = response.range.inc;
            range.unit = response.range.inc;

			if (range.inc == 0) return false;
            if (range.inc != 1)
            {
                range.min = ALIGNUP(range.min, range.inc);
                range.max = ALIGNDOWN(range.max, range.inc);
            }

            return true;
        }
    }

    return false;
}

bool Device::getRoiYRange(HscIntRange &range,RoiTypes roi_type) const
{
	if (roi_type != kDeviceRoi)
	{
		QRect device_roi = GetRoi(kDeviceRoi);
		range.min = device_roi.top();
		range.max = device_roi.top() + device_roi.height() - 1;
		range.inc = 1;
		return true;
	}
    std::string requestStr, responseStr;
    if (getDeviceCapability(HSC_CT_ROI_OFFSET_Y_RANGE, requestStr, responseStr) == HSC_OK)
    {
        agile_device::capability::roi_offset_y_range::Response response;
        if (response.parse(responseStr))
        {
            range.min = response.range.min;
            range.max = response.range.max;
            range.inc = response.range.inc;
            range.unit = response.range.inc;

            if (range.inc != 0 && range.inc != 1)
            {
                range.min = ALIGNUP(range.min, range.inc);
                range.max = ALIGNDOWN(range.max, range.inc);
            }

            return true;
        }
    }

    return false;
}

bool Device::getRoiWidthRange(qint64 x, HscIntRange &range, RoiTypes roi_type) const
{
	if (roi_type != kDeviceRoi)
	{
		QRect device_roi = GetRoi(kDeviceRoi);
		range.min = 1;
		if (x <= device_roi.x())
		{
			range.max = device_roi.width();
		}
		else
		{
			range.max = device_roi.x() + device_roi.width() - x;
		}

		range.inc = 1;

		return true;
	}
    agile_device::capability::roi_width_range::Request request;
    request.offsetX = x;

    std::string requestStr;
    if (request.pack(requestStr))
    {
        std::string responseStr;
        if (getDeviceCapability(HSC_CT_ROI_WIDTH_RANGE, requestStr, responseStr) == HSC_OK)
        {
            agile_device::capability::roi_width_range::Response response;
            if (response.parse(responseStr))
            {
                range.min = response.range.min;
                range.max = response.range.max;
                range.inc = response.range.inc;
                range.unit = response.range.inc;

                if (range.inc != 0 && range.inc != 1)
                {
                    range.min = ALIGNUP(range.min, range.inc);
                    range.max = ALIGNDOWN(range.max, range.inc);
                }

                return true;
            }
        }
    }

    return false;
}

bool Device::getRoiHeightRange(qint64 y, HscIntRange &range, RoiTypes roi_type) const
{
	if (roi_type != kDeviceRoi)
	{
		QRect device_roi = GetRoi(kDeviceRoi);
		range.min = 1;
		if (y <= device_roi.y())
		{
			range.max = device_roi.height();
		}
		else
		{
			range.max = device_roi.y() + device_roi.height() - y;
		}
		range.inc = 1;

		return true;
	}
    agile_device::capability::roi_height_range::Request request;
    request.offsetY = y;

    std::string requestStr;
    if (request.pack(requestStr))
    {
        std::string responseStr;
        if (getDeviceCapability(HSC_CT_ROI_HEIGHT_RANGE, requestStr, responseStr) == HSC_OK)
        {
            agile_device::capability::roi_height_range::Response response;
            if (response.parse(responseStr))
            {
                range.min = response.range.min;
                range.max = response.range.max;
                range.inc = response.range.inc;
                range.unit = response.range.inc;

                if (range.inc != 0 && range.inc != 1)
                {
                    range.min = ALIGNUP(range.min, range.inc);
                    range.max = ALIGNDOWN(range.max, range.inc);
                }

                return true;
            }
        }
    }

    return false;
}

uint16_t Device::GetDeviceIndex() const
{
	return getProperty(PropDeviceIndex).toInt();
}

agile_device::capability::Units Device::GetRealExposureTimeUnit() const
{
	if (device_capability_.exposureTimeUnit == 0)
	{
		return agile_device::capability::kUnitUs;
	}

	if (device_capability_.exposureTimeUnit == 1)
	{
		return agile_device::capability::kUnit100ns;
	}

	if (device_capability_.exposureTimeUnit == 2)
	{
		return agile_device::capability::kUnit10ns;
	}
	if (device_capability_.exposureTimeUnit == 3)
	{
		return agile_device::capability::kUnitNs;
	}

	return agile_device::capability::kUnitUs;
}

int64_t Device::ConvertExposureTime(int64_t expsoure_time, agile_device::capability::Units src_unit, agile_device::capability::Units dest_unit, int64_t frame_rate, bool do_correct)
{
	double destTime = convertTime(expsoure_time, src_unit, dest_unit);

	if (do_correct)
	{
		HscIntRange range;
		if (GetExposureTimeRangeByFrameRate(frame_rate, dest_unit, range))
		{
			if (destTime < range.min)
			{
				return range.min;
			}

			if (destTime > range.max)
			{
				return range.max;
			}
		}
	}

	return (int64_t)destTime;
}

Device::DrawTypeStatusInfo Device::getDrawTypeStatusInfo()
{
	return m_iDrawTypeStatusInfo;
}

void Device::setDrawTypeStatusInfo(DrawTypeStatusInfo status)
{
	//m_iDrawTypeStatusInfo = DTSI_Noraml;
	m_iDrawTypeStatusInfo = status;
	emit signalUpdateDrawStatusInfo(m_iDrawTypeStatusInfo);
}

	// 修正二次曝光
void Device::correctEdrExposure()
{
	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);
	if (settingsNeedsLoadFromDevice())
	{
		DWORD exposure_time = 1;
		auto res = HscGetExposureTime(device_handle_, &exposure_time);
		if (res != HSC_OK)
		{
			return;			
		}
		if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam()) {

			HscEDRParamV2 param{};
			auto res = HscGetEDRParamV2(device_handle_, &param);
			if (res != HSC_OK)
			{
				return;
			}
			if (param.doubleExposureTime > exposure_time)
			{
				param.doubleExposureTime = exposure_time;
				HscSetEDRParamV2(device_handle_, param);
			}
		}
		else {
			HscEDRParam param{};
			res = HscGetEDRParam(device_handle_, &param);
			if (res != HSC_OK)
			{
				return;
			}
			if (param.doubleExposureTime > exposure_time)
			{
				param.doubleExposureTime = exposure_time;
				HscSetEDRParam(device_handle_, param);
			}
		}
	}
}

bool Device::GetExposureTimeRangeByFrameRate(int64_t frameRate, agile_device::capability::Units exposureTimeUnit, HscIntRange & range)
{
	return getExposureTimeRange(toAcquisitionPeriod(frameRate), exposureTimeUnit, range);
}

HscResult Device::GetNetConfig(HscDeviceNetParam & net_param) const
{
	return HscGetDeviceNetConfig(device_handle_, &net_param);
}

HscResult Device::SetNetConfig(const HscDeviceNetParam & net_param)
{
	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);
	HscResult res = HscSetDeviceNetConfig(device_handle_, net_param);
	if (res != HSC_OK)
	{
		emit errorOccurred(res, m_bShowTip);
	}
	return res;
}

bool Device::getAcquisitionPeriodRange(const QRect &roi, HscIntRange & range) const
{
    agile_device::capability::acquisition_period_range::Request request;
    request.roi.x = roi.x();
    request.roi.y = roi.y();
    request.roi.width = roi.width();
    request.roi.height = roi.height();

    request.exposure_time = 0;
    request.exposure_time_unit = (agile_device::capability::Units)getPeriodUnit();

    std::string requestStr;
    if (!request.pack(requestStr))
    {
        return false;
    }

    std::string responseStr;
    if (getDeviceCapability(HSC_CT_ACQUISITION_PERIOD_RANGE, requestStr, responseStr) != HSC_OK)
    {
        return false;
    }

    agile_device::capability::acquisition_period_range::Response response;
    if (!response.parse(responseStr))
    {
        return false;
    }

    range.min = response.range.min;
    range.max = response.range.max;
    range.inc = response.range.inc;
    range.unit = response.range.unit;

    return true;
}

bool Device::getExposureTimeRange(qint64 acquisition_period, agile_device::capability::Units exposureTimeUnit, HscIntRange &range) const
{
    agile_device::capability::exposure_time_range::Request request;
    request.acquisition_period = acquisition_period;

    std::string requestStr;
    if (request.pack(requestStr))
    {
        std::string responseStr;
        if (getDeviceCapability(HSC_CT_EXPOSURE_TIME_RANGE, requestStr, responseStr) == HSC_OK)
        {
            agile_device::capability::exposure_time_range::Response response;
            if (response.parse(responseStr))
            {
                range.min = response.range.min;
                range.max = response.range.max;
                range.inc = response.range.inc;
                range.unit = response.range.unit;
				//单位转换
				range.min = (int64_t)ceil(convertTime(range.min, (agile_device::capability::Units) range.unit, exposureTimeUnit));
				range.max = (int64_t)floor(convertTime(range.max, (agile_device::capability::Units) range.unit, exposureTimeUnit));
				range.unit = exposureTimeUnit;
				if (range.max < range.min) range.min = range.max;
                return true;
            }
        }
    }

    return false;
}

HscResult Device::getDeviceCapability(HscCapabilityType type, const std::string &in, std::string &out) const
{
    QByteArray xml_byte_array(1024*1024, 0);
    HscResult res = HscGetDeviceCapability(device_handle_, type, const_cast<char*>(in.c_str()), DWORD(in.length()), xml_byte_array.data(), xml_byte_array.size());
    if (res == HSC_OK)
    {
        out = xml_byte_array.data();
    }

    return res;
}

HscResult Device::GetAllDiskVolumes(std::vector<HscDiskMessage> & diskmessages) const {
	HscResult res = HscGetAllDiskVolumes(device_handle_, diskmessages);
	return res;
}
HscResult Device::GetCurrentDisk(std::string &disk_names) {
	HscResult res = HscGetCurrentDisk(device_handle_, disk_names);
	return res;
}
HscResult Device::SetCurrentDisk(std::string disk_names) const {
	HscResult res = HscSetCurrentDisk(device_handle_, disk_names);
	return res;
}

bool Device::isLastestFrame(BYTE time_stamp[]) const
{
    if (memcmp(time_stamp, last_realtime_timestamp_, 9) != 0)
    {
        return true;
    }

    if (memcmp(time_stamp, kZeroTimestamp, 9) == 0)
    {
        return true;
    }

    return false;
}

int Device::getLuminance() const
{
    return getProperty(Device::PropLuminance).toInt();
}

int Device::getContrast() const
{
    return getProperty(Device::PropContrast).toInt();
}

DeviceModel Device::getModel() const
{
    return info_.model;
}
bool Device::isGrabberDevice() const {
	if (DeviceModel::DEVICE_GRABBER_100T <= info_.model && info_.model <= DeviceModel::DEVICE_GRABBER_SIMULATE) {
		return true;
	}
	else {
		return false;
	}
}

bool Device::isGatewayDevice() const
{
	if (HScDevDiscoveryType::DT_GATEWAY == info_.discovery_type) {
		return true;
	}
	else {
		return false;
	}
}

void Device::setDeviceInfo(HscDeviceInfo info)
{
	info_ = info;
}

HscResult Device::startExport(int id)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	DeviceState state = getState();
	if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
	{
		return HSC_IO_ERROR;
	}
	HscResult res = HscStartExport(device_handle_, id);
	state = getState();
	if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
	{
		return HSC_IO_ERROR;
	}
	if (res == HSC_OK)
	{
		setState(DeviceState::Exporting);
	}
    return res;
}

HscResult Device::stopExport()
{
	LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);
	DeviceState state = getState();
	if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
	{
		return HSC_IO_ERROR;
	}
	HscResult res = HscStopExport(device_handle_);
	DeviceState newstate = getState();
	if (newstate == DeviceState::Unconnected ||
		newstate == DeviceState::Disconnected ||
		newstate == DeviceState::StandBy ||
		newstate == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
	{
		return HSC_IO_ERROR;
	}
	if (res == HSC_OK)
	{
		setState(DeviceState::Connected);
	}
	return res;
}

bool Device::isExportByIntervalSupported() const
{
    return info_.model != DEVICE_5KFSeries;
}

HscResult Device::exportVideoClip(int id, qint64 start_frame, qint64 end_frame, bool new_interface)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

	Q_UNUSED(new_interface);

    DeviceState state = getState();
    if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
    {
        return HSC_IO_ERROR;
    }

    if (m_export_mode_ == 0) return HscExportVideoClip(device_handle_, id, start_frame, end_frame);
	else return HscExportVideoClipEx(device_handle_, id, start_frame, end_frame);
}

HscResult Device::exportByInterval(int id, qint64 start_frame, qint64 end_frame, qint64 frame_count, qint64 interval)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    DeviceState state = getState();
    if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
    {
        return HSC_IO_ERROR;
    }

	if (m_export_mode_ == 1) return HscExportByIntervalEx(device_handle_, id, start_frame, end_frame, frame_count, interval);
	else return HscExportByInterval(device_handle_, id, start_frame, end_frame, frame_count, interval);
}

std::shared_ptr<ImageProcessor> Device::getProcessor() const
{
    return image_processor_ptr_;
}

bool Device::getPlaybackFrame(CAGBuffer &buffer)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    DeviceState state = getState();
    if (state == DeviceState::Unconnected ||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::Reconnecting ||
		state == DeviceState::ToWakeup)
    {
        return false;
    }

    CAGBuffer *pBufRaw = HscGetFirstFrame(device_handle_);
    if (pBufRaw)
    {
        memcpy(&buffer, pBufRaw, pBufRaw->frame_head.frame_size);
        return true;
    }

    return false;
}

void Device::updateVideoSegmentList()
{

    QMutexLocker locker(&video_segment_map_mutex_);

	CSLOG_INFO("updateVideoSegmentList");

	//查找全部视频项,移除
	QList<VideoItem> items = VideoItemManager::instance().findVideoItems(VideoItem::Remote, getIpOrSn());
	for (auto item : items)
	{
		VideoItemManager::instance().removeVideoItem(item.getId());
		m_video_list_thumbnail.remove(item.getId());
	}
    video_segment_map_.clear();

    int count = HscGetVideoClipsNum(device_handle_);
    for (int i = 0; i < count; i++)
    {
        QSharedPointer<HscVideoClipInfo> video_segment_ptr(new HscVideoClipInfo);
        HscResult res = HscGetVideoClipInfo(device_handle_, i, video_segment_ptr.data());
        if (res != HSC_OK)
        {
			if (!video_segment_ptr.isNull())
			{
				video_segment_ptr.clear();
			}
            continue;
        }

        video_segment_map_.insert(video_segment_ptr->id, video_segment_ptr);

        // 视频管理
        QVariant video_id = VideoUtils::packDeviceVideoId(getIpOrSn(), video_segment_ptr->id);
        VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
		{
			QString name = video_segment_ptr->name;

            video_item = VideoItem(video_id, VideoItem::Remote);
            video_item.setName(name);
            video_item.setBeginFrameIndex(0);
			video_item.setVideoFrameCount(video_segment_ptr->frame_num);
            video_item.setEndFrameIndex(video_segment_ptr->frame_num - 1);
			video_item.setKeyFrameIndex(video_segment_ptr->key_frame_no);
			int fps = video_segment_ptr->fps;
			video_item.setFPS(fps);
			video_item.setBinningModeEnable(video_segment_ptr->binningMode);
			video_item.setValidBitsPerPixel(video_segment_ptr->significant_bits_per_pixel);
			int exposure_time = video_segment_ptr->exposure_time;
			if (isGrabberDevice() && device_capability_.exposureTimeUnit == 0) {
				exposure_time = exposure_time * 10;
			}
			video_item.setExposureTime(exposure_time);
            video_item.setFrameStep(0);
            video_item.setLuminance(video_segment_ptr->luminance);
            video_item.setContrast(video_segment_ptr->contrast);
			video_item.setAntiColorEnable(video_segment_ptr->enable_anti_color);
            video_item.setOsdVisible(true);
			HscDisplayMode display_mode;
			if (video_segment_ptr->display_mode != 0)
			{
				display_mode = HscDisplayMode(video_segment_ptr->display_mode);
			}
			else
			{
				display_mode = (video_segment_ptr->color_mode == HSC_COLOR_MONO) ? HSC_DISPLAY_MONO : HSC_DISPLAY_COLOR;
			}
			video_item.setDisplayMode(display_mode);
			CameraWindowRect roi = video_segment_ptr->display_roi;
			video_item.setRoi(QRect(roi.x, roi.y, roi.width, roi.height));
			video_item.setRotationType(video_segment_ptr->rotation_type);
			video_item.setProperty(VideoItem::StreamType, getProperty(Device::PropStreamType));
			video_item.setProperty(VideoItem::VideoFormat, getProperty(Device::PropVideoFormat));
			video_item.setProperty(VideoItem::TimeStamp, QVariant::fromValue(video_segment_ptr->time_stamp));
			video_item.setProperty(VideoItem::VideoSize, QVariant::fromValue(video_segment_ptr->size));
			video_item.setProperty(VideoItem::ColorCorrectInfo, QVariant::fromValue(video_segment_ptr->color_correct_info));
			video_item.setProperty(VideoItem::EnablePiv, QVariant::fromValue(video_segment_ptr->enable_piv));


            VideoItemManager::instance().addVideoItem(video_item);
			if (isSupportGetVideoThumbnail())
			{
				uint8_t* res_size = new uint8_t[1024*10];
				auto video_scale = (VIDEOCLIP_SCALE*)res_size;
				if (HSC_OK == HscGetVideoClipScale(device_handle_, VideoUtils::parseVideoSegmentId(video_item.getId()), *video_scale)) {
					if (video_scale->errcode == 0 && video_scale->fileSize > 0)
					{
						vector<char> vec(video_scale->filePtr, video_scale->filePtr + video_scale->fileSize);
						auto mat_temp = cv::imdecode(Mat(vec), -1).clone();
						m_video_list_thumbnail.insert(video_item.getId(), mat_temp);
					}
				}
				delete [] res_size;
			}
        }
    }

    emit videoSegmentListUpdated();

	CSLOG_INFO("updateVideoSegmentList finished");

}

void Device::correctRoi(QRect & roi, RoiTypes roi_type)
{
	QRect correct_roi = roi;
	if (roi_type == kDeviceRoi)
	{
		HscIntRange range;
		if (getRoiXRange(range))
		{
			if (range.inc != 0)
			{
				correct_roi.moveLeft(ALIGNDOWN(correct_roi.x(), range.inc));
			}

			if (correct_roi.x() < range.min)
			{
				correct_roi.moveLeft(range.min);
			}
			else if (correct_roi.x() > range.max)
			{
				correct_roi.moveLeft(range.max);
			}
		}

		if (getRoiYRange(range))
		{
			if (range.inc != 0)
			{
				correct_roi.moveTop(ALIGNDOWN(correct_roi.y(), range.inc));
			}

			if (correct_roi.y() < range.min)
			{
				correct_roi.moveTop(range.min);
			}
			else if (correct_roi.y() > range.max)
			{
				correct_roi.moveTop(range.max);
			}
		}

		bool bXCorrect = false;
		bool bYCorrect = false;
		Device::RoiConstraintCondition condition =  (Device::RoiConstraintCondition)GetRoiConstraintCondition();
		if (condition == Device::kVerCenterConstraint || condition == kCenterConstraint)
		{
			bYCorrect = true;
			HscIntRange height_range{};
			if (getRoiHeightRange(0, height_range))
			{
				int nYCenter = height_range.max / 2;
				int nNewY = correct_roi.top();
				int nNewHeight = ALIGNUP(correct_roi.height(), height_range.inc);
				if (nNewHeight < height_range.min)
				{
					nNewHeight = height_range.min;
				}
				else if (nNewHeight > height_range.max)
				{
					nNewHeight = height_range.max;
				}
				int nNewCenter = nNewY + nNewHeight / 2;
				if (nNewCenter != nYCenter)
				{
					int nPos = nYCenter - nNewHeight / 2;
					if (nPos < 0)
					{
						nPos = 0;
						nNewHeight = height_range.max;
					}
					correct_roi.moveTop(nPos);
				}
				correct_roi.setHeight(nNewHeight);
			}
		}
		if (condition == Device::kHorCenterConstraint || condition == kCenterConstraint)
		{
			bXCorrect = true;
			HscIntRange width_range{};
			if (getRoiWidthRange(0, width_range))
			{
				int nXCenter = width_range.max / 2;
				int nNewX = correct_roi.left();
				int nNewWidth = ALIGNUP(correct_roi.width(), width_range.inc);
				if (nNewWidth < width_range.min)
				{
					nNewWidth = width_range.min;
				}
				else if (nNewWidth > width_range.max)
				{
					nNewWidth = width_range.max;
				}
				int nNewCenter = nNewX + nNewWidth / 2;
				if (nNewCenter != nXCenter)
				{
					int nPos = nXCenter - nNewWidth / 2;
					if (nPos < 0)
					{
						nPos = 0;
						nNewWidth = width_range.max;
					}
					correct_roi.moveLeft(nPos);
				}
				correct_roi.setWidth(nNewWidth);
			}
		}
		if (!bXCorrect)
		{
			if (getRoiWidthRange(correct_roi.x(), range))
			{
				if (range.inc != 0)
				{
					correct_roi.setWidth(ALIGNUP(correct_roi.width(), range.inc));
				}

				if (correct_roi.width() < range.min)
				{
					correct_roi.setWidth(range.min);
				}
				else if (correct_roi.width() > range.max)
				{
					correct_roi.setWidth(range.max);
				}
			}
		}
		if (!bYCorrect)
		{
			if (getRoiHeightRange(correct_roi.y(), range))
			{
				if (range.inc != 0)
				{
					correct_roi.setHeight(ALIGNUP(correct_roi.height(), range.inc));
				}

				if (correct_roi.height() < range.min)
				{
					correct_roi.setHeight(range.min);
				}
				else if (correct_roi.height() > range.max)
				{
					correct_roi.setHeight(range.max);
				}
			}
		}
		roi = correct_roi;
	}
	else {
		QRect device_roi = GetRoi(kDeviceRoi);
		if (roi.left() > device_roi.right() || roi.right() < device_roi.left() || roi.top() > device_roi.bottom() || roi.bottom() < device_roi.top())
		{
			// 无效设置自动恢复上次有效设置
			correct_roi = GetRoi(roi_type);
		}
		else {
			correct_roi.setX(qMax(roi.x(), device_roi.x()));
			correct_roi.setY(qMax(roi.y(), device_roi.y()));
			correct_roi.setWidth(qMin(roi.right(), device_roi.right())-correct_roi.left() + 1);
			correct_roi.setHeight(qMin(roi.bottom(), device_roi.bottom()) - correct_roi.top() + 1);
		}
		roi = correct_roi;
	}
}

//QVariant Device::makeRMAImageOSDInfo(const QString & timestamp /*= QString()*/) const
//{
//	return makeRMAImageOSDInfo(getFormattedStateStr(), timestamp);
//}
//
//QVariant Device::makeRMAImageOSDInfo(const QString & state_str, const QString & timestamp) const
//{
//	QStringList osd;
//	osd << QString("<font color=#FFFFFF>%1: </font>%2").arg(tr("Device State")).arg(state_str);
//	osd << QString("<font color=#FFFFFF>%1: %2</font>").arg(tr("Device Name")).arg(getProperty(PropName).toString());
//	osd << QString("<font color=#FFFFFF>%1: %2</font>").arg(tr("Device IP")).arg(getIp());
//	osd << QString("<font color=#FFFFFF>%1: %2</font>").arg(tr("Timestamp")).arg(timestamp);
//
//	return qMove(osd);
//}

QList<OSDInfo> Device::makeRMAImageOSDInfo(const QString & state_str, const QString & timestamp)
{
	return makeOsdInfo(state_str, timestamp,DeviceUtils::getFormatedDeviceUserNameAndIpOrSn(getProperty(Device::PropType::PropName).toString(), getIpOrSn()),getStateStrColor()).osdInfos;
}

QList<OSDInfo> Device::makeRMAImageOSDInfo(const QString & timestamp /*= QString()*/)
{
	QString state_str = getFormattedStateStr();
	if (state_str == getFormattedStateStr(toStateStr(DeviceState::Recording))) {
		state_str = getFormattedStateStr(toStateStr(DeviceState::Recording) + getDotStr());
	}
	return makeRMAImageOSDInfo(state_str, timestamp);
}

HscResult Device::exportPreview(int id, qint64 frame_no)
{
    LOCK_TEST(&mutex_);//QMutexLocker locker(&mutex_);

    DeviceState state = getState();
    if (state == DeviceState::Unconnected||
		state == DeviceState::Disconnected ||
		state == DeviceState::StandBy ||
		state == DeviceState::ToWakeup)
    {
        return HSC_IO_ERROR;
    }

    return HscExportPreview(device_handle_, id, frame_no);
}

void Device::reconnect()
{
	std::thread(&Device::doReconnect, this).detach();
}

void Device::doReconnect()
{
	//QMutexLocker locker(&mutex_);

	CSLOG_INFO("{} Reconnect",getIpOrSn().toStdString());

	setState(DeviceState::Reconnecting);
	{
		LOCK_TEST(&mutex_);
		disconnectDevice(false, false);

	}

	HscResult res = HSC_OK;
	QElapsedTimer timer;
	timer.start();
	//重置重连状态
	m_destory = false;
	m_quit_reconnect = false;
	m_reconnect_timeout = false;
	do
	{
		{
			LOCK_TEST(&mutex_);
			res = connectDevice(false, false);
		}
		if (HSC_OK == res)
		{
			m_quit_reconnect = true;//重连成功则退出重连状态
			emit reconnectFinished();
			if (needsDataRecovery())
			{
				setState(DeviceState::Connected);
				emit SignalRecoveringData();
				return ;
			}
// 			if (!IsStandByModeSupported())
// 			{
// 				setState( DeviceState::Connected );
// 			}
// 			else
			{
				DeviceState state = GetWorkStatusFromDevice();
				setState(state);
				if (IsCamera() && (state == Acquiring || state == Previewing || state == Recording))
				{
					emit orgStateIsAcquring(getIpOrSn());//正在输出图像,添加到主视图中

					if (!realtime_frame_timer_ptr_->isActive())
					{
						emit startedUpdateRealtimeFrameTimer();
					}
				}
			}
			break;
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

	} while (/*!timer.hasExpired(kReconnectTimeout_) && */!m_quit_reconnect && !m_reconnect_timeout);//重连未超时或者没有断开则继续重连

	if (res != HSC_OK && !m_quit_reconnect)
	{
		setState(DeviceState::Disconnected);//重连失败显示断开状态
		{
			LOCK_TEST(&mutex_);
			disconnectDevice(false, true);
		}
	}

	if (!m_destory) 
	{
		emit reconnectFinished();
	}
}

void Device::updateReconnectOsd()
{
	QString str = toStateStr(DeviceState::Reconnecting);
	str.append(QString("(%1s)").arg(reconnect_count_down_));
	int dot_count = reconnect_count_down_ % 3 + 1;
	for (int i = 0; i < dot_count; ++i)
	{
		str.append(".");
	}
	str = getFormattedStateStr(str);
	reconnect_count_down_--;
	if (reconnect_count_down_ < 0)
	{
		m_reconnect_timeout = true;//根据界面计时来确定是否停止重连
		reconnect_count_down_ = 0;
	}
#ifdef CSRCCAPP
	if (m_quit_reconnect)//已经退出重连则不刷新
	{
		return;
	}
	RccFrameInfo frameInfo{};
	frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(str, QString()));
	emit updateFrame(frameInfo);
#else

	RMAImage rma_image;
	rma_image.SetOSD(makeRMAImageOSDInfo(str, QString()));

	emit realtime_player_controller_ptr_->sigImageReady(rma_image);
#endif //CSRCCAPP
}

void Device::doMsgProcess()
{
	while (!bdevice_msg_process_thread_exit_)
	{
		QSharedPointer<HscEvent> event_ptr;
		{
			std::unique_lock<std::mutex> locker(device_cv_mutex_);
			device_msg_cv_.wait(locker, [this, &event_ptr]
			{
				if (bdevice_msg_process_thread_exit_)
				{
					return true;
				}
				if (device_msg_vector_.isEmpty())
				{
					return false;
				}
				event_ptr = device_msg_vector_.front().evt;
				device_msg_vector_.pop_front();
				return true;
			}
			);
		}

		if (bdevice_msg_process_thread_exit_)
		{
			break;
		}

		if (!event_ptr)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			continue;
		}

		switch (event_ptr->msg_type)
		{
		case MSG_UPDATE_VIDEOLIST: // 视频列表更新
			CSLOG_INFO("MSG_UPDATE_VIDEOLIST");
			//在线程中刷新视频列表
			std::thread(&Device::updateVideoSegmentList, this).detach();
			break;
		case MSG_EXTERNAL_TRIG://外触发信号
		{
			CSLOG_INFO("MSG_EXTERNAL_TRIG");
			if (Previewing == getState()) {
				emit signalETSnap();
			}
			else {
				setState(Recording);
			}
			break;
		}
		case MSG_CAPTURE_FINISHED: // 采集完成
		{
			// CSLOG_INFO("MSG_CAPTURE_FINISHED");
			bool berforeStopRecing = 0;
			int vid = -1;
			if (event_ptr->msg_len != 0)
			{
				berforeStopRecing = *(bool*)(event_ptr->msg_data);
				vid = *(int*)(event_ptr->msg_data + sizeof(berforeStopRecing));
			}
			finishRecording(vid, berforeStopRecing);
			
		}
		break;
		case MSG_NETWORK_ERROR: //网络错误
		{
			CSLOG_INFO("MSG_NETWORK_ERROR");
			if (getState() != DeviceState::Reconnecting)
			{
				// [2022/8/17 rgq]: HSC_NETWORK_ERROR原是不提示错误，修改后要提示，故这里不再发HSC_NETWORK_ERROR错误消息出去
				//emit errorOccurred(HSC_NETWORK_ERROR); 
				// [2023/2/21 cl]: 回放界面相机网络断开应有异常提示
				emit networkErrorOccurred(HSC_NETWORK_ERROR);
				emit reconnectStart();
				reconnect();
			}
		}
		break;
		case MSG_FILESYSTEM_FORMAT_PROGRESS://格式化进度更新
		{
			//向进度条对话框发送进度更新消息
			//格式化进度,0-100 int值
			int pnProgress = *(int*)(event_ptr->msg_data);
			emit SignalFormatProgress(pnProgress);
		}
			break;
		case MSG_FRAME_RATE:
		{
			int fps{};
			if (event_ptr->msg_len >= 4 && event_ptr->msg_data)
			{
				memcpy(&fps, event_ptr->msg_data, sizeof(int));
			}
			if (getState() != DeviceState::Previewing && getState() != DeviceState::Acquiring && getState() != DeviceState::Recording)
				fps = 0;
			emit updateFrameRate(fps);
		}
		break;
		case MSG_UPDATE_MODULES_TEMPERATURE:
		{
			std::string temp_str(event_ptr->msg_data, event_ptr->msg_len);
			QString temperature_info = QString::fromStdString(temp_str);
			if (temperature_info.indexOf(QChar::Null)>0)
			{
				temperature_info.truncate(temperature_info.indexOf(QChar::Null));
			}
			if (!m_bTemperatureShowTip)
			{
				temperatureParse(temperature_info);
			}
			emit updateTemperatureInfo(temperature_info);

		}
		break;
		case MSG_HARDWARE_AWAKE_STATE:
		{
			CSLOG_INFO("MSG_HARDWARE_AWAKE_STATE");

			bool ok = *(bool*)(event_ptr->msg_data);
			FinishHardwareAwake(ok);
		}
		break;
		case MSG_AVG_LUM:
		{
			uint32_t avg_lum{};
			if (sizeof(int) == event_ptr->msg_len)
			{
				avg_lum = *(uint32_t*)(event_ptr->msg_data);
			}
			else if (sizeof(int16_t) == event_ptr->msg_len)
			{
				avg_lum = *(uint16_t*)(event_ptr->msg_data);
				int bits = getProperty(PropPixelBitDepth).toInt();
				if ((IsIntelligentTriggerV4Supported() || isSupportHighBitParam())&& bits >= 8)
				{
					avg_lum = ImageBitsChange(avg_lum, 16, bits);
				}
			}
			else
			{
				avg_lum = *(uint8_t*)(event_ptr->msg_data);
			}
			//avg_lum = (sizeof(int) == event_ptr->msg_len) ? *(uint32_t*)(event_ptr->msg_data) : *(uint8_t*)(event_ptr->msg_data);
			emit SignalAvgLum(avg_lum);
			break;
		}
		case MSG_UPS_ELECTRICITY_LOW:
		{
			{
				std::lock_guard<std::mutex> lockGuard(m_mutex);
				Warning warning;
				warning.hDevice = device_handle_;
				warning.msg_type = MSG_UPS_ELECTRICITY_LOW;
				bool hasfound = false;
				for (int index = 0; index < m_vecWarnings.size(); index++)
				{
					if (m_vecWarnings.at(index).hDevice == device_handle_)
					{
						hasfound = true;
						break;
					}
				}
				if (!hasfound)
				{
					m_vecWarnings.push_back(warning);
					emit SignalAlarmMsg();
				}
			}
			break;
		}
		case MSG_TOPO_ERROR:
		{
			SetExternalSyncAbnormal(true);
		}
	     break;
		case MSG_TOPO_OK:
		{
			SetExternalSyncAbnormal(false);
		}
		break;
		case MSG_BCODE_STATE:
		{
			bool b_code{false};
			if (event_ptr->msg_len >= 1 && event_ptr->msg_data)
			{
				memcpy(&b_code, event_ptr->msg_data, sizeof(char));
			}

			emit updateBcode(b_code);
		}
		break;
		case MSG_RECOVER_DATA_PROGRESS:
		{
			int progressValue = *(int*)(event_ptr->msg_data);

			emit SignalRecoveringDataProgress(progressValue);
		}
		break;
		case MSG_ERR:
		{
			int errorCode = *(int*)(event_ptr->msg_data);
			emit ErrMsg(errorCode);
			break;
		}
		case MSG_ACTIVE_PIXEL:
		{
			int64_t val = *(int64_t*)(event_ptr->msg_data);
			emit updateActivePixel(val);
		}
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}
}

void Device::doUpdateHealthRecord()
{
	if (!health_record_ptr)
	{
		health_record_ptr.reset(new HealthRecord);
	}

	//帧头
	memset(health_record_ptr.get(), 0, sizeof(HealthRecord));
	const uint8_t kHeadMark[4] = { 'X', 'J', 'X', 'G' };
	std::memcpy(health_record_ptr->head_mark, kHeadMark, sizeof(kHeadMark));

	//设备编号
	health_record_ptr->device_index = GetDeviceIndex();

	//判断连接状态,检查设备错误信息
	DeviceState state = getState();
	if (state == Connected || state == Previewing || state == Acquiring || state == Recording )
	{
		//错误码
		ErrorInfo errors[MAX_SYSTEM_SELF_CHECK_COUNT];
		health_record_ptr->fault_count = SystemSelfCheck(errors);
		uint32_t max_fault_count = sizeof(health_record_ptr->fault_codes) / sizeof(health_record_ptr->fault_codes[0]);
		if (health_record_ptr->fault_count > max_fault_count)
		{
			health_record_ptr->fault_count = max_fault_count;
		}

		for (int i = 0; i < health_record_ptr->fault_count; i++)
		{
			health_record_ptr->fault_codes[i] = errors[i].error_code_;
		}

		//温度
		std::map<TemperatureTypes, double> map_temperatures;
		HscResult res = GetTemperatures(map_temperatures);
		if (res == HSC_OK)
		{
			auto iter = map_temperatures.find(kTempMainBoard);
			if (iter != map_temperatures.end())
			{
				health_record_ptr->temperatures[0] = iter->second;
			}

			iter = map_temperatures.find(kTempSlaveBoard);
			if (iter != map_temperatures.end())
			{
				health_record_ptr->temperatures[1] = iter->second;
			}

			iter = map_temperatures.find(kTempArmChip);
			if (iter != map_temperatures.end())
			{
				health_record_ptr->temperatures[2] = iter->second;
			}
		}


	}
	//设备工作模式转换
	// 工作模式：0-未连接，1-正在连接，2-正在重连，3-已连接，4-正在断开，5-断开，6-预览，7-高速采集，8-正在录制，9-正在断电数据恢复，10-慢速回放，11-待机
	uint8_t work_mode = 0;
	switch (state)
	{
	case Unconnected:
		work_mode = 0;
		break;
	case Connecting:
		work_mode = 1;
		break;
	case Connected:
		work_mode = 3;
		break;
	case Previewing:
		work_mode = 6;
		break;
	case Acquiring:
		work_mode = 7;
		break;
	case Recording:
		work_mode = 8;
		break;
	case Replaying:
		work_mode = 10;
		break;
	case Exporting:
		work_mode = 10;
		break;
	case Disconnecting:
		work_mode = 4;
		break;
	case Disconnected:
		work_mode = 5;
		break;
	case Reconnecting:
		work_mode = 2;
		break;
	case StandBy:
		work_mode = 11;
		break;
	default:
		work_mode = 0;
		break;
	}
	health_record_ptr->work_mode = work_mode;

	//写入到健康管理
	CSHealthManager::instance().updateRecord(getIpOrSn(), *health_record_ptr);
	

}

void Device::doUpdateCurBootTimeAndTotalWorkTime()
{
	LOCK_TEST(&mutex_);
	UpdateCurBootTimeAndTotalWorkTime();

}

void Device::UpdateCurBootTimeAndTotalWorkTime()
{
	DeviceState status = getState();
	if (status == Connected || status == Previewing || status == Acquiring || status == Recording)
	{
		uint64_t cur_boot_time = 0, total_work_time = 0;
		HscResult res = HscGetCurBootTimeAndTotalWorkTime(device_handle_, &cur_boot_time, &total_work_time);

		if (res == HSC_OK)
		{
			struct tm * local_time { 0 };
			std::time_t time = cur_boot_time;
			local_time = std::localtime( &time);
			if (local_time)
			{
				char str_time[64];
				std::strftime(str_time, 64, "%Y-%m-%d %H:%M:%S", local_time);

				//日志输出
				CSLOG_INFO("Boot Time:{} , Total Work Time: {} mins.", str_time, total_work_time / 60);
			}
		}
	}
}

void Device::startMsgProcessThread()
{
	exitMsgProcessThread();
	bdevice_msg_process_thread_exit_ = false;
	device_msg_process_thread_ = std::move(std::thread(&Device::doMsgProcess, this));
}

void Device::exitMsgProcessThread()
{
	bdevice_msg_process_thread_exit_ = true;
	if (device_msg_process_thread_.joinable())
	{
		device_msg_cv_.notify_one();
		device_msg_process_thread_.join();
	}

	std::unique_lock<std::mutex> locker(device_cv_mutex_);
	device_msg_vector_.clear();
}

void Device::updateColorAlgorithmInfo(int vid)
{
	auto processer = getProcessor();
	if (processer) {
		auto info = processer->getColorCorrectInfo();
		HscUpdateVideoColorCorrectInfo(device_handle_, vid, &info);
	}
}

QRect Device::GetRoi(RoiTypes roi_type /*= kDeviceRoi*/) const
{
	if (roi_type == kIntelligentTriggerRoi)
	{
		return CameraWindowRect2QRect(getProperty(PropType::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi);
	}
	else if (roi_type == kAutoExposureRoi)
	{
		auto val = getProperty(PropType::PropAutoExposure);
		if (val.isNull())
			return QRect{};
		return CameraWindowRect2QRect(val.value<HscAutoExposureParameter>().autoExpArea);
	}
	else if(roi_type==kDeviceRoi)
	{
		return getProperty(PropType::PropRoi).value<QRect>();
	}
	return  QRect{};
}

QRect Device::GetMaxRoi(RoiTypes roi_type /*= kDeviceRoi*/) const
{
	QRect max_roi{};
	getMaxRoi(max_roi,roi_type);
	return max_roi;
}

QRgb Device::GetRoiColor(RoiTypes roi_type /*= kDeviceRoi*/) const
{
	switch (roi_type)
	{
	case Device::kDeviceRoi:
		return qRgb(255,0,0);
	case Device::kAutoExposureRoi:
		if (!IsAutoExposureSupported())
			break;
		return getProperty(PropAutoExposure).value<HscAutoExposureParameter>().roi_color;
	case Device::kIntelligentTriggerRoi:
		if (!IsIntelligentTriggerSupported())
			break;
		return getProperty(PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi_color;
	}
	return qRgb(0, 0, 0);
}

bool Device::IsRoiVisible(RoiTypes roi_type /*= kDeviceRoi*/) const
{
	switch (roi_type)
	{
	case Device::kDeviceRoi:
		return getProperty(PropRoiVisible).toBool();
	case Device::kAutoExposureRoi:
		if(!IsAutoExposureSupported() || !IsAutoExposureEnabled())
			return false;
		return getProperty(PropAutoExposure).value<HscIntelligentTriggerParamV2>().roi_visible;
	case Device::kIntelligentTriggerRoi:
		if (!IsIntelligentTriggerSupported() ||!IsIntelligentTriggerEnabled())
			return false;
		return getProperty(PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi_visible;
	}
	return false;
}

QRect Device::getCenterRoi(RoiTypes roi_type /*= kDeviceRoi*/) const
{
	QRect device_roi = GetRoi(kDeviceRoi);
	if (roi_type == kDeviceRoi)
	{
		QRect max_roi = GetMaxRoi(kDeviceRoi);
		device_roi.moveCenter(max_roi.center());
		return device_roi;
	}
	QRect roi = GetRoi(roi_type);
	roi.moveCenter(device_roi.center());
	return roi;
}

bool Device::IsChangePreviewResoultionSupport() const
{
	return device_capability_.supportChangePreviewResoultion;
}

void Device::updateViewOSD()
{
	RccFrameInfo frame_info{};
	frame_info.osdInfos = qMove(makeRMAImageOSDInfo());
	emit updateFrame(frame_info);
}

CameraWindowRect Device::QRectTCameraWindowRect(const QRect& rect)
{
	CameraWindowRect rect_private{};
	rect_private.x = rect.x();
	rect_private.y = rect.y();
	rect_private.width = rect.width();
	rect_private.height = rect.height();
	return rect_private;
}

QRect Device::CameraWindowRect2QRect(const CameraWindowRect& rect_private)
{
	return QRect{ QPoint{ rect_private.x,rect_private.y},QSize{ rect_private.width,rect_private.height} };
}

//百分比和数量之间相互转换
double Device::CountToPercentage(int count, QRect rect)
{
	double percentage = 0;
	int32_t total = rect.width()*rect.height();
	if (total)
	{
		percentage = count *1.0 / total;
	}
	else
	{
		percentage = 0;
	}
	return percentage * 100;
}

int Device::PercentageToCount(double percentage, QRect rect)
{
	int count = 0;
	int32_t total = rect.width()*rect.height();

	count = total * (percentage / 100);

	return count;
}

bool Device::isCurrentDeviceInComputerGateway()
{
	bool bCommonGateway = false;
	QString current_device_ip = QString::fromLatin1(info_.ip);
	do
	{
		if (current_device_ip.isEmpty() || (0 == m_computer_ip_qset.size()) || HScDevDiscoveryType::DT_GATEWAY != info_.discovery_type) {
			bCommonGateway = true;
			break;
		}

		for (auto item : m_computer_ip_qset) {
			QStringList current_device_ip_list = current_device_ip.split(".");
			QStringList computer_ip_list = item.split(".");

			if ((4 == current_device_ip_list.size()) && (4 == computer_ip_list.size())) {
				if ((current_device_ip_list[0] == computer_ip_list[0]) && (current_device_ip_list[1] == computer_ip_list[1])\
					&& (current_device_ip_list[2] == computer_ip_list[2])) {
					bCommonGateway = true;
					break;
				}
			}
		}
		
	} while (false);
	
	return bCommonGateway;
}

void Device::initComputerIpList()
{
	QList<QNetworkInterface> interface_list = QNetworkInterface::allInterfaces();
	for (auto interface_item : interface_list) {
		interface_item.humanReadableName();
		QList<QNetworkAddressEntry> entry_list = interface_item.addressEntries();
		for (auto entry_item : entry_list) {
			QString ip_str = entry_item.ip().toString();
			QString mask_str = entry_item.netmask().toString();

			QStringList ip_list = ip_str.split(".");
			QStringList mask_list = mask_str.split(".");
			
			if ((4 == ip_list.size()) && (4 == mask_list.size())) {
				QString real_ip_str = QString("%1.%2.%3.%4").arg((ip_list[0].toInt() & mask_list[0].toInt())).\
					arg((ip_list[1].toInt() & mask_list[1].toInt())).arg((ip_list[2].toInt() & mask_list[2].toInt()))\
					.arg((ip_list[3].toInt() & mask_list[3].toInt()));
				m_computer_ip_qset.insert(real_ip_str);
			}
		}
	}
}

void Device::videoThumbnailProcess(cv::Mat &src, const VideoItem &item)
{
	if (src.cols <= 0 || src.rows <= 0)//图像不存在,返回
	{
		return;
	}
	//裁剪:如果分辨率 高度 + 能力集字段 "frameProtocolH" : 4,（可能=0 或=4）的值 realH < 128;
	//拼接个数 = ceil(128 / realH) 向上取整
	if (device_capability_.supportGetVideoThumbnail == 1 )
	{
		int realH = item.getRoi().height() + 4 ;
		if (realH < 128)
		{
			int clipH = 128/std::ceil(128.0 / realH);
			src = src(cv::Range(0, 0 + clipH), cv::Range(0, 0 + 128));
		}
	}

	//亮度对比度反色
	//像素融合(提亮4倍)
	if (image_processor_ptr_)
	{
		image_processor_ptr_->cv_process(src, item.getLuminance(), item.getContrast(), item.isAntiColorEnable());
	}
	if (item.getBinningModeEnable())
	{
		src *= 4;
	}

	//旋转
	int rotationType = item.getRotationType();
	if (rotationType == 1) // 顺时针旋转90度
	{
		cv::transpose(src, src); // 矩阵转置
		cv::flip(src, src, 1); // 沿Y轴翻转
	}
	else if (rotationType == 2) // 顺时针旋转180度
	{
		cv::flip(src, src, -1); // 先沿X轴翻转，再沿Y轴翻转
	}
	else if (rotationType == 3) // 顺时针旋转270度
	{
		cv::transpose(src, src); // 矩阵转置
		cv::flip(src, src, 0); // 沿X轴翻转
	}

}

double Device::convertTime(int64_t srcTime, agile_device::capability::Units srcUnit, agile_device::capability::Units destUnit) const
{
	if(time_unit_ratio_map_.contains(srcUnit) && time_unit_ratio_map_.contains(destUnit))
	{
		return srcTime * time_unit_ratio_map_.value(destUnit) / time_unit_ratio_map_.value(srcUnit);
	}

	return srcTime;
}

void Device::SetExternalSyncAbnormal(bool abnormal)
{
	external_sync_abnormal_ = abnormal;
}
bool Device::GetExternalSyncAbnormal() const
{
	return external_sync_abnormal_;
}

int Device::GetIconIndex() const
{ //目前只有四种状态
	int iconIndex = -1;
		DeviceState state = getState();
		if (state == Unconnected || state == Disconnected)
		{
			if (IsTrigger())
			{
				iconIndex = (state == Unconnected) ? kIUnconnectedTrigger : kIAbnormalTrigger;
			}
			else
			{/*
				if (GetPtzNumber() > 0)
				{
					iconIndex = (state == Unconnected) ? kIUnconnectedCameraWithPTZ : kIAbnormalCameraWidthPTZ;
				}*/
				//else
			//	{
				iconIndex = (state == Unconnected) ? kIUnconnectedCamera : kIAbnormalCamera;
			//	}
				
			}
		}
		else
		{
			if (IsTrigger())
			{
				iconIndex = kINormalTrigger;
			}
			else
			{
				if (GetExternalSyncAbnormal())
				{
					//if (GetPtzNumber() > 0)
						//iconIndex = abnormal_external_sync_on_ ? kIAbnormalExternalSyncCameraWithPTZ : kINormalCameraWithPTZ;
					//else
						iconIndex = external_sync_abnormal_on ? kIAbnormalExternalSyncCamera : kINormalCamera;

						external_sync_abnormal_on = !external_sync_abnormal_on;
				}
				else
				{
					//if (GetPtzNumber() > 0)
						//iconIndex = kINormalCameraWithPTZ;
					//else
						iconIndex = kINormalCamera;
				}
			}
		}
	

	return iconIndex;
}
	
uint64_t Device::GetExportSegmentFrameCount() const
{
	uint64_t frame_count = 0;
	HscResult res = HscGetExportSegmentFrameCount(device_handle_, &frame_count);
	if (res == HSC_OK)
	{
		return frame_count;
	}

	return 0;
}

HscResult Device::SetHardwareStandby()
{
	// 直接先停机
	stopDevice(false, m_export_mode_);

	HscResult res = HscSetHardwareStandby(device_handle_);
	if (res == HSC_OK)
	{
		setState(StandBy);
	}
	else if (res == HSC_IO_ERROR)
	{
		setState(Disconnected);
	}
	else
	{
		disconnectDevice(false, false);
	}

	return res;
}

HscResult Device::StartHardwareAwake()
{
	HscResult res = HscSetHardwareAwake(device_handle_);
	if (res != HSC_OK)
	{
		disconnectDevice(false, false);
	}
	setState(ToWakeup);
	return res;
}

void Device::FinishHardwareAwake(bool ok)
{
	if (ok)
	{
		updateVideoSegmentList();
		setState(Connected);
	}
}

void Device::GetAvailableStorageSpace(uint64_t & available, RecordType & unit)
{
	qint64 length_min = 0;
	qint64 length_max = 1000;
	qint64 length_inc = 1;
	getPropertyRange(Device::PropRecordingLength, length_min, length_max, length_inc);
	unit = getProperty(PropType::PropRecordingUnit).value<RecordType>();
	available = length_max;
}

void Device::GetAvailableFrameStorageSpace(uint64_t & available, RecordType & type)
{
	qint64 length_min_frame = 0;
	qint64 length_max_frame = 1000;
	qint64 length_inc = 1;
	getPropertyRange(Device::PropRecordingFrameLength, length_min_frame, length_max_frame, length_inc);
	RecordType unit = getProperty(PropType::PropRecordingUnit).value<RecordType>();

	if (unit != RECORD_BY_FRAME)
	{
		double time_max = length_max_frame;
		qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
		if (unit == RECORD_BY_TIME)
		{			
			time_max = time_max * acquisition_period / (getCalPeriodDivisor() /1000);
		}
		else if (unit == RECORD_BY_TIME_S)
		{
			time_max = time_max * acquisition_period / getCalPeriodDivisor() ;
		}

		if (time_max < 1)
		{
			type = RECORD_BY_FRAME;
			available = length_max_frame;
		}
		else
		{
			type = unit;
			available = floor(time_max);
		}
	}
	else
	{
		type = unit;
		available = length_max_frame;
	}
}

bool Device::needsDataRecovery() const
{
	return info_.search_result == HSC_NEED_RECVOER_DATA;
}

HscResult Device::StartDataRecovery()
{
	LOCK_TEST(&mutex_);

	if (getState() != Connected)
	{
		return HSC_ERROR;
	}

	HscResult res = HscRecoverData(device_handle_);
	if (res == HSC_OK)
	{
		setState(RecoveringData);
	}
	else
	{
		disconnectDevice(true);
	}

	return res;
}

void Device::FinishDataRecovery(bool b_cancel)
{
	LOCK_TEST(&mutex_);
	
	//下发指令数据恢复流程
	if (b_cancel)
	{
		HscResult res = HscCancelRecoverData(device_handle_);
		if (res != HSC_OK)
		{
			emit errorOccurred(res, m_bShowTip);
		}
	}

	if (getState() == RecoveringData)
	{
		setState(Connected);
	}
}

uint8_t Device::GetRoiConstraintCondition()
{

	RoiConstraintCondition condition = (RoiConstraintCondition)m_roi_constraint_;
	return condition;
}

bool Device::IsSupportCMOSDigitGain() const
{
	return (bool)m_support_cmos_digitalGain_;
}

bool Device::IsSupportAcquireEdit() const
{
	return (bool)m_support_edit_property_;
}

bool Device::IsSupportIsDigitalGainSupportedV3() const
{
	return (bool)m_support_cmos_digitalGain_;
}

bool Device::isSupportIntelligent()
{
	return (bool)m_intelligent_trigger;
}

bool Device::isDigitalGainEnable()
{
	return !(device_capability_.digitalGainType == 99);
}

bool Device::isSupportManualWhiteBalanceMode() const
{
	bool bRet = (1 == device_capability_.supportManualAwbMode || 2 == device_capability_.supportManualAwbMode) ? true : false;
	return bRet;
}

bool Device::isSupportManualWhiteBalanceAutoMode() const
{
	bool bRet = (1 == device_capability_.supportManualAwbMode || (2 == device_capability_.supportManualAwbMode && info_.dev_connect_method != HScDevConnectMethod::CM_UDP)) ? true : false;
	return bRet;
}

HscResult Device::GetManualWhiteBalance(HscHisiManualAwb &param)
{


	if (!connected())
	{
		return HSC_ERROR;
	}

	HscResult res = HscgetWhiteBalance(device_handle_, param);
	return res;
}

HscResult Device::SetManualWhiteBalance(const HscHisiManualAwb &param)
{

	if (!connected())
	{
		return HSC_ERROR;
	}

	HscResult res = HscsetWhiteBalance(device_handle_, param);
	return res;
}

bool Device::isSupportRecingStop()
{
	return device_capability_.supportRecingStop == 1;
}

bool Device::isSupportGetVideoThumbnail() const
{
	return (bool)device_capability_.supportGetVideoThumbnail;
}

bool Device::isSupportHighBitParam() const
{
	return (bool)device_capability_.supportHighBitParam;
}

HscResult Device::SetVideoListItemInfo(const HscVideoListItemInfo& info)
{
	return HscSetVideoListItemInfo(device_handle_, info);
}

cv::Mat Device::getVideoClipScale(const QVariant& vid, const VideoItem* video_info /*= nullptr*/)
{
	cv::Mat dst{};
	if (!isSupportGetVideoThumbnail())
	{
		return dst;
	}
	if (m_video_list_thumbnail.contains(vid)) {
		dst = m_video_list_thumbnail[vid].clone();
	}
	if (video_info)
	{
		videoThumbnailProcess(dst, *video_info);
	}
	return dst;
}


std::vector<QString> Device::getAnalogain()
{
	std::vector<QString> values;
	if (m_analog_gain_values)
	{
		auto strText = tr("%1").arg(m_analog_gain_values);
		if (!strText.isEmpty())
		{
			QStringList vctList = strText.split(',',QString::SkipEmptyParts);
			for (auto text : vctList)
			{
				values.push_back("x" + text);
			}
		}
	}
	return values;
}

void Device::doImageProcess()
{
	while (!bdevice_image_process_thread_exit_)
	{
		if (bdevice_image_process_thread_exit_)
		{
			break;
		}

		DeviceState cur_state = getState();//切换状态时不刷新图像,避免界面锁死
		if (cur_state == Connecting ||
			cur_state == ToAcquire ||
			cur_state == ToExport ||
			cur_state == ToPreview ||
			cur_state == Disconnecting ||
			cur_state == Reconnecting ||
			cur_state == Connected)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			continue;
		}

		if ((cur_state != DeviceState::Previewing) && (cur_state != DeviceState::Acquiring) && (cur_state != DeviceState::Recording))
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			continue;
		}
		if (m_bManualWhiteBalance)
		{
			{
				std::unique_lock<std::mutex> locker(device_image_cv_mutex_);
				if (device_image_vector_.size() > 0)
				{
					device_image_vector_.clear();
				}
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
			continue;
		}

		CAGBuffer* buffer_ptr = HscGetLastFrame(device_handle_);
		if (buffer_ptr && !isLastestFrame(buffer_ptr->frame_head.time_stamp))
		{
			if (mTimer.hasExpired(10000))
			{
				CSLOG_INFO("timestamp is invalid");
				mTimer.restart();
			}
		}
		if (buffer_ptr && isLastestFrame(buffer_ptr->frame_head.time_stamp))
		{
			memcpy(last_realtime_timestamp_, buffer_ptr->frame_head.time_stamp, 9);
			DeviceState state = getState();
			cv::Mat image;
// 			if (m_frame_head_type_ == 2 && buffer_ptr->frame_head.rect.width *  buffer_ptr->frame_head.rect.height > 3 * 1024 * 1024)
// 			{
// 				image = image_processor_ptr_->cv_process_no_copy(buffer_ptr, state == DeviceState::Previewing ? 0 : 1, true);
// 			}
// 			else
			{
				image = image_processor_ptr_->cv_process(buffer_ptr, state == DeviceState::Previewing ? 0 : 1, true);
			}

			image = image_processor_ptr_->cv_process(image, getContrast(), getLuminance(), false);

			RccFrameInfo frameInfo{};
			if (m_frame_head_type_ == HEAD_TYPE::eMType) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestamp(last_realtime_timestamp_)));
			}
			else if (m_frame_head_type_ == HEAD_TYPE::eGTypeNs) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampG(last_realtime_timestamp_)));
			}
			else if (m_frame_head_type_ == HEAD_TYPE::eS1315) {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatNewTimestampNs(last_realtime_timestamp_)));
			}
			else {
				frameInfo.osdInfos = qMove(makeRMAImageOSDInfo(DeviceUtils::formatTimestamp(last_realtime_timestamp_)));
			}
			frameInfo.raw_image = image.clone();
			frameInfo.valid_bits = buffer_ptr->frame_head.bpp;
			QImage qt_image;

			//判断需要移位的位数
			if (isSupportHighBitParam())
			{
				int bpp = getProperty(PropPixelBitDepth).toInt();
				DisplayBitDepth dbd = getProperty(PropDisplayBitDepth).value<DisplayBitDepth>();
				int left_shift = bpp - 8 - dbd;
				//带上高位截断就是低位深的图像,但是观感不好,不便于客户理解,遂注释,后面按需开启
				//int highest_cut = (1 << (16 - left_shift)) - 1;
				//image &= highest_cut;
				image *= (1 << left_shift);
			}
			CPlayerViewBase::cvMat2QImage(image, qt_image);

			frameInfo.image = qt_image;
			frameInfo.device_name = getProperty(PropName).toString();
			frameInfo.ip_or_sn = getIpOrSn();
			// 获取视频扩展信息
			getVideoExtendInfo(buffer_ptr->frame_head, frameInfo.extend_info);

			for (int i = 0; i < 9; i++)
			{
				frameInfo.timestamp[i] = last_realtime_timestamp_[i];
			}

			//西光所xj1310调试信息中输出俯仰角信息
			if (FunctionCustomizer::GetInstance().isXiguangsuoVersion() && getModel() == DEVICE_XJ1310)
			{
				printf("azimuth=%d, pitch=%d, focal_length=%d\n", frameInfo.extend_info.azimuth, frameInfo.extend_info.pitch, frameInfo.extend_info.focal_length);
			}
			RccImageFrameInfo rccImage;
			rccImage.image = frameInfo;
			rccImage.image_trigger_avg_avg_lum = buffer_ptr->frame_head.image_trigger_avg_avg_lum;
			rccImage.auto_exposure_area_avg_lum = buffer_ptr->frame_head.auto_exposure_area_avg_lum;
			if (isGType()|| IsIntelligentTriggerV5Supported())
			{
				int32_t value = 0;
				auto res = HscDevGet(device_handle_, HSC_CFG_INTELLLIGENT_AVR, &value, sizeof(value));	
				if (res == HSC_OK) rccImage.active_pixel_num_ = value;
			}
			{
				std::unique_lock<std::mutex> locker(device_image_cv_mutex_);
				if (device_image_vector_.size() >= kFRAME_BUFFER_MAX)
				{
					device_image_vector_.pop_front();
				}
				device_image_vector_.append(rccImage);
			}
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(20));
	}
	CSLOG_INFO("{} rcc image prcocess finished.", getIpOrSn().toStdString());
}

bool Device::GetOneRccImageFrame(RccImageFrameInfo &rccImage)
{
	std::unique_lock<std::mutex> locker(device_image_cv_mutex_);
	if (device_image_vector_.isEmpty())
	{
		return false;
	}
	rccImage = device_image_vector_.front();
	device_image_vector_.pop_front();
	return true;
}

void Device::startRccImageProcessThread()
{
	exitRccImageProcessThread();
	{
		std::lock_guard<std::recursive_mutex> lock(thread_process_mutex_);
		bdevice_image_process_thread_exit_ = false;
		device_image_process_thread_ = std::move(std::thread(&Device::doImageProcess, this));
		SetThreadPriority(device_image_process_thread_.native_handle(), THREAD_PRIORITY_LOWEST);
	}
	CSLOG_INFO("{} start rcc image prcocess.", getIpOrSn().toStdString());
}

void Device::exitRccImageProcessThread()
{
	{
		std::lock_guard<std::recursive_mutex> lock(thread_process_mutex_);
		{
			bdevice_image_process_thread_exit_ = true;
			if (device_image_process_thread_.joinable())
			{
				device_image_cv_.notify_one();
				device_image_process_thread_.join();
			}
		}
	}
	{
		std::unique_lock<std::mutex> locker(device_image_cv_mutex_);
		device_image_vector_.clear();
	}
	//析构时输出日志导致崩溃,暂时去除
	//CSLOG_INFO("{} stop rcc image prcocess.", getIpOrSn().toStdString());
}

void Device::setShowTip(bool bShow)
{
	m_bShowTip = bShow;
}

bool Device::isGType()
{
	return (m_frame_head_type_ == HEAD_TYPE::eGType || m_frame_head_type_ == HEAD_TYPE::eGTypeNs);
}

void Device::setFilterEnable(uint8_t enable) {
	mFilterEnable = enable; 
	if (image_processor_ptr_) image_processor_ptr_->setFilerEnable(enable);
}

bool Device::temperatureParse(QString strTemperature)
{
	if (strTemperature.isEmpty())
	{
		return false;
	}
	if (map_temperature_range_.size() == 0)
	{
		map_temperature_range_ = SystemSettingsManager::instance().getTemperatureThreshold();
	}
	auto it = map_temperature_range_.begin();

	QStringList listType2TemperatureStr = strTemperature.split(";");
	for (auto type2TempStr : listType2TemperatureStr)
	{
		QStringList tempSplits = type2TempStr.split(":");
		if (tempSplits.size() == 2)
		{
			QString type = tempSplits.at(0);
			type = type.toUpper();
			if (map_temperature_range_.find(type) != map_temperature_range_.end())
			{
				QString tempreature = tempSplits.at(1);
				bool ok = false;
				int temp = tempreature.toInt(&ok);
				if (ok)
				{
					if (map_temperature_range_[type].first > temp || temp > map_temperature_range_[type].second)
					{
						m_bTemperatureShowTip = true;
						QString strName = getIpOrSn();
						emit signalTemperatureAbnormal("(" + strName + ")");
						break;
					}
				}
			}
		}
	}
	return true;
}