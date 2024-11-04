#include "csplaybackcacher.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
//#include "HscAPI.h"
//#include "../../../../Common/Util/HscSystemElapsedTimer.h"
#include "Util/HscSystemElapsedTimer.h"
#include "Device/imageprocessor.h"
#include "Device/deviceutils.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include "Common/LogUtils/logutils.h"
#include <QBuffer>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
//#define MODE1

CSPlaybackCacher::CSPlaybackCacher(QSharedPointer<Device> pDevice, int video_segment_id, const VideoItem &video_item, bool auto_cycle /*= true*/)
	:device_ptr_(pDevice),
	video_segment_id_(video_segment_id),
	auto_cycle_(auto_cycle),
	frame_map_(kDefaultFrameCountPerExport * 3)
{
	m_export_mode_ = device_ptr_->getExportMode();
	m_stop_mode_ = device_ptr_->getStopMode();
	memset(m_zero_timestamp, 0, sizeof(m_zero_timestamp));

	m_videoinfo.m_width = video_item.getRoi().width();
	m_videoinfo.m_height = video_item.getRoi().height();
	m_videoinfo.m_fps = video_item.getFPS();
	m_videoinfo.m_displaymode = video_item.getDisplayMode();
	m_videoinfo.m_bpp = video_item.getValidBitsPerPixel();
	m_videoinfo.m_streamtype = video_item.getStreamType();
	device_handle_ = pDevice->device_handle_;
	frame_size_ = getFrameSize();
#ifdef MODE1
	if (!m_data_) m_data_ = new char[frame_size_];
#endif
	//if (!auto_cycle) m_is_thumbnail_ = true;
}

CSPlaybackCacher::~CSPlaybackCacher()
{
	stop(Device::eNomal);
#ifdef MODE1
	if (m_data_) {
		delete []m_data_;
		m_data_ = nullptr;
	}
#endif
}

bool CSPlaybackCacher::start(int64_t start_frame_no, int64_t end_frame_no, int64_t frame_interval, int64_t current_frame_no, const AGStreamCB onAGFrame)
{
	mOnAGFrame = onAGFrame;
 	frame_map_.clear();
	m_framemgr_ = nullptr;
	mRevCount = 0;
	lasttimestamp = 0;

	auto processer = device_ptr_->getProcessor();
	if (!processer) return false;
	processer->SetDisplayMode((HscDisplayMode)m_videoinfo.m_displaymode);
	frame_size_ = getFrameSize();

	if (m_videoinfo.m_fps > 0) diff_first_frame_timestamp = (1000000 / m_videoinfo.m_fps) * start_frame_no;
	if (device_ptr_ && device_ptr_->getModel() == DEVICE_5KFSeries)
	{
		HscResult res= device_ptr_->startExport(video_segment_id_);	
		return (res == HSC_OK);
	}

 	if (device_ptr_ && (m_export_mode_ == 0 || m_stop_mode_ == 0))
	{
		stop();
	}
	if (!m_export_mode_) {
		device_ptr_->startExport(video_segment_id_);
		//m_start_export = 1;
	}
	if (exportNew())  max_frame_count_per_export_ = end_frame_no - start_frame_no + 1;
	else max_frame_count_per_export_ = kDefaultFrameCountPerExport;

	int framecount = (end_frame_no - start_frame_no) / frame_interval + 1;
	if (framecount > 0)
		max_frame_count_per_export_ = qMin(framecount, max_frame_count_per_export_);
	else
		max_frame_count_per_export_ = max_frame_count_per_export_;

	// mMemFile
	if (m_count_ == 0) m_count_ = end_frame_no - start_frame_no + 1;

	if (exportNew()) {
		play_frame_no_ = start_frame_no;
		m_cache_map_size_ = qMin(framecount, 150);;
		frame_map_.resize(m_cache_map_size_);
		//frame_map_.resize(framecount);
	}
	else {
		frame_map_.resize(max_frame_count_per_export_ * 3);
	}
	 
	{
		std::lock_guard<std::mutex> lock(mtx_);//参数修改加锁
		start_frame_no_ = start_frame_no;
		end_frame_no_ = end_frame_no;
		frame_interval_ = frame_interval;

		cache_start_frame_no_ = current_frame_no < start_frame_no ? start_frame_no : current_frame_no;
		if (current_frame_no < 0) cache_start_frame_no_ = start_frame_no;
		cache_end_frame_no_ = calcCacheEndFrameNo(cache_start_frame_no_, frame_interval_);
	}

	if (mCacheStream) {
		frame_size_ = getFrameSize();
		mCacheStream->restart(cache_start_frame_no_, cache_end_frame_no_, frame_interval, frame_size_, m_videoinfo.m_fps);
	}
	bool jonable = false;
#ifdef MODE1
	jonable = m_process_thread_ && m_process_thread_->joinable();
	if (jonable)//线程未结束先结束之前的线程
	{
		m_process_thread_running = false;
		m_CondVar.notify_one();
		m_process_thread_->join();
		m_process_thread_.reset();
	} 
#endif

	jonable = thread_ptr_ && thread_ptr_->joinable();
	if (jonable)//线程未结束先结束之前的线程
	{
		in_running_ = State::STOP;
		cv_.notify_one();
		thread_ptr_->join();
		thread_ptr_.reset();
	}


	CSLOG_INFO("***********************start finish: start_frame_no_:{},end_frame_no_:{},frame_interval_:{},play_frame_no_:{},pid:{}", start_frame_no_, end_frame_no_, frame_interval_, play_frame_no_, getpid());
	in_running_ = State::PLAY;
	if (exportNew()) {
		
#ifdef MODE1
		thread_ptr_ = std::make_unique<std::thread>(&CSPlaybackCacher::doExportMode1, this);
		m_process_thread_running = true;
		m_process_thread_ = std::make_unique<std::thread>(&CSPlaybackCacher::doProcess, this);
#else
		//thread_ptr_ = std::make_unique<std::thread>(&CSPlaybackCacher::doExportMode2, this);
		doExportMode2();
#endif
	}
	else {
        thread_ptr_ = std::make_unique<std::thread>(&CSPlaybackCacher::doExport, this);
	}
	return true;
}

