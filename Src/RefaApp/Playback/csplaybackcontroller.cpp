#include "csplaybackcontroller.h"
#include "Video/VideoUtils/videoutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Device/device.h"
#include "Device/imageprocessor.h"
#ifndef _WINDOWS
#include "UtilityNS/UtilityNS.h"
#endif
#include <QCoreApplication>


CSPlaybackController::CSPlaybackController(VideoItem video_item, QObject *parent)
	: QObject(parent), m_video_item(video_item)
{
	InitData();
}

CSPlaybackController::~CSPlaybackController()
{
	m_update_running = false;
	if (m_update_thread.joinable())
	{
		m_cv_ctrl.notify_one();
		m_update_thread.join();
	}
	//if (m_doLoadKeyFrameThrd.joinable()) m_doLoadKeyFrameThrd.join();
	/*if (m_thumbnail_timer.joinable()) {
		m_thumbnail_timer.join();
	}*/
	if (m_thumbnail_cacher_ptr)
	{
		m_thumbnail_cacher_ptr.reset();
	}
	if (m_export_cacher_ptr)
	{
		m_export_cacher_ptr.reset();
	}
}



VideoItem CSPlaybackController::GetVideoItem()
{
	VideoItem res_item = m_video_item;
	//更新当前设备设置
	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();
	if (!export_path.isEmpty())
	{
		res_item.setExportPath(export_path);
	}
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{
		res_item.setProperty(VideoItem::PropType::StreamType, device_ptr->getProperty(Device::PropStreamType));
		res_item.setProperty(VideoItem::PropType::VideoFormat, device_ptr->getProperty(Device::PropVideoFormat));
		res_item.setProperty(VideoItem::PropType::OsdVisible, device_ptr->getProperty(Device::PropWatermarkEnable));
	}
	//同步当前变更
	res_item.setBeginFrameIndex(m_start_frame_no);
	res_item.setEndFrameIndex(m_end_frame_no);
	res_item.setLuminance(m_luminance);
	res_item.setContrast(m_contrast);
	res_item.setAntiColorEnable(m_anti_color);
	res_item.setKeyFrameIndex(m_key_frame_no);
	return res_item;
}

void CSPlaybackController::InitData()
{
	//获取到当前视频对应的设备
	QString device_ip = VideoUtils::parseDeviceIp(m_video_item.getId());
	int vid = VideoUtils::parseVideoSegmentId(m_video_item.getId());
	m_device_ptr = DeviceManager::instance().getDevice(device_ip);
	Q_ASSERT(m_device_ptr);
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr&&device_ptr->getProcessor())
	{
		device_ptr->getProcessor()->SetRoi(m_video_item.getRoi());
	}

	m_export_mode_ = device_ptr->getExportMode();
	m_stop_mode_ = device_ptr->getStopMode();

	//初始化设备导出缓存
	m_export_cacher_ptr = std::make_shared<CSPlaybackCacher>(DeviceManager::instance().getDevice(device_ip), vid, m_video_item, true);
	m_thumbnail_cacher_ptr = std::make_shared<CSPlaybackCacher>(DeviceManager::instance().getDevice(device_ip), vid, m_video_item, false);
	m_thumbnail_timer_ptr = new QTimer;
	m_thumbnail_timer_ptr->setInterval(50);
	QObject::connect(m_thumbnail_timer_ptr.data(), &QTimer::timeout, this, &CSPlaybackController::checkTimeOutThumbnails);
	QObject::connect(this, &CSPlaybackController::signalStartLoadingThumbnailTimers, this, &CSPlaybackController::startThumbnailLoadingTimers);

	m_video_total_frame_count = m_video_item.getVideoFrameCount();
	m_start_frame_no = m_video_item.getBeginFrameIndex();
	m_current_frame_no = m_start_frame_no;
	m_end_frame_no = m_video_item.getEndFrameIndex();
	m_play_speed = 1;
	m_fps = m_video_item.getFPS();
	m_key_frame_no = m_video_item.getKeyFrameIndex();
	m_last_key_frame_no = m_key_frame_no;
	m_origin_key_frame_no = m_key_frame_no;
	m_contrast = m_video_item.getContrast();
	m_luminance = m_video_item.getLuminance();

	//获取关键帧图像
	//暂时取消初始化时对关键帧的加载动作
	//m_doLoadKeyFrameThrd = std::thread(&CSPlaybackController::doLoadKeyFrame, this);

	//开启刷新数据线程
	m_update_thread = std::thread(&CSPlaybackController::doUpdate, this);
}

void CSPlaybackController::doUpdate()
{
	std::chrono::system_clock::time_point current_time_point;

	while (m_update_running)
	{
		{
			std::unique_lock<std::mutex> locker(m_playback_mutex);
			m_cv_ctrl.wait(locker, [this] {
				if (m_quit_previewing)//需要退出缩略图预览状态
				{
					m_thumbnail_previewing = false;//退出缩略图预览
					m_quit_previewing = false;
				}
				//if (m_image_changed)
				//{
				//	m_image_changed = false;
				//	return true;
				//}
				if (!m_update_running)
				{
					return true;
				}

				if (m_state == PlayState::PS_PLAY)
				{
					return true;
				}
				if (m_state == PlayState::PS_PAUSE)
				{
					if (m_last_frame_no != m_current_frame_no || m_thumbnail_previewing)
					{
						return true;
					}
				}

				return false;
			});
		}

		{
			std::unique_lock<std::mutex> locker(m_playback_mutex);


			RccFrameInfo frame_image;
			//根据当前模式获取需要显示的图像 
			if (m_thumbnail_previewing)//正在预览缩略图
			{
				frame_image = m_preview_frame;
			}
			else//正在正常播放
			{
				if (!m_export_cacher_ptr || !m_export_cacher_ptr->getFrame(m_current_frame_no, frame_image))
				{
					locker.unlock();
					boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
					continue;
				}

				// 调节亮度对比度
				m_export_cacher_ptr->adjustLuminanceAndContrast(frame_image, m_luminance, m_contrast, m_anti_color);

				frame_image.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
				frame_image.playback_info.end_frame_no = m_end_frame_no;
				frame_image.playback_info.is_key_frame = (frame_image.playback_info.frame_no == m_key_frame_no);

				m_last_frame_no = m_current_frame_no;
			}
			m_current_frame_backup = frame_image;//备份当前数据
			locker.unlock();
			emit ImageUpdate(frame_image);
		}
		if (m_state == PlayState::PS_PLAY)
		{
			setCurrentFrameNo(GetNextFrameNo());
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(m_play_period));//等待播放间隔结束

	}
}

