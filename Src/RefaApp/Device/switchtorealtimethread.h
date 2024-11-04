#ifndef SWITCHTOREALTIMETHREAD_H
#define SWITCHTOREALTIMETHREAD_H

#include <QThread>
#include <memory>

class PlaybackPlayerController;

/**
 * @brief 切换实时图像线程
 */
class SwitchToRealtimeThread : public QThread
{
    Q_OBJECT

public:
    SwitchToRealtimeThread(std::shared_ptr<PlaybackPlayerController> playback_controller_ptr, QObject *parent);

protected:
    void run();

private:
    std::weak_ptr<PlaybackPlayerController> playback_controller_wptr_;
};

#endif // SWITCHTOREALTIMETHREAD_H