bool CSPlaybackCacher::start(const PlaybackCacherParams & params, const AGStreamCB onAGFrame)
{
	return start(params.startFrameNo, params.endFrameNo, params.frameInterval, params.curFrameNo, onAGFrame);

}

void CSPlaybackCacher::stop(bool stop_mode)
{ 
	CSLOG_INFO("========================== CSPlaybackCacher::stop start: {},{},{}:",start_frame_no_, end_frame_no_, frame_interval_);
	if (device_ptr_ && device_ptr_->getModel() == DEVICE_5KFSeries)
	{
		DeviceState state = device_ptr_->getState();
		if (state != Connected && (state == Exporting || state == Recording || state == ToExport || state == Replaying || state == ToReplay))
		{
			device_ptr_->stopExport();
		}
		return;
	}
#ifdef MODE1
	m_process_thread_running = false;
	m_CondVar.notify_one();
	if (m_process_thread_ != nullptr && m_process_thread_->joinable())
	{
		m_process_thread_->join();
		m_process_thread_.reset();
	}
#else
	{
		std::lock_guard<std::mutex> lk(mtx_);
		in_running_ = State::STOP;
		cv_.notify_one();
		if (mCacheStream)
		{
			mCacheStream->stop(false);
		}
	}
#endif
		
	if (thread_ptr_ != nullptr && thread_ptr_->joinable())
	{
		thread_ptr_->join();
		thread_ptr_.reset();
	}


	if (device_ptr_ && device_ptr_->getState() != Connected)
	{
		DeviceState state = device_ptr_->getState();
		if (state == Exporting || state == Recording || state == ToExport || state == Replaying || state == ToReplay)
		{
			if (m_stop_mode_ != Device::eNoStop) device_ptr_->stopExport();
		}
	}
	CSLOG_INFO("========================== CSPlaybackCacher::stop end: {},{},{}:", start_frame_no_, end_frame_no_, frame_interval_);
}

