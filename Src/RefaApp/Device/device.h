#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QPointer>
#include <QMutex>
#include <QMap>
#include <functional>
#include <map>
#include <string>
#include <memory>
#include <QVector>
#include <condition_variable>
#include <atomic>
#include <QImage>
#include <thread>
#include "devicestate.h"
#include "devicelicense.h"
#include "deviceframe.h"


#include "HscAPIStructer.h"
#include "HscSelectors.h"
#include <AgCapability/capability_types.h>
#include "Main/rccapp/csglobaldefine.h"
#include <QDataStream>
#include <Device/csframeinfo.h>
#include <mutex>
#include <QSet>

#include "CF18/CF18SyncTrigger.h"
#include "Util/HscElapsedTimer.h"

#define VALID_MAX INT32_MAX

class MsgListener;
class QTimer;
class ImageProcessor;
class VideoItem;
class cv::Mat;
struct HealthRecord;

#ifdef CSRCCAPP

#else
class RMAImage;
class PlayerControllerInterface;
#endif //CSRCCAPP

struct EventMsg
{
	QSharedPointer<HscEvent> evt;
	void *parameters;
	EventMsg(HscEvent *event, void *paramters)
	{
		evt = QSharedPointer<HscEvent>(event);
		parameters = paramters;
	}

	EventMsg()
	{
		parameters = nullptr;
	}

	~EventMsg()
	{
	}
};

// 版本信息
struct VersionInfo
{
	QString type;
	QString version;
};

// 无云台相机
static const int kIUnconnectedCamera = 8; // 未连接
static const int kIAbnormalCamera = 1; // 异常
static const int kINormalCamera = 0; // 正常
static const int kIAbnormalExternalSyncCamera = 10; // 外同步异


// 带云台相机
static const int kIUnconnectedCameraWithPTZ = 9; // 未连接
static const int kIAbnormalCameraWidthPTZ = 7; // 异常
static const int kINormalCameraWithPTZ = 6; // 正常
static const int kIAbnormalExternalSyncCameraWithPTZ = 11; // 外同步异常

														   // 同步控制器
static const int kIUnconnectedTrigger = 23; // 未连接
static const int kIAbnormalTrigger = 5; // 异常
static const int kINormalTrigger = 4; // 正常

class Device : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 设备属性类型枚举
     */
	enum PropType {
		PropParentIp, // 父设备IP
		PropName, // 设备名称
		PropDeviceIndex, // 设备序号
		PropOsdVisible, // OSD可见
		PropFocusPoint, // 焦点
		PropFocusPointVisible, // 焦点可见
		PropRoi, // ROI
		PropRoiVisible, // ROI可见
		PropBlackFieldEnable, // 暗场校正使能
		PropLut,// Lut
		PropDigitalGain,// 数字增益:Lut的第一个y值
		PropDisplayMode,// 显示模式,单色/彩色
		PropWbEnv,//拍摄环境
		PropWhiteBalance, //白平衡参数
		PropLuminance,// 亮度
		PropContrast,// 对比度
        PropFrameRate, // 帧率
        PropExposureTime, // 曝光时间
		PropExposureTimeUnit,//曝光时间单位
		PropOverExposureTipEnable, // 过曝提示使能
        PropStreamType, // 协议格式
        PropAnalogGain, // 模拟增益
        PropSyncSource, // 同步方式
        PropTriggerMode, // 触发方式
        PropExTriggerMode, //外触发方式
		PropJitterEliminationLength, //消抖长度
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
		PropRecordingFrameLength, //保存帧长度
        PropRecordingUnit, // 保存单位
        PropVideoFormat, // 视频格式
		PropWatermarkEnable, // 叠加水印
		PropAngleDataCorrection, // 角度数据矫正
		PropSdiFpsResols, // SDI参数
		PropTriggerSyncEnable, // 触发同步
		PropRotateType, // 旋转类型
		PropIntelligentTrigger,//智能触发
		PropAutoExposure,//自动曝光
		PropImageCtrl,//图像控制，镜像、倒像、翻转
		PropEdrExposure,//二次曝光
		PropEdrExposureUnit,//二次曝光时间单位
		PropHardwareStandby,//相机待机功能支持
		PropPivParam, //piv参数
		PropTemperaturePanelEnable, //温控列表使能
		PropPixelBitDepth, // 像素位深
		PropDisplayBitDepth, // 界面显示位深
		PropPixelTriggerMode, // 激活像素触发模式
		PropPixelTriggerNumber, // 激活像素触发数量
		PropPixelTriggerCurrent, // 当前激活像素数量HSC_CFG_CMOS_DIGITAL_VAL
		PropCmosDigitalVal, // CMos数字增益
		//CF18-control-begin//////////////////////////////////////////////////
		PropCF18ETJitterEliminationTime, // 外触发信号-消抖时间/μs
		PropCF18ETPolarityReversal, // 外触发信号-极性反转
		PropCF18ETRisingCount, // 外触发信号-上升沿计数
		PropCF18ETFallingCount, // 外触发信号-下降沿计数
		PropCF18ENJitterEliminationTime, // 外同步信号-消抖时间/μs
		PropCF18ENPolarityReversal, // 外同步信号-极性反转
		PropCF18ENRisingCount, // 外同步信号-上升沿计数
		PropCF18ENFallingCount, // 外同步信号-下降沿计数
		PropCF18BJitterEliminationTime, // B码信号-消抖时间/μs
		PropCF18INChannel, // 内同步信号-通道
		PropCF18Cycle, // 周期
		PropCF18Unit, // 单位
		PropCF18INHighLevelWidth, // 内同步信号-高电平宽度/μs
		PropCF18INRisingDelay, // 内同步信号-上升沿延迟/μs
		PropCF18ITChannel, // 内触发信号-通道
		PropCF18ITHighLevelWidth, // 内触发信号-高电平宽度/μs
		PropCF18ITRisingDelay, // 内触发信号-上升沿延迟/μs
		PropCF18ControlPanelEnable, // 控制面板显示使能
		//CF18-control-end////////////////////////////////////////////////////
		PropLowLightMode, //低照度值
		PropEdrExposureV2,//二次曝光V2
    };
	enum RoiTypes
	{
		kUnknownRoi,
		kDeviceRoi,
		kAutoExposureRoi,
		kIntelligentTriggerRoi,
		kManualWhiteBalance,
		kMeasureLine,
	};

	enum TemperatureTypes
	{
		kTempMainBoard,
		kTempSlaveBoard,
		kTempArmChip,
		kTempFpga,
		kTempCmos,
		kTempChamber,
	};

	enum StopMode
	{
		eNomal = 0,
		eNoStop = 1
	};

	// ROI绘制区域约束
	enum RoiConstraintCondition
	{
		kNoConstraint,				// 无绘制区域约束
		kVerCenterConstraint,		// 垂直绘制区域约束 Y轴方向以中心的对称
		kHorCenterConstraint,		// 水平绘制区域约束 X轴方向以中心的对称
		kCenterConstraint,			// 中心点绘制区域约束 以中心点对称
	};

	// 鼠标绘制类型状态
	enum DrawTypeStatusInfo
	{
		DTSI_Noraml,				// 无绘制
		DTSI_DrawROI,				// 绘制ROI
		DTSI_DrawAutoExposure,		// 绘制自动曝光区域
		DTSI_DrawIntelligentTrigger,// 绘制智能触发
		DTSI_DrawMeasure,			// 绘制测量
	};

	//界面显示的图像位深
	enum DisplayBitDepth
	{
		DBD_NULL = -1,
		DBD_0_7 = 0,	//显示范围 0~7 以下类同
		DBD_1_8 = 1,
		DBD_2_9 = 2,
		DBD_3_10 = 3,
		DBD_4_11 = 4,
	};

    Q_ENUM(PropType)
	Q_ENUM(RoiTypes)
	Q_ENUM(RoiConstraintCondition)
	Q_ENUM(DrawTypeStatusInfo)
	Q_ENUM(DisplayBitDepth)

