#ifndef RCCFRAMEINFO_H
#define RCCFRAMEINFO_H
#include <QImage>
#include <QString>
#include <QByteArray>
#include <QList>
#include "Device/deviceutils.h"
#include <opencv2/opencv.hpp>
struct OSDInfo
{
  enum PosType{
    LT,
	LB,
	RT,
	RB,
	CC
  };
  PosType pos_type;
  QString OsdText;
  QColor color;
};

struct PlaybackInfo//帧的回放相关属性
{
	int64_t frame_no{ 0 };
	int64_t start_frame_no{ 0 };
	int64_t end_frame_no{ 0 };
	//缩略图相关
	enum ThumbNailState {
		TN_VOID,//无图像
		TN_NORMAL,//图像正常
		TN_LOADING,//正在加载
		TN_FAIL//加载失败
	};
	ThumbNailState thumb_nail_state = TN_VOID;//缩略图状态
	bool is_key_frame{ false };//是否为关键帧
	bool is_current_frame{ false };//是否为当前帧
	bool is_highlight_frame{ false };//是否为高亮帧(预览缩略图中心帧)
	bool m_image_changed{ false };

};

//图像拓展信息
struct VideoExtendInfo
{
	uint16_t device_index = 0; // 设备编号
	uint16_t device_state = 0; // 设备状态
	int32_t azimuth = 0; // 方位角
	int32_t pitch = 0; // 俯仰角
	int32_t focal_length = 0; // 焦距
	uint64_t distance = 0L; // 距离值
	uint16_t frame_rate = 0; // 帧率
	uint8_t frame_header_rows = 0; // 帧头行数
	uint32_t device_model = 0; // 设备型号
	uint32_t exposure_time = 0; // 曝光时间（μs）
	uint8_t trigger_mode = 0; // 触发模式：00-外触发，01-内触发
	uint8_t external_clock_status = 0; // 外部时钟状态：0-有效，1-无效
	uint8_t time_source = 0; // 时间源：0-内置时钟，1-AC数据
	uint8_t exposure_mode = 1; // 曝光模式：00-自动曝光，01-固定曝光，10-脉宽曝光
	uint8_t gain = 0; // 增益
	uint32_t capture_start_time = 0; // 拍摄起始时刻，24小时，单位：秒
};

struct RccFrameInfo
{
  QImage image;
  cv::Mat raw_image;//QImage暂不支持16bit图像显示,添加cv::Mat用于获取像素数据
  quint8 valid_bits{ 8 };
  QString device_name;
  QString ip_or_sn;
  quint8 timestamp[9]{ 0 };//使用 DeviceUtils::formatFileTimestamp 或DeviceUtils::formatTimestamp生成时间戳字符串
  QList<OSDInfo> osdInfos;
  PlaybackInfo playback_info{};
  VideoExtendInfo extend_info; // 图像拓展信息
};


inline RccFrameInfo makeOsdInfo(
	QImage image,
	const QString & fmt_status_str,
	const QString & fmt_time_stamps,
	const QString & fmt_device_name,
	const QColor & state_color,
	const QByteArray& extendInfo
)
{
	Q_UNUSED(extendInfo);
	RccFrameInfo frameInfo;

	OSDInfo osdInfo_LT{};
	osdInfo_LT.pos_type = OSDInfo::LT;
	osdInfo_LT.OsdText = fmt_device_name;
	osdInfo_LT.color = QColor(255, 0, 0);
	frameInfo.osdInfos << osdInfo_LT;

	OSDInfo osdInfo_RT{};
	osdInfo_RT.pos_type = OSDInfo::RT;
	osdInfo_RT.OsdText = fmt_status_str;
	osdInfo_RT.color = state_color;
	frameInfo.osdInfos << osdInfo_RT;

	OSDInfo osdInfo_LB{};
	osdInfo_LB.pos_type = OSDInfo::LB;
	osdInfo_LB.OsdText = fmt_time_stamps;
	osdInfo_LB.color = QColor(255, 0, 0);
	frameInfo.osdInfos << osdInfo_LB;

	frameInfo.image = image;
	return frameInfo;
}

inline RccFrameInfo makeOsdInfo()
{
	return makeOsdInfo(QImage{}, QString("<font color=#FF0000>%1</font>").arg(QObject::tr("no device associate")), QString{}, QString{},QColor(255, 0, 0), QByteArray{});
}


inline RccFrameInfo makeOsdInfo(const QString & fmt_status_str, const QString & fmt_time_stamps, const QString fmt_device_name ,const QColor state_color)
{
	return makeOsdInfo(QImage{}, fmt_status_str, fmt_time_stamps, fmt_device_name,state_color, QByteArray{});
}

struct RccImageFrameInfo
{
	RccFrameInfo image;
	uint16_t image_trigger_avg_avg_lum; // 智能触发区域平均亮度
	uint16_t auto_exposure_area_avg_lum; // 自动曝光区域平均亮度
	uint32_t active_pixel_num_;
};

Q_DECLARE_METATYPE(OSDInfo::PosType)
Q_DECLARE_METATYPE(OSDInfo)
Q_DECLARE_METATYPE(RccFrameInfo)
Q_DECLARE_METATYPE(PlaybackInfo)
Q_DECLARE_METATYPE(VideoExtendInfo)
Q_DECLARE_METATYPE(PlaybackInfo::ThumbNailState)
#endif // RCCFRAMEINFO_H
