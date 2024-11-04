#include "playbackplayercontroller.h"
#include "PlaybackCacher/playbackcacher.h"

#include <QTimer>
#include <QElapsedTimer>

#include "HscAPIStructer.h"
#include "PlayerParams.h"
#include "MediaInfo.h"
#include "Device/device.h"

PlaybackPlayerController::PlaybackPlayerController(QSharedPointer<Device> device_ptr, QSharedPointer<HscVideoClipInfo> video_segment_ptr) : device_wptr_(device_ptr)
{
    media_info_ptr_.reset(new MediaInfo(QStringList({ device_ptr->getIp() }),
                                        QString(),
                                        video_segment_ptr->id,
                                        device_ptr->getModelName(),
                                        device_ptr->getIp(),
                                        QString(),
                                        VIDEO_FMT_IPC,
                                        video_segment_ptr->display_roi.width,
                                        video_segment_ptr->display_roi.height,
                                        PIXEL_FMT_BGR24,
                                        COLOR_PATTERN_COLOR, video_segment_ptr->fps,
                                        0,
                                        video_segment_ptr->frame_num));

    export_cacher_ptr_.reset(new PlaybackCacher(device_ptr, video_segment_ptr->id));
    thumbnail_cacher_ptr_.reset(new PlaybackCacher(device_ptr, video_segment_ptr->id, false));
    thumbnail_timer_ptr_ = new QTimer;
    thumbnail_timer_ptr_->setInterval(50);
    QObject::connect(thumbnail_timer_ptr_.data(), &QTimer::timeout, this, &PlaybackPlayerController::onThumbnailLoading);
    rma_image_.reset(new RMAImage);
    player_params_ptr_.reset(new PlayerParams(video_segment_ptr->frame_num));

	start_frame_no_ = 0;
	end_frame_no_ = video_segment_ptr->frame_num - 1;
	play_speed_ = 1;

    ctrl_thread_ = std::thread(&PlaybackPlayerController::doUpdate, this);
}

PlaybackPlayerController::~PlaybackPlayerController()
{
    ctrl_running_ = false;
    cv_ctrl_.notify_one();
    if (ctrl_thread_.joinable())
    {
        ctrl_thread_.join();
    }
}

void PlaybackPlayerController::Play()
{
    if (state_ == State::kPlay)
    {
        return;
    }

    state_ = State::kPlay;

    if (cacher_params_.startFrameNo != start_frame_no_ || cacher_params_.endFrameNo != end_frame_no_ || cacher_params_.frameInterval != play_speed_)
    {
        if (export_cacher_ptr_)
        {
            PlaybackCacherParams cacher_params;
            cacher_params.startFrameNo = start_frame_no_;
            cacher_params.endFrameNo = end_frame_no_;
            cacher_params.frameInterval = play_speed_;
            cacher_params.curFrameNo = getCurrentFrameNo();

            if (export_cacher_ptr_->start(cacher_params))
            {
                cacher_params_ = cacher_params;
            }
            else
            {
                state_ = State::kStop;
            }
        }
    }

    cv_ctrl_.notify_one();
}

void PlaybackPlayerController::Pause()
{
    if (state_ == State::kStop)
    {
        state_ = State::kPause;

        PlaybackCacherParams cacher_params;
        cacher_params.startFrameNo = start_frame_no_;
        cacher_params.endFrameNo = end_frame_no_;
        cacher_params.frameInterval = play_speed_;
        cacher_params.curFrameNo = getCurrentFrameNo();

        if (export_cacher_ptr_->start(cacher_params))
        {
            cacher_params_ = cacher_params;
        }
        else
        {
            state_ = State::kStop;
        }
    }
    else
    {
        state_ = State::kPause;
    }
}

void PlaybackPlayerController::Stop()
{
    if (state_ == State::kStop)
    {
        return;
    }

    if (export_cacher_ptr_)
    {
        export_cacher_ptr_->stop();
    }

    cacher_params_ = {};

    state_ = State::kStop;
}

void PlaybackPlayerController::NextFrame()
{
    setCurrentFrameNo(getNextFrameNo());

    cv_ctrl_.notify_one();
}

void PlaybackPlayerController::PreviousFrame()
{
    setCurrentFrameNo(getPreviousFrameNo());

    cv_ctrl_.notify_one();
}

void PlaybackPlayerController::SeekFrame(FRAME_INDEX index, bool brel_index)
{
    setCurrentFrameNo(index);

    cv_ctrl_.notify_one();
}