public:
	struct Warning
	{
		DeviceHandle hDevice;		//设备句柄
		int msg_type;				//警告类型
	};
	//报警信息容器
	typedef QVector<Warning> WaringVector;

    explicit Device(const HscDeviceInfo & info, QObject *parent = 0);
	~Device();

	static int ImageBitsChange(int nValue, int nSrcBits, int nDstBits);

	/**
	*@brief	是否支持编辑采集窗口
	**/
	bool allowEditRoi() const;

	bool connected() const;
	/**
	 * @brief 挂起
	 */
	void suspend();

	/**
	 * @brief 恢复
	 */
	void resume();

    /**
     * @brief 连接设备
     */
    void connect();

    /**
     * @brief 连接后刷新OSD
     * 在设备连接完成时调用,避免连接后无法刷新OSD
     */
	void refreshOSDAfterConnect();

    /**
     * @brief 是否允许预览
     * @return true-允许，false-不允许
     */
    bool allowsPreview() const;

    /**
     * @brief 预览
	 * @bAutoConnect 是否自动连接，自动连接提示连接异常
     */
    void preview(bool bAutoConnect = false);

    /**
     * @brief 是否允许采集
     * @return true-允许，false-不允许
     */
    bool allowsAcquire() const;

    /**
     * @brief 采集
     */
	void acquire();

    /**
     * @brief 是否允许触发
     * @return true-允许，false-不允许
     */
    bool allowsTrigger() const;

    /**
     * @brief 触发
     */
    void trigger();

	/**
	* @brief 停止录制
	* @param save_video_clip 是否保存该视频
	* @param next_acquiring_enabled 是否继续进入高采
	*/
	void stopCapture(bool save_video_clip, bool next_acquiring_enabled = false);

	//是否继续进入高采
	bool isNextAcquiringEnabled() { return m_next_acquiring_enabled; }

	/**
	 * @brief 完成录制
	 * @param vid 视频片段ID
	 */
	void finishRecording(int vid, bool berforeStopRecing);

    /**
     * @brief 是否允许停机
     * @return
     */
    bool allowsStop() const;

    /**
     * @brief 停机
     */
    void stop(uint8_t mode = 0);

	/**
	* @brief 触发器开启
	*/
	void triggerStart();

	/**
	* @brief 停机
	*/
	void triggerStop();

    /**
     * @brief 断开设备
	 * @param b_wait 是否等待断连结束
     */
    void disconnect(bool b_wait = false);

	/**
	* @brief 是否允许断线
	* @return true-允许，false-不允许
	*/
	bool allowsDisconnect() const;


	/**
	* @brief 设备是否是相机
	*/
	bool IsCamera() const;

	/**
	* @brief 设备是否是触发器
	*/
	bool IsTrigger() const;

	bool IsCF18() const;

	//是否为根节点触发器
	bool IsRootTrigger() const
	{
		return IsTrigger() && isRoot();
	}

	//是否为根节点
	bool isRoot() const;

    /**
     * @brief 获取型号名称
     * @return 型号名称
     */
    QString getModelName() const;

	void setModelName(const QString &strModelName);
    /**
     * @brief 设置设备IP
     * @param ip 设备IP
     */
    void setIp(const QString & ip);

	QString getIpOrSn() const;

    /**
     * @brief 获取设备IP
     * @return 设备IP
     */
    QString getIp() const;

	/**
	* @brief 获取设备SN
	* @return 设备SN
	*/
	QString getSn() const;


	QString getDescription() const;

    /**
     * @brief 设置授权文件路径
     * @param path 授权文件路径
     */
    HscResult setLicensePath(const QString & path);

    /**
     * @brief 获取授权信息
     * @param license 授权信息
     */
     void getLicense(DeviceLicense & license) const;

    /**
     * @brief 设置属性
     * @param type 属性类型
     * @param value 属性值
     */
    void setProperty(PropType type, const QVariant & value);

    /**
     * @brief 获取属性
	 * @param type 属性类型
	 * @param from_local 是否从直接从本地获取属性
     * @return 属性值
     */
    QVariant getProperty(PropType type ,bool from_local = false) const;

    /**
     * @brief 获取典型属性值
     * @param type 属性类型：ROI、帧率、曝光时间
	 * @param need_costom: 是否需要自定义选项
     * @param values 属性值
     */
    void getTypicalProperties(PropType type, QVariantList & values ,bool need_costom = true) const;

	/**
	* @brief 获取支持的属性类型(用于高采之前应用参数)
	* @return 支持的属性类型列表
	*/
	QList<Device::PropType> getSupportedPropertyTypes(Device* device_ptr) const;

	/**
	* @brief 获取需要保存的属性类型
	* @return 需要保存的属性类型列表
	*/
	QList<Device::PropType> getSavingPropertyTypes() const;

	/**
	* @brief 获取高采时需要下发的属性类型
	* @return 属性类型列表
	*/
	QList<Device::PropType> getAcquirePropertyTypes() const;

	/**
	* @brief 从本地配置文件中加载设置到设备中
	* @return 结果
	*/
	HscResult reloadPropertiesFromLocal();

    /**
     * @brief 获取支持的属性值
     * @param type 属性类型：模拟增益、触发方式、保存起点方式、保存单位、保存格式
     * @param values 属性值
     */
    void getSupportedProperties(PropType type, QVariantList & values) const;

	/**
	* @brief	根据协议类型获取支持的视频格式
	* @param	[in]stream_type 协议类型
	* @param	[out]video_formats 支持的视频格式
	*/
	void GetSupportedVideoFormats(StreamType stream_type, QList<VideoFormat> & video_formats) ;

	/**
	* @brief	根据协议类型获取支持的色彩模式
	* @param	[in]stream_type 协议类型
	* @param	[out]display_modes 支持的色彩模式
	*/
	void GetSupportedDisplayModes(StreamType stream_type, QList<HscDisplayMode> & display_modes) ;


    /**
     * @brief 获取属性范围
     * @param type 属性类型：帧率、曝光时间、保存起点、保存长度
     * @param min 最小值
     * @param max 最大值
     * @param inc 增量
     */
    void getPropertyRange(PropType type, qint64 & min, qint64 & max, qint64 & inc) const;

    /**
     * @brief 获取设备状态
     * @return 设备状态
     */
    DeviceState getState() const;

    /**
     * @brief 获取设备状态字符串
     * @return 设备状态字符串
     */
    QString getStateStr() const;

    /**
     * @brief 获取格式化的设备状态字符串
     * @return 设备状态字符串
     */
    QString getFormattedStateStr() const;

	/**
	* @brief 获取格式化的设备状态字符串颜色
	* @return 设备状态字符串颜色
	*/
	QColor getStateStrColor() const;

	/**
	* @brief 获取格式化的字符串
	* @param str 字符串
	* @return 格式化后的字符串
	*/
	inline QString getFormattedStateStr(const QString & str) const;

    /**
     * @brief 获取设备状态字符串
     * @param state 设备状态
     * @return 设备状态字符串
     */
    static QString toStateStr(DeviceState state);

	/**
	* @brief 获取图像处理器
	* @return 图像处理器
	*/
	std::shared_ptr<ImageProcessor> getProcessor() const;

    /**
     * @brief 设置父设备
     * @param parent 父设备指针
     */
    void setParent(QSharedPointer<Device> parent);

    /**
     * @brief 获取父设备
     * @return 父设备指针
     */
    QSharedPointer<Device> getParent() const;

	/**
	* @brief 是否允许图像训练
	* @return true-允许，false-不允许
	*/
	bool allowsImageTraining() const;

	/**
	 * @brief 是否正在图像训练
	 * @return bool 是/否
	 */
	bool isImagingTrainingEnabled() const;

	/**
	 * @brief 图像训练使能
	 * @param enable 是否使能
	 */
	void enableImageTraining(bool enable);

	/**
	* @brief 主板图像训练
	*/
	void mainBoardPhaseAdjust();

	/**
	* @brief 从板图像训练
	*/
	void slvPhaseAdjust();

	/**
	* @brief 系统自检	
	* @param ErrorInfo error_info[MAX_SYSTEM_SELF_CHECK_COUNT] 错误信息结构体的数组，由调用者负责提供内存
	* @return int 返回自检信息的条数，SDK会改写该数量的数组内容
	*/
	int SystemSelfCheck(ErrorInfo error_info[MAX_SYSTEM_SELF_CHECK_COUNT]);

	// 是否支持从相机获取参数
	bool isGetParamsFromDevice() const;


	//设备属性复制相关接口
	bool AllowsSetCameraSettings() const;//是否允许设备设置参数

	/**
	 * @brief 复制相机参数配置
	 * @param device_ptr 需要拷贝参数的设备
	 * @return HscResult 结果码
	 * @note 
	 */
	HscResult CopyCameraSettingsFrom(QSharedPointer<Device> device_ptr);
	HscResult refreshCurrentDeviceSettings();

	//判断当前位深是否和本地复制参数位深不一致
	bool IsPixelDepthDifferent();

	/**
	* @brief	是否支持角度数据矫正
	*/
	bool IsDataCorrectionSupported() const;

	/**
	* @brief	是否允许编辑角度数据矫正
	* @param	stream_type 协议格式
	*/
	bool AllowsEditDataCorrectionFrameOffset(StreamType stream_type) const;

	//白平衡功能相关接口 begin:

	//颜色矫正信息
	HscResult setColorCorrectInfo(HscColorCorrectInfo info);
	HscResult getColorCorrectInfo(HscColorCorrectInfo &info) const;

	// 是否允许编辑白平衡模式
	bool allowsEditWbMode() const;

	// 是否允许编辑ARM白平衡模式
	bool allowsEditArmWbMode() const;

	// 获取白平衡模式
	HscWhiteBalanceMode GetWbMode() const;

	// 应用白平衡模式
	HscResult ApplyWbMode(HscWhiteBalanceMode wb_mode);

	// 获取ARM白平衡模式
	HscResult GetArmWbMode(uint8_t & wb_mode) const;

	// 获取ARM手动白平衡增益范围
	HscResult GetArmMwbGainRange(HscIntRange & range) const;

	// 获取ARM手动白平衡增益
	HscResult GetArmMwbGain(uint16_t & r_gain, uint16_t & gr_gain, uint16_t & gb_gain, uint16_t & b_gain) const;

	// 应用ARM手动白平衡R增益
	HscResult ApplyArmRGain(uint16_t & gain);

	// 应用ARM手动白平衡GR增益
	HscResult ApplyArmGrGain(uint16_t & gain);

	// 应用ARM手动白平衡GB增益
	HscResult ApplyArmGbGain(uint16_t & gain);

	// 应用ARM手动白平衡B增益
	HscResult ApplyArmBGain(uint16_t & gain);

	/**************************
	* @brief: 恢复出厂设置
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/10
	***************************/
	HscResult RestoreFactorySetting();

	/**************************
	* @brief: 从设备中读取参数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/10
	***************************/
	HscResult LoadSettingsFromDevice();

	/**************************
	* @brief: 是否允许恢复出厂设置
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/10
	***************************/
	bool IsFactoryResetSupported() const;

	/**************************
	* @brief: 是否支持网络配置
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/11
	***************************/
	bool IsNetConfigSupported() const;

	// 是否支持ARM白平衡
	bool isArmWbSupported() const;

	// 是否支持彩色
	bool isColorSupported() const;

	//白平衡功能相关接口 end

	//是否允许报靶功能
	bool AllowsEnableTargetScoring() const;

	//暗场校正相关接口 begin

	//支持暗场校正使能
	bool AllowsEnableBlackField() const;

	//支持暗场校正
	bool AllowsBlackFieldCalibration();

	HscResult BlackFieldCalibration();

	//暗场校正相关接口 end

	//增益设置相关接口 begin

	//允许设置增益
	bool AllowsSetGain() const;

	//是否支持数字增益
	bool IsDigitalGainSupported() const;

	//是否支持模拟增益
	bool IsAnalogGainSupported() const;

	// 获取LUT值的最大值
	uint16_t GetLUTValueMax();

	// 获取LUT缺省控制点
	void GetDefaultLUTCtrlPoints(QList<QVariant> & ctrlPoints);

	//增益设置相关接口 end

	//色彩模式相关接口 begin

	//是否支持彩色
	bool IsColorSupported() const;

	//是否支持显示温控面板
	bool IsTemperaturePanelSupported() const;

	//是否允许编辑显示模式
	bool AllowsEditDisplayMode() const;

	//色彩模式相关接口 end

	//SDI相关接口
	//是否支持SDI设置
	bool IsSdiCtrlSupported() const;
	//是否支持SDI回放设置
	bool IsSdiPlaybackCtrlSupported() const;


	//获取支持的SDI播放速度
	//param [out] play_speeds 播放速度列表
	void GetSupportedSdiPlaySpeeds(QStringList & play_speeds) const;
	/**
	* @brief	获取sdi范围,间隔(速度)
	* @param	start_frame_no 开始帧号
	* @param	end_frame_no 结束帧号
	* @param	interval 间隔(速度)
	* @return	HscResult 结果
	* @note		
	*/
	HscResult GetSdiRange(uint32_t & start_frame_no, uint32_t & end_frame_no, double & interval) const;

	/**
	* @brief	设置sdi范围,间隔(速度)
	* @param	start_frame_no 开始帧号
	* @param	end_frame_no 结束帧号
	* @param	interval 间隔(速度)
	* @return	HscResult 结果
	* @note
	*/
	HscResult ApplySdiRange(uint32_t start_frame_no, uint32_t end_frame_no, double interval);

	/**
	* @brief	设置sdi状态
	* @param	state 开始,停止,暂停状态
	* @return	HscResult 结果
	* @note		
	*/
	HscResult SetSdiState(HscSDIState state);
	HscResult GetSdiState(HscSDIState & state) const;

	//sdi向前一帧
	HscResult SdiBackward();
	//sdi向后一帧
	HscResult SdiForward();


	//水印相关接口
	//是否支持水印
	bool IsWatermarkSupported() const;
	bool IsWatermarkSupported(VideoFormat video_format) const;
	//是否允许编辑水印
	bool AllowsEditWatermark() const;

	int GettProcessorBitsPerPixel();

	//触发同步相关接口
	//是否支持触发同步
	bool IsTriggerSyncSupported() const;
	// 是否允许编辑触发同步使能
	bool AllowsEditTriggerSyncEnabled() const;

	bool AllowsEditTriggerMode() const;//是否允许设置触发方式

	//外触发方式相关
	bool IsExternalTriggerModeSupported() const;//是否支持外触发方式
	bool AllowsEditExternalTriggerMode() const;//是否允许设置外触发方式

	//消抖长度相关
	bool IsJitterEliminationLengthSupported() const;//是否支持消抖长度
	bool AllowsEditJitterEliminationLength() const;//是否允许编辑消抖长度

	//是否允许编辑亮度对比度
	bool AllowsEditLuminanceAndContrast() const;

	//是否允许显示设备信息
	bool AllowsShowDeviceProperties() const;

	// 重载设备信息
	HscResult ReloadDeviceInfo();

	// 获取视频存储总容量,MB
	int GetDeviceTotalMemorySize() const;

	// 获取视频存储已使用容量,MB
	int GetDeviceUsedMemorySize() const;

	/**
	* @brief 获取设备温度字符串
	* @param [out] out 设备温度字符串
	* @return HscResult 获取结果
	* @note 字符串描述设备全部模块温度信息
	*/
	HscResult GetTemperatures(QString & out) ;

	/**
	* @brief 获取设备温度数据
	* @param [out] map_temperature 设备模块温度数据
	* @return HscResult 获取结果
	* @note
	*/
	HscResult GetTemperatures(std::map<TemperatureTypes, double> & map_temperature) ;

	//获取线路延迟
	HscResult GetRouteDelay(DWORD & dwDelay);

	//设备授权相关
	HscResult GetAuthInfo(HscAuthInfo *auth_info);//获取授权信息
	bool IsAuthEnabled() const; // 设备授权是否可用

	bool IsOnlineUpdate() const;    //在线升级是否可用

	// 开始在线升级
	HscResult StartOnlineUpgrade(const UpgradeInfo & info);

	// 获取版本信息
	HscResult GetDeviceVersion(std::vector<VersionInfo> & versions);

	//获取sdk版本信息
	HscResult GetSdkVersion(VersionInfo &version_info);

	//获取插件版本信息
	HscResult GetPluginVersion(VersionInfo & version_info);


	//是否允许控制风扇状态
	bool AllowEditFanCtrl() const;

	/************************
	* @brief: 是否支持风扇控制
	* @author:mpp
	*************************/
	bool IsFanCtrlSupported() const;

	/************************
	* @brief: 设置风扇状态
	* @param: 风扇状态
	* @author:mpp
	*************************/
	HscResult SetFanState(uint8_t state);

	/************************
	* @brief: 获取风扇状态
	* @param: 风扇状态
	* @author:mpp
	*************************/
	HscResult GetFanState(uint8_t & state);

	/************************
	* @brief: 设置像素融合模式
	* @param: 像素融合模式
	* @author:chenyun
	*************************/
	HscResult SetBinningMode(bool enable);

	/************************
	* @brief: 获取像素融合模式
	* @param: 像素融合模式
	* @author:chenyun
	*************************/
	HscResult GetBinningMode(bool &enable);


	bool IsBinningModeSupported() const;

	bool AllowsSetBinningMode() const;

	HscResult SetNetConfigByGateway(uint8_t* devMacAddr, const char* newIP, const char* newMask, const char* newGateway);
    
	/**
	*@brief	智能触发是否支持，暂支持V2版本
	**/
	bool IsIntelligentTriggerSupported() const;

	/**
	*@brief	智能触发V4是否支持
	**/
	bool IsIntelligentTriggerV4Supported() const;

	/**
	*@brief	智能触发V5是否支持(界面可支持亮暗两种模式)
	**/
	bool IsIntelligentTriggerV5Supported() const;

	/**
	*@brief 脉冲宽度单位是不是百纳秒
	**/
	bool IsPulseWidthUnit100Ns() const;

	/**
	* @brief 是否允许设置智能触发参数，使能/禁用，参数设置等
	* @return true-允许，false-禁止
	*/
	bool AllowsSetIntelligentTrigger() const;

	/**
	* @brief 智能触发是否开启
	* @return true-开启，false-关闭
	*/
	bool IsIntelligentTriggerEnabled() const;


	/**
	*@brief	自动曝光是否支持
	*@param
	*@return
	**/
	bool IsAutoExposureSupported() const;


	/**
	* @brief 是否允许设置自动曝光参数，使能/禁用，参数设置等
	* @return true-支持，false-不支持
	*/
	bool AllowSetAutoExposure() const;


	/**
	*@brief	自动曝光是否使能
	*@param
	*@return
	**/
	bool IsAutoExposureEnabled() const;

	//是否支持过曝提示
	bool IsOverExposureTipSupported() const;
	//是否允许编辑过曝提示
	bool AllowsEditOverExposureTip() const;

	//是否允许编辑曝光时间
	bool AllowsEditExposureTime() const;

	//是否允许编辑帧率
	bool AllowsEditFrameRate() const;
	bool AllowsEditFrameRate(SyncSource sync_source) const;

	//是否允许编辑采集相关参数
	bool AllowsEditRecordingOffsetMode() const;
	bool AllowsEditRecordingUnit() const;
	bool AllowsEditRecordingSettings() const;
	bool AllowsEditRecordingSettings(SyncSource sync_source) const;

	//是否支持实时导出
	bool IsRealtimeExportSupported() const;

	//录制后事件
	bool IsAutoExportEnabled() const;//自动导出

	bool IsAutoExportAndTriggerAfterRecordingEnabled() const;//自动导出并循环录制

	bool AllowsAutoPlayback() const;//自动回放

	bool IsAutoDeleteVideoSegmentSupported() const;//是否允许自动删除视频

	bool IsMemoryManagementSupported() const;//是否支持停机内存管理(保存起点和长度自动修正)


	/**
	*@brief	是否支持镜像、倒像、翻转
	*@param 
	*@return
	**/
	bool IsImageCtrlSupported() const;

	bool AllowsImageCtrl() const;

	/************************
	* @brief: 二次曝光是否支持
	* @return true-支持，false-不支持
	* @author: mpp
	*************************/
	bool IsEdrExposureSupported() const;

	/************************
	* @brief: 是否允许设置二次曝光参数，使能/禁用，参数设置等
	* @return true-支持，false-不支持
	* @author: mpp
	*************************/
	bool AllowSetEdrExposure() const;

	/************************
	* @brief: 自动曝光是否使能
	* @return true-使能，false-不使能
	* @author: mpp
	*************************/
	bool IsEdrExposureEnabled() const;

	//是否支持设备序号
	bool IsDeviceIndexSupported() const;

	//是否支持待机
	bool IsStandByModeSupported() const;

	//是否允许编辑待机模式
	bool AllowEditStandByMode() const;

	//从设备获取当前状态
	DeviceState GetWorkStatusFromDevice();

	/**
	* @brief PIV 是否支持
	* @return bool
	*/
	bool IsPIVSupported() const;

	bool allowsEditPIVEnable() const;

	/**
	* @brief PIV 是否使能
	* @return bool
	*/
	bool IsPIVEnabled() const;
	bool IsPIVEnabled(SyncSource sync_source) const;


	/**************************
	* @brief: 获取收到的警告信息容器
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	WaringVector GetVecWarnings() const
	{
		return m_vecWarnings;
	}

	/**************************
	* @brief: 清除收到的警告信息容器
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	void ClearWaringVector()
	{
		m_vecWarnings.clear();
	}
#ifdef CSRCCAPP

#else

    /**
     * @brief 获取实时播放控制器
     * @return
     */
    std::shared_ptr<PlayerControllerInterface> getRealtimePlayerController();
