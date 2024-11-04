#include "videoutils.h"

#include <QHash>
#include <QStringList>
#include <QRect>
#include <QDateTime>
#include <cmath>

static const QChar kDeviceVideoIdSeparator('|');
static const QChar kVideoNameSeparator('-');
static const QString kVideoTime2StringFormat("yyyyMMdd-hhmmss");

QVariant VideoUtils::packDeviceVideoId(const QString &ip, int id)
{
    return QString("%1%2%3").arg(ip).arg(kDeviceVideoIdSeparator).arg(id);
}

QString VideoUtils::parseDeviceIp(const QVariant &device_video_id)
{
    QStringList splits = device_video_id.toString().split(kDeviceVideoIdSeparator);
    if (splits.size() == 2)
    {
        return splits.at(0);
    }

    return QString();
}

int VideoUtils::parseVideoSegmentId(const QVariant &device_video_id)
{
    QStringList splits = device_video_id.toString().split(kDeviceVideoIdSeparator);
    if (splits.size() == 2)
    {
        return splits.at(1).toInt();
    }

    return -1;
}

//向上对齐
#define ALIGNUP(size,alignSize)    ((size)%(alignSize)==0?(size):(size+alignSize-(size)%(alignSize)))
void VideoUtils::correctRoi(QRect & roi, const QRect & video_rect, uint32_t width_inc/* = 4*/, uint32_t height_inc/* = 4*/)
{
	if (width_inc != 0)
	{
		roi.setWidth(ALIGNUP(roi.width(), width_inc));
	}
	if (height_inc != 0)
	{
		roi.setHeight(ALIGNUP(roi.height(), height_inc));
	}

	if (roi.x() < video_rect.x())
	{
		roi.setX(video_rect.x());
	}

	if (roi.y() < video_rect.y())
	{
		roi.setY(video_rect.y());
	}

	if (roi.right() > video_rect.right())
	{
		roi.setX(video_rect.right() - roi.width());
	}

	if (roi.bottom() > video_rect.bottom())
	{
		roi.setY(video_rect.bottom() - roi.height());
	}

	if (!video_rect.contains(roi))
	{
		roi = video_rect;
	}
}

QVariant VideoUtils::getVideoName(const QString & ip, const quint64 & video_time_ms)
{
	QDateTime time = QDateTime::fromMSecsSinceEpoch(video_time_ms);
	return QString("%1%2%3").arg(ip).arg(kVideoNameSeparator).arg(time.toString(kVideoTime2StringFormat));
}

quint64 VideoUtils::parseVideoTimestampFromVideoName(const QVariant & video_name)
{
	QString time_str = video_name.toString();
	QStringList splits = time_str.split(kVideoNameSeparator);
	if (splits.size() < 3)
		return 0;
	time_str = time_str.remove(0, splits.at(0).size() + 1);
	QDateTime date_time = QDateTime::fromString(time_str, kVideoTime2StringFormat);
	return date_time.toMSecsSinceEpoch();
}

int64_t VideoUtils::msToFrameId(const double ms, const int64_t fps)
{
	return (int64_t)((ms * fps) / 1000);
}

int64_t VideoUtils::msToFrameIdOfRound(const double ms, const int64_t fps)
{
	return (int64_t)(std::round((ms * fps) / 1000));
}

double VideoUtils::frameIdToMs(const int64_t id, const int64_t fps)
{
	//TODO:临时规避帧率为0时会崩溃的问题，还需万钟霖确认方案后再修改
	int64_t tempFps = (0 == fps) ? 1 : fps;
	return (double)((id * 1000*1.0) / tempFps);
}

QString VideoUtils::parseVideoTimestampStrFromVideoName(const QVariant & video_name)
{
	QString time_str = video_name.toString();
	QStringList splits = time_str.split(kVideoNameSeparator);
	if (splits.size() < 3)
		return 0;
	time_str = time_str.remove(0, splits.at(0).size() + 1);
	return time_str;
}