bool CSPlaybackCacher::getFrame(int64_t frame_no, RccFrameInfo & frameInfo, bool trigger_cache /*= true*/)
{
	if (exportNew())
	{

#ifdef MODE1
		if (!m_framemgr_) return false;
		frameInfo.device_name = device_ptr_->getProperty(Device::PropName).toString();
		frameInfo.ip_or_sn = device_ptr_->getIpOrSn();
		frameInfo.playback_info.frame_no = frame_no;

		std::lock_guard<std::mutex> lock(mtx_test_);
		FrameInfo info;
		bool rslt = m_framemgr_->readData(frame_no, m_data_, frame_size_, info);
		if (!rslt) {
			if (trigger_cache)
			{
				if (frame_no > cache_frame_no_ && (frame_no - cache_frame_no_) / frame_interval_  <= 50 && cache_frame_no_ >= 0 && frame_no < end_frame_no_) return false;
				triggered_ = true;
				cache_start_frame_no_ = frame_no;
				if (cache_start_frame_no_ < start_frame_no_)
				{
					cache_start_frame_no_ = start_frame_no_;
				}
				cache_end_frame_no_ = calcCacheEndFrameNo(cache_start_frame_no_, frame_interval_);

			}
			return false;
		}
		memcpy(frameInfo.timestamp, info.m_timestamp_, sizeof(frameInfo.timestamp));
		cv::Mat image_mat(m_image_info.rows_, m_image_info.cols_, m_image_info.type_, m_data_);

		CPlayerViewBase::cvMat2QImage(image_mat.clone(), frameInfo.image);
#else 
		{
			std::lock_guard<std::mutex> lk(mtx_);
			if (!mCacheStream || in_running_ == STOP) return false;
			frameInfo.device_name = device_ptr_->getProperty(Device::PropName).toString();
			frameInfo.ip_or_sn = device_ptr_->getIpOrSn();
			frameInfo.playback_info.frame_no = frame_no;

			AGCacheStream::ImageInfo image_info;
			char *data = mCacheStream->getFrameProcessed(frame_no, image_info);
			if (data == nullptr) return false;
			if (m_videoinfo.m_streamtype == StreamType::TYPE_RAW8)
			{
				frameInfo.valid_bits = m_videoinfo.m_bpp;
			}
			cv::Mat image_mat(image_info.rows_, image_info.cols_, image_info.type_, data + sizeof(IOCacheOBJ::FrameInfo));
			memcpy(frameInfo.timestamp, data, sizeof(frameInfo.timestamp));
			frameInfo.raw_image = image_mat.clone();
			//判断需要移位的位数
			if (device_ptr_->isSupportHighBitParam())
			{
				int bpp = device_ptr_->getProperty(Device::PropPixelBitDepth).toInt();
				Device::DisplayBitDepth dbd = device_ptr_->getProperty(Device::PropDisplayBitDepth).value<Device::DisplayBitDepth>();
				int left_shift = bpp - 8 - dbd;
				//带上高位截断就是低位深的图像,但是观感不好,不便于客户理解,遂注释,后面按需开启
				//int highest_cut = (1 << (16 - left_shift)) - 1;
				//image &= highest_cut;
				image_mat *= (1 << left_shift);
			}
			CPlayerViewBase::cvMat2QImage(image_mat.clone(), frameInfo.image);
			return true;
		}

		bool rslt = frame_map_.get(frame_no, frameInfo);
		if (!rslt) {
			if (trigger_cache)
			{
				//if (frame_no > cache_frame_no_ && (frame_no - cache_frame_no_) / frame_interval_ <= 50 && cache_frame_no_ >= 0 && frame_no < end_frame_no_) return false;
				triggered_ = 1;
				cache_start_frame_no_ = frame_no;
				if (cache_start_frame_no_ < start_frame_no_)
				{
					cache_start_frame_no_ = start_frame_no_;
				}
				cache_end_frame_no_ = calcCacheEndFrameNo(cache_start_frame_no_, frame_interval_);
				cv_.notify_one();
			}
			return false;
		}
#endif
		
		// 更新读取数据数量，排除相同帧的
		auto itr = m_get_frame_no_map_.find(frame_no);
		if (itr != m_get_frame_no_map_.end()) {
			m_get_frame_no_map_[frame_no]++;
			//CSLOG_INFO("ReadMulti:{},{},{},{},{}:", cache_start_frame_no_, cache_end_frame_no_, frame_interval_, frame_no, m_get_image_num_);
		}
		else {
			m_get_frame_no_map_[frame_no] = 1;
			++m_get_image_num_;
		}
		//CSLOG_INFO("ReadOneFrame:{},{},{},{},{}:", cache_start_frame_no_, cache_end_frame_no_, frame_interval_, frame_no, m_get_image_num_);
		
		return true;
	}
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
					CPlayerViewBase::cvMat2QImage(image_mat, frameInfo.image);
					frameInfo.raw_image = image_mat.clone();
					frameInfo.valid_bits = buffer_ptr_->frame_head.bpp;
					frameInfo.device_name = device_ptr_->getProperty(Device::PropName).toString();
					frameInfo.ip_or_sn = device_ptr_->getIpOrSn();
					frameInfo.playback_info.frame_no = buffer_ptr_->frame_head.frameno;
					for (int i = 0; i < 9; i++)
					{
						frameInfo.timestamp[i] = buffer_ptr_->frame_head.time_stamp[i];
					}

					ok = true;
					break;
				}

				if (elapsed_timer.hasExpired(5000))
				{
					break;
				}

				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
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
		if (!isCacheFrameNo(frame_no))//判断是不是当前正在缓存的帧
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