#endif //CSRCCAPP

    /**
     * @brief 获取视频片段列表
     * @return 视频片段列表
     */
    QList<QSharedPointer<HscVideoClipInfo>> getAllVideoSegments() const;

    /**
     * @brief 获取视频片段
     * @param id 视频片段ID
     * @return 视频片段
     */
    QSharedPointer<HscVideoClipInfo> getVideoSegment(int id) const;

	// 获取视频片段删除模式
	HscVideoClipDeleteMode GetVideoClipDeleteMode() const;

    /**
     * @brief 删除视频片段
     * @param id 视频片段ID
     * @return 结果码
     */
    quint64 removeVideoSegment(int id);

	/**
	* @brief	格式化视频存储
	* @return	结果码
	*/
	HscResult DeleteAllVideoClips();

	/**
	*@brief	获取当前roi
	*@param 
	*@return
	**/
	QRect GetRoi(RoiTypes roi_type = kDeviceRoi) const;

	/**
	*@brief	获取ROI最大值
	*@param 
	*@return
	**/
	QRect GetMaxRoi(RoiTypes roi_type = kDeviceRoi) const;

	/**
	*@brief	获取ROI颜色
	*@param
	*@return
	**/
	QRgb GetRoiColor(RoiTypes roi_type = kDeviceRoi) const;

	/**
	*@brief	roi是否显示
	**/
	bool IsRoiVisible(RoiTypes roi_type = kDeviceRoi) const;

	/**
	*@brief	获取居中roi
	**/
	QRect getCenterRoi(RoiTypes roi_type = kDeviceRoi) const;

	/**
	*@brief	是否支持在预览时改变输出图像分辨率
	**/
	bool IsChangePreviewResoultionSupport() const;


	void updateViewOSD();

	/**
	* @brief 获取剩余存储空间
	* @param [out] avaible 剩余存储空间
	* @param [out] unit 剩余存储空间单位
	*/
	void GetAvailableStorageSpace(uint64_t & available, RecordType & unit);

	/**
	* @brief 获取剩余存储空间(不足1s/1ms时为帧显示)
	* @param [out] avaible 剩余帧
	*/
	void GetAvailableFrameStorageSpace(uint64_t & available, RecordType & type);

	/**
	* @brief 是否需要断电数据恢复
	*/
	bool needsDataRecovery() const;

	/**
	* @brief 开始断电数据恢复
	*/
	HscResult StartDataRecovery();

	/**
	* @brief 完成断电数据恢复
	*/
	void FinishDataRecovery(bool b_cancel = false);

	/**
	* @brief 获取ROI约束条件
	*/
	uint8_t GetRoiConstraintCondition();
	/**
	* @brief 是否支持CMos数字增益
	*/
	bool IsSupportCMOSDigitGain() const;
	/**
	* @brief 是否高采下编辑
	*/
	bool IsSupportAcquireEdit() const;
	/**
	* @brief 是否使用数字增益设置界面V3版界面
	*/
	bool IsSupportIsDigitalGainSupportedV3() const;
	/**
	* @brief 是否使用G270智能触发界面
	*/
	bool isSupportIntelligent();
	/**
	* @brief G270是否使用使能数字增益
	*/
	bool isDigitalGainEnable();

	/**
	* @brief 设置手动白平衡
	*/
	void setManualWhiteBalance(bool bEnable)
	{
		m_bManualWhiteBalance = bEnable;
	}

	bool isSupportManualWhiteBalanceMode() const;
	/**
	* @brief 能力集为2时:不支持自动模式
	*/
	bool isSupportManualWhiteBalanceAutoMode() const;

	HscResult GetManualWhiteBalance(HscHisiManualAwb &param);

	HscResult SetManualWhiteBalance(const HscHisiManualAwb &param);

	//是否支持即录即停功能
	bool isSupportRecingStop();

	// 是否支持获取视频缩略图
	bool isSupportGetVideoThumbnail() const;

	// 是否支持高比特位深参数 
	bool isSupportHighBitParam() const;

	HscResult SetVideoListItemInfo(const HscVideoListItemInfo& info);

	cv::Mat getVideoClipScale(const QVariant& vid, const VideoItem* video_info = nullptr);

	void setShowTip(bool bShow);