void CSPlaybackController::doLoadKeyFrame()
{
	if (m_key_frame_no >= 0 && m_key_frame_no != m_image_keyframe.playback_info.frame_no)
	{
		QElapsedTimer elapsedTimer;
		elapsedTimer.restart();

		while (!m_export_cacher_ptr->getFrame(m_key_frame_no, m_image_keyframe))
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));

			if (elapsedTimer.hasExpired(5000))
			{
				break;
			}
		}

		m_image_keyframe.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
		m_image_keyframe.playback_info.end_frame_no = m_end_frame_no;
		m_image_keyframe.playback_info.is_key_frame = (m_image_keyframe.playback_info.frame_no == m_key_frame_no);
	}
}

void CSPlaybackController::stopLoadingThumbnails()
{
	//if (m_thumbnail_loading_thrd == false)
	//{
	//	CSLOG_INFO("CSPlaybackController::stopLoadingThumbnails {},{}", m_thumbnail_loading, m_state);
	//}
	//m_thumbnail_loading_thrd = false;
	//std::unique_lock<std::mutex> locker(m_thumbnails_mutex);
	m_thumbnail_cacher_ptr->stop(m_stop_mode_);
	m_thumbnail_timer_ptr->stop();
#ifdef _WINDOWS
	sThumbnailTrack.mEndThumbnail = GetTickCount64();
#else
    sThumbnailTrack.mEndThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
	sThumbnailTrack.mInRecState = true;
	sThumbnailTrack.mRecvCount = true;

	// 恢复
	restoreState();
	m_thumbnail_loading = false;
}

qint64 CSPlaybackController::getThumbnailExportInterval(qint64 start_frame_no, qint64 end_frame_no, qint64 thumbnail_count)
{
	qint64 interval = 1;

	if (thumbnail_count > 1)
	{
		interval = (end_frame_no - start_frame_no) / (thumbnail_count - 1);
		if (interval < 1)
		{
			interval = 1;
		}
	}

	return interval;
}

void CSPlaybackController::setCurrentFrameNo(int64_t frame_no)
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	m_current_frame_no = frame_no;
	//范围控制
	if (m_current_frame_no < m_start_frame_no)
	{
		m_current_frame_no = m_start_frame_no;
	}

	if (m_current_frame_no > m_end_frame_no)
	{
		m_current_frame_no = m_end_frame_no;
	}

	if (m_last_frame_no == m_current_frame_no)
	{
		m_last_frame_no -= 1;
	}
}

void CSPlaybackController::saveState()
{
	if (!m_state_saved)//没有保存状态的时候才保存
	{
		m_state_saved = true;
		m_previous_state = m_state;
	}
}

void CSPlaybackController::restoreState()
{
	// 恢复
	m_state_saved = false;
	if (m_previous_state == PlayState::PS_PLAY)
	{
		SwitchState(PS_PLAY);
	}
	// [2023/2/21 rgq]: 去掉下面的判断，会出现（上一次是暂停，当前处于停止状态）无法恢复的问题
	else /*if (m_previous_state != PlayState::PS_PAUSE)*/
	{
		SwitchState(PS_PAUSE);
	}
}

bool CSPlaybackController::SwitchState(PlayState play_state)
{ 
	switch (play_state)
	{
	case PS_STOP://停止
	{
		if (m_state == PlayState::PS_STOP)
		{
			break;
		}

		if (m_export_cacher_ptr)
		{
			m_export_cacher_ptr->stop(m_stop_mode_);
		}

		m_cacher_params = PlaybackCacherParams();

		m_state = PlayState::PS_STOP;

	}
	break;
	case PS_PLAY://播放
	{
		if (m_state == PlayState::PS_PLAY || m_disable_get_frame || m_thumbnail_loading)
		{
			break;
		}

		m_state = PlayState::PS_PLAY;
		if (m_cacher_params.startFrameNo != m_start_frame_no ||
			m_cacher_params.endFrameNo != m_end_frame_no ||
			m_cacher_params.frameInterval != m_play_speed)//判断播放参数是否相同,不相同则开始加载
		{
			if (m_export_cacher_ptr)
			{
				if (m_end_frame_no - m_start_frame_no < m_play_speed)
				{
					m_play_speed = m_end_frame_no - m_start_frame_no;
					if (m_play_speed < 1) m_play_speed = 1;
				}
				PlaybackCacherParams cacher_params;
				cacher_params.startFrameNo = m_start_frame_no;
				cacher_params.endFrameNo = m_end_frame_no;
				cacher_params.frameInterval = m_play_speed;
				cacher_params.curFrameNo = GetCurrentFrameNo();

				if (m_export_cacher_ptr->start(cacher_params))
				{
					m_cacher_params = cacher_params;
				}
				else
				{
					m_state = PlayState::PS_STOP;
				}
			}
		}
		else
		{
			m_export_cacher_ptr->restart();
		}
		m_quit_previewing = true;
		m_cv_ctrl.notify_one();
	}
	break;
	case PS_PAUSE://暂停
	{
		if ( m_disable_get_frame || m_thumbnail_loading)
		{
			break;
		}
		if (m_state == PlayState::PS_STOP && !m_disable_get_frame)
		{
			if (m_end_frame_no - m_start_frame_no < m_play_speed)
			{
				m_play_speed = m_end_frame_no - m_start_frame_no;
				if (m_play_speed < 1) m_play_speed = 1;
			}
			m_state = PlayState::PS_PAUSE;
			PlaybackCacherParams cacher_params;
			cacher_params.startFrameNo = m_start_frame_no;
			cacher_params.endFrameNo = m_end_frame_no;
			cacher_params.frameInterval = m_play_speed;
			cacher_params.curFrameNo = GetCurrentFrameNo();

			if (m_export_cacher_ptr->start(cacher_params))
			{
				m_cacher_params = cacher_params;
			}
			else
			{
				m_state = PlayState::PS_STOP;
			}
		}
		else
		{
			m_export_cacher_ptr->pause();
			m_state = PlayState::PS_PAUSE;
		}
	}
	break;
	default:
		break;
	}

	return true;
}