void CSPlaybackCacher::adjustLuminanceAndContrast(RccFrameInfo & frame_image, const int luminance, const int contrast,const bool anti_color)
{
	auto processor_ptr = device_ptr_->getProcessor();
	if (processor_ptr)
	{
		cv::Mat mat;
// 		if (frame_image.raw_image.data)
// 		{
// 			mat = frame_image.raw_image;
// 		}
// 		else
		{
			CPlayerViewBase::QImage2CvMat(frame_image.image, mat);
		}

		mat = processor_ptr->cv_process(mat, contrast, luminance, anti_color);

		CPlayerViewBase::cvMat2QImage( mat,frame_image.image);

	}
}

int64_t CSPlaybackCacher::calcCacheEndFrameNo(int64_t cache_start_frame_no, int64_t interval)
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

bool CSPlaybackCacher::isCacheFrameNo(int64_t frame_no)
{
	if (frame_no < cache_start_frame_no_ || frame_no > cache_end_frame_no_)//不在缓存范围
	{
		return false;
	}
	if ((frame_no - cache_start_frame_no_) % frame_interval_ != 0)//不是抽帧数的整数倍
	{
		return false;
	}

	return true;
}

void CSPlaybackCacher::doExport()
{
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
	HscSystemElapsedTimer elapsed_timer;//超时计时器
	cv::Mat last_cached_frame_info;//最后一帧缓存数据

	//进入导出循环
	while (in_running_) {
		bool needsBreak = false;
		bool needsAutoCacheNext = false;
		{
			std::unique_lock<std::mutex> lock(mtx_);

			if (cache_start_frame_no != cache_start_frame_no_ || cache_end_frame_no != cache_end_frame_no_)
			{
				// 起始帧或结束帧有变化 ,准备执行中断操作
				cache_start_frame_no = cache_start_frame_no_;
				cache_end_frame_no = cache_end_frame_no_;
				cache_frame_no = cache_start_frame_no_;

				needsBreak = true;
			}
			else//起始结束无变化
			{
				if ((cache_frame_no > cache_end_frame_no))//已经加载到片段结尾
				{

					if (image_process_executor_.task_size() > 0) // 前一分段的图像未处理完,暂不进行导出
					{
						boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
						continue;
					}

					if (cache_frame_no > end_frame_no_)//已经加载到片段结尾,且已经加载到视频结尾
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

						cache_frame_no = start_frame_no_;//从视频起始处加载

					}
					else//已经加载到片段结尾,且未加载到视频结尾
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
					//重新计算下一个加载片段的起始和结束位置
					cache_start_frame_no = cache_frame_no;
					if (cache_start_frame_no < start_frame_no_)
					{
						cache_start_frame_no = start_frame_no_;
					}
					cache_end_frame_no = calcCacheEndFrameNo(cache_start_frame_no, frame_interval_);

					//判断自动加载状态,重置重试次数
					if (cache_start_frame_no_ != cache_start_frame_no
						|| cache_end_frame_no_ != cache_end_frame_no)
					{
						cache_start_frame_no_ = cache_start_frame_no;
						cache_end_frame_no_ = cache_end_frame_no;
						cache_frame_no = cache_start_frame_no;

						needsAutoCacheNext = true;
					}
				}
			}
		}

		if (needsBreak)
		{
			image_process_executor_.reset();

			// 中断时，增加重新导出操作
			if (!m_stop_mode_) device_ptr_->stopExport();
			device_ptr_->startExport(video_segment_id_);
		}

		if (needsBreak || needsAutoCacheNext)
		{//需要导出新片段
			ExportFrame(cache_start_frame_no, cache_end_frame_no, frame_interval_);

			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));

			elapsed_timer.restart();

			continue;
		}

		//尝试获取图像
		if (!device_ptr_->getPlaybackFrame(*pBuffer.get()))
		{
			if (cache_frame_no == cache_start_frame_no) // 首帧
			{
				if (elapsed_timer.hasExpired(kFirstFrameTimeoutByMs)) // 首帧超时
				{
					// 重试
					ExportFrame(cache_start_frame_no, cache_end_frame_no, frame_interval_);
					cache_frame_no = cache_start_frame_no;

					elapsed_timer.restart();
				}
			}
			else // 非首帧
			{
				if (elapsed_timer.hasExpired(kNonFirstFrameTimeoutByMs)) // 非首帧超时
				{
					// 补帧
					for (int frame_no = cache_frame_no; frame_no <= cache_end_frame_no; frame_no += frame_interval_)
					{
						cv::Mat frame_info = last_cached_frame_info;
						image_process_executor_.commit(std::bind(&CSPlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), frame_info, frame_no));

						cache_frame_no = frame_no;
					}
					cache_frame_no++;
				}
			}

			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			continue;
		}
		elapsed_timer.restart();

		if (!device_ptr_->isExportByIntervalSupported())
		{
			//控制帧号抽帧间隔
			if ((cache_frame_no - start_frame_no_) % frame_interval_)
			{
				cache_frame_no++;
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
				continue;
			}

			pBuffer->frame_head.frameno = cache_frame_no;
			cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get()).clone();

			image_process_executor_.commit(std::bind(&CSPlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), buffer_mat, cache_frame_no));

			last_cached_frame_info = buffer_mat;

			cache_frame_no++;
		}
		else
		{
			pBuffer->frame_head.frameno = cache_frame_no;
			cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get()).clone();

			image_process_executor_.commit(std::bind(&CSPlaybackCacher::doImageProcess, this, device_ptr_->getProcessor(), buffer_mat, cache_frame_no));
			last_cached_frame_info = buffer_mat;

			cache_frame_no += frame_interval_;
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
	}

	image_process_executor_.reset();
}

