#ifndef SWITCHTODEVICEPLAYBACKTHREAD_H
#define SWITCHTODEVICEPLAYBACKTHREAD_H

#include <QThread>
#include <memory>

class PlaybackPlayerController;

/**
 * @brief 切换设备回放线程
 */
class SwitchToPlaybackThread : public QThread
{
    Q_OBJECT

public:
    SwitchToPlaybackThread(std::shared_ptr<PlaybackPlayerController> playback_controller_ptr, QObject *parent);

protected:
    void run() override;

private:
    std::weak_ptr<PlaybackPlayerController> playback_controller_wptr_;
};

#endif // SWITCHTODEVICEPLAYBACKTHREAD_H