PlayState CSPlaybackController::GetState() const
{
	return m_state;
}

void CSPlaybackController::DisableGetFrame(bool b_disable)
{
	//更新状态
	m_disable_get_frame = b_disable;

	if (m_disable_get_frame)//禁止获取图像时需要停止正在进行的导出相关操作
	{
		std::lock_guard<std::mutex> lock(m_thumbnails_mutex);
		{
			if (m_thumbnail_loading)
			{
				stopLoadingThumbnails();
			}
		}

		if (GetState() != PS_STOP)
		{
			SwitchState(PS_STOP);
		}
	}
}

void CSPlaybackController::NextFrame()
{
	if (GetSpeed() != 1)
	{
		SwitchSpeed(1);
	}

	setCurrentFrameNo(GetNextFrameNo()/* + m_interval*/);
	
	if (GetState() != PS_PAUSE)
	{
		SwitchState(PS_PAUSE);
	}

	m_cv_ctrl.notify_one();

}

void CSPlaybackController::PreviousFrame()
{
	if (GetSpeed() != 1)
	{
		SwitchSpeed(1);
	}
	setCurrentFrameNo(GetPreviousFrameNo()/* - m_interval*/);
	if (GetState() != PS_PAUSE)
	{
		SwitchState(PS_PAUSE);
	}
	m_cv_ctrl.notify_one();
}

void CSPlaybackController::RefreshImage()
{
	//m_image_changed = true;
	changeThumbnailsState(true);
	m_cv_ctrl.notify_one();
}

void CSPlaybackController::SeekFrame(int64_t play_frame)
{
	setCurrentFrameNo(play_frame);
	m_quit_previewing = true;//退出缩略图预览
	m_cv_ctrl.notify_one();
}

void CSPlaybackController::SeekFrameMs(double play_frame)
{
	SeekFrame(VideoUtils::msToFrameId(play_frame, m_fps));
}

int64_t CSPlaybackController::GetCurrentFrameNo() const
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	return m_current_frame_no;
}

double CSPlaybackController::GetCurrentFrameMS() const
{
	return VideoUtils::frameIdToMs(GetCurrentFrameNo(), m_fps);
}

int64_t CSPlaybackController::GetPreviousFrameNo() const
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	int64_t previousPlayFrameNo = m_current_frame_no;

	if (previousPlayFrameNo > m_end_frame_no)
	{
		previousPlayFrameNo = m_end_frame_no;
	}
	else
	{
		previousPlayFrameNo -= m_play_speed;
		//判断播放到最前一帧之后回到最后
		if (previousPlayFrameNo < m_start_frame_no)
		{
			previousPlayFrameNo = m_start_frame_no;
		}

	}
	return previousPlayFrameNo;
}

double CSPlaybackController::GetPreviousFrameMs() const
{
	return VideoUtils::frameIdToMs(GetPreviousFrameNo(), m_fps);
}

int64_t CSPlaybackController::GetNextFrameNo() const
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	int64_t nextPlayFrameNo = m_current_frame_no;

	if (nextPlayFrameNo < m_start_frame_no)
	{
		nextPlayFrameNo = m_start_frame_no;
	}
	else
	{
		nextPlayFrameNo += m_play_speed;
		//判断播放到最后一帧之后回到开头
		if (nextPlayFrameNo > m_end_frame_no)
		{
			nextPlayFrameNo = m_start_frame_no;
		}
	}

	return nextPlayFrameNo;
}

double CSPlaybackController::GetNextFrameMs() const
{
	return VideoUtils::frameIdToMs(GetNextFrameNo(), m_fps);
}