signals:
    /**
     * @brief 设备已连接
     * @param ip 设备IP
     */
    void deviceConnected(const QString & ip);

    /**
     * @brief 设备已断开
     * @param ip 设备IP
     */
    void deviceDisconnected(const QString & ip);

    /**
     * @brief 设备已预览
     * @param ip
     */
    void devicePreviewed(const QString & ip);

    /**
     * @brief 设备状态变化
     * @param state 设备状态
     */
    void stateChanged(const QString &ip, DeviceState state);
	void cf18StateChanged(const QString &ip, DeviceState state);

	/**
	* @brief 原始状态正在采集图像
	* @param ip 设备ip
	*/
	void orgStateIsAcquring(const QString & ip);

	/**
	* @brief 网络错误发生
	* @param error 网络错误码
	*/
	void networkErrorOccurred(quint64 error);

    /**
     * @brief 错误发生
     * @param error 错误码
	 * @param bShowTip 是否显示
	 * @param extTip 额外提示
     */
    void errorOccurred(quint64 error, bool bShowTip = true, QString extTip = QString());

    /**
     * @brief 实时帧变化
     * @param image
     */
    void realtimeFrameChanged(const DeviceFrame & image);

    /**
     * @brief 属性变化
     * @param type 属性类型
     * @param value 属性值
     */
    void propertyChanged(PropType type, const QVariant & value);

    /**
     * @brief 视频片段列表已更新
     */
    void videoSegmentListUpdated();

	/**
	 * @brief 录制完成
	 * @param video_id 视频ID
	 */
	void recordingFinished(const QVariant & video_id, bool berforeStopRecing, uint8_t state);

	/**
	*@brief 开启刷新图像定时器
	*@note QTimer不可以跨线程调用，在线程中开启预览/停机时需要发出信号开启定时器
	**/
	void startedUpdateRealtimeFrameTimer();

	/**
	*@brief 自动重连开始
	**/
	void reconnectStart();

	/**
	*@brief 自动重连结束
	**/
	void reconnectFinished();

	/**
	*@brief 发送图像
	**/
	void updateFrame(RccFrameInfo image);

	/**
	*@brief 更新帧
	*@note 主要是为了在手动白平衡中使用
	**/
	void updateWhiteBalanceFrame(CAGBuffer* buffer_ptr, RccFrameInfo frameInfo);

	/**************************
	* @brief: 发送智能触发亮度值
	* @param:value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SignalUpdateFrameIntelligentAvgBright(const QString &ip, const int value);

	/**************************
	* @brief: 发送自动曝光灰度值
	* @param:value 灰度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SignalUpdateFrameAutoExposureAvgGray(const QString &ip, const int value);

	/**
	*@brief 更新帧率
	**/
	void updateFrameRate(qreal fps);
	
	/**
	*@brief 更新温度信息
	**/
	void updateTemperatureInfo(QString temperature_info);

	/**************************
	* @brief: 发送平均亮度信号
	* @param: avgLum-平均亮度值
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	void SignalAvgLum(const uint32_t avgLum);

	/**************************
	* @brief: 运行报警信息信号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	void SignalAlarmMsg();

	/**
	* @brief	设备格式化进度信号
	* @param	进度
	* @note		进度值为[0,100]
	*/
	void SignalFormatProgress(int progress);

	/**
	* @brief	连接B码
	* @param	是否连接
	* @note		0/1
	*/
	void updateBcode(bool bcode);
	void ErrMsg(int errorCode);

	/**************************
	* @brief: 断电数据恢复信号
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void SignalRecoveringData();

	/**************************
	* @brief: 断电数据恢复信号进度
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void SignalRecoveringDataProgress(int nProgress);

	/**************************
	* @brief: 更新界面信息
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void SignalUpdateUIInfo();

	void updateActivePixel(int64_t val);

	void signalUpdateDrawStatusInfo(DrawTypeStatusInfo info);
	void signalETSnap();

	/**************************
	* @brief: 温度异常消息
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void signalTemperatureAbnormal(QString strName);
private:
    /**
     * @brief SDK消息更新
     * @param msg 消息
     * @param parameters 消息参数
     */
    void Update(HscEvent *msg, void *parameters);

