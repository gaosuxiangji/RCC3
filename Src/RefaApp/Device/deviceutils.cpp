#include "deviceutils.h"

#include <QDate>
#include <QtEndian>
#include <QObject>
#include "device.h"
#include "HscAPIStructer.h"
#include "AgCapability/capability_types.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"

void DeviceUtils::getGeneralTypicalResolutions(QList<QSize> &resolutions, DeviceModel model, int type)
{
	if (type == 1)
	{
		resolutions << QSize(2304, 1088);
		resolutions << QSize(1920, 1080);
		resolutions << QSize(2160, 960);
		resolutions << QSize(1280, 720);
		resolutions << QSize(800, 600);
		resolutions << QSize(640, 512);
		resolutions << QSize(256, 128);
		return;
	}

	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion() && model == DeviceModel::DEVICE_XJ1310)
	{
		resolutions << QSize(1280, 1024);
		resolutions << QSize(1024, 768);
		resolutions << QSize(768, 576);
		resolutions << QSize(640, 512);
		resolutions << QSize(320, 256);
		resolutions << QSize(512, 128);
	}
	else if (model == DeviceModel::DEVICE_GRABBER_100T) {
		resolutions << QSize(1920, 1088);
		resolutions << QSize(1920, 1080);
		resolutions << QSize(1344, 960);
		resolutions << QSize(768, 480);
		resolutions << QSize(768, 32);
	}
	else if (model == DeviceModel::DEVICE_GRABBER_120 || model == DeviceModel::DEVICE_GRABBER_110) {
		resolutions << QSize(1024, 1024);
		resolutions << QSize(1024, 768);
		resolutions << QSize(1280, 768);
		resolutions << QSize(768, 768);
		resolutions << QSize(512, 512);
		resolutions << QSize(256, 256);
		resolutions << QSize(128, 128);
		resolutions << QSize(128, 32);
	}
	else if (model == DeviceModel::DEVICE_GRABBER_220) {
		resolutions << QSize(1920, 1088);
		resolutions << QSize(1920, 768);
		resolutions << QSize(1920, 128); 
		resolutions << QSize(768, 32);
	}
	else
	{
		resolutions << QSize(3840, 3840);
		resolutions << QSize(1920, 1440);
		resolutions << QSize(1920, 1080);
		resolutions << QSize(1920, 540);
		resolutions << QSize(1280, 1024);
		resolutions << QSize(1280, 960);
		resolutions << QSize(1280, 800);
		resolutions << QSize(1280, 768);
		resolutions << QSize(1280, 720);
		resolutions << QSize(1280, 600);
		resolutions << QSize(1024, 768);
		resolutions << QSize(800, 600);
		resolutions << QSize(640, 512);
		resolutions << QSize(640, 480);
		resolutions << QSize(256, 128);
		resolutions << QSize(256, 8);
	}
	
}

void DeviceUtils::getGeneralTypicalFrameRates(QList<qint64> &frame_rates)
{
    frame_rates << 100;
	frame_rates << 125;
	frame_rates << 200;
	frame_rates << 250;
	frame_rates << 400;
    frame_rates << 500;
	frame_rates << 800;
    frame_rates << 1000;
	frame_rates << 1600;
    frame_rates << 2000;
	frame_rates << 2500;
    frame_rates << 4000;
	frame_rates << 5000;
    frame_rates << 8000;
	frame_rates << 10000;
	frame_rates << 20000;
	frame_rates << 40000;
	frame_rates << 50000;
	frame_rates << 100000;
	frame_rates << 200000;
	frame_rates << 250000;
	frame_rates << 500000;
	frame_rates << 1000000;
}

void DeviceUtils::getGeneralTypicalExposureTimes(QList<qint64> &exposure_times)
{
    exposure_times << 1;
    exposure_times << 10;
    exposure_times << 20;
    exposure_times << 50;
    exposure_times << 100;
    exposure_times << 200;
    exposure_times << 500;
    exposure_times << 800;
    exposure_times << 1000;
}