void CSPlaybackCacher::doImageProcess(std::shared_ptr<ImageProcessor> image_processor_ptr, cv::Mat buffer_mat , int64_t frame_no)
{
	
	if (image_processor_ptr != nullptr && !buffer_mat.empty())
	{
		CSLOG_INFO("start doProcess frame:{}", frame_no);
		CAGBuffer *buffer_ptr = (CAGBuffer*)buffer_mat.data;
		cv::Mat image_mat = image_processor_ptr->cv_process(buffer_ptr, 3, true);
//cv::imwrite("aaq1.bmp", image_mat);
#ifdef MODE1
		if (exportNew() && m_framemgr_) {
			++m_thread_active_count;
			if (m_image_info.cols_ == 0) {
				m_image_info = ImageInfo(image_mat.rows, image_mat.cols, image_mat.type(), image_mat.step);
			}

			FrameInfo info;
			memcpy(info.m_timestamp_, buffer_ptr->frame_head.time_stamp, sizeof(info.m_timestamp_));

			m_framemgr_->writeData(frame_no, (char*)image_mat.data, frame_size_, info);
			--m_thread_active_count;
		
			cache_frame_no_ = frame_no;
		}
#else 	
		{
			RccFrameInfo frame_image;
			CPlayerViewBase::cvMat2QImage(image_mat, frame_image.image);
			frame_image.raw_image = image_mat.clone();
			frame_image.valid_bits = buffer_ptr->frame_head.bpp;
			frame_image.device_name = device_ptr_->getProperty(Device::PropName).toString();
			frame_image.ip_or_sn = device_ptr_->getIpOrSn();
			frame_image.playback_info.frame_no = frame_no;
			for (int i = 0; i < 9; i++)
			{
				frame_image.timestamp[i] = buffer_ptr->frame_head.time_stamp[i];
			
			}

			frame_map_.put(frame_no, frame_image);
			cache_frame_no_ = frame_no;
			
			CSLOG_INFO("finish doProcess frame:{}", frame_no);
		}
#endif
	}
	else
	{
		CSLOG_INFO("no frame:{}", frame_no);
	}
}

void CSPlaybackCacher::ExportFrame(int64_t start_frame_no, int64_t end_frame_no, int64_t interval)
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