private:
    /**
     * @brief 采集周期转帧率
     * @param acquisition_peroid 采集周期
     * @return 帧率
     */
    static qint64 toFrameRate(qint64 divisor, qint64 acquisition_period);

    /**
     * @brief 帧率转采集周期
     * @param frame_rate 帧率
     * @return 采集周期
     */
    qint64 toAcquisitionPeriod(qint64 frame_rate) const;

    /**
     * @brief 帧率转采集周期
     * @param frame_rate 帧率
     * @param min_acquisition_period 最小采集周期
     * @return 采集周期
     */
    static qint64 toAcquisitionPeriod(qint64 divisor, qint64 frame_rate, qint64 min_acquisition_period);

	/**
	* @brief 计算同步脉宽范围
	* @param period 周期
	* @param min 最小值
	* @param max 最大值
	*/
    void getPulseWidthRange(qint64 period, qint64 &min, qint64 &max);

private slots:
	/**
	 * 开始更新实时帧
	 */
	void startUpdateRealtimeFrame();

	/**
	*@brief 相机属性变化响应槽函数
	*@param [in] : type : Device::PropType，发生变化的属性
				   value : const QVariant&，属性值
	**/
	void onPropertyChanged(PropType type, const QVariant & value);

private:

    /**
     * @brief 连接
     */
    void doConnect();

    /**
     * @brief 预览
     */
    void doPreview();

    /**
     * @brief 采集
     */
    void doAcquire();

	/**
	 * @brief 完成录制
	 * @param vid 视频片段ID
	 */
	void doFinishRecording(int vid, bool berforeStopRecing);

    /**
     * @brief 停机
     */
    void doStop(uint8_t mode = 0);

    /**
     * @brief 断开
     */
    void doDisconnect();

    /**
     * @brief 连接设备
     * @param update_state 更新状态：true-更新，false-不更新
     * @param emit_device_connected 是否触发已连接信号：true-是，false-否
     * @return 结果码
     */
    HscResult connectDevice(bool update_state, bool emit_device_connected = false);

    /**
     * @brief 设备预览
     * @return 结果码
     */
    HscResult previewDevice();

    /**
     * @brief 设备采集
     * @return 结果码
     */
    HscResult acquireDevice();

    /**
     * @brief 设备触发
     * @return 结果码
     */
    HscResult triggerDevice();

    /**
     * @brief 停止设备
     * @param update_state 更新状态：true-更新，false-不更新
     * @return 结果码
     */
    HscResult stopDevice(bool update_state, uint8_t mode=0 );

	/**
	* @brief 停止采集
	* @param save_video_clip 是否保存该视频
	* @param next_acquiring_enabled 是否继续进入高采
	* @return 结果码
	*/
	HscResult stopCaptureDevice(bool save_video_clip, bool next_acquiring_enabled = false);

    /**
     * @brief 断开设备
     * @param update_state 更新状态：true-更新，false-不更新
     * @param emit_device_disconnected 是否触发已断开信号：true-是，false-否
     * @return 结果码
     */
    HscResult disconnectDevice(bool update_state, bool emit_device_disconnected = false);

	//初始化CF18心跳进程
	void initCF18HeartBeatThread();
	void exitCF18HeartBeatThread();

	//CF18心跳
	void doCF18HeartBeat();

    /**
     * @brief 更新实时帧
     */
    void updateRealtimeFrame();

	/**
	* @brief 获取视频扩展信息
	* @param [in] frame_head 帧头
	* @param [out] extend_info 视频扩展信息
	*/
	void getVideoExtendInfo(const FrameHead & frame_head,  VideoExtendInfo & extend_info) ;

