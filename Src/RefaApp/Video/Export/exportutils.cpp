#include "exportutils.h"

#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStaticText>

#include "HscAPIHeader.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "opencv2/opencv.hpp"
#include "HscExportHeader.h"


QString ExportUtils::getExportVideoPath(const VideoItem &video_item)
{
    // 名称
    QString name = video_item.getName();

    // 导出时间
    QDateTime current_date_time = QDateTime::currentDateTime();

    QString filename = QString("%1-%2").arg(name).arg(current_date_time.toString("yyyyMMdd-HHmmss"));

    switch(video_item.getVideoFormat())
    {
    case VideoFormat::VIDEO_RHVD:
        filename = filename + ".rhvd";
        break;
    case VideoFormat::VIDEO_AVI:
        filename = filename + ".avi";
        break;
    case VideoFormat::VIDEO_MP4:
        filename = filename + ".mp4";
        break;
    case VideoFormat::VIDEO_BMP:
        filename = "BMP" + filename;
        break;
    case VideoFormat::VIDEO_JPG:
        filename = "JPG" + filename;
        break;
    case VideoFormat::VIDEO_TIF:
        filename = "TIF" + filename;
        break;
	case VideoFormat::VIDEO_PNG:
		filename = "PNG" + filename;
		break;
    }

    QString video_path;
    if (!filename.isEmpty())
    {
        video_path = video_item.getExportPath() + QDir::separator() + filename;
    }

    return video_path;
}

#include "Video/VideoUtils/videoutils.h"
QString ExportUtils::getExportVideoPathSelfDef(VideoItem &video_item)
{
	QString filename = getFilePath(video_item);

	switch (video_item.getVideoFormat())
	{
	case VideoFormat::VIDEO_RHVD:
		filename = filename + ".rhvd";
		break;
	case VideoFormat::VIDEO_AVI:
		filename = filename + ".avi";
		break;
	case VideoFormat::VIDEO_MP4:
		filename = filename + ".mp4";
		break;
// 	case VideoFormat::VIDEO_BMP:
// 		filename = "BMP" + filename;
// 		break;
// 	case VideoFormat::VIDEO_JPG:
// 		filename = "JPG" + filename;
// 		break;
// 	case VideoFormat::VIDEO_TIF:
// 		filename = "TIF" + filename;
// 		break;
	}

	QString strAddExperiment;
// 	QString strPathCurrentExperiment = SystemSettingsManager::instance().getCurrentExperiment().code;
// 	if (!strPathCurrentExperiment.isEmpty())
// 	{
// 		QString CurrentExperiment_path = video_item.getExportPath() + QDir::separator() + strPathCurrentExperiment;
// 		QDir dir(CurrentExperiment_path);
// 		if (dir.exists())
// 		{
// 			strAddExperiment = QDir::separator() + strPathCurrentExperiment;
// 		}
// 	}

	QString video_path;
	if (filename != "")
	{
		video_path = video_item.getExportPath() + strAddExperiment + QDir::separator() + filename;
	}
	else
	{
		video_path = video_item.getExportPath() + strAddExperiment;
	}

	return video_path;
}

QString ExportUtils::getFilePath(VideoItem &video_item)
{
	QString device_ip = VideoUtils::parseDeviceIp(video_item.getId());
	QString frame_rate = video_item.getFrameRate();
	QString record_time = video_item.getTimeStamp();
	QDateTime time = QDateTime::fromString(record_time, "yyyy-MM-dd hh:mm:ss");
	record_time = time.toString("yyyyMMdd-HHmmss");
	auto format = video_item.getVideoFormat();
	QString resolution_ratio = video_item.getResolution();
	QString record_date = video_item.getRecordDate();
	QString format_str = formatToStr(format);

	QString re_timestamp = VIDEONAME_TIMESTAMP;
	QString re_frame_no = VIDEONAME_FRAME_NO;
	QString re_sub_frame_no = VIDEONAME_FRAME_SUB_NO;

	QString name = video_item.getRuleName();
	QString des_name;
	bool is_image = false;
	switch (format)
	{
	case VideoFormat::VIDEO_RHVD:
	case VideoFormat::VIDEO_AVI:
	case VideoFormat::VIDEO_MP4:
		re_timestamp = record_time;
// 		re_frame_no = "0";
// 		re_sub_frame_no = "0";
		break;

	case VideoFormat::VIDEO_BMP:
	case VideoFormat::VIDEO_JPG:
	case VideoFormat::VIDEO_TIF:
	case VideoFormat::VIDEO_PNG:
// 		QDateTime current_date_time = QDateTime::currentDateTime();
// 		QString crt_time = current_date_time.toString("yyyyMMdd-HHmmss");
// 		des_name = QString("%1-%2-%3").arg(device_ip).arg(record_time).arg(crt_time);
 		is_image = true;
		break;
	}

	replaceVideoName(name, VIDEONAME_IP, device_ip);
	replaceVideoName(name, VIDEONAME_FRAME_RATE, frame_rate);
	replaceVideoName(name, VIDEONAME_RECORD_TIME, record_time);
	replaceVideoName(name, VIDEONAME_FORMAT, format_str);
	replaceVideoName(name, VIDEONAME_RESOLUTION_RATIO, resolution_ratio);
	replaceVideoName(name, VIDEONAME_RECORD_DATE, record_date);
	replaceVideoName(name, VIDEONAME_TIMESTAMP, re_timestamp);
	replaceVideoName(name, VIDEONAME_FRAME_NO, re_frame_no);
	replaceVideoName(name, VIDEONAME_FRAME_SUB_NO, re_sub_frame_no);
	replaceVideoName(name, VIDEONAME_VIDEO_NAME, video_item.getName());

	QString strDir = name;

	switch (format)
	{
	case VideoFormat::VIDEO_BMP:
	case VideoFormat::VIDEO_JPG:
	case VideoFormat::VIDEO_TIF:
	case VideoFormat::VIDEO_PNG:
	{
		strDir.clear();
		QStringList vctList = name.split("/");
		int nCountDir = vctList.size();
		if (nCountDir > 1)
		{
			for (int i = 0; i < nCountDir - 1; i++)
			{
				if (i != 0)
				{
					strDir += QDir::separator();
				}
				strDir += vctList[i];
			}
			name = vctList[nCountDir - 1];
		}
		else
		{
			strDir = record_time;
		}
		break;
	}
	default:
		break;
	}
	if (is_image)
	{
		if (name.indexOf(re_timestamp, 0, Qt::CaseInsensitive) < 0 && name.indexOf(re_frame_no, 0, Qt::CaseInsensitive) < 0)
		{
			name += "_";
			name += re_frame_no;
		}
	}
	video_item.setRuleName(name);
	if (des_name != "") return des_name;
	return strDir;
}

