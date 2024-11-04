#ifndef EXPORTUTILS_H
#define EXPORTUTILS_H


#include "Video/VideoItem/videoitem.h"

#include <QImage>

/**
 * @brief 导出工具类
 */
class ExportUtils
{
public:


    /**
     * @brief 获取视频导出路径
     * @param video_item 视频项
     * @return
     */
    static QString getExportVideoPath(const VideoItem & video_item);

	/**
	 * @brief 获取快照路径
	 * @param ip 设备IP
	 * @return 快照路径
	 */
	static QString getSnapshotPath(const QString & ip);

	/**
	 * @brief 获取快照路径
	 * @param video_item 视频项
	 * @return 快照路径
	 */
	static QString getSnapshotPath(const VideoItem & video_item);



	//水印位置
	enum WatermarkPosition
	{
		WATERMARK_TOPLEFT,			// 水印在左上角
		WATERMARK_TOPRIGHT,			// 水印在右上角
		WATERMARK_BOTTOMLEFT,		// 水印在左下角  
		WATERMARK_BOTTOMRIGHT,		// 水印在右下角
	};

	/**
	*@brief 绘制水印至QImage
	*@param image 输入/输出图像
	*@param text 水印文字
	*@param padding 边界
	*@param position 水印位置
	*@return
	**/
	static void paintWatermark2Image(QImage & image, const QString & text, const int padding = 5, const WatermarkPosition position = WATERMARK_TOPRIGHT);

	static QString getExportVideoPathSelfDef(VideoItem &video_item);
	static void replaceVideoName(QString &source_name, const QString&str, const QString &replace_data);
	static QString getFilePath(VideoItem &video_item);
	static QString formatToStr(int format);
};

#endif // EXPORTUTILS_H