bool PlaybackPlayerController::GetImage(REL FRAME_INDEX index, RMAImage& image, bool brel_index /*= true*/) const
{
	if (export_cacher_ptr_)
	{
		if (export_cacher_ptr_->getFrame(index, *rma_image_))
		{
			image = *rma_image_;
			return true;
		}
	}
	return false;
}

void PlaybackPlayerController::SpeedTimes(unsigned char times, SpeedTimeMethod method)
{
    if (method != SpeedTimeMethod::SPEED_TIME_METHOD_SKIP_FRAME)
    {
        return;
    }

    if (play_speed_ == times)
    {
        return;
    }

    // 停止
    saveState();
    if (previous_state_ != State::kStop)
    {
        Stop();
    }

    // 设置
    play_speed_ = times;
    player_params_ptr_->SetFrameStep(times);

    // 恢复
    restoreState();
}

std::shared_ptr<PlayerParams> PlaybackPlayerController::GetPlayerParams()
{
    return player_params_ptr_;
}

std::shared_ptr<MediaInfo> PlaybackPlayerController::GetVideoInfo() const
{
    return media_info_ptr_;
}

void PlaybackPlayerController::SetEditPos(FRAME_INDEX begin, FRAME_INDEX end)
{
    if (start_frame_no_ == begin && end_frame_no_ == end)
    {
        return;
    }

    // 停止
    State state = state_;
    if (state != State::kStop)
    {
        Stop();
    }

    // 设置
    start_frame_no_ = begin;
    end_frame_no_ = end;
    player_params_ptr_->SetEditPos(begin, end);

    // 恢复
    if (state == State::kPlay)
    {
        Play();
    }
    else if (state != State::kPause)
    {
        Pause();
    }
}

void PlaybackPlayerController::EnableLoopPlay(bool enabled)
{
	player_params_ptr_->EnableLoopPlay(enabled);
}

qint64 PlaybackPlayerController::getCurrentFrameNo() const
{
    std::lock_guard<std::mutex> lock(playback_mutex_);

    return current_frame_no_;
}

qint64 PlaybackPlayerController::getPreviousFrameNo() const
{
    std::lock_guard<std::mutex> lock(playback_mutex_);

	qint64 previous_frame_no = current_frame_no_;
	previous_frame_no = player_params_ptr_->GetPreviousFrameIndex_Absolute(previous_frame_no);

    return previous_frame_no;
}

qint64 PlaybackPlayerController::getNextFrameNo() const
{
    std::lock_guard<std::mutex> lock(playback_mutex_);

	qint64 next_frame_no = current_frame_no_;
	next_frame_no = player_params_ptr_->GetNextFrameIndex_Absolute(next_frame_no);

    return next_frame_no;
}

void PlaybackPlayerController::setCurrentFrameNo(qint64 frame_no)
{
    std::lock_guard<std::mutex> lock(playback_mutex_);

    current_frame_no_ = frame_no;
    if (current_frame_no_ < start_frame_no_)
    {
        current_frame_no_ = start_frame_no_;
    }

    if (current_frame_no_ > end_frame_no_)
    {
        current_frame_no_ = end_frame_no_;
    }

	if (last_frame_no_ == current_frame_no_)
	{
		last_frame_no_ -= 1;
	}
}

void PlaybackPlayerController::doUpdate()
{
    std::chrono::system_clock::time_point current_time_point;

    while (ctrl_running_)
    {
        {
            std::unique_lock<std::mutex> locker(playback_mutex_);
            cv_ctrl_.wait(locker, [this]{
                if (!ctrl_running_)
                {
                    return true;
                }

                if (state_ == State::kPlay)
                {
                    return true;
                }

                if (state_ == State::kPause)
                {
                    if (last_frame_no_ != current_frame_no_)
                    {
                        return true;
                    }
                }

                return false;
            });
        }

        current_time_point = std::chrono::system_clock::now();

        RMAImage rma_image;
        if (!export_cacher_ptr_ || !export_cacher_ptr_->getFrame(current_frame_no_, rma_image))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
        }

		// 调节亮度对比度
		export_cacher_ptr_->adjustLuminanceAndContrast(rma_image, luminance_, contrast_);

        last_frame_no_ = current_frame_no_;
        emit sigImageReady(rma_image);

        if (state_ == State::kPlay)
        {
            setCurrentFrameNo(getNextFrameNo());
        }

        std::this_thread::sleep_until(current_time_point += play_period_);
    }
}

