#ifndef VIDEOUTILS_H
#define VIDEOUTILS_H

#include <QVariant>

/**
 * @brief 视频工具类
 */
class VideoUtils
{
public:
    /**
     * @brief 打包设备视频ID
     * @param ip 设备IP
     * @param id 视频片段ID
     * @return 视频ID
     */
    static QVariant packDeviceVideoId(const QString & ip, int id);

    /**
     * @brief 从设备视频ID中解析设备IP
     * @param device_video_id 设备视频ID
     * @return
     */
    static QString parseDeviceIp(const QVariant & device_video_id);

    /**
     * @brief 从设备视频ID中解析视频片段ID
     * @param device_video_id 设备视频ID
     * @return 视频片段ID
     */
    static int parseVideoSegmentId(const QVariant & device_video_id);

	/**
	*@brief 校正roi区域
	*@param roi 输入/输出roi
	*@param video_rect 视频图像大小
	*@param width_inc 宽度增量，默认为4
	*@param height_inc 高度增量，默认为4
	**/
	static void correctRoi(QRect & roi, const QRect & video_rect, uint32_t width_inc = 4, uint32_t height_inc = 4);

	/**
	* @brief 获取视频名称
	* @param ip 设备IP
	* @param video_time_ms 视频采集时间，单位：ms
	* @return 视频名称
	*/
	static QVariant getVideoName(const QString & ip, const quint64 & video_time_ms);

	/**
	*@brief 从视频名称解析视频采集时间
	*@param video_name 视频名称
	*@return 视频采集时间
	**/
	static quint64 parseVideoTimestampFromVideoName(const QVariant & video_name);

	/**
	* @brief 通过fps将毫秒转换为帧号
	* @param ms 毫秒数据
	* @param fps 拍摄速度
	* @return 帧号
	*/
	static int64_t msToFrameId(const double ms, const int64_t fps);
	static int64_t msToFrameIdOfRound(const double ms, const int64_t fps);

	/**
	* @brief 通过fps将帧号转换为毫秒
	* @param id 帧号
	* @param fps 拍摄速度
	* @return 毫秒数据
	*/
	static double frameIdToMs(const int64_t id, const int64_t fps);

	static QString parseVideoTimestampStrFromVideoName(const QVariant & video_name);
};

#endif // VIDEOUTILS_H
