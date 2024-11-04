#ifndef EXPORT_CACHE_FRAME_
#define EXPORTL_CACHE_FRAME_

#include <unordered_map>
#include <memory>
#include "Device/device.h"
#include "Video/VideoItem/videoitem.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/mat.hpp"

class ExportCacheFrame
{
	enum 
	{
		eRecordingTimeOutMs = 2000,
		eGetFrameTimeOutMs = 500
	};
	
	struct WaterMarkInfo
	{
		int x = 0;
		int y = 0;
		double alpha = 1.0;
		double beta = 1.0;
		double gamma = 0;
		QImage image_watermark;
	};

private:
	struct ExportInfo
	{
		int m_video_id = -1;
		int64_t m_start_frame_no = 0;
		int64_t m_end_frame_no = 0;
		int64_t m_count = 0;
		int64_t m_interval = 0;


		ExportInfo(int video_id, int64_t start_frame_no, int64_t end_frame_no, int64_t count, int64_t interval)
			:
			m_video_id(video_id),
			m_start_frame_no(start_frame_no),
			m_end_frame_no(end_frame_no),
			m_count(count),
			m_interval(interval)
		{
		}

	};

private:
	QSharedPointer<Device> device_ptr_;
	std::unordered_map<int64_t, std::shared_ptr<CAGBuffer>> m_cache_frame_map;

	bool												m_running = false;
	std::thread											m_export_thrd;
	mutable std::mutex									m_export_vec_mtx;
	mutable std::mutex									m_export_map_mtx;
	//std::condition_variable								m_condvar;

	int													m_crtidx = 0;
	std::vector<ExportInfo>								m_export_info_vec;
	std::unordered_map<int, int>						m_videoid_to_crtidx;

	std::unordered_map<int, WaterMarkInfo>				m_watermark_map;
	std::atomic<bool>									m_exporting{ false };


public:
	ExportCacheFrame();
	~ExportCacheFrame();
	ExportCacheFrame(QSharedPointer<Device> device_ptr, int video_num);

	bool doExportProcess();

	bool doExport(int video_id, int64_t start_frame_no, int64_t end_frame_no, int64_t count, int64_t interval);

	void doExportAsync(int video_id, int64_t start_frame_no, int64_t end_frame_no, int64_t count, int64_t interval);
	
	bool getFrame(int64_t vid, VideoItem &item, cv::Mat &mat);

	void adjustLuminanceAndContrast(CAGBuffer *buffer, const int luminance, const int contrast, const bool anti_color, cv::Mat &mat);

	//bool addWaterMark(char *src_data, int water_mark_type);

	void paintDefWatermark(cv::Mat &mat, QString &timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, const VideoItem &item);

	QImage getDefWaterMarkImage(QString &timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, VideoItem &video_itme);
	void paintSelfDefWatermark(int video_id,cv::Mat &mat);

	void updateWaterMark(int video_id, QImage &image);
	void deleteWaterMark(int video_id);
	bool hasWaterMark(int video_id);

	bool stop();

private:
	bool getFrame(int64_t video_id, CAGBuffer *&pBuffer);

	QString getDefWaterMarkText(QString &timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, const VideoItem &video_itme);
    void convertTextToImage(const QString &text, QImage &image);
	bool    addWaterMark(int x, int y, double alpha, double beta, double gamma, cv::Mat &dst_mat, QImage &image);
};


#endif