bool CSPlaybackController::SwitchRange(int64_t begin, int64_t end, bool save_change)
{
	if (begin > end)
	{
		return false;
	}

	{
		std::unique_lock<std::mutex> locker(m_switch_range_mutex);
		if (m_switch_range_flag == 0) m_switch_range_flag = 1;

		/*m_switch_range_condvar.wait(locker, [this] {
			return (m_switch_range_flag == 0); });
		m_switch_range_flag = 1;*/
		locker.unlock();
	}

	if (m_start_frame_no == begin && m_end_frame_no == end)
	{
		if (save_change)
		{
			//记录
			m_range_stack.push(CSPlayRange(m_start_frame_no, m_end_frame_no));

			int nSize = m_range_stack.size();
			emit signalRangeStackChange(nSize);
		}
		std::lock_guard<std::mutex> locker(m_switch_range_mutex);
		m_switch_range_flag = 0;
		return true;
	}

	// 停止
	saveState();
	if (m_state != PlayState::PS_STOP)
	{
		SwitchState(PS_STOP);
	}

	if (save_change)
	{
		//记录
		m_range_stack.push(CSPlayRange(m_start_frame_no, m_end_frame_no));

		int nSize = m_range_stack.size();
		emit signalRangeStackChange(nSize);
	}

	// 设置
	m_start_frame_no = begin;
	m_end_frame_no = end;

	//修正
	//范围控制
	if (m_current_frame_no < m_start_frame_no)
	{
		m_current_frame_no = m_start_frame_no;
	}

	if (m_current_frame_no > m_end_frame_no)
	{
		m_current_frame_no = m_end_frame_no;
	}

	if (m_last_frame_no == m_current_frame_no)
	{
		m_last_frame_no -= 1;
	}


	//发送范围变更信号
	emit signalVideoRangeChanged(m_start_frame_no, m_end_frame_no);

	//重新加载缩略图
	StartLoadThumbnails();

	std::lock_guard<std::mutex> locker(m_switch_range_mutex);
	m_switch_range_flag = 0;
	// 	// 恢复
	// 	if (state == PlayState::PS_PLAY)
	// 	{
	// 		SwitchState(PS_PLAY);
	// 	}
	// 	else if (state != PlayState::PS_PAUSE)
	// 	{
	// 		SwitchState(PS_PAUSE);
	// 	}

	return true;
}

bool CSPlaybackController::SwitchRangeMs(double start_frame_ms, int64_t end_frame_ms, bool save_change)
{
	return SwitchRange(
		VideoUtils::msToFrameId(start_frame_ms, m_fps),
		VideoUtils::msToFrameId(end_frame_ms, m_fps),
		save_change);
}

bool CSPlaybackController::SwitchToPreviousRange()
{
	if (m_range_stack.size() == 0)
	{
		return false;
	}

	CSPlayRange range = m_range_stack.top();
	m_range_stack.pop();

	SwitchRange(range.first, range.second, false);

	int nSize = m_range_stack.size();
	emit signalRangeStackChange(nSize);
	return true;
}

int64_t CSPlaybackController::GetStartFrameNo()
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	return m_start_frame_no;
}

double CSPlaybackController::GetStartFrameMs()
{
	return VideoUtils::frameIdToMs(GetStartFrameNo(), m_fps);
}

int64_t CSPlaybackController::GetEndFrameNo()
{
	std::lock_guard<std::mutex> lock(m_playback_mutex);

	return m_end_frame_no;
}

double CSPlaybackController::GetEndFrameMs()
{
	return VideoUtils::frameIdToMs(GetEndFrameNo(), m_fps);
}

bool CSPlaybackController::SwitchSpeed(int64_t play_speed)
{
	/*if (play_speed < 1 || play_speed >16)
	{
		return false;
	}*/

	// 停止
	saveState();
	if (m_state != PlayState::PS_STOP)
	{
		SwitchState(PS_STOP);
	}

	// 设置
	if (play_speed == 0) play_speed = 1;
	m_play_speed = play_speed;
	restoreState();
	return true;
}

void CSPlaybackController::SkipFrame(int64_t play_speed)
{
	m_interval = play_speed - 1;
}

int64_t CSPlaybackController::GetSpeed()
{
	return m_play_speed;
}

void CSPlaybackController::SetKeyFrameNo(int64_t key_frame_no)
{
	m_last_key_frame_no = m_key_frame_no;
	m_key_frame_no = key_frame_no;

	//重新获取关键帧图像(规避加载动作,在目前已有的数据中查找关键帧)
	bool has_key_frame = false;//有没有在当前帧和缩略图中找到关键帧
	//判断当前帧是不是关键帧
	if (m_current_frame_backup.playback_info.frame_no == key_frame_no)
	{
		m_image_keyframe = m_current_frame_backup;
		has_key_frame = true;
		m_image_keyframe.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
		m_image_keyframe.playback_info.end_frame_no = m_end_frame_no;
		m_image_keyframe.playback_info.is_key_frame = (m_image_keyframe.playback_info.frame_no == m_key_frame_no);
	}
	else
	{
		//判断缩略图中有没有当前帧
		//for (auto thumbnail : m_thumbnails)
		auto itr = m_thumbnails.begin();
		for (; itr != m_thumbnails.end(); itr++)
		{
			auto thumbnail = itr->second;
			if (thumbnail.playback_info.frame_no == key_frame_no)
			{
				m_image_keyframe = thumbnail;
				has_key_frame = true;
				m_image_keyframe.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
				m_image_keyframe.playback_info.end_frame_no = m_end_frame_no;
				m_image_keyframe.playback_info.is_key_frame = (m_image_keyframe.playback_info.frame_no == m_key_frame_no);
				break;
			}
		}
	}

	if (!has_key_frame)//没找到关键帧,重新加载
	{
		doLoadKeyFrame();
	}

	//m_image_changed = true;
	m_cv_ctrl.notify_one();
	RefreshDisplayThumbnails();

}

int64_t CSPlaybackController::GetKeyFrameNo()
{
	return m_key_frame_no;
}

void CSPlaybackController::ResetKeyFrameNo()
{
	if (m_origin_key_frame_no != -1)
	{
		SetKeyFrameNo(m_origin_key_frame_no);
	}
}

int64_t CSPlaybackController::GetOriginKeyFrameNo()
{
	return m_origin_key_frame_no;
}