QString DeviceUtils::formatTimestamp(quint8 timestamp[])
{
    //9个字节时间戳定义
    //0-1，2Byte代表距离本年1月1日的天数
    //2， 1Byte代表小时
    //3， 1Byte代表分钟
    //4， 1Byte代表秒
    //5-8，4Byte代表微秒，高位在前
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year){
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) +2000;
		uint8_t remain = (timestamp[0] & 1);
		days=  ((timestamp[1]) | (remain & 0xff) <<8);
	}

    quint32 hour = timestamp[2];
    quint32 min = timestamp[3];
    quint32 sec = timestamp[4];
    quint32 usec = qFromBigEndian<quint32>((void*)&timestamp[5]);
    QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

    return QString("%1-%2-%3 %4:%5:%6.%7").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
            .arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0'));
}

QString DeviceUtils::formatFileTimestamp(quint8 timestamp[], uint8_t frame_head_type)
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-8，4Byte代表微秒，高位在前
	if (frame_head_type == eGTypeNs) {
		return formatNewFileTimestampG(timestamp);
	}
	else if (frame_head_type == HEAD_TYPE::eS1315) {
		return formatNewFileTimestampNs(timestamp);
	}
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}
	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];
	quint32 usec = qFromBigEndian<quint32>((void*)&timestamp[5]);


	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1%2%3_%4%5%6_%7").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0'));
}

