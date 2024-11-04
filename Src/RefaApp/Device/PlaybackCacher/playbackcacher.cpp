#include "playbackcacher.h"
#include "HscAPI.h"
#include "Util/HscSystemElapsedTimer.h"
#include "RMAGlobalFunc.h"

#include "Device/device.h"
#include "Device/imageprocessor.h"
#include "Device/deviceutils.h"

PlaybackCacher::PlaybackCacher(QSharedPointer<Device> pDevice, int video_segment_id, bool auto_cycle /*= true*/, int max_retry_count /*= 0*/) :device_ptr_(pDevice), video_segment_id_(video_segment_id), auto_cycle_(auto_cycle), frame_map_(kDefaultFrameCountPerExport * 2), max_retry_count_(max_retry_count)
{
}


PlaybackCacher::~PlaybackCacher()
{
	stop();
}

bool PlaybackCacher::start(const PlaybackCacherParams & params)
{
	return start(params.startFrameNo, params.endFrameNo, params.frameInterval, params.curFrameNo);
}

bool PlaybackCacher::start(int64_t start_frame_no, int64_t end_frame_no, int64_t frame_interval, int64_t current_frame_no)
{
	frame_map_.clear();

	if (device_ptr_ && device_ptr_->getModel() == DEVICE_5KFSeries)
	{
		HscResult res = device_ptr_->startExport(video_segment_id_);
		return (res == HSC_OK);
	}

	stop();

	device_ptr_->startExport(video_segment_id_);

	max_frame_count_per_export_ = kDefaultFrameCountPerExport;
	int framecount = (end_frame_no - start_frame_no) / frame_interval + 1;
	if (framecount > 0)
		max_frame_count_per_export_ = qMin(framecount, max_frame_count_per_export_);
	else
		max_frame_count_per_export_ = max_frame_count_per_export_;

	frame_map_.resize(max_frame_count_per_export_ * 2);

	{
		std::lock_guard<std::mutex> lock(mtx_);//参数修改加锁
		start_frame_no_ = start_frame_no;
		end_frame_no_ = end_frame_no;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           		frame_interval_ = frame_interval;

		cache_start_frame_no_ = current_frame_no < start_frame_no ? start_frame_no : current_frame_no;
		cache_end_frame_no_ = calcCacheEndFrameNo(cache_start_frame_no_, frame_interval_);
	}

	in_running_ = true;
	bool jonable = thread_ptr_ && thread_ptr_->joinable();
	thread_ptr_ = std::make_unique<std::thread>(&PlaybackCacher::doExport, this);

	return true;
}

void PlaybackCacher::stop()
{
	if (device_ptr_ && device_ptr_->getModel() == DEVICE_5KFSeries)
	{
		device_ptr_->stopExport();
		return;
	}

	in_running_ = false;
	cv_.notify_one();
	if (thread_ptr_ != nullptr && thread_ptr_->joinable())
	{
		thread_ptr_->join();
		thread_ptr_.reset();
	}

	if (device_ptr_)
	{
		device_ptr_->stopExport();
	}
}

