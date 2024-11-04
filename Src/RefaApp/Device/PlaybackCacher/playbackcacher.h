#ifndef PLAYBACKCACHER_H
#define PLAYBACKCACHER_H

#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

#include <QSharedPointer>

#include "playbackcachertypes.h"

#include "opencv2/opencv.hpp"

#include "ThreadPool/ThreadPool.h"
#include "cache/FIFOCache.h"
#include "RMAImage.h"
#include "playbackcachertypes.h"


class bufferArray;
struct HscVideoClipInfo;
typedef struct _CAGBuffer CAGBuffer;
class CAgCameraInterface;
enum DeviceModel;
class Device;
class ImageProcessor;
class PlaybackCacher
{
public:
    PlaybackCacher(QSharedPointer<Device> pDevice, int video_segment_id, bool auto_cycle = true, int max_retry_count = 0);
	~PlaybackCacher();
    bool start(const PlaybackCacherParams & params);
	bool start(int64_t start_frame_no, int64_t end_frame_no, int64_t frame_interval, int64_t current_frame_no);
	void stop();

	bool getFrame(int64_t frame_no, RMAImage & rma_image, bool trigger_cache = true);
	
	/**
	*@brief 调整图像亮度对比度
	*@param [in/out] : rma_image，图像
				   luminance : const int，亮度
				   const int，对比度
	**/
	void adjustLuminanceAndContrast(RMAImage & rma_image, const int luminance, const int contrast);

private:
	int64_t calcCacheEndFrameNo(int64_t cache_start_frame_no, int64_t interval);
	void doExport();
    void doImageProcess(std::shared_ptr<ImageProcessor> pBufferProcessor, cv::Mat buffer_mat);
	void ExportFrame(int64_t start_frame_no, int64_t end_frame_no, int64_t interval);
private:
    QSharedPointer<Device> device_ptr_;
	int64_t video_segment_id_;
	std::unique_ptr<std::thread> thread_ptr_;
	std::atomic_bool in_running_ = false;
	std::atomic_int64_t frame_interval_ = -1;
	std::atomic_int64_t cache_start_frame_no_ = -1;
	std::atomic_int64_t cache_end_frame_no_ = -1;
	std::atomic_int64_t play_frame_no_ = 0;
	std::atomic_bool triggered_ = false;

	bool auto_cycle_ = true;

	int64_t start_frame_no_ = -1;
	int64_t end_frame_no_ = -1;

	mutable std::mutex mtx_;
	std::condition_variable cv_;

	agile_device::FIFOCache<int64_t, RMAImage> frame_map_;

	agile_device::ThreadPool image_process_executor_{ 2 };

	int max_retry_count_{ 0 };
	const int kTimeoutByMs = 5000;

	std::unique_ptr<CAGBuffer> buffer_ptr_;

	const int kDefaultFrameCountPerExport = 50;
	int max_frame_count_per_export_{ kDefaultFrameCountPerExport };
};

#endif // PLAYBACKCACHER_H