QString ExportUtils::formatToStr(int format)
{
	switch (format)
	{
	case VideoFormat::VIDEO_RHVD:
		return "rhvd";
	case VideoFormat::VIDEO_AVI:
		return "avi";
	case VideoFormat::VIDEO_MP4:
		return "mp4";
	case VideoFormat::VIDEO_BMP:
		return "bmp";
	case VideoFormat::VIDEO_JPG:
		return "jpg";
	case VideoFormat::VIDEO_TIF:
		return "tif";
	case VideoFormat::VIDEO_PNG:
		return "png";
	default:
		return "";
	}
}

QString ExportUtils::getSnapshotPath(const QString & ip)
{
	// [workpath]/[ip]/[ip]-[datetime].bmp
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString work_path = SystemSettingsManager::instance().getWorkingDirectory();
	return QString("%1/%2/%2-%3.bmp").arg(work_path).arg(ip).arg(current_date_time.toString("yyyyMMdd-HHmmss"));
}

QString ExportUtils::getSnapshotPath(const VideoItem & video_item)
{
	// [workpath]/[videoname]/[videoname]-[datetime].bmp
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString work_path = SystemSettingsManager::instance().getWorkingDirectory();
	return QString("%1/%2/%2-%3.bmp").arg(work_path).arg(video_item.getName()).arg(current_date_time.toString("yyyyMMdd-HHmmss"));
}


void ExportUtils::paintWatermark2Image(QImage & image, const QString & text, const int padding /*= 5*/, const WatermarkPosition position /*= WATERMARK_TOPRIGHT*/)
{
	if (image.isNull() || text.isEmpty())
		return;

	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
	QFont font = painter.font();
	font.setWeight(QFont::Thin);
	font.setPointSize(12);
  	font.setFamily("MS UI Gothic");
	painter.setFont(font);



	QRect buffer_rc(image.rect());
	QStaticText static_text(text);
	QTextOption text_option = static_text.textOption();
	text_option.setWrapMode(QTextOption::WrapMode::NoWrap);//取消自动换行
	static_text.setTextOption(text_option);
	static_text.prepare(painter.transform(), painter.font());
	QSizeF static_text_rc = static_text.size();

	QPoint start_pt(buffer_rc.right() - static_text_rc.width() - padding, buffer_rc.y() + padding);
	switch (position)
	{
	case WATERMARK_TOPLEFT:
		start_pt = QPoint(buffer_rc.x() + padding, buffer_rc.y() + padding);
		break;
	case WATERMARK_BOTTOMLEFT:
		start_pt = QPoint(buffer_rc.x() + padding, buffer_rc.bottom() - static_text_rc.height() - padding);
		break;
	case WATERMARK_BOTTOMRIGHT:
		start_pt = QPoint(buffer_rc.right() - static_text_rc.width() - padding, buffer_rc.bottom() - static_text_rc.height() - padding);
		break;
	default:
		break;
	}

	QPoint start_pt_shadow = QPoint(start_pt.x() + 1, start_pt.y() + 1);
	painter.setPen(Qt::black);
	painter.drawStaticText(start_pt_shadow, static_text);
	painter.setPen(Qt::white);
	painter.drawStaticText(start_pt, static_text);
}

void ExportUtils::replaceVideoName(QString &source_name, const QString&str, const QString &replace_data)
{
	source_name.replace(str, replace_data, Qt::CaseInsensitive);
}