public:
		uint16_t GetExtraHeight() const;
private:


    /**
     * @brief 设置状态
     * @param state 状态
     */
    void setState(DeviceState state);

    /**
     * @brief 应用属性到设备
	 * @param from_local 属性是否从本地获取
     * @return
     */
    HscResult applyProperties2Device(bool from_local = false);




    /**
     * @brief 设置属性到设备
     * @param type 属性类型
     * @param value 属性值
     * @return 结果码
     */
    HscResult setProperty2Device(PropType type, const QVariant &value);

	//保存本机参数到本地
	void saveConfig2Local();

    /**
     * @brief 获取设备配置KEY
     * @param type 属性类型
     * @return 设备配置KEY
     */
    QString settingsKey(PropType type);

    /**
     * @brief 设备参数是否需要从设备侧载入
     * @return
     */
    bool settingsNeedsLoadFromDevice() const;

    /**
     * @brief 应用配置到设备
     */
    void applySettingsToDevice();

    /**
     * @brief 初始化函数对象
     */
    void initFunctions();

    /**
     * @brief 获取最大ROI
     * @param roi
     */
    void getMaxRoi(QRect & roi,RoiTypes roi_type= kDeviceRoi) const;

    /**
     * @brief 获取典型分辨率
     * @param resolutions 典型分辨率
	 * @param need_costom: 是否需要自定义选项
     */
    void getTypicalResolutions(QVariantList & resolutions , bool need_costom = true) const;

    /**
     * @brief 获取典型帧率
     * @param frame_rates 典型帧率
     */
    void getTypicalFrameRates(QVariantList & frame_rates) const;

    /**
     * @brief 获取典型曝光时间
     * @param exposure_times 典型曝光时间
     */
    void getTypicalExposureTimes(QVariantList & exposure_times) const;
public:
    /**
     * @brief 获取ROI-X范围
     * @param range ROI-X范围
     * @return true-成功，false-失败
     */
    bool getRoiXRange(HscIntRange & range,RoiTypes roi_type = kDeviceRoi) const;

    /**
     * @brief 获取ROI-Y范围
     * @param range ROI-Y范围
     * @return true-成功，false-失败
     */
    bool getRoiYRange(HscIntRange & range, RoiTypes roi_type = kDeviceRoi) const;

    /**
     * @brief 获取ROI宽度范围
     * @param x ROI-X
     * @param range ROI宽度范围
     * @return true-成功，false-失败
     */
    bool getRoiWidthRange(qint64 x, HscIntRange & range, RoiTypes roi_type = kDeviceRoi) const;

    /**
     * @brief 获取ROI高度范围
     * @param y ROI-Y
     * @param range ROI高度范围
     * @return true-成功，false-失败
     */
    bool getRoiHeightRange(qint64 y, HscIntRange & range, RoiTypes roi_type = kDeviceRoi) const;

	/**
	* @brief 获取设备编号(特定型号功能)
	* @return uint16_t 设备编号
	*/
	uint16_t GetDeviceIndex() const;

	/**************************
	* @brief: 获取网络配置
	* @param:net_param-网络配置参数
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	HscResult GetNetConfig(HscDeviceNetParam & net_param) const;

	/**************************
	* @brief: 设置网络配置
	* @param:net_param-网络配置参数
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	HscResult SetNetConfig(const HscDeviceNetParam & net_param);

	// 获取设备内部真实的曝光时间单位：0-μs, 1-100ns, 2-10ns, 3-1ns
	agile_device::capability::Units GetRealExposureTimeUnit() const;

	//转换曝光时间,并且根据帧率限制范围
	int64_t ConvertExposureTime(int64_t expsoure_time, agile_device::capability::Units src_unit, agile_device::capability::Units dest_unit, int64_t frame_rate,bool do_correct = true);

	DrawTypeStatusInfo getDrawTypeStatusInfo();

	void setDrawTypeStatusInfo(DrawTypeStatusInfo status);

	// 校正二次曝光
	void correctEdrExposure();
public:
    /**
     * @brief 获取采集周期范围
     * @param roi ROI
     * @param range 采集周期范围
     * @return true-成功，false-失败
     */
    bool getAcquisitionPeriodRange(const QRect & roi, HscIntRange & range) const;

	//获取曝光时间范围
	bool GetExposureTimeRangeByFrameRate(int64_t frameRate, agile_device::capability::Units exposureTimeUnit, HscIntRange & range);
    /**
     * @brief 获取曝光时间范围
     * @param acquisition_period 采集周期
     * @param range 曝光时间范围
     * @return true-成功，false-失败
     */
    bool getExposureTimeRange(qint64 acquisition_period, agile_device::capability::Units exposureTimeUnit, HscIntRange & range) const;

    /**
     * @brief 获取设备能力集
     * @param type 类型
     * @param in 输入
     * @param out 输出
     * @return 错误码
     */
    HscResult getDeviceCapability(HscCapabilityType type, const std::string & in, std::string & out) const;


	HscResult GetAllDiskVolumes(std::vector<HscDiskMessage> & diskmessages) const;
	HscResult GetCurrentDisk(std::string &disk_names); 
	HscResult SetCurrentDisk(std::string disk_names) const;
    /**
     * @brief 是否为最新帧
     * @param time_stamp 时间戳
     * @return true-是，false-否
     */
    bool isLastestFrame(BYTE time_stamp[9]) const;

    /**
     * @brief 获取亮度
     * @return 亮度
     */
    int getLuminance() const;

    /**
     * @brief 获取对比度
     * @return 对比度
     */
    int getContrast() const;

    /**
     * @brief 获取型号
     * @return 型号
     */
    DeviceModel getModel() const;
	bool isGrabberDevice() const;
	bool isGatewayDevice() const;
	HscDeviceInfo getDeviceInfo() {
		return info_;
	}

	void setDeviceInfo(HscDeviceInfo info);

    /**
     * @brief 开始导出
     * @param id 视频片段ID
     * @return 结果码
     */
    HscResult startExport(int id);

    /**
     * @brief 停止导出
     * @return 结果码
     */
    HscResult stopExport();

    /**
     * @brief 是否支持抽帧导出
     * @return true-支持, false-不支持
     */
    bool isExportByIntervalSupported() const;

    /**
     * @brief 导出
     * @param id 视频片段ID
     * @param start_frame 开始帧
     * @param end_frame 结束帧
     * @return 结果码
     */
    HscResult exportVideoClip(int id, qint64 start_frame, qint64 end_frame, bool new_interface = false);

    /**
     * @brief 抽帧导出
     * @param id 视频片段ID
     * @param start_frame 开始帧
     * @param end_frame 结束帧
     * @param frame_count 帧数
     * @param interval 帧间隔
     * @return 结果码
     */
    HscResult exportByInterval(int id, qint64 start_frame, qint64 end_frame, qint64 frame_count, qint64 interval);

    /**
     * @brief 导出预览
     * @param id 视频片段ID
     * @param frame_no 帧号
     * @return 结果码
     */
	HscResult exportPreview(int id, qint64 frame_no);



    /**
     * @brief 获取回放帧
     * @param buffer 回放帧
     * @return true-成功，false-失败
     */
    bool getPlaybackFrame(CAGBuffer & buffer);

    /**
     * @brief 更新视频片段列表
     */
    void updateVideoSegmentList();

    /**
     * @brief roi参数修正
     * @param [in][out]roi
     */
    void correctRoi(QRect & roi, RoiTypes roi_type=kDeviceRoi);

	///**
	//*@brief 制作图像OSD信息
	//*@param [in] timestamp 时间戳，默认为空
	//*@return OSD信息
	//**/
	//inline QVariant makeRMAImageOSDInfo(const QString & timestamp = QString()) const;

	///**
	//*@brief 制作图像OSD信息
	//*@param [in] state_str 状态字符串
	//*@param [in] timestamp 时间戳，默认为空
	//*@return OSD信息
	//**/
	//inline QVariant makeRMAImageOSDInfo(const QString & state_str, const QString & timestamp) const;


	QList<OSDInfo> makeRMAImageOSDInfo(const QString & state_str, const QString & timestamp);


	QList<OSDInfo> makeRMAImageOSDInfo(const QString & timestamp = QString());
	/**
	*@brief 设备重连
	**/
	void reconnect();

	/**
	* @brief 重连
	*/
	void doReconnect();

	/**
	* @brief 更新重连时的osd信息
	*/
	void updateReconnectOsd();

	/**
	* @brief 消息处理
	*/
	void doMsgProcess();


	/**
	* @brief 更新开机和工作时长
	*/
	void doUpdateCurBootTimeAndTotalWorkTime();

	/**
	* @brief 更新开机和工作时长
	*/
	void UpdateCurBootTimeAndTotalWorkTime();

	/**
	* @brief 开启消息处理线程
	*/
	void startMsgProcessThread();

	/**
	* @brief 退出消息处理线程
	*/
	void exitMsgProcessThread();

	/**
	* @brief 视频彩色参数信息存储
	*/
	void updateColorAlgorithmInfo(int vid);

	uint64_t GetExportSegmentFrameCount() const;

	// 待机
	HscResult SetHardwareStandby();

	// 唤醒
	HscResult StartHardwareAwake();
	void FinishHardwareAwake(bool ok);

	/**
	* @brief 图像获取及处理
	*/
	void doImageProcess();