void CSPlaybackCacher::doImageProcess1(std::shared_ptr<ImageProcessor> image_processor_ptr, std::shared_ptr<CAGBuffer> &buffer, int64_t frame_no)
{
	cv::Mat buffer_mat = cv::Mat(1, buffer->frame_head.frame_size, CV_8UC1, buffer.get());
	CAGBuffer *buffer_raw = (CAGBuffer*)buffer_mat.data;
	cv::Mat image_mat = image_processor_ptr->cv_process(buffer_raw, 3, true);

	RccFrameInfo frame_image;
	CPlayerViewBase::cvMat2QImage(image_mat, frame_image.image);
	frame_image.raw_image = image_mat.clone();
	frame_image.valid_bits = buffer->frame_head.bpp;
	frame_image.device_name = device_ptr_->getProperty(Device::PropName).toString();
	frame_image.ip_or_sn = device_ptr_->getIpOrSn();
	frame_image.playback_info.frame_no = frame_no;
	for (int i = 0; i < 9; i++)
	{
		frame_image.timestamp[i] = buffer_raw->frame_head.time_stamp[i];
	}
	frame_map_.put(frame_no, frame_image);
}

void CSPlaybackCacher::doExportMode2()
{
	std::shared_ptr<CAGBuffer> pBuffer(new CAGBuffer);
	std::unique_lock<std::mutex> lock(mtx_);
	int64_t start_frame_no = cache_start_frame_no_;
	int64_t end_frame_no = cache_end_frame_no_;
	int64_t cache_frame_no = cache_start_frame_no_;
	int64_t cache_end_no = cache_end_frame_no_;
	int32_t frame_interval = frame_interval_;
	get_frame_no_ = start_frame_no;
	cache_frame_no_ = cache_frame_no;
	auto image_processor_ptr = device_ptr_->getProcessor();
	m_get_image_num_ = 0;
	m_put_image_num_ = 0;
	m_get_frame_no_map_.clear();
	lock.unlock();

	CAGBuffer *buffer = nullptr;
	if (!mCacheStream)
	{
		mCacheStream = std::make_shared<AGCacheStream>(device_ptr_, device_ptr_->getProperty(Device::PropStreamType).value<StreamType>());
		frame_size_ = getFrameSize();
		mCacheStream->start(device_handle_, video_segment_id_, start_frame_no, end_frame_no, frame_interval, frame_size_, m_videoinfo.m_fps, mOnAGFrame);
	}
	return;

	HscSystemElapsedTimer elapsed_timer;
	while (in_running_) {
		if (!in_running_)
		{
			CSLOG_INFO("thread stop:");
			if (image_process_executor_.task_size() > 0 && m_thread_active_count == 0)
			{
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
				continue;
			}
			break;
		}

		if (triggered_ && (cache_start_frame_no_ != start_frame_no || cache_end_frame_no_ != end_frame_no))
		{
			CSLOG_INFO("=======================seek old: {},{},{},{} new: {},{},{},{}", start_frame_no, end_frame_no, get_frame_no_, cache_frame_no, cache_start_frame_no_, cache_end_frame_no_, m_put_image_num_, m_get_image_num_);
			image_process_executor_.reset();
			start_frame_no = cache_start_frame_no_;
			end_frame_no = cache_end_frame_no_;
			get_frame_no_ = start_frame_no;
			cache_frame_no_ = -1;
			lasttimestamp = 0;
			m_get_image_num_ = 0;
			m_put_image_num_ = 0;
			m_get_frame_no_map_.clear();
			//buffer = mCacheStream->seekTo(get_frame_no_);
		}
		{
			std::lock_guard<std::mutex> lk(mtx_);
			cv_.wait(lock, [this, end_frame_no] {
				return !in_running_ || ((get_frame_no_ <= end_frame_no) || triggered_);
			});

			cv_.wait(lock, [this] {
				return (!in_running_ || (in_running_ != State::PAUSE  && (m_put_image_num_ < m_cache_map_size_))|| triggered_);
			});
			triggered_ = 0;
		}

        buffer = mCacheStream->getFrame(get_frame_no_);	
		if (!buffer)
		{
			if (elapsed_timer.hasExpired(kNonFirstFrameTimeoutByMs))
			{
				std::lock_guard<std::mutex> lk(mtx_);
				if (!in_running_) continue;
				mCacheStream->stop(false);
				mCacheStream->restart(cache_start_frame_no_, cache_end_frame_no_, frame_interval, frame_size_, m_videoinfo.m_fps);
				elapsed_timer.restart();
			}
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			continue;
		}

		
		uint64_t crttimestamp = DeviceUtils::getTimestamp(buffer->frame_head.time_stamp);
		if (lasttimestamp > 0 && m_videoinfo.m_fps > 0) Assert(crttimestamp == lasttimestamp + (1000000 / m_videoinfo.m_fps) * frame_interval_, "");
		lasttimestamp = crttimestamp;

		buffer->frame_head.frameno = get_frame_no_;
		cv::Mat buffer_mat = cv::Mat(1, buffer->frame_head.frame_size, CV_8UC1, buffer).clone();
		if (m_put_image_num_ < m_cache_map_size_) m_put_image_num_++;
		image_process_executor_.commit(std::bind(&CSPlaybackCacher::doImageProcess, this, image_processor_ptr, buffer_mat, buffer->frame_head.frameno));
		get_frame_no_ += frame_interval;
		elapsed_timer.restart();
		// 根据获取的数据数量和读取的数据数量做比较，判断是否要继续获取数据（临时处理）
		//{
		//	std::lock_guard<std::mutex> lk(mtx_);
		//	cv_.wait(lock, [this] {
		//		return !in_running_ || ((m_put_image_num_ - m_get_image_num_ + 1 < m_cache_map_size_) || triggered_);
		//	});
		//}
	}
	CSLOG_INFO("thread doExportMode2 stop finish:{}", getpid());
	image_process_executor_.reset();
}

