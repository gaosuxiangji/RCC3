#ifndef REALTIMEPLAYERCONTROLLER_H
#define REALTIMEPLAYERCONTROLLER_H

#include "PlayerControllerInterface.h"

class RealtimePlayerController : public PlayerControllerInterface
{
    Q_OBJECT

public:
    RealtimePlayerController(const QString & device_ip);

    virtual void Play() override {}
    virtual void Pause() override {}
    virtual void Stop() override {}
    virtual void NextFrame() override {}
    virtual void PreviousFrame() override {}
    virtual void SeekFrame(REL FRAME_INDEX index, bool brel_index = true) override {}
	virtual bool GetImage(REL FRAME_INDEX index, RMAImage& image, bool brel_index = true) const override { return false; };
    virtual void SpeedTimes(unsigned char times, SpeedTimeMethod method = SPEED_TIME_METHOD_SKIP_FRAME) override {}
    virtual TIMESTAMP GetTimeStamp(ABS FRAME_INDEX index) { return 0; }
	virtual std::shared_ptr<PlayerParams> GetPlayerParams() override;
    virtual std::shared_ptr<MediaInfo> GetVideoInfo() const override;
    virtual std::shared_ptr<ISPUtil> GetISPHandle() override { return std::shared_ptr<ISPUtil>(); }
    virtual void Reset() override {}
    virtual void EnableFpsControl(bool enable) override {}
    virtual void SetEditPos(ABS FRAME_INDEX begin, ABS FRAME_INDEX end) override {}
    virtual void EnableLoopPlay(bool) override {}
	virtual void Close() override {};

private:
    std::shared_ptr<MediaInfo> media_info_ptr_;
	std::shared_ptr<PlayerParams> player_params_ptr_;
};

#endif // REALTIMEPLAYERCONTROLLER_H