void PlaybackPlayerController::startLoadingThumbnails(int thumbnail_count)
{
    if (thumbnail_loading_)
    {
        stopLoadingThumbnails();
    }
    thumbnail_count_ = thumbnail_count;
    thumbnail_loading_ = true;



    // 停止
    saveState();
    if (previous_state_ != State::kStop)
    {
        Stop();
    }

    PlaybackCacherParams cacher_params;
    cacher_params.startFrameNo = start_frame_no_;
    cacher_params.endFrameNo = end_frame_no_;
    if (thumbnail_count_ > end_frame_no_ - start_frame_no_ + 1)
    {
        thumbnail_count_ = end_frame_no_ - start_frame_no_ + 1;
    }
	thumbnail_frame_interval_ = getThumbnailExportInterval(cacher_params.startFrameNo, cacher_params.endFrameNo, thumbnail_count_);
	cacher_params.frameInterval = thumbnail_frame_interval_;
    if (!thumbnail_cacher_ptr_->start(cacher_params))
    {
        emit thumbnailLoadingFinished(false);
        thumbnail_loading_ = false;

        restoreState();

        return;
    }

	{
		std::lock_guard<std::mutex> locker(thumbnails_mutex_);
		thumbnails_.clear();
	}
    thumbnail_elapsed_timer_.start();
    thumbnail_timer_ptr_->start();
}

bool PlaybackPlayerController::isThumbnailsLoaded() const
{
	std::lock_guard<std::mutex> locker(thumbnails_mutex_);
	return !thumbnails_.empty();
}

void PlaybackPlayerController::setCurrentThumbnail(int thumbnail_index)
{
	RMAImage rma_image;
	{
		std::lock_guard<std::mutex> locker(thumbnails_mutex_);

		if (0 <= thumbnail_index && thumbnail_index < thumbnails_.size())
		{
			rma_image = thumbnails_.at(thumbnail_index);
		}
	}

	if (!rma_image.Empty())
	{
		SeekFrame(rma_image.GetAbsoluteFrameIndex());

		emit sigImageReady(rma_image);
	}
}

void PlaybackPlayerController::updateThumbnailsLuminanceAndContrast()
{
	std::lock_guard<std::mutex> locker(thumbnails_mutex_);

	int thumbnail_index = thumbnails_.size();
	for (int i = 0; i < thumbnail_index; ++i)
	{
		RMAImage rma_image = thumbnails_.at(i);
		// 调节亮度对比度
		thumbnail_cacher_ptr_->adjustLuminanceAndContrast(rma_image, luminance_, contrast_);

		emit thumbnailUpdated(i, rma_image);
	}
}

QSharedPointer<Device> PlaybackPlayerController::getDevice() const
{
	return device_wptr_.lock();
}

void PlaybackPlayerController::suspend()
{
	saveState();

	Stop();
}

void PlaybackPlayerController::resume()
{
	restoreState();
}

void PlaybackPlayerController::setLuminanceAndContrast(const int luminance, const int contrast)
{
	luminance_ = luminance;
	contrast_ = contrast;
}

qint64 PlaybackPlayerController::getThumbnailExportInterval(qint64 start_frame_no, qint64 end_frame_no, qint64 thumbnail_count)
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

void PlaybackPlayerController::stopLoadingThumbnails()
{
    thumbnail_cacher_ptr_->stop();
    thumbnail_timer_ptr_->stop();

    // 恢复
    restoreState();

    thumbnail_loading_ = false;
}

void PlaybackPlayerController::onThumbnailLoading()
{
     if (thumbnail_elapsed_timer_.hasExpired(kThumbnailTimeout))
     {
        stopLoadingThumbnails();
        emit thumbnailLoadingFinished(false);
        return;
     }

	 std::lock_guard<std::mutex> locker(thumbnails_mutex_);

     int thumbnail_index = thumbnails_.size();
	 int frame_index = start_frame_no_ + thumbnail_index * thumbnail_frame_interval_;
     RMAImage rma_image;
     if (thumbnail_cacher_ptr_->getFrame(frame_index, rma_image, false))
     {
          thumbnails_.push_back(rma_image);

		  // 调节亮度对比度
		  RMAImage rma_image_adjust = rma_image;
		  thumbnail_cacher_ptr_->adjustLuminanceAndContrast(rma_image_adjust, luminance_, contrast_);

          emit thumbnailUpdated(thumbnail_index, rma_image_adjust);

		  thumbnail_elapsed_timer_.restart();

          if (thumbnails_.size() >= thumbnail_count_)
          {
              stopLoadingThumbnails();
              emit thumbnailLoadingFinished(true);
              return;
          }
     }
}

void PlaybackPlayerController::saveState()
{
    previous_state_ = state_;
}

void PlaybackPlayerController::restoreState()
{
    if (previous_state_ == State::kPlay)
    {
        Play();
    }
    else if (previous_state_ != State::kPause)
    {
        Pause();
    }
}