void CSPlaybackController::PreviewKeyFrame()
{
	//获取关键帧图像
	if (m_key_frame_no >= 0 && (m_image_keyframe.image.isNull() || m_image_keyframe.playback_info.m_image_changed))
	{
		m_export_cacher_ptr->getFrame(m_key_frame_no, m_image_keyframe);
		m_export_cacher_ptr->adjustLuminanceAndContrast(m_image_keyframe, m_luminance, m_contrast, m_anti_color);
		m_image_keyframe.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
		m_image_keyframe.playback_info.end_frame_no = m_end_frame_no;
		m_image_keyframe.playback_info.is_key_frame = (m_image_keyframe.playback_info.frame_no == m_key_frame_no);
	}
	// 	emit ImageUpdate(m_image_keyframe);
		//切换到缩略图预览状态
	std::lock_guard<std::mutex> locker(m_playback_mutex);
	{
		m_preview_frame = m_image_keyframe;
		m_preview_frame.image.swap(m_image_keyframe.image);
	}
	
	m_thumbnail_previewing = true;
	m_cv_ctrl.notify_one();
}

void CSPlaybackController::SetLuminance(const int luminance)
{
	m_luminance = luminance;
	//m_image_changed = true;
	changeThumbnailsState(true);
	m_cv_ctrl.notify_one();
	RefreshDisplayThumbnails();
}

int CSPlaybackController::GetLuminance()
{
	return m_luminance;
}

void CSPlaybackController::SetContrast(const int contrast)
{
	m_contrast = contrast;
	//m_image_changed = true;
	changeThumbnailsState(true);
	m_cv_ctrl.notify_one();
	RefreshDisplayThumbnails();
}

int CSPlaybackController::GetContrast()
{
	return m_contrast;
}

void CSPlaybackController::SetAntiColorEnable(const bool enable)
{
	m_anti_color = enable;
	//m_image_changed = true;
	changeThumbnailsState(true);
	m_cv_ctrl.notify_one();
	RefreshDisplayThumbnails();
}

bool CSPlaybackController::isAntiColorEnable()
{
	return m_anti_color;
}

int64_t CSPlaybackController::GetVideoTotalFrameCount() const
{
	return m_video_total_frame_count;
}

void CSPlaybackController::StartLoadThumbnails()
{
	{
		std::lock_guard<std::mutex> lock(m_thumbnails_mutex);
		if (m_thumbnail_loading)
		{
			/*m_thumbnail_loading_thrd = false;
			m_thumbnail_cacher_ptr->stop(m_stop_mode_);

			if (m_thumbnail_timer.joinable()) {
				m_thumbnail_timer.join();
			}*/
			//std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
			m_thumbnail_cacher_ptr->stop(m_stop_mode_);
			m_thumbnail_loading = false;
		}
	}
	// 停止
	saveState();
	if (m_state != PlayState::PS_STOP)
	{
		SwitchState(PS_STOP);
	}

	//根据当前参数计算缩略图数量
	int frame_count = GetEndFrameNo() - GetStartFrameNo() + 1;
	m_thumbnail_count = (std::min)(frame_count, kDefaultThumbnailCount);
	m_display_thumbnail_count = (std::min)(m_thumbnail_count, kDefaultDisplayThumbnailCount);
	m_previous_thumbnail_center_span = -1;
	//计算需要显示的缩略图序号队列
	calcDisplayThumbnailIndexes();

	//清除以往缓存
	{
		std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
		m_thumbnails.clear();
		m_mini_thumbnails.clear();
	}
	PlaybackCacherParams cacher_params;
	cacher_params.startFrameNo = GetStartFrameNo();
	cacher_params.endFrameNo = GetEndFrameNo();

	m_thumbnail_frame_interval = getThumbnailExportInterval(cacher_params.startFrameNo, cacher_params.endFrameNo, m_thumbnail_count);
	cacher_params.frameInterval = m_thumbnail_frame_interval;
	calcThumbnailIndexes();//计算需要缓存的缩略图对应的实际帧号
	//刷新全部缩略图状态(正在载入,空)
	for (int i = 0; i < kDefaultDisplayThumbnailCount; i++)
	{
		RccFrameInfo frame_info{};
		if (i < m_display_thumbnail_count)
		{
			frame_info.playback_info.thumb_nail_state = PlaybackInfo::TN_LOADING;
			frame_info.playback_info.is_key_frame = (m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_key_frame_no);
			frame_info.playback_info.is_current_frame = (m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_current_frame_no);
			frame_info.playback_info.frame_no = m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i));
			frame_info.playback_info.start_frame_no = m_start_frame_no;
			frame_info.playback_info.end_frame_no = m_end_frame_no;
			frame_info.playback_info.m_image_changed = true;
		}
		else
		{
			frame_info.playback_info.frame_no = -1;
			frame_info.playback_info.thumb_nail_state = PlaybackInfo::TN_VOID;
		}
		emit thumbnailUpdated(i, frame_info);
	}
	//QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);


	//开始加载图像
	m_thumbnail_loading = true;
	m_thumbnail_loading_thrd = true;


	auto onAGFrame = [this](CAGBuffer* agFrame) {
		if (m_thumbnail_loading)
		{
			onThumbnailLoading(agFrame);
		}
	};
	sThumbnailTrack.reset();
	sThumbnailTrack.mInRecState = false;
#ifdef _WINDOWS
	sThumbnailTrack.mBeginEnter = GetTickCount64();
#else
	sThumbnailTrack.mBeginEnter = UtilityNS::CTimeUtil::getTickCount();
#endif
	sThumbnailTrack.mBeginThumbnail = sThumbnailTrack.mBeginEnter;
	
	if (!m_thumbnail_cacher_ptr->start(cacher_params, onAGFrame))
	{
		CSLOG_INFO("CSPlaybackController::StartLoadThumbnails thumbnailLoadingFinished for start failed.");
		emit thumbnailLoadingFinished(false);//载入失败,停止
		m_thumbnail_loading = false;

		restoreState();

		return;
	}


	//开启缩略图刷新计时器
	emit signalStartLoadingThumbnailTimers();

}