void CSPlaybackCacher::doExportMode1()
{
	CSLOG_INFO("start thread doExportMode1 {}:", getpid());
	HscResult res(HSC_OK);
	frame_map_.clear();
	
	int64_t framecount = (cache_end_frame_no_ - cache_start_frame_no_) / frame_interval_ + 1;
	res = device_ptr_->exportByInterval(video_segment_id_, cache_start_frame_no_, cache_end_frame_no_, framecount, frame_interval_);
	if (res != HSC_OK) return;

	std::shared_ptr<CAGBuffer> pBuffer(new CAGBuffer);
	HscSystemElapsedTimer elapsed_timer;//超时计时器
	cv::Mat last_cached_frame_info;//最后一帧缓存数据

	std::unique_lock<std::mutex> lock(mtx_);
	int64_t start_frame_no = cache_start_frame_no_;
	int64_t end_frame_no = cache_end_frame_no_;
	int64_t cache_frame_no = cache_start_frame_no_;
	int64_t cache_end_no = cache_end_frame_no_;
	int32_t frame_interval = frame_interval_;
	cache_frame_no_ = cache_frame_no;
	int64_t play_no = play_frame_no_;
	get_frame_no_ = start_frame_no;
	bool find_first_frame = false;
	lock.unlock();
	auto image_processor_ptr = device_ptr_->getProcessor();
	//进入导出循环
	while (in_running_) {
		if (!in_running_)
		{
			CSLOG_INFO("thread stop:");
			if (image_process_executor_.task_size() > 0 && m_thread_active_count == 0)
			{
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
				continue;
			}
			break;
		}
		if (triggered_ && (cache_start_frame_no_ != start_frame_no || cache_end_frame_no_ != end_frame_no))
		{
			image_process_executor_.reset();
			start_frame_no = cache_start_frame_no_;
			end_frame_no = cache_end_frame_no_;
			ExportFrame(start_frame_no, end_frame_no, frame_interval_);
			cache_frame_no = start_frame_no;
			cache_frame_no_ = -1;
			get_frame_no_ = start_frame_no;
			triggered_ = false;
			if (m_videoinfo.m_fps > 0) diff_first_frame_timestamp = (1000000 / m_videoinfo.m_fps) * start_frame_no;
			find_first_frame = false;
			mRevCount = 0;
		}

		if (get_frame_no_ > end_frame_no)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			continue;
		}
	
		//if ((cache_frame_no - play_frame_no_) + 1 / frame_interval_ > max_cache_size_)
		//{
		//	boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		//	continue;
		//}
		auto rslt = device_ptr_->getPlaybackFrame(*pBuffer.get());
		if (!rslt)
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			continue;
		}
		elapsed_timer.restart();
		{
			uint64_t interval_timestamp = cache_start_frame_no_ * (1e6 / m_videoinfo.m_fps) + (1e6 / m_videoinfo.m_fps) * (mRevCount * frame_interval_);
			pBuffer->frame_head.frameno = get_frame_no_;
			uint64_t timestamp = DeviceUtils::getTimestamp(pBuffer->frame_head.time_stamp);
			//uint64_t timestamp = DeviceUtils::getTimestampTons(pBuffer->frame_head.time_stamp);
			
			if (first_frame_timestamp == 0)
			{
				first_frame_timestamp = timestamp - diff_first_frame_timestamp;
				if (timestamp < diff_first_frame_timestamp) {
					CSLOG_ERROR("timestamp invalid:{},{}", timestamp, diff_first_frame_timestamp);
					continue;;
				}
			
				Assert(pBuffer->frame_head.frameno == 0, "");
			}
			if (first_frame_timestamp + diff_first_frame_timestamp == timestamp) {
				CSLOG_INFO("=================================find first frame:{},{},{},{},{}", first_frame_timestamp, timestamp, diff_first_frame_timestamp, start_frame_no, pBuffer->frame_head.frameno);
				find_first_frame = true;
			}
			else if (first_frame_timestamp + diff_first_frame_timestamp != timestamp && !find_first_frame) {
				CSLOG_INFO("filter data:{},{},{},{},{}", first_frame_timestamp, timestamp, diff_first_frame_timestamp, start_frame_no, get_frame_no_);
				continue;
			}
			//else if (first_frame_timestamp + interval_timestamp != timestamp)
			//{
			//	CSLOG_INFO("filter data:{},{},{},{},{}", first_frame_timestamp, timestamp, interval_timestamp, diff_first_frame_timestamp, get_frame_no_);
			//	continue;
			//}

			cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get());
			//cv::Mat buffer_mat = cv::Mat(1, pBuffer->frame_head.frame_size, CV_8UC1, pBuffer.get()).clone();
			if (!m_framemgr_)
			{
				CAGBuffer *buffer_ptr = (CAGBuffer*)buffer_mat.data;
				cv::Mat image_mat = image_processor_ptr->cv_process(buffer_ptr, 3, true);
				frame_size_ = image_mat.rows * image_mat.cols* image_mat.channels();
				QString filename = "fhjd_memFile_@@@";
				if (auto_cycle_)filename += "_1";
				else filename += "_0";
				m_framemgr_ = std::make_shared<FrameMgrFixed>((end_frame_no_-start_frame_no_+1)/frame_interval, frame_size_, m_count_, frame_interval,filename);
			}
			{
				if (m_data_cache_queue.size() > 400) {
					// TODO: need stop
					boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
				}
				{
					std::lock_guard<std::mutex> lk(m_ProcessQueueMtx);
					m_data_cache_queue.push(std::move(buffer_mat.clone()));
					m_CondVar.notify_one();
				}
				
			}
			get_frame_no_ += frame_interval_;
			mRevCount++;
		}
	}
	CSLOG_INFO("thread stop finish:{}", getpid());
	image_process_executor_.reset();
}