QString DeviceUtils::getStreamTypeText(int stream_type)
{
	QString str;
	switch (stream_type)
	{
	case StreamType::TYPE_RAW8:
		str = QObject::tr("RAW");
		break;
	case StreamType::TYPE_YUV420:
		str = QObject::tr("YUV420");
		break;
	case StreamType::TYPE_H264:
		str = QObject::tr("H264");
		break;
	case StreamType::TYPE_RGB888:
		str = QObject::tr("RGB");
		break;
	case StreamType::TYPE_RGB8888:
		str = QObject::tr("RGB48");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getAnalogGainText(int gain)
{
	QString str;
	switch (gain)
	{
	case ANALOG_GAIN_TYPE::AAG_1:
		str = "1";
		break;
	case ANALOG_GAIN_TYPE::AAG_2:
		str = "2";
		break;
	case ANALOG_GAIN_TYPE::AAG_4:
		str = "4";
		break;
	case ANALOG_GAIN_TYPE::AAG_8:
		str = "8";
		break;
	case ANALOG_GAIN_TYPE::AAG_16:
		str = "16";
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getAutoExposureGrayModeText(int mode)
{
	QString str;
	switch (mode)
	{
	case AutoExposureGrayMode::AutoExposureGrayMode_Avg:
	{
		str = QObject::tr("Average Gray");
		break;
	}
	case AutoExposureGrayMode::AutoExposureGrayMode_Peak:
	{
		str = QObject::tr("Peak Gray");
		break;
	}
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getTriggerModeText(int mode)
{
	QString str;
	switch (mode)
	{
	case TRIGGER_INTERNAL:
		str = QObject::tr("Internal Trigger");
		break;
	case TRIGGER_EXTERNAL:
		str = QObject::tr("External Trigger");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getExternalTriggerModeText(int mode)
{
	QString str;
	switch (mode)
	{
	case ExternalTriggerMode::TRIGGER_RAISING:
		str = QObject::tr("Raising Edge");
		break;
	case ExternalTriggerMode::TRIGGER_FALLING:
		str = QObject::tr("Falling Edge");
		break;
	case ExternalTriggerMode::TRIGGER_SHORT_CIRCUIT:
		str = QObject::tr("Short Circuit");
		break;
	case ExternalTriggerMode::TRIGGER_OPEN_CIRCUIT:
		str = QObject::tr("Open Circuit");
		break;
	case ExternalTriggerMode::TRIGGER_DOUBLE_EDGE:
		str = QObject::tr("Double Edge");
		break;
	case ExternalTriggerMode::TRIGGER_HIGH_LEVEL:
		str = QObject::tr("High Level");
		break;
	case ExternalTriggerMode::TRIGGER_LOW_LEVEL:
		str = QObject::tr("Low Level");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getSyncSourceText(int mode)
{
	QString str;
	switch (mode)
	{
	case SyncSource::SYNC_INTERNAL:
		str = QObject::tr("Internal Sync");
		break;
	case SyncSource::SYNC_EXTERNAL:
		str = QObject::tr("External Sync");
		break;
	default:
		break;
	}

	return str;
}


QString DeviceUtils::getExposureTimeUnitText(int mode)
{
	QString str;
	switch (mode)
	{
	case agile_device::capability::Units::kUnitNone:
		str = QObject::tr("None Unit");
		break;
	case agile_device::capability::Units::kUnitSecs:
		str = QObject::tr("s");
		break;
	case agile_device::capability::Units::kUnitMs:
		str = QObject::tr("ms");
		break;
	case agile_device::capability::Units::kUnitUs:
		str = QObject::tr("us");
		break;
	case agile_device::capability::Units::kUnit100ns:
		str = QObject::tr("100 ns");
		break;
	case agile_device::capability::Units::kUnit10ns:
		str = QObject::tr("10 ns");
		break;
	case agile_device::capability::Units::kUnitNs:
		str = QObject::tr("ns");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getSdiFpsResolsText(int index)
{
	QString str{};
	switch (index)
	{
	case SdiFpsResol_720P_25FPS:
		str = QObject::tr("720P&25 Frame/s");
		break;
	case SdiFpsResol_720P_50FPS:
		str = QObject::tr("720P&50 Frame/s");
		break;
	case SdiFpsResol_1080P_25FPS:
		str = QObject::tr("1080P&25 Frame/s");
		break;
	case SdiFpsResol_1080P_50FPS:
		str = QObject::tr("1080P&50 Frame/s");
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getRecordingOffsetModeText(int mode)
{
	QString str;
	switch (mode)
	{
	case RecordMode::RECORD_BEFORE_SHUTTER:
		str = QObject::tr("Before Shutter");
		break;
	case RecordMode::RECORD_AFTER_SHUTTER:
		str = QObject::tr("After Shutter");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getRecordingUnitText(int unit)
{
	QString str;
	switch (unit)
	{
	case RecordType::RECORD_BY_FRAME:
		str = QObject::tr("Frame");
		break;
	case RecordType::RECORD_BY_TIME:
		str = QObject::tr("ms");
		break;
	case RecordType::RECORD_BY_TIME_S:
		str = QObject::tr("s");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getVideoFormatText(int format)
{
	QString str;
	switch (format)
	{
	case VideoFormat::VIDEO_RHVD:
		str = QObject::tr("RHVD");
		break;
	case VideoFormat::VIDEO_AVI:
		str = QObject::tr("AVI");
		break;
	case VideoFormat::VIDEO_BMP:
		str = QObject::tr("BMP");
		break;
	case VideoFormat::VIDEO_JPG:
		str = QObject::tr("JPEG");
		break;
	case VideoFormat::VIDEO_MP4:
		str = QObject::tr("MP4");
		break;
	case VideoFormat::VIDEO_TIF:
		str = QObject::tr("TIFF");
		break;
	case VideoFormat::VIDEO_RAW:
		str = QObject::tr("RAW");
		break;
	case VideoFormat::VIDEO_PNG:
		str = QObject::tr("PNG");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getColorModeText(int mode)
{
	QString str;
	switch (mode)
	{
	case HscDisplayMode::HSC_DISPLAY_MONO:
		str = QObject::tr("Monochrome");
		break;
	case HscDisplayMode::HSC_DISPLAY_COLOR:
		str = QObject::tr("Color");
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getSDIParamText(int param)
{
	QString str;
	switch (param)
	{
	case SDIFpsResol::SdiFpsResol_720P_25FPS:
		str = QObject::tr("720p@25hz");
		break;
	case SDIFpsResol::SdiFpsResol_720P_50FPS:
		str = QObject::tr("720p@50hz");
		break;
	case SDIFpsResol::SdiFpsResol_1080P_25FPS:
		str = QObject::tr("1080p@25hz");
		break;
	case SDIFpsResol::SdiFpsResol_1080P_50FPS:
		str = QObject::tr("1080p@50hz");
		break;
	default:
		break;
	}

	return str;
}

QString DeviceUtils::getOnOffText(int enable)
{
	QString str;
	switch (enable)
	{
	case 0:
		str = QObject::tr("Off");
		break;
	case 1:
		str = QObject::tr("On");
		break;
	default:
		str = QObject::tr("Off");
		break;
	}

	return str;
}

bool DeviceUtils::IsCamera(DeviceModel model)
{
	return (DEVICE_UNKNOWN < model && model < DEVICE_TRIGGER);

}

bool DeviceUtils::IsTrigger(DeviceModel model)
{
	return (DEVICE_TRIGGER == model || DEVICE_TRIGGER_CF18 == model || DEVICE_TRIGGER_ASG8x00 == model);

}

bool DeviceUtils::IsCF18(DeviceModel model)
{
	return (DEVICE_TRIGGER_CF18 == model);
}

QString DeviceUtils::getFormatedDeviceUserNameAndIpOrSn(const QString& user_name, const QString& ipOrSn)
{
	QString dstStr = user_name;
	if (dstStr.length() > 5)
	{
		dstStr = dstStr.left(5) + "...";
	}
	return QString("<font color=#FF0000>%1 %2</font>").arg(dstStr).arg(ipOrSn);
}


uint64_t DeviceUtils::getTimestamp(uint8_t timestamp[])
{
	uint8_t remain = (timestamp[0] & 1);
	quint32 days = ((timestamp[1]) | (remain & 0xff) << 8);
	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];
	quint32 usec = qFromBigEndian<quint32>((void*)&timestamp[5]);
	uint64_t total_usec = usec + (sec + min * 60 + hour * 3600 + days * 24 * 3600) * (1000000);
	return total_usec;
}

QString DeviceUtils::getPixelBitDepthText(int value)
{
	QString str;
	str = QObject::tr("%1bit").arg(value);
	return str;
}

QString DeviceUtils::getDisplayBitDepthText(int value)
{
	QString str;
	Device::DisplayBitDepth dbd = (Device::DisplayBitDepth)value;
	switch (dbd)
	{
	case Device::DBD_0_7:
		str = "0 ~ 7";
		break;
	case Device::DBD_1_8:
		str = "1 ~ 8";
		break;
	case Device::DBD_2_9:
		str = "2 ~ 9";
		break;
	case Device::DBD_3_10:
		str = "3 ~ 10";
		break;
	case Device::DBD_4_11:
		str = "4 ~ 11";
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getPixelTriggerModel(int model)
{
	QString str;
	switch (model)
	{
	case 0:
		str = QObject::tr("Light Area");
		break;
	case 1:
		str = QObject::tr("Dark Area");
		break;
	default:
		str = QObject::tr("Light Area");
		break;
	}
	return str;
}

QString DeviceUtils::formatNewTimestamp(quint8 timestamp[])
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-8，4Byte代表纳秒，高位在前
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		// |         8 bits        |      8 bits     | 
		// |1    | 1 1 1 1 1 1 | 1 | 1 1 1 1 1 1 1 1 |
		// |flag | year        | days of year        |
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}

	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];

	quint8 nsec1 = timestamp[5];
	quint8 usec1 = timestamp[6];
	quint8 usec2 = timestamp[7];
	quint8 usec3 = timestamp[8];
	quint32 usec = usec1 << 16 | usec2 << 8 | usec3;
	quint32 nsec = nsec1 * 40;

	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1-%2-%3 %4:%5:%6.%7.%8").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0')).arg(nsec, 3, 10, QChar('0'));
}

QString DeviceUtils::formatNewTimestampG(quint8 timestamp[])
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-7，4Byte代表微秒，高位在前
	//8， 1Byte代表百纳秒
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		// |         8 bits        |      8 bits     | 
		// |1    | 1 1 1 1 1 1 | 1 | 1 1 1 1 1 1 1 1 |
		// |flag | year        | days of year        |
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}

	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];

	auto usec1 = timestamp[5];
	auto usec2 = timestamp[6];
	auto usec3 = timestamp[7];
	auto usec = usec1 << 16 | usec2 << 8 | usec3;
	auto nsec = timestamp[8]*100;

	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1-%2-%3 %4:%5:%6.%7.%8").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0')).arg(nsec, 3, 10, QChar('0'));
}

uint64_t DeviceUtils::getTimestampTons(uint8_t timestamp[])
{
	uint8_t remain = (timestamp[0] & 1);
	quint32 days = ((timestamp[1]) | (remain & 0xff) << 8);
	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];
	quint8 usec1 = timestamp[6];
	quint8 usec2 = timestamp[7];
	quint8 usec3 = timestamp[8];
	quint32 usec = usec1 << 16 | usec2 << 8 | usec3;
	quint8 nsec1 = timestamp[5];
	quint32 nsec = nsec1 * 40;
	uint64_t total_usec = usec + (sec + min * 60 + hour * 3600 + days * 24 * 3600) * (1000000);
	return total_usec;
}

QString DeviceUtils::getCF18PolarityText(int index)
{
	QString str;
	switch (index)
	{
	case 0:
		str = QObject::tr("Close");
		break;
	case 1:
		str = QObject::tr("Open");
		break;
	default:
		str = QObject::tr("Close");
		break;
	}
	return str;
}

QString DeviceUtils::getCF18Channel(int index)
{
	QString str;
	switch (index)
	{
	case CF18_CHANNEL_1:
		str = QObject::tr("Chnannel1");
		break;
	case CF18_CHANNEL_2:
		str = QObject::tr("Chnannel2");
		break;
	case CF18_CHANNEL_3:
		str = QObject::tr("Chnannel3");
		break;
	case CF18_CHANNEL_4:
		str = QObject::tr("Chnannel4");
		break;
	case CF18_CHANNEL_5:
		str = QObject::tr("Chnannel5");
		break;
	case CF18_CHANNEL_6:
		str = QObject::tr("Chnannel6");
		break;
	case CF18_CHANNEL_7:
		str = QObject::tr("Chnannel7");
		break;
	case CF18_CHANNEL_8:
		str = QObject::tr("Chnannel8");
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getCF18SignalUnit(int index)
{
	QString str;
	switch (index)
	{
	case 0:
		str = QString("Hz");
		break;
	case 1:
		str = QString::fromLocal8Bit("μs");
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::getCF18SignalType(int index)
{
	QString str{};
	switch (index)
	{
	case CF18_INTERNAL_SYNCHRONOUS:
		str = QObject::tr("Internal Synchronous Signal");
		break;
	case CF18_EXTERNAL_SYN:
		str = QObject::tr("External Synchronous Signal");
		break;
	case CF18_INTERNAL_TRIGGER:
		str = QObject::tr("Internal Trigger Signal");
		break;
	case CF18_EXTERNAL_TRIGGER:
		str = QObject::tr("External Trigger Signal");
		break;
	case CF18_SEQUENCE_GENERATOR:
		str = QObject::tr("Sequence Generator Signal");
		break;
	case CF18_B_CODE:
		str = QObject::tr("B Code Signal");
		break;
	default:
		break;
	}
	return str;
}

QString DeviceUtils::formatNewFileTimestampG(quint8 timestamp[])
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-7，4Byte代表微秒，高位在前
	//8， 1Byte代表百纳秒
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		// |         8 bits        |      8 bits     | 
		// |1    | 1 1 1 1 1 1 | 1 | 1 1 1 1 1 1 1 1 |
		// |flag | year        | days of year        |
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}

	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];

	auto usec1 = timestamp[5];
	auto usec2 = timestamp[6];
	auto usec3 = timestamp[7];
	auto usec = usec1 << 16 | usec2 << 8 | usec3;
	auto nsec = timestamp[8] * 100;

	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1%2%3_%4%5%6_%7.%8").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0')).arg(nsec, 3, 10, QChar('0'));
}

QString DeviceUtils::formatNewFileTimestampNs(quint8 timestamp[])
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-7，4Byte代表微秒，高位在前
	//8， 1Byte代表百纳秒
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		// |         8 bits        |      8 bits     | 
		// |1    | 1 1 1 1 1 1 | 1 | 1 1 1 1 1 1 1 1 |
		// |flag | year        | days of year        |
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}

	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];

	auto nsec1 = timestamp[5];
	auto nsec2 = timestamp[6];
	auto nsec3 = timestamp[7];
	auto nsec4 = timestamp[8];
	auto nsec = nsec1 << 24 | nsec2 << 16 | nsec3 << 8 | nsec4;

	auto usec = nsec / 1000;
	auto nsec_100 = (nsec % 1000) / 100 * 100;

	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1%2%3_%4%5%6_%7.%8").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0')).arg(nsec_100, 3, 10, QChar('0'));
}

QString DeviceUtils::formatNewTimestampNs(quint8 timestamp[])
{
	//9个字节时间戳定义
	//0-1，2Byte代表距离本年1月1日的天数
	//2， 1Byte代表小时
	//3， 1Byte代表分钟
	//4， 1Byte代表秒
	//5-7，4Byte代表微秒，高位在前
	//8， 1Byte代表百纳秒
	bool timepoint_has_year = ((timestamp[0] & 0x80) >> 7);//timestamp[0]最高位为1代表后续
	quint32 days = qFromBigEndian<quint16>((void*)&timestamp[0]);

	auto year = QDate::currentDate().year();
	if (timepoint_has_year) {
		// |         8 bits        |      8 bits     | 
		// |1    | 1 1 1 1 1 1 | 1 | 1 1 1 1 1 1 1 1 |
		// |flag | year        | days of year        |
		year = static_cast<int>(((timestamp[0] & 0x7E) >> 1)) + 2000;
		uint8_t remain = (timestamp[0] & 1);
		days = ((timestamp[1]) | (remain & 0xff) << 8);
	}

	quint32 hour = timestamp[2];
	quint32 min = timestamp[3];
	quint32 sec = timestamp[4];

	auto nsec1 = timestamp[5];
	auto nsec2 = timestamp[6];
	auto nsec3 = timestamp[7];
	auto nsec4 = timestamp[8];
	auto nsec = nsec1 << 24 | nsec2 << 16 | nsec3 << 8 | nsec4;
	
	auto usec = nsec / 1000;
	auto nsec_100 = (nsec % 1000) / 100 * 100;

	QDate current_date = QDate::currentDate();
	//有b码时间时不适用系统时间
	if (0 != days)
	{
		if (current_date.setDate(year, 1, 1))
		{
			current_date = current_date.addDays(days - 1);
		}
	}

	return QString("%1-%2-%3 %4:%5:%6.%7.%8").arg(current_date.year(), 4, 10, QChar('0')).arg(current_date.month(), 2, 10, QChar('0')).arg(current_date.day(), 2, 10, QChar('0'))
		.arg(hour, 2, 10, QChar('0')).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(usec, 6, 10, QChar('0')).arg(nsec_100, 3, 10, QChar('0'));
}