void CSPlaybackController::calcThumbnailIndexes()
{
	m_thumbnail_indexes.clear();
	//从已加载的缩略图cacher中均匀选择出需要缓存的缩略图序号
	for (int i = 0; i < m_thumbnail_count; i++)
	{
		int index = (i  * m_thumbnail_frame_interval) + m_start_frame_no;
		m_thumbnail_indexes.push_back(index);
	}
}

void CSPlaybackController::calcDisplayThumbnailIndexes()
{
	m_display_thumbnail_indexes.clear();
	//从已缓存的缩略图队列中均匀选择出需要显示的缩略图序号
	for (int i = 0; i < m_display_thumbnail_count - 1; i++)
	{
		int index = m_thumbnail_count*i / (m_display_thumbnail_count - 1);
		m_display_thumbnail_indexes.push_back(index);
	}
	//同步最后一张
	m_display_thumbnail_indexes.push_back(m_thumbnail_count - 1);
}

void CSPlaybackController::RefreshDisplayThumbnails(int height_light_index)
{
	//发送需要显示的缩略图到界面
	std::unique_lock<std::mutex> locker(m_thumbnails_mutex);

	for (int i = 0; i < m_display_thumbnail_count; i++)
	{
		int current_display_index = m_display_thumbnail_indexes.at(i);
		auto itr = m_mini_thumbnails.find(current_display_index);
		if (itr != m_mini_thumbnails.end())
		//if (current_display_index < m_mini_thumbnails.size())//加载到了
		{
			// 调节亮度对比度
			//RccFrameInfo frame_image_adjust = m_mini_thumbnails.at(current_display_index);
			RccFrameInfo frame_image_adjust = m_mini_thumbnails[current_display_index];
			if (height_light_index == i)//判断是不是高亮帧
			{
				frame_image_adjust.playback_info.is_highlight_frame = true;
			}
			//判断是不是关键帧
			frame_image_adjust.playback_info.is_key_frame = (frame_image_adjust.playback_info.frame_no == m_key_frame_no);
			frame_image_adjust.playback_info.is_current_frame = (frame_image_adjust.playback_info.frame_no == m_current_frame_no);
			if ((frame_image_adjust.playback_info.is_key_frame && frame_image_adjust.playback_info.frame_no != m_last_key_frame_no)
				|| frame_image_adjust.playback_info.frame_no == m_last_key_frame_no ) frame_image_adjust.playback_info.m_image_changed = true;
			//else frame_image_adjust.playback_info.m_image_changed = m_image_changed;

			m_thumbnail_cacher_ptr->adjustLuminanceAndContrast(frame_image_adjust, m_luminance, m_contrast, m_anti_color);
			m_mini_thumbnails[current_display_index].playback_info.m_image_changed = false;
			locker.unlock();
			emit thumbnailUpdated(i, frame_image_adjust);

		}
		else//没加载到
		{
			RccFrameInfo frame_info{};
			if (height_light_index == i)//判断是不是高亮帧
			{
				frame_info.playback_info.is_highlight_frame = true;
			}
			//判断是不是关键帧
			frame_info.playback_info.is_key_frame = (m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_key_frame_no);
			frame_info.playback_info.is_current_frame = (m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_current_frame_no);
			frame_info.playback_info.frame_no = m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i));
			frame_info.playback_info.start_frame_no = m_start_frame_no;
			frame_info.playback_info.end_frame_no = m_end_frame_no;
			frame_info.playback_info.thumb_nail_state = m_thumbnail_loading ? PlaybackInfo::TN_LOADING : PlaybackInfo::TN_FAIL;
			locker.unlock();
			emit thumbnailUpdated(i, frame_info);
		}
		locker.lock();
	}
}

// void CSPlaybackController::updateCurrentThumbnailState()
// {
// 	if (m_thumbnails.size() == 0)
// 	{
// 		return;
// 	}
// 	std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
// 
// 	//去除原缩略图的当前帧状态
// 	for (int i = 0; i < m_display_thumbnail_count; i++)
// 	{
// 		int current_thumbnail_index = m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i));
// 		if (m_display_thumbnail_indexes.at(i) < m_thumbnails.size())
// 		{
// 			if (current_thumbnail_index == m_last_frame_no && current_thumbnail_index != m_current_frame_no)//去除当前帧状态(重新设置普通状态)
// 			{
// 				RccFrameInfo thumbnail = m_thumbnails.at(m_display_thumbnail_indexes.at(i));
// 				//判断是不是关键帧
// 				thumbnail.playback_info.is_key_frame = (thumbnail.playback_info.frame_no == m_key_frame_no);
// 				m_thumbnail_cacher_ptr->adjustLuminanceAndContrast(thumbnail, m_luminance, m_contrast, m_anti_color);
// 				emit thumbnailUpdated(i, thumbnail);
// 			}
// 			if (current_thumbnail_index == m_current_frame_no)//添加当前帧状态(重新设置其他状态)
// 			{
// 				RccFrameInfo thumbnail = m_thumbnails.at(m_display_thumbnail_indexes.at(i));
// 				//判断是不是关键帧
// 				thumbnail.playback_info.is_key_frame = (thumbnail.playback_info.frame_no == m_key_frame_no);
// 				m_thumbnail_cacher_ptr->adjustLuminanceAndContrast(thumbnail, m_luminance, m_contrast, m_anti_color);
// 				thumbnail.playback_info.is_current_frame = true;
// 				emit thumbnailUpdated(i, thumbnail);
// 			}
// 		}
// 	}
// }