void CSPlaybackCacher::doProcess()
{
	//int32_t cache_frame_no = cache_frame_no_;
	//std::queue<cv::Mat> tem_queue;
	cv::Mat buffer_mat;
	while (m_process_thread_running)
	{
		if (!m_process_thread_running)
		{
			CSLOG_INFO("thread stop:");
			if (image_process_executor_.task_size() > 0 && m_thread_active_count == 0)
			{
				boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
				continue;
			}
			break;
		}
		std::unique_lock<std::mutex> lk(m_ProcessQueueMtx);
		m_CondVar.wait(lk, [this] { return (!m_data_cache_queue.empty() || !m_process_thread_running); });
		if (!m_process_thread_running) return;
		buffer_mat = std::move(m_data_cache_queue.front());
		m_data_cache_queue.pop();
		lk.unlock();

		auto image_processor_ptr = device_ptr_->getProcessor();
		CAGBuffer *buffer_ptr = (CAGBuffer*)buffer_mat.data;
		int32_t cache_frame_no = buffer_ptr->frame_head.frameno;
		image_process_executor_.commit(std::bind(&CSPlaybackCacher::doImageProcess, this, image_processor_ptr, buffer_mat, cache_frame_no));

	}
	image_process_executor_.reset();
}


int32_t CSPlaybackCacher::getFrameSize()
{
	int display_mode = m_videoinfo.m_displaymode;
	int channel = 1;
	int stream_type = m_videoinfo.m_streamtype;

	//计算单帧数据量 : 
	if (HSC_DISPLAY_MONO != display_mode)  channel = 3;
	if (TYPE_RGB888 == stream_type || TYPE_H264 == stream_type) channel = 3;
	frame_size_ = channel * m_videoinfo.m_width * m_videoinfo.m_height * ((m_videoinfo.m_bpp > 8 &&TYPE_RAW8 == stream_type) ? 2 : 1);
	if (TYPE_YUV420 == stream_type) frame_size_ = frame_size_ * 3 / 2;
	 
	//frame_size_+= sizeof(IOCacheOBJ::FrameInfo);
	frame_size_ +=FrameHeadlen();
	return frame_size_;
}