void PlaybackCacher::doExport()
{
#ifdef _TEST
		_cprintf("cache_start_frame_no:%d,cache_end_frame_no:%d,frame_interval_:%d\n", cache_start_frame_no_, cache_end_frame_no_, frame_interval_);
#endif
	HscResult res(HSC_OK);
	if (!device_ptr_->isExportByIntervalSupported())
		res = device_ptr_->exportVideoClip(video_segment_id_, cache_start_frame_no_, cache_end_frame_no_);
	else
	{
		int64_t framecount = (cache_end_frame_no_ - cache_start_frame_no_) / frame_interval_ + 1;
		res = device_ptr_->exportByInterval(video_segment_id_, cache_start_frame_no_, cache_end_frame_no_, framecount, frame_interval_);
	}
	if (res != HSC_OK)
	{
		return;
	}

	std::shared_ptr<CAGBuffer> pBuffer(new CAGBuffer);

	int64_t cache_start_frame_no = cache_start_frame_no_;
	int64_t cache_end_frame_no = cache_end_frame_no_;
	int64_t cache_frame_no = cache_start_frame_no_;

	HscSystemElapsedTimer elapsed_timer;
	int retry_count = 0;
	std::vector<cv::Mat> retry_caches;

	while (in_running_) {
		bool needsBreak = false;
		bool needsAutoCacheNext = false;
		{
			std::unique_lock<std::mutex> lock(mtx_);

			if (cache_start_frame_no != cache_start_frame_no_ || cache_end_frame_no != cache_end_frame_no_)
			{
				// 中断操作
				cache_start_frame_no = cache_start_frame_no_;
				cache_end_frame_no = cache_end_frame_no_;
				cache_frame_no = cache_start_frame_no_;

				needsBreak = true;
			}
			else
			{
				if ((cache_frame_no > cache_end_frame_no))
				{
					if (max_retry_count_ > 0)
					{
						for (auto & cache : retry_caches)
						{
							image_process_executor_.commit(std::bind(&PlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), cache));
						}
						retry_caches.clear();
					}

					if (image_process_executor_.task_size() > 0) // 前一分段的图像未处理完
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
						continue;
					}

					if (cache_frame_no > end_frame_no_)
					{
						if (!auto_cycle_)
						{
							// 非循环模式，退出
							break;
						}

						cv_.wait(lock, [this] {
							return !in_running_ || ((end_frame_no_ - play_frame_no_) / frame_interval_ + 1 <= max_frame_count_per_export_) || triggered_;
						});
						triggered_ = false;

						if (!in_running_)
						{
							break;
						}

						cache_frame_no = start_frame_no_;
					
					}
					else
					{
						cv_.wait(lock, [this, cache_frame_no] {
							if (!in_running_ || triggered_)
							{
								return true;
							}

							if (cache_frame_no > play_frame_no_)
							{
								return ((cache_frame_no - play_frame_no_) / frame_interval_ + 1 <= max_frame_count_per_export_);
							}
							else
							{
								return ((cache_frame_no + end_frame_no_ - start_frame_no_ + 1 - play_frame_no_) / frame_interval_ <= max_frame_count_per_export_);
							}
						});

						triggered_ = false;
						if (!in_running_)
						{
							break;
						}
					}
					
					cache_start_frame_no = cache_frame_no;
					if (cache_start_frame_no < start_frame_no_)
					{
						cache_start_frame_no = start_frame_no_;
					}
					cache_end_frame_no = calcCacheEndFrameNo(cache_start_frame_no, frame_interval_);

					if (cache_start_frame_no_ != cache_start_frame_no
						|| cache_end_frame_no_ != cache_end_frame_no)
					{
						cache_start_frame_no_ = cache_start_frame_no;
						cache_end_frame_no_ = cache_end_frame_no;
						cache_frame_no = cache_start_frame_no;

						needsAutoCacheNext = true;
					}
					retry_count = 0;
				}
			}
		}

		if (needsBreak)
		{
			image_process_executor_.reset();

			// 中断时，增加重新导出操作
			device_ptr_->stopExport();
			device_ptr_->startExport(video_segment_id_);
		}

		if (needsBreak || needsAutoCacheNext)
		{
			ExportFrame(cache_start_frame_no, cache_end_frame_no, frame_interval_);

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (max_retry_count_ > 0)
			{
				retry_caches.clear();
				elapsed_timer.restart();
			}

			continue;
		}

		if (!device_ptr_->getPlaybackFrame(*pBuffer.get()))
		{
			if (max_retry_count_ > 0 && elapsed_timer.hasExpired(kTimeoutByMs))
			{
				retry_count++;

				if (retry_count <= max_retry_count_)
				{
					ExportFrame(cache_start_frame_no, cache_end_frame_no, frame_interval_);
					cache_frame_no = cache_start_frame_no;
#ifdef _TEST
					_cprintf("retry: %d start:%d end:%d interval:%d\n", retry_count, cache_start_frame_no, cache_end_frame_no, frame_interval_);
#endif
					retry_caches.clear();
					elapsed_timer.restart();
				}
				else
				{
					break;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		if (max_retry_count_ > 0)
		{
			elapsed_timer.restart();
		}

		if (!device_ptr_->isExportByIntervalSupported())
		{
			if ((cache_frame_no - start_frame_no_) % frame_interval_)
			{
				cache_frame_no++;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			pBuffer->frame_head.frameno = cache_frame_no;
			cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get()).clone();
			if (max_retry_count_ > 0)
			{
				retry_caches.push_back(buffer_mat);
			}
			else
			{
				image_process_executor_.commit(std::bind(&PlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), buffer_mat));
			}
			
			cache_frame_no++;
		}
		else
		{
			pBuffer->frame_head.frameno = cache_frame_no;
			cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get()).clone();
			if (max_retry_count_ > 0)
			{
				retry_caches.push_back(buffer_mat);
			}
			else
			{
				image_process_executor_.commit(std::bind(&PlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), buffer_mat));
			}

			
			cache_frame_no += frame_interval_;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	image_process_executor_.reset();
}

void PlaybackCacher::doImageProcess(std::shared_ptr<ImageProcessor> image_processor_ptr, cv::Mat buffer_mat)
{
    if (image_processor_ptr != nullptr && !buffer_mat.empty())
	{
		CAGBuffer *buffer_ptr = (CAGBuffer*)buffer_mat.data;
        cv::Mat image_mat = image_processor_ptr->cv_process(buffer_ptr, 3, true);

		PixelFormat pixel_format = image_mat.channels() == 1 ? PIXEL_FMT_GRAY8 : PIXEL_FMT_BGR24;
		RMAImage rma_image(image_mat.cols, image_mat.rows, pixel_format, image_mat.data);
		rma_image.SetAbsoluteFrameIndex(buffer_ptr->frame_head.frameno);

		// OSD
		QStringList osd;
		QString frame_no = QString(QObject::tr("<font color=#FF0000>%1</font>")).arg(buffer_ptr->frame_head.frameno);
		osd << QString("<font color=#FFFFFF>%1: </font>%2").arg(QObject::tr("Frame No")).arg(frame_no);
		osd << QString("<font color=#FFFFFF>%1: %2</font>").arg(QObject::tr("Timestamp")).arg(DeviceUtils::formatTimestamp(buffer_ptr->frame_head.time_stamp));
		rma_image.SetOSD(osd);

		frame_map_.put(buffer_ptr->frame_head.frameno, rma_image);
	}
}

