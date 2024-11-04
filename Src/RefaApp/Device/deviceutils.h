#ifndef DEVICEUTILS_H
#define DEVICEUTILS_H

#include <QList>
#include <QSize>
#include <QString>
#include "HscAPIStructer.h"

/**
 * @brief 设备工具类
 */
class DeviceUtils
{
public:
    /**
     * @brief 获取通用典型分辨率
     * @param resolutions 典型分辨率
     */
    static void getGeneralTypicalResolutions(QList<QSize> & resolutions,DeviceModel model, int type = 0);

    /**
     * @brief 获取通用典型帧率
     * @param frame_rates 典型帧率
     */
    static void getGeneralTypicalFrameRates(QList<qint64> & frame_rates);

    /**
     * @brief 获取通用典型曝光时间
     * @param exposure_times 典型曝光时间
     */
    static void getGeneralTypicalExposureTimes(QList<qint64> & exposure_times);

    /**
     * @brief 格式化时间戳
     * @param timestamp 时间戳
     * @return 时间戳字符串，格式：yyyy-MM-dd hh:mm:ss.uuuuuu
     */
    static QString formatTimestamp(quint8 timestamp[9]);


	/**
	* @brief 格式化文件名时间戳
	* @param timestamp 时间戳
	* @return 时间戳字符串，格式：yyyyMMdd_hhmmss_uuuuuu
	*/
	static QString formatFileTimestamp(quint8 timestamp[9], uint8_t frame_header_type=eXType);

	
	/**
	 * @brief 获取协议格式文本
	 * @param stream_type 协议格式
	 * @return 协议格式文本
	 */
	static QString getStreamTypeText(int stream_type);

	/**
	 * @brief 获取模拟增益文本
	 * @param gain 模拟增益
	 * @return 模拟增益文本
	 */
	static QString getAnalogGainText(int gain);

	/**
	* @brief 获取灰度模式
	* @param gain 灰度模式
	* @return 灰度模式文本
	*/
	static QString getAutoExposureGrayModeText(int mode);

	/**
	 * @brief 获取触发方式文本
	 * @param mode 触发方式
	 * @return 触发方式文本
	 */
	static QString getTriggerModeText(int mode);

	/**
	 * @brief 获取外触发方式文本
	 * @param mode 外触发方式
	 * @return 外触发方式文本
	 */
	static QString getExternalTriggerModeText(int mode);

	/**
	* @brief 获取同步方式文本
	* @param mode 同步方式
	* @return 同步方式文本
	*/
	static QString getSyncSourceText(int mode);

	/**
	* @brief 获取曝光单位文本
	* @param mode 曝光单位
	* @return 曝光单位文本
	*/
	static QString getExposureTimeUnitText(int mode);

	/**************************
	* @brief: 获取分辨率&播放速率帧率
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	static QString getSdiFpsResolsText(int index);
	/**
	 * @brief 获取保存起点方式文本
	 * @param mode 保存起点方式
	 * @return 保存起点方式文本
	 */
	static QString getRecordingOffsetModeText(int mode);

	/**
	 * @brief 获取保存单位文本
	 * @param unit 保存单位
	 * @return 保存单位文本
	 */
	static QString getRecordingUnitText(int unit);

	/**
	 * @brief 获取视频格式文本
	 * @param format 视频格式
	 * @return 视频格式文本
	 */
	static QString getVideoFormatText(int format);

	/**
	* @brief 获取色彩模式文本
	* @param mode 色彩模式
	* @return 色彩模式文本
	*/
	static QString getColorModeText(int mode);

	/**
	* @brief 获取SDI参数文本
	* @param param 参数枚举值
	* @return SDI参数文本
	*/
	static QString getSDIParamText(int param);

	/**
	* @brief 获取开启关闭文本
	* @param enable 0-关闭 1-打开
	* @return 开启关闭文本
	*/
	static QString getOnOffText(int enable);

	// 判定是否是相机
	static bool IsCamera(DeviceModel model);

	// 判定是否是同步控制器
	static bool IsTrigger(DeviceModel model);

	static bool IsCF18(DeviceModel model);

	//获取格式化设备自定义名+IP
	static QString getFormatedDeviceUserNameAndIpOrSn(const QString& user_name,const QString& ipOrSn);

	static uint64_t getTimestamp(uint8_t timestamp[]);

	/**
	* @brief 获取像素位深
	* @param value 显示为value Bit
	* @return 开启关闭文本
	*/
	static QString getPixelBitDepthText(int value);

	/**
	* @brief 获取像素位深
	* @param value 显示为value Bit
	* @return 开启关闭文本
	*/
	static QString getDisplayBitDepthText(int value);

	/**
	* @brief 获取像素数量触发模式
	* @param model 0-高于 1-低于
	* @return 开启关闭文本
	*/
	static QString getPixelTriggerModel(int model);

	static QString formatNewTimestamp(quint8 timestamp[]);
	static QString formatNewTimestampG(quint8 timestamp[]);
	static QString formatNewTimestampNs(quint8 timestamp[]);
	static uint64_t getTimestampTons(uint8_t timestamp[]);

	static QString getCF18PolarityText(int index);//极性反转
	static QString getCF18Channel(int index);//通道
	static QString getCF18SignalUnit(int index);//内同步信号单位
	static QString getCF18SignalType(int index);//信号类型
	static QString formatNewFileTimestampG(quint8 timestamp[]);
	static QString formatNewFileTimestampNs(quint8 timestamp[]);

};

#endif // DEVICEUTILS_H
