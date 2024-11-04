#ifndef VIDEOITEMMANAGER_H
#define VIDEOITEMMANAGER_H

#include <QMap>
#include <QList>
#include <QMutex>

#include "Video/VideoItem/videoitem.h"

/**
 * @brief 视频项管理器类
 */
class VideoItemManager
{
public:
    static VideoItemManager & instance();

    /**
     * @brief 查找视频项
     * @param video_type 视频类型
     * @param ip 设备IP，视频类型为Remote时有效
     * @return 视频项列表,id由低到高排序
     */
    QList<VideoItem> findVideoItems(int video_type, const QString & ip = QString()) ;

	/**
	* @brief 对视频项排序
	* @param [in][out]items 视频列表
	* @note id由低到高排序
	*/
	void sortVideoItems(QList<VideoItem> & items);

    /**
     * @brief 添加视频项
     * @param video_item 视频项
     */
    void addVideoItem(const VideoItem & video_item);

    /**
     * @brief 设置视频项
     * @param video_id 视频ID
     * @param video_item 视频项
     */
    void setVideoItem(const QVariant & video_id, const VideoItem & video_item);

    /**
     * @brief 删除视频项
     * @param video_id
     */
    void removeVideoItem(const QVariant & video_id);

    /**
     * @brief 获取视频项
     * @param video_id 视频ID
     * @return 视频项
     */
    VideoItem getVideoItem(const QVariant & video_id) const;

    /**
     * @brief 视频项是否已存在
     * @param video_id 视频ID
     * @return
     */
    bool containsVideoItem(const QVariant & video_id) const;

private:
    VideoItemManager();

// 	bool videoItemLessThan(const VideoItem& item_l, const VideoItem& item_r);

private:
    QList<VideoItem> local_video_items_;
    QMap<QVariant, VideoItem> video_item_map_;
    mutable QMutex video_item_mutex_;
};

#endif // VIDEOITEMMANAGER_H