public:
	/**
	* @brief 设置外同步值
	*/
	void SetExternalSyncAbnormal(bool abnormal);
	/**
	* @brief 设置外同步值
	*/
	bool GetExternalSyncAbnormal() const;


	int  GetIconIndex()const;

	double convertTime(int64_t srcTime, agile_device::capability::Units srcUnit, agile_device::capability::Units destUnit) const;

public:
	uint8_t getExportMode() { return m_export_mode_; }
	uint8_t getStopMode() { return m_stop_mode_; }
	uint8_t getFrameHeadType() { return m_frame_head_type_; }
	std::vector<QString> getAnalogain();

public:
	static CameraWindowRect QRectTCameraWindowRect(const QRect& rect);

	static QRect CameraWindowRect2QRect(const CameraWindowRect&);

	static double CountToPercentage(int count, QRect rect);
	static int PercentageToCount(double percentage, QRect rect);
private:
	template <typename T>
	bool checkVariantType(const QVariant& value)
	{
		typedef T ValueType;
		if (value.isNull() || qMetaTypeId<ValueType>() != value.userType())
			return false;
		return true;
	}


	inline QString getDotStr() { int count = ((++mDotCount) / 6) % 6; QString ss("");  for (int i = 0; i <= count; i++) ss += " ."; return ss; }//六个点循环播放

	uint64_t getCalPeriodDivisor() const{ 
		if ((agile_device::capability::Units)m_period_unit_ == agile_device::capability::Units::kUnit100ns) return 1e7; return 1e6; 
	}

	int getPeriodUnit() const{
		return agile_device::capability::Units::kUnitUs;
	}

	bool isCurrentDeviceInComputerGateway();
	void initComputerIpList();

	//针对视频列表缩略图进行图像处理
	void videoThumbnailProcess(cv::Mat& src,const VideoItem &item);

	bool GetOneRccImageFrame(RccImageFrameInfo &rccImage);

	void startRccImageProcessThread();
	void exitRccImageProcessThread();
	bool isGType();

	bool temperatureParse(QString strTemperature);
private:
    HscDeviceInfo info_; // 设备信息

    DeviceHandle device_handle_{ NULL }; // 设备句柄

    DeviceState state_{ DeviceState::Unconnected }; // 设备状态

    DeviceCapability device_capability_{}; // 设备能力集

	//白平衡相关参数:是否支持ARM白平衡
	bool m_support_arm_wb{false};

	uint8_t m_support_trigger_sync{ 0 }; // 是否支持触发同步

	bool m_support_jitter_elimination_length{ false }; // 是否支持消抖长度

	bool m_support_binning_mode{ false }; // 是否支持像素融合功能

    std::map<PropType, std::function<HscResult(PropType, const QVariant&)>> map_prop_set_; //设置函数对象映射
    std::map<PropType, std::function<HscResult(QVariant &,bool )>> map_prop_get_; // 获取函数对象映射
    std::map<PropType, std::function<void(QVariantList &)>> map_prop_get_supported_; // 获取支持属性值函数对象映射
    std::map<PropType, std::function<void(qint64 &, qint64 &, qint64 &)>> map_prop_get_range_; // 获取属性值范围函数对象映射

	std::map<PropType, std::function<HscResult(PropType, const QVariant&)>> map_cf18_prop_set_; //CF18-设置函数对象映射
	std::map<PropType, std::function<HscResult(QVariant &, bool)>> map_cf18_prop_get_; // CF18-获取函数对象映射
	std::map<PropType, std::function<void(qint64 &, qint64 &, qint64 &)>> map_cf18_prop_get_range_; // 获取属性值范围函数对象映射

    HscDeviceParam param_; // 设备参数

	uint8_t current_out_bpp_ = 0;//当前输出位深,为0代表未读取

	DisplayBitDepth current_display_bpp_ = DBD_NULL; //记录当前显示位深

    QWeakPointer<Device> parent_wptr_; // 父设备

    QMutex mutex_;

    QPointer<QTimer> realtime_frame_timer_ptr_; // 实时帧定时器
    quint8 last_realtime_timestamp_[9]{ 0 };
    const quint8 kZeroTimestamp[9]{ 0 };
	std::mutex m_mutex;	//警告信息容器写入锁

	uint8_t  m_fan_state = 0;//风扇状态

	bool m_next_acquiring_enabled{ true }; // 是否开启下次采集
   //外同步方式默认为false
	bool external_sync_abnormal_{ false };
	mutable bool external_sync_abnormal_on{ false };

	uint16_t m_fps_backup{ 0 };//当前帧率备份
	int mDotCount = 0;
#ifdef CSRCCAPP

#else

    std::shared_ptr<PlayerControllerInterface> realtime_player_controller_ptr_;
    std::unique_ptr<RMAImage> rma_image_ptr_;

    friend class PlaybackCacher;
#endif //CSRCCAPP
	friend class CSPlaybackCacher;

    mutable QMutex video_segment_map_mutex_;
    QMap<int, QSharedPointer<HscVideoClipInfo>> video_segment_map_;

    friend class ImageProcessor;
    std::shared_ptr<ImageProcessor> image_processor_ptr_;

	

	bool m_enable_image_training = false; // 训练模式使能标记

	friend class PreviewThread;
	friend class AcquireThread;

	friend class DeviceExportingWidget;
	friend class CSDlgDeviceVideoExport;

    friend class MsgListener;
    std::unique_ptr<MsgListener> msg_listener_ptr_;

	DeviceState previous_state_{ DeviceState::Unconnected };

	static const int kReconnectTimeout_{ 60000 };//自动重连超时时间60s
	int reconnect_count_down_{ 60 };//自动重连倒计时，单位:s
	QPointer<QTimer> reconnected_timer_ptr_; // 自动重连定时器
	std::atomic_bool m_quit_reconnect{false};//退出重连
	std::atomic_bool m_reconnect_timeout{false};//重连超时
	std::atomic_bool m_destory{ false };//退出
	


	//设备温度相关
	const int kTemperatureBufLen = 1024;//温度字段长度
	char *temperature_buf_ptr_{ nullptr };//温度字段

	std::thread device_msg_process_thread_;//设备消息处理线程
	std::atomic_bool bdevice_msg_process_thread_exit_{ true };
	std::condition_variable device_msg_cv_;
	mutable std::mutex device_cv_mutex_;
	QVector<EventMsg> device_msg_vector_;
	QMap<agile_device::capability::Units, double> time_unit_ratio_map_;
	WaringVector m_vecWarnings;	//收到的警告信息容器

	bool m_bAutoConnect{ false };	//自动连接标志
	uint8_t m_export_mode_ = 0;
	uint8_t m_stop_mode_ = 0;
	int32_t		m_support_edit_property_ = 0;
	int32_t		m_roi_constraint_ = 0; // 0:无，1：中心，2：上下，3：左右
	bool		m_support_cmos_digitalGain_ = 0;
	int8_t		m_resolution_ratio_type_ = -1;
	uint8_t		m_exposure_value_type = 0; // 0: 峰值、平均值 ，1：峰值，2：平均值
	uint8_t		m_intelligent_trigger = 0; // 是否用G270智能触发界面
	char*       m_bits = nullptr;  // 支持bit配置
	char*       m_gain_range = nullptr;
	int32_t		m_pulsewidth_mode = 0;
	int8_t		m_period_unit_ = agile_device::capability::Units::kUnitUs;
	int8_t		m_recording_offset_mode_ = 0;
	char*		m_analog_gain_values = nullptr;
	int8_t		m_frame_head_type_ = (int8_t)HEAD_TYPE::eXType;
		
	bool		m_bManualWhiteBalance{ false }; // false:不进行手动白平衡 ，true:进行手动白平衡
	QSet<QString> m_computer_ip_qset{};
	// 记录鼠标处于的状态， 设备状态切换时鼠标恢复初始状态，
	DrawTypeStatusInfo m_iDrawTypeStatusInfo{ DTSI_Noraml };

	std::thread cf18_heart_beat_thread_;//CF18心跳处理线程
	std::atomic_bool bcf18_heart_beat_thread_exit_{ true };

	std::thread device_image_process_thread_;//设备获取图像处理线程
	std::atomic_bool bdevice_image_process_thread_exit_{ true };
	std::condition_variable device_image_cv_;
	mutable std::mutex device_image_cv_mutex_;
	std::recursive_mutex thread_process_mutex_;
	QVector<RccImageFrameInfo> device_image_vector_;
	const int kFRAME_BUFFER_MAX{ 2 };//图像队列缓存最大值	

	QMap<QVariant, cv::Mat> m_video_list_thumbnail{};

	bool m_bShowTip{ false };
	HscElapsedTimer		mTimer;

	// s1315 filter test
	uint8_t mFilterEnable = 0;

	QMap<QString, std::pair<int, int>> map_temperature_range_;
	// 一次连接仅做一次提示
	bool m_bTemperatureShowTip{ false };
