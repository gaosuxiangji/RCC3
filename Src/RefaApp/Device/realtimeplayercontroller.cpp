#include "realtimeplayercontroller.h"

#include "PlayerParams.h"

RealtimePlayerController::RealtimePlayerController(const QString & device_ip)
{
	media_info_ptr_.reset(new MediaInfo(QStringList({ device_ip }), QString(), 0, QString(), device_ip, QString(), VIDEO_FMT_IPC, 0, 0, PIXEL_FMT_BGR24, COLOR_PATTERN_COLOR, 0, 0, 0));
	player_params_ptr_.reset(new PlayerParams(0));
}

std::shared_ptr<PlayerParams> RealtimePlayerController::GetPlayerParams()
{
	return player_params_ptr_;
}

std::shared_ptr<MediaInfo> RealtimePlayerController::GetVideoInfo() const
{
    return media_info_ptr_;
}
