#include "videoitem.h"

#include <QMutexLocker>
#include <QDateTime>

VideoItem::VideoItem()
{

}

VideoItem::VideoItem(const VideoItem &other) : id_(other.id_),
    type_(other.type_),
    feature_item_map_(other.feature_item_map_),
    property_map_(other.property_map_)
{

}

VideoItem::VideoItem(const QVariant &id, int type) : id_(id), type_(type)
{

}

bool VideoItem::isValid() const
{
    return id_.isValid();
}

QVariant VideoItem::getId() const
{
    return id_;
}

void VideoItem::setType(int type)
{
    type_ = type;
}

int VideoItem::getType() const
{
    return type_;
}

void VideoItem::setName(const QString &name)
{
    setProperty(PropType::Name, name);
}

QString VideoItem::getName() const
{
    return getProperty(PropType::Name).toString();
}

void VideoItem::setRuleName(const QString &name)
{
    setProperty(PropType::RuleName, name);
}

QString VideoItem::getRuleName() const
{
    return getProperty(PropType::RuleName).toString();
}

QString VideoItem::getTimeStamp() const
{
	uint64_t time_stamp = getProperty(VideoItem::TimeStamp).toLongLong();

	QDateTime q_time = QDateTime::fromTime_t(time_stamp);
	QString strTime = q_time.toString("yyyy-MM-dd hh:mm:ss");
	//strTime += QString(":%1").arg(time_stamp % 1000000, 6, 10, QLatin1Char('0'));
	return  strTime;
}

QString VideoItem::getRecordDate() const
{
	uint64_t time_stamp = getProperty(VideoItem::TimeStamp).toLongLong();

	QDateTime q_time = QDateTime::fromTime_t(time_stamp);
	QString strTime = q_time.toString("yyyy-MM-dd");
	//strTime += QString(":%1").arg(time_stamp % 1000000, 6, 10, QLatin1Char('0'));
	return  strTime;
}

QString VideoItem::getOriginalTimeStamp() const
{
	uint64_t time_stamp = getProperty(VideoItem::TimeStamp).toLongLong();
	QDateTime q_time = QDateTime::fromTime_t(time_stamp);
	QString strTime_t = q_time.toString("yyyy-MM-dd hhmmss");
	uint64_t dMicrosecond = ((double)rand()/RAND_MAX)*(599999-100000)+100000;
	QString strTime = QString("%1.%2").arg(strTime_t).arg(dMicrosecond);
	return strTime;
}

QString VideoItem::getTimeLength() const
{
	double time_length = (getEndFrameIndex() - getBeginFrameIndex()+1)*(1.0) / getFPS() * 1000;    //时长 ms
	return QString::number(time_length, 'f', 1);
}

QString VideoItem::getFrameRate() const
{
	return QString::number(getFPS());
}

QString VideoItem::getResolution() const
{
	QRect rect = getRoi();
	return (QString::number(rect.width()) + QString("x") + QString::number(rect.height()));
}

QString VideoItem::getTotalFrame() const
{
	return QString::number(getEndFrameIndex()-getBeginFrameIndex()+1);
}

void VideoItem::setRoi(const QRect &roi)
{
    setProperty(PropType::Roi, roi);
}

QRect VideoItem::getRoi() const
{
    return getProperty(PropType::Roi).toRect();
}

void VideoItem::setRotationType(int type)
{
	setProperty(PropType::RotationType, type);
}

int VideoItem::getRotationType() const
{
	return getProperty(PropType::RotationType, 0).toInt();
}

void VideoItem::setFocusPoint(const QVariant &point)
{
    setProperty(PropType::FocusPoint, point);
}

QVariant VideoItem::getFocusPoint() const
{
    return getProperty(PropType::FocusPoint);
}

void VideoItem::setLuminance(int luminance)
{
    setProperty(PropType::Luminance, luminance);
}

int VideoItem::getLuminance() const
{
    return getProperty(PropType::Luminance, 50).toInt();
}

void VideoItem::setContrast(int contrast)
{
    setProperty(PropType::Contrast, contrast);
}

int VideoItem::getContrast() const
{
    return getProperty(PropType::Contrast, 50).toInt();
}

void VideoItem::setAntiColorEnable(bool enable)
{
	setProperty(PropType::AntiColorEnable, enable);
}

bool VideoItem::isAntiColorEnable() const
{
	return getProperty(PropType::AntiColorEnable, true).toBool();
}

void VideoItem::setOsdVisible(int8_t visible)
{
    setProperty(PropType::OsdVisible, visible);
}

bool VideoItem::isOsdVisible() const
{
    return getProperty(PropType::OsdVisible, false).toBool();
}

void VideoItem::setStreamType(int type)
{
	setProperty(PropType::StreamType, type);
}

int VideoItem::getStreamType() const
{
	return getProperty(PropType::StreamType, 0).toInt();
}

void VideoItem::setVideoFormat(int format)
{
    setProperty(PropType::VideoFormat, format);
}

int VideoItem::getVideoFormat() const
{
    return getProperty(PropType::VideoFormat, 0).toInt();
}

void VideoItem::setDisplayMode(int mode)
{
	setProperty(PropType::DisplayMode, mode);
}

int VideoItem::getDisplayMode() const
{
	return getProperty(PropType::DisplayMode, 1).toInt();
}

