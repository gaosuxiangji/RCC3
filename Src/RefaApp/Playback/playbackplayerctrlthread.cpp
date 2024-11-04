#include "playbackplayerctrlthread.h"
#include <PlayerControllerInterface.h>
#include <PlayerParams.h>
#include <QMutexLocker>

PlaybackPlayerCtrlThread::PlaybackPlayerCtrlThread(QObject *parent)
    : QThread(parent)
{
}

PlaybackPlayerCtrlThread::PlaybackPlayerCtrlThread(std::shared_ptr<PlayerControllerInterface> pctrl, QObject *parent/* = Q_NULLPTR*/)
    : QThread(parent)
    , pplayer_ctrl_(pctrl)
{
}

void PlaybackPlayerCtrlThread::run()
{
    QMutexLocker locker(&mutex_);
    if(!pplayer_ctrl_)
        return;
    switch (current_opt_)
    {
    case PLAY_OPT_PAUSE:
        pplayer_ctrl_->Pause();
        break;
	case PLAY_OPT_STOP:
		pplayer_ctrl_->Stop();
		break;
    case PLAY_OPT_FORWARD_PALY:
		pplayer_ctrl_->SpeedTimes(1);
        pplayer_ctrl_->Play();
        break;
    case PLAY_OPT_BACKWARD_PLAY:
        break;
    case PLAY_OPT_FAST_FORWARD:
        pplayer_ctrl_->SpeedTimes(kPlaySpeed_);
		pplayer_ctrl_->Play();
        break;
    case PLAY_OPT_FAST_BACKWARD:
        break;
    case PLAY_OPT_FORWARD:
		// 前进1帧/后退1帧需要暂停播放且前进/后退量为1
		// 设置播放间隔后再次设置暂停是因为SpeedTimes会将模式设为stop，此时再向前/后1帧不生效，需要将其设为暂停
		pplayer_ctrl_->Pause();
		pplayer_ctrl_->SpeedTimes(1);//前进1帧设置时抽帧间隔设为1
		pplayer_ctrl_->Pause();
        pplayer_ctrl_->NextFrame();
        break;
    case PLAY_OPT_BACKWARD:
		pplayer_ctrl_->Pause();
		pplayer_ctrl_->SpeedTimes(1);//后退1帧设置时抽帧间隔设为1
		pplayer_ctrl_->Pause();
        pplayer_ctrl_->PreviousFrame();
        break;
    case PLAY_OPT_SET_PLAYER_RANGE:
		pplayer_ctrl_->Pause();
        if(bgein_frameno_ > end_frameno_)
            std::swap(bgein_frameno_, end_frameno_);
        if(bgein_frameno_ < 0)
            bgein_frameno_ = 0;
        if(end_frameno_ >= pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute())
            end_frameno_ = pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute() - 1;
        pplayer_ctrl_->SetEditPos(bgein_frameno_, end_frameno_);
        break;
    case PLAY_OPT_SET_CURRENT_FRAME:
		pplayer_ctrl_->Pause();
        if(current_frameno_ < 0)
            current_frameno_ = 0;
        else if(current_frameno_ >= pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute())
            current_frameno_ = pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute() - 1;
        pplayer_ctrl_->SeekFrame(current_frameno_, false);
        break;
	case PLAY_OPT_SET_PLAYER_BEGIN:
		if (bgein_frameno_ < 0)
			bgein_frameno_ = 0;
		end_frameno_ = pplayer_ctrl_->GetPlayerParams()->GetEndIndex();
		if (bgein_frameno_ > end_frameno_)
			std::swap(bgein_frameno_, end_frameno_);
		pplayer_ctrl_->SetEditPos(bgein_frameno_, end_frameno_);
		break;
	case PLAY_OPT_SET_PLAYER_END:
		if (end_frameno_ >= pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute())
			end_frameno_ = pplayer_ctrl_->GetPlayerParams()->GetTotalFrameCnt_Absolute() - 1;
		bgein_frameno_ = pplayer_ctrl_->GetPlayerParams()->GetBeginIndex();
		if (bgein_frameno_ > end_frameno_)
			std::swap(bgein_frameno_, end_frameno_);
		pplayer_ctrl_->SetEditPos(bgein_frameno_, end_frameno_);
		break;
    default:
        break;
    }
}

void PlaybackPlayerCtrlThread::resetPlayerController(std::shared_ptr<PlayerControllerInterface> pctrl)
{
	if (isRunning())
	{
		terminate();
		wait();
	}
    QMutexLocker locker(&mutex_);
    pplayer_ctrl_ = pctrl;
    bgein_frameno_ = 0;
    end_frameno_ = 0;
    current_frameno_ = 0;
}

void PlaybackPlayerCtrlThread::setPlayOperation(PlayOperationType opt)
{
    QMutexLocker locker(&mutex_);
    current_opt_ = opt;
}

PlayOperationType PlaybackPlayerCtrlThread::getPlayOperation() const
{
	QMutexLocker locker(&mutex_);
	return current_opt_;
}

void PlaybackPlayerCtrlThread::setPlayerRange(FRAME_INDEX begin, FRAME_INDEX end)
{
    QMutexLocker locker(&mutex_);
    bgein_frameno_ = begin;
    end_frameno_ = end;
}

void PlaybackPlayerCtrlThread::setPlayerBegin(FRAME_INDEX begin)
{
	QMutexLocker locker(&mutex_);
	bgein_frameno_ = begin;
}

void PlaybackPlayerCtrlThread::setPlayerEnd(FRAME_INDEX end)
{
	QMutexLocker locker(&mutex_);
	end_frameno_ = end;
}

void PlaybackPlayerCtrlThread::setCurrentFrameNo(FRAME_INDEX index)
{
    QMutexLocker locker(&mutex_);
    current_frameno_ = index;
}