public:
	void setFilterEnable(uint8_t enable);
};

Q_DECLARE_METATYPE(Device::PropType)
Q_DECLARE_METATYPE(StreamType)
Q_DECLARE_METATYPE(ANALOG_GAIN_TYPE)
Q_DECLARE_METATYPE(SyncSource)
Q_DECLARE_METATYPE(TriggerMode)
Q_DECLARE_METATYPE(ExternalTriggerMode)
Q_DECLARE_METATYPE(RecordMode)
Q_DECLARE_METATYPE(RecordType)
Q_DECLARE_METATYPE(VideoFormat)
Q_DECLARE_METATYPE(HscRotationType)
Q_DECLARE_METATYPE(HscDisplayMode)
Q_DECLARE_METATYPE(HScWbEnv)
Q_DECLARE_METATYPE(SDIFpsResol)
Q_DECLARE_METATYPE(HisiAwbMode)
inline QDataStream& operator<<(QDataStream& arch, const CameraWindowRect& obj)
{
	arch << obj.x;
	arch << obj.y;
	arch << obj.height;
	arch << obj.width;
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, CameraWindowRect& obj)
{
	arch >> obj.x;
	arch >> obj.y;
	arch >> obj.height;
	arch >> obj.width;
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscIntelligentTriggerParamV2& obj)
{
	arch << obj.enable;
	arch << obj.roi;
	arch << obj.roi_visible;
	arch << obj.roi_color;
	arch << obj.threshold;
	arch << obj.trigger_type;
	arch << obj.pixel_number_thres;
	arch << obj.pixel_active_thres;
	arch << obj.threshold_new;
	arch << obj.bit_width;
	arch.writeRawData((const char*)obj.res, 35);
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscIntelligentTriggerParamV2& obj)
{
	arch >> obj.enable;
	arch >> obj.roi;
	arch >> obj.roi_visible;
	arch >> obj.roi_color;
	arch >> obj.threshold;
	arch >> obj.trigger_type;
	arch >> obj.pixel_number_thres;
	arch >> obj.pixel_active_thres;
	arch >> obj.threshold_new;
	arch >> obj.bit_width;
	arch.readRawData((char*)obj.res, 35);
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscEDRParam& obj)
{
	arch << obj.enabled;
	arch << obj.lowerLuminance;
	arch << obj.upperLuminance;
	arch << obj.doubleExposureTime;
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscEDRParam& obj)
{
	arch >> obj.enabled;
	arch >> obj.lowerLuminance;
	arch >> obj.upperLuminance;
	arch >> obj.doubleExposureTime;
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscEDRParamV2& obj)
{
	arch << obj.enabled;
	arch << obj.lowerLuminance;
	arch << obj.upperLuminance;
	arch << obj.doubleExposureTime;
	arch.readRawData((char*)obj.res, 16);
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscEDRParamV2& obj)
{
	arch >> obj.enabled;
	arch >> obj.lowerLuminance;
	arch >> obj.upperLuminance;
	arch >> obj.doubleExposureTime;
	arch.writeRawData((const char*)obj.res, 16);
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscAutoExposureParameter& obj)
{
	arch << obj.bEnable;
	arch << obj.autoExpArea;
	arch << obj.roi_visible;
	arch << obj.roi_color;
	arch << static_cast<qint32>(obj.autoExpGrayMode);
	arch << obj.targetGray;
	arch << obj.targetGrayNew;
	arch.writeRawData((const char*)obj.reserved, 41);
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscAutoExposureParameter& obj)
{
	arch >> obj.bEnable;
	arch >> obj.autoExpArea;
	arch >> obj.roi_visible;
	arch >> obj.roi_color;

	qint32 iautoExpGrayMode{};
	arch >> iautoExpGrayMode;
	obj.autoExpGrayMode = static_cast<AutoExposureGrayMode>(iautoExpGrayMode);
	arch >> obj.targetGray;
	arch >> obj.targetGrayNew;
	arch.readRawData((char*)obj.reserved, 41);
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscImageCtrlParam& obj)
{
	arch << obj.enableMirrorImage;
	arch << obj.enableInvertImage;
	arch << obj.enableFlipImage;
	arch.writeRawData((const char*)obj.res, 13);
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscImageCtrlParam& obj)
{
	arch >> obj.enableMirrorImage;
	arch >> obj.enableInvertImage;
	arch >> obj.enableFlipImage;
	arch.readRawData((char*)obj.res, 13);
	return arch;
}


inline QDataStream& operator<<(QDataStream& arch, const HscLaserParam& obj)
{
	arch << obj.pulseWidth1;
	arch << obj.pulseWidth2;
	arch << obj.pulseInterval;
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscLaserParam& obj)
{
	arch >> obj.pulseWidth1;
	arch >> obj.pulseWidth2;
	arch >> obj.pulseInterval;
	return arch;
}
inline QDataStream& operator<<(QDataStream& arch, const HscPIVParam& obj)
{
	arch << obj.enabled;
	arch << obj.acquisitionFrequency;
	arch << obj.laserInterval;
	arch << obj.lasterParam[0];
	arch << obj.lasterParam[1];
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscPIVParam& obj)
{
	arch >> obj.enabled;
	arch >> obj.acquisitionFrequency;
	arch >> obj.laserInterval;
	arch >> obj.lasterParam[0];
	arch >> obj.lasterParam[1];
	return arch;
}

inline QDataStream& operator<<(QDataStream& arch, const HscHisiManualAwb& obj)
{
	arch << static_cast<qint32>(obj.mode);
	arch << obj.r_gain;
	arch << obj.gr_gain;
	arch << obj.gb_gain;
	arch << obj.b_gain;
	return arch;
}

inline QDataStream& operator >> (QDataStream& arch, HscHisiManualAwb& obj)
{
	qint32 iautoExpGrayMode{};
	arch >> iautoExpGrayMode;
	obj.mode = static_cast<HisiAwbMode>(iautoExpGrayMode);
	arch >> obj.r_gain;
	arch >> obj.gr_gain;
	arch >> obj.gb_gain;
	arch >> obj.b_gain;
	return arch;
}



Q_DECLARE_METATYPE(CameraWindowRect)
Q_DECLARE_METATYPE(HscIntelligentTriggerParamV2)
Q_DECLARE_METATYPE(HscEDRParam)
Q_DECLARE_METATYPE(HscEDRParamV2)
Q_DECLARE_METATYPE(HscAutoExposureParameter)
Q_DECLARE_METATYPE(HscImageCtrlParam)
Q_DECLARE_METATYPE(HscLaserParam)
Q_DECLARE_METATYPE(HscPIVParam)
Q_DECLARE_METATYPE(HscColorCorrectInfo)
Q_DECLARE_METATYPE(HscHisiManualAwb)
Q_DECLARE_METATYPE(agile_device::capability::Units)

#endif // DEVICE_H