void CSPlaybackController::onThumbnailLoading(CAGBuffer* agFrame) {
	//std::unique_lock<std::mutex> locker(m_thumbnails_mutex);
	std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
	int thumbnail_index = m_thumbnails.size();
	if (thumbnail_index == m_thumbnail_count) {
		return;
	}
	if (thumbnail_index >= m_thumbnail_indexes.size())
	{
		//locker.unlock();
		return;
	}
	int frame_index = agFrame->frame_head.frameno; // m_thumbnail_indexes.at(thumbnail_index);
	RccFrameInfo frame_image;
	RccFrameInfo frame_image_mini;//小尺寸缩略图
	// CSLOG_INFO("thumbnail - SPlaybackController::onThumbnailLoading:{}", frame_index);
	if (m_thumbnail_cacher_ptr->getFrame(frame_index, frame_image, false))
	{
		frame_image.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
		frame_image.playback_info.end_frame_no = m_end_frame_no;
		frame_image.playback_info.thumb_nail_state = PlaybackInfo::TN_NORMAL;//加载成功
		frame_image_mini.playback_info.start_frame_no = m_start_frame_no;//补充帧数信息
		frame_image_mini.playback_info.end_frame_no = m_end_frame_no;
		frame_image_mini.playback_info.thumb_nail_state = PlaybackInfo::TN_NORMAL;//加载成功
		frame_image_mini.playback_info.frame_no = frame_image.playback_info.frame_no;
		frame_image_mini.image = frame_image.image.scaledToWidth(frame_image.image.width() / 4);//单边缩小4倍
		
		//m_thumbnails.push_back(frame_image);
		//m_mini_thumbnails.push_back(frame_image_mini);
		auto tmp_count = m_display_thumbnail_count;
		auto tmp_indexes = m_display_thumbnail_indexes;
		//locker.unlock();

		//发送需要显示的缩略图到界面
		for (int i = 0; i < tmp_count; i++)
		{
			if (i < m_thumbnail_indexes.size() && m_thumbnail_indexes.at(i) == frame_index)
			{
				m_thumbnails[i] = frame_image;
				m_mini_thumbnails[i] = frame_image_mini;

				// 调节亮度对比度
				RccFrameInfo frame_image_adjust = frame_image_mini;
				m_thumbnail_cacher_ptr->adjustLuminanceAndContrast(frame_image_adjust, m_luminance, m_contrast, m_anti_color);
				//判断是不是关键帧
				frame_image_adjust.playback_info.is_key_frame = (frame_image_adjust.playback_info.frame_no == m_key_frame_no);
				frame_image_adjust.playback_info.is_current_frame = (frame_image_adjust.playback_info.frame_no == m_current_frame_no);
				frame_image_adjust.playback_info.m_image_changed = true;
				CSLOG_INFO("CSPlaybackController::onThumbnailLoading thumbnailUpdated frame_index={},i={},thumbnail_index={}", frame_index, i, thumbnail_index);
				emit thumbnailUpdated(i, frame_image_adjust);
				break;
			}
		}
		//locker.lock();

		//std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
		m_thumbnail_elapsed_timer.restart();//重新开始计算超时
		size_t thumbnails_count = m_thumbnails.size();
		int tmp_thumbnail_count = m_thumbnail_count;
		//locker.unlock();

		if (thumbnails_count >= tmp_thumbnail_count)//加载到了全部缩略图
		{
			m_thumbnail_timer_ptr->stop();
			std::unique_lock<std::mutex> switch_range_locker(m_switch_range_mutex);
			if (m_switch_range_flag == 1)
			{
				switch_range_locker.unlock();
				return;
			}
			m_switch_range_flag = 1;
			//QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
			stopLoadingThumbnails();
			CSLOG_INFO("CSPlaybackController::onThumbnailLoading thumbnailLoadingFinished for finishing loading.");
			if (m_thumbnail_load_finish_func_) m_thumbnail_load_finish_func_(true);
			switch_range_locker.unlock();
			emit thumbnailLoadingFinished(true);
			
			return;
		}
	}
	//else locker.unlock();
}

void CSPlaybackController::startThumbnailLoadingTimers()
{
	m_test_count = 0;
	CSLOG_INFO("startThumbnailLoadingTimers");
	m_thumbnail_elapsed_timer.start();//开始记录是否超时
	m_thumbnail_timer_ptr->start();
	//m_thumbnail_loading_thrd = false;
	//if (m_thumbnail_timer.joinable()) {
	//	m_thumbnail_timer.join();
	//}
	//m_thumbnail_loading_thrd = true;
	//m_thumbnail_loading = true;
	//m_thumbnail_timer = std::thread(
	//	[this]() {
	//	while (m_thumbnail_loading_thrd) {
	//		if (m_test_count == 0)
	//		{
	//			CSLOG_INFO("start onThumbnailLoading");
	//			m_test_count++;
	//		}
	//		onThumbnailLoading();
	//		boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
	//	}
	//});
}