void VideoItem::setExportPath(const QString &path)
{
    setProperty(PropType::ExportPath, path);
}

QString VideoItem::getExportPath() const
{
    return getProperty(PropType::ExportPath).toString();
}

void VideoItem::setVideoFrameCount(qint64 count)
{
	setProperty(PropType::VideoFrameCount, count);
}

qint64 VideoItem::getVideoFrameCount() const
{
	return getProperty(PropType::VideoFrameCount).toLongLong();
}

void VideoItem::setBeginFrameIndex(qint64 index)
{
    setProperty(PropType::BeginFrameIndex, index);
}

qint64 VideoItem::getBeginFrameIndex() const
{
    return getProperty(PropType::BeginFrameIndex).toLongLong();
}

void VideoItem::setEndFrameIndex(qint64 index)
{
    setProperty(PropType::EndFrameIndex, index);
}

qint64 VideoItem::getEndFrameIndex() const
{
    return getProperty(PropType::EndFrameIndex).toLongLong();
}

void VideoItem::setKeyFrameIndex(qint64 index)
{
	if (index == -1)
	{
		index = 0;
	}
	setProperty(PropType::KeyFrameIndex, index);
}

qint64 VideoItem::getKeyFrameIndex() const
{
	return getProperty(PropType::KeyFrameIndex,0).toLongLong();
}

void VideoItem::setExportFrameRate(qint64 frame_rate)
{
	if (frame_rate <= 0) frame_rate = 1;
	if (frame_rate > 1000) frame_rate = 1000;

	setProperty(PropType::ExportFrameRate, frame_rate);
}

qint64 VideoItem::getExportFrameRate() const
{
	return getProperty(PropType::ExportFrameRate, 25).toLongLong();
}

void VideoItem::setFrameStep(qint64 step)
{
    setProperty(PropType::FrameStep, step);
}

qint64 VideoItem::getFrameStep() const
{
    return getProperty(PropType::FrameStep, 0).toLongLong();
}

void VideoItem::setValidBitsPerPixel(qint8 bpp)
{
	setProperty(PropType::ValidBitsPerPix, bpp);
}

qint8 VideoItem::getValidBitsPerPixel() const
{
	return getProperty(PropType::ValidBitsPerPix, 8).toInt();
}

void VideoItem::setFPS(qint64 fps)
{
	setProperty(PropType::FPS, fps);
}

qint64 VideoItem::getFPS() const
{
	return getProperty(PropType::FPS, 1).toLongLong();
}

void VideoItem::setExposureTime(qint64 time)
{
	setProperty(PropType::ExposureTime, time);
}

qint64 VideoItem::getExposureTime() const
{
	return getProperty(PropType::ExposureTime, 1).toLongLong();
}

void VideoItem::setVideoNameType(int32_t type)
{
	setProperty(PropType::VideoNameType, type);
}

int32_t VideoItem::getVideoNameType() const
{
	return getProperty(PropType::VideoNameType, 0).toInt();
}

void VideoItem::setAVIcompress(bool enable)
{
	setProperty(PropType::AVIcompress, enable);
}

bool VideoItem::getAVIcompress() const
{
	return getProperty(PropType::AVIcompress, false).toBool();
}

void VideoItem::setBinningModeEnable(bool enable)
{
	setProperty(PropType::BinningMode, enable);
}

bool VideoItem::getBinningModeEnable() const
{
	return getProperty(PropType::BinningMode, false).toBool();
}

void VideoItem::setProperty(int type, const QVariant &value)
{
    QMutexLocker locker(&property_mutex_);

    property_map_.insert(type, value);
}

QVariant VideoItem::getProperty(int type, const QVariant & default_value) const
{
    QMutexLocker locker(&property_mutex_);

    return property_map_.value(type, default_value);
}

QList<FeatureItem> VideoItem::getFeatureItems() const
{
    QMutexLocker locker(&feature_item_mutex_);

    return feature_item_map_.values();
}

void VideoItem::addFeatureItem(const FeatureItem &item)
{
    QMutexLocker locker(&feature_item_mutex_);

    if (item.isValid())
    {
        feature_item_map_[item.getId()] = item;
    }
}

FeatureItem VideoItem::getFeatureItem(const QVariant &feature_id) const
{
    QMutexLocker locker(&feature_item_mutex_);

    return feature_item_map_.value(feature_id);
}

void VideoItem::setFeatureItem(const QVariant &feature_id, const FeatureItem &item)
{
    QMutexLocker locker(&feature_item_mutex_);

    if ((item.getId() == feature_id) && feature_item_map_.contains(feature_id))
    {
        feature_item_map_[feature_id] = item;
    }
}

void VideoItem::removeFeatureItem(const QVariant &feature_id)
{
    QMutexLocker locker(&feature_item_mutex_);

    feature_item_map_.remove(feature_id);
}

void VideoItem::clearFeatureItems()
{
    QMutexLocker locker(&feature_item_mutex_);

    feature_item_map_.clear();
}

VideoItem &VideoItem::operator=(const VideoItem &other)
{
    if (this == &other)
    {
        return *this;
    }

    this->id_ = other.id_;
    this->type_ = other.type_;
    this->feature_item_map_ = other.feature_item_map_;
    this->property_map_ = other.property_map_;

    return *this;
}
