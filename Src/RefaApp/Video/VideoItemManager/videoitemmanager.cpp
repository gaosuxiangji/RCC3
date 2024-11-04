#include "videoitemmanager.h"

#include <QMutexLocker>

#include "Video/VideoUtils/videoutils.h"

VideoItemManager &VideoItemManager::instance()
{
    static VideoItemManager inst;
    return inst;
}

QList<VideoItem> VideoItemManager::findVideoItems(int video_type, const QString & ip) 
{
    QMutexLocker locker(&video_item_mutex_);

    if (video_type == VideoItem::Local)
    {
        return local_video_items_;
    }

    QList<VideoItem> video_items;
    for (auto iter = video_item_map_.begin(); iter != video_item_map_.end(); iter++)
    {
        VideoItem video_item = iter.value();
        if (video_item.getType() != video_type)
        {
            continue;
        }

        if (!ip.isEmpty())
        {
            QString video_item_ip = VideoUtils::parseDeviceIp(video_item.getId());
            if (video_item_ip != ip)
            {
                continue;
            }
        }

        video_items.append(video_item);
    }

	sortVideoItems(video_items);

    return video_items;
}

bool videoItemLessThan(const VideoItem& item_l, const VideoItem& item_r)
{
	int id_l = 0;
	int id_r = 0;
	id_l = VideoUtils::parseVideoSegmentId(item_l.getId());
	id_r = VideoUtils::parseVideoSegmentId(item_r.getId());
	if (id_l < id_r)
	{
		return true;
	}
	else
	{
		return false;
	}
	return true;
}


void VideoItemManager::sortVideoItems(QList<VideoItem> & items)
{
	qSort(items.begin(), items.end(), videoItemLessThan);
}

void VideoItemManager::addVideoItem(const VideoItem &video_item)
{
    QMutexLocker locker(&video_item_mutex_);

    if (video_item.isValid())
    {
        video_item_map_[video_item.getId()] = video_item;

        if (video_item.getType() == VideoItem::Local)
        {
            local_video_items_.push_front(video_item);
        }
    }
}

void VideoItemManager::setVideoItem(const QVariant &video_id, const VideoItem &video_item)
{
    QMutexLocker locker(&video_item_mutex_);

    if ((video_item.getId() == video_id) && (video_item_map_.contains(video_id)))
    {
        video_item_map_[video_id] = video_item;

        if (video_item.getType() == VideoItem::Local)
        {
            for (int i = 0; i < local_video_items_.size(); i++)
            {
                if (local_video_items_[i].getId() == video_id)
                {
                    local_video_items_[i] = video_item;
                    break;
                }
            }
        }
    }
}

void VideoItemManager::removeVideoItem(const QVariant &video_id)
{
    QMutexLocker locker(&video_item_mutex_);

    video_item_map_.remove(video_id);

    for (int i = 0; i < local_video_items_.size(); i++)
    {
        if (local_video_items_[i].getId() == video_id)
        {
            local_video_items_.removeAt(i);
            break;
        }
    }
}

VideoItem VideoItemManager::getVideoItem(const QVariant &video_id) const
{
    QMutexLocker locker(&video_item_mutex_);

    return video_item_map_.value(video_id);
}

bool VideoItemManager::containsVideoItem(const QVariant &video_id) const
{
    QMutexLocker locker(&video_item_mutex_);

    return video_item_map_.contains(video_id);
}

VideoItemManager::VideoItemManager()
{

}