void CSPlaybackController::PreviewThumbnails(const int64_t center_frame_index)
{
	//计算以center_frame为中心的缩略图编号
	//找到中心帧对应的区间
	Q_ASSERT(center_frame_index >= m_start_frame_no && center_frame_index <= m_end_frame_no);
	int center_span_index = -1;
	for (int i = m_thumbnail_count - 1; i >= 0; i--)
	{
		if (center_frame_index >= m_thumbnail_indexes.at(i))
		{
			//中心帧属于第i区间
			center_span_index = i;
			break;
		}
	}
	//判断是否与之前的预览区间相同,如果相同则不刷新
	if (center_span_index == -1 ||
		center_span_index == m_previous_thumbnail_center_span)
	{
		return;
	}
	else
	{
		m_previous_thumbnail_center_span = center_span_index;
	}

	//确定帧编号区间 start+count -1 ==end
	int display_start_index = 0;
	int display_end_index = 0;
	if (center_span_index - m_display_thumbnail_count / 2 < 0)//贴左边缘
	{
		display_start_index = 0;
		display_end_index = display_start_index + m_display_thumbnail_count - 1;
	}
	else if ((center_span_index + m_display_thumbnail_count / 2 - (m_display_thumbnail_count % 2 == 0))
	> (m_thumbnail_count - 1))//贴右边缘
	{
		display_end_index = m_thumbnail_count - 1;
		display_start_index = display_end_index - m_display_thumbnail_count + 1;
	}
	else//中间
	{
		display_start_index = center_span_index - m_display_thumbnail_count / 2;
		display_end_index = center_span_index + m_display_thumbnail_count / 2 - (m_display_thumbnail_count % 2 == 0);
	}

	m_display_thumbnail_indexes.clear();//重新计算显示缩略图编号
	for (int i = 0; i < m_display_thumbnail_count; i++)
	{
		if (display_start_index + i <= display_end_index)
		{
			m_display_thumbnail_indexes.push_back(display_start_index + i);
		}
	}

	//找到对应的缩略图序号
	int highlight_index = -1;
	for (int i = 0; i < m_display_thumbnail_count; i++)
	{
		if (center_span_index == m_display_thumbnail_indexes.at(i))
		{
			highlight_index = i;
			break;
		}
	}

	emit signalHighlightThumbnail(highlight_index);
}

void CSPlaybackController::PreviewSingleThumbnail(const int64_t display_thumbnail_index)
{
	if (display_thumbnail_index < 0 || display_thumbnail_index >= m_display_thumbnail_indexes.size())
	{
		return;
	}
	std::lock_guard<std::mutex> locker(m_thumbnails_mutex);
	//if (m_display_thumbnail_indexes.at(display_thumbnail_index) >= m_thumbnails.size())//还没加载到该缩略图
	auto itr = m_thumbnails.find(m_display_thumbnail_indexes.at(display_thumbnail_index));
	if (itr == m_thumbnails.end())
	{
		return;
	}
	//发送需要显示的缩略图到界面
	// 调节亮度对比度
	//RccFrameInfo frame_image_adjust = m_thumbnails.at(m_display_thumbnail_indexes.at(display_thumbnail_index));
	RccFrameInfo frame_image_adjust = m_thumbnails[m_display_thumbnail_indexes.at(display_thumbnail_index)];
	m_thumbnail_cacher_ptr->adjustLuminanceAndContrast(frame_image_adjust, m_luminance, m_contrast, m_anti_color);
	frame_image_adjust.playback_info.is_key_frame = (frame_image_adjust.playback_info.frame_no == m_key_frame_no);
	// 	emit ImageUpdate(frame_image_adjust);
		//切换到缩略图预览状态
	{
		std::lock_guard<std::mutex> locker(m_playback_mutex);
		m_preview_frame = frame_image_adjust;
		m_preview_frame.image.swap(frame_image_adjust.image);
	}
	m_thumbnail_previewing = true;
	m_cv_ctrl.notify_one();
}

void CSPlaybackController::ResetThumbnails()
{
	m_previous_thumbnail_center_span = -1;
	calcDisplayThumbnailIndexes();//重新计算
	RefreshDisplayThumbnails();
}

void CSPlaybackController::changeThumbnailsState(bool image_changed)
{
	for (int i = 0; i < m_display_thumbnail_count; i++)
	{
		int current_display_index = m_display_thumbnail_indexes.at(i);
		auto itr = m_mini_thumbnails.find(current_display_index);
		if (itr != m_mini_thumbnails.end())
		//if (current_display_index < m_mini_thumbnails.size())
		{
			//auto &thumbnail = m_mini_thumbnails.at(current_display_index);
			auto &thumbnail = itr->second;
			thumbnail.playback_info.m_image_changed = image_changed;
		}
	}
	m_image_keyframe.playback_info.m_image_changed = image_changed;
}


void CSPlaybackController::checkTimeOutThumbnails()
{
	if (!m_thumbnail_elapsed_timer.hasExpired(kThumbnailTimeout)) return;
	std::unique_lock<std::mutex> locker(m_thumbnails_mutex);

	//补齐未加载图像状态
	RccFrameInfo frame_info{};
	for (int i = 0; i < m_display_thumbnail_count; i++)
	{
		//if (m_display_thumbnail_indexes.at(i) >= m_thumbnails.size())
		auto itr = m_thumbnails.find(m_display_thumbnail_indexes.at(i));
		if (itr == m_thumbnails.end())
		{
			//判断是不是关键帧
			frame_info.playback_info.is_key_frame =
				(m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_key_frame_no);
			frame_info.playback_info.is_current_frame =
				(m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i)) == m_current_frame_no);
			frame_info.playback_info.thumb_nail_state = PlaybackInfo::TN_FAIL;
			frame_info.playback_info.frame_no = m_thumbnail_indexes.at(m_display_thumbnail_indexes.at(i));
			frame_info.playback_info.start_frame_no = m_start_frame_no;
			frame_info.playback_info.end_frame_no = m_end_frame_no;

			emit thumbnailUpdated(i, frame_info);
		}
	}
	
	if (m_thumbnail_loading == false) {
		locker.unlock();
		return;
	}
	stopLoadingThumbnails();
	CSLOG_INFO("CSPlaybackController::checkTimeOutThumbnails thumbnailLoadingFinished for timeout.");
	m_thumbnail_timer_ptr->stop();
	if (m_thumbnail_load_finish_func_) m_thumbnail_load_finish_func_(true);
	emit thumbnailLoadingFinished(false);
	locker.unlock();
	return;
}