void PlaybackCacher::ExportFrame(int64_t start_frame_no, int64_t end_frame_no, int64_t interval)
{
	if (!device_ptr_->isExportByIntervalSupported())
	{
		DeviceModel deviceModel = device_ptr_->getModel();
		if (DEVICE_X213 == deviceModel)
			device_ptr_->exportVideoClip(video_segment_id_, start_frame_no, end_frame_no + 1);
		else if (DEVICE_5KFSeries == deviceModel)
		{
			device_ptr_->stopExport();
			device_ptr_->startExport(video_segment_id_);
			device_ptr_->exportVideoClip(video_segment_id_, start_frame_no, end_frame_no);
		}
		else
			device_ptr_->exportVideoClip(video_segment_id_, start_frame_no, end_frame_no);
	}
	else
	{
		int64_t framecount = (end_frame_no - start_frame_no) / frame_interval_ + 1;
		device_ptr_->exportByInterval(video_segment_id_, start_frame_no, end_frame_no, framecount, frame_interval_);
	}
}

bool PlaybackCacher::getFrame(int64_t frame_no, RMAImage & frameInfo, bool trigger_cache)
{
	if (device_ptr_ && device_ptr_->getModel() == DEVICE_5KFSeries)//ML相机流程与其他相机不同，每一帧图像都单独要
	{
		bool ok = false;
		HscResult res = device_ptr_->exportPreview(video_segment_id_, frame_no);
		if (res == HSC_OK)
		{
			if (!buffer_ptr_)
			{
				buffer_ptr_.reset(new CAGBuffer);
			}

            auto pBufferProcessor = device_ptr_->getProcessor();
			if (!pBufferProcessor)
			{
				return false;
			}

			HscSystemElapsedTimer elapsed_timer;
			do
			{
				if (device_ptr_->getPlaybackFrame(*buffer_ptr_))
				{
                    cv::Mat image_mat = pBufferProcessor->cv_process(buffer_ptr_.get(), 3, true);
					PixelFormat pixel_format = image_mat.channels() == 1 ? PIXEL_FMT_GRAY8 : PIXEL_FMT_BGR24;
					frameInfo = RMAImage(buffer_ptr_->frame_head.rect.width, buffer_ptr_->frame_head.rect.height, pixel_format, image_mat.data);

					ok = true;
					break;
				}

				if (elapsed_timer.hasExpired(5000))
				{
					break;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			} while (true);
		}
		return ok;
	}

	if (frame_map_.get(frame_no, frameInfo))
	{
		if (play_frame_no_ != frame_no)
		{
			play_frame_no_ = frame_no;
			cv_.notify_one();
		}
		
		return true;
	}

	if (trigger_cache)
	{
		std::lock_guard<std::mutex> lock(mtx_);
		if (frame_no < cache_start_frame_no_ || frame_no > cache_end_frame_no_)
		{
			cache_start_frame_no_ = frame_no;
			if (cache_start_frame_no_ < start_frame_no_)
			{
				cache_start_frame_no_ = start_frame_no_;
			}
			cache_end_frame_no_ = calcCacheEndFrameNo(cache_start_frame_no_, frame_interval_);

			triggered_ = true;
			cv_.notify_one();

		}
	}

	return false;
}

void PlaybackCacher::adjustLuminanceAndContrast(RMAImage & rma_image, const int luminance, const int contrast)
{
	auto processor_ptr = device_ptr_->getProcessor();
	if (processor_ptr)
	{
		cv::Mat mat;
		RMAIMAGE2CVMAT(rma_image, mat);

		mat = processor_ptr->cv_process(mat, contrast, luminance, false);

		RMAImage img_tmp;
		img_tmp.CopyProperty(rma_image);
		rma_image = RMAImage(mat.cols, mat.rows, rma_image.GetPixelFormat(), mat.data);
		rma_image.CopyProperty(img_tmp);
	}
}

int64_t PlaybackCacher::calcCacheEndFrameNo(int64_t cache_start_frame_no, int64_t interval)
{
	int64_t cache_end_frame_no = cache_start_frame_no;
	for (int64_t k = 1; k < max_frame_count_per_export_; ++k) {
		cache_end_frame_no += interval;
		if (cache_end_frame_no > end_frame_no_) {
			cache_end_frame_no -= interval;
			break;
		}
	}
	return cache_end_frame_no;
}
