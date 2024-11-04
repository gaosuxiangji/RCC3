#include "switchtoplaybackthread.h"
#include "Device/playbackplayercontroller.h"
#include "Device/devicemanager.h"
#include "Device/device.h"

SwitchToPlaybackThread::SwitchToPlaybackThread(std::shared_ptr<PlaybackPlayerController> playback_controller_ptr, QObject *parent) : playback_controller_wptr_(playback_controller_ptr), QThread(parent)
{

}

void SwitchToPlaybackThread::run()
{
    // 挂起设备
    QList<QSharedPointer<Device>> devices;
    DeviceManager::instance().getDevices(devices);
    for (auto device_ptr : devices)
    {
        if (!device_ptr)
        {
            continue;
        }

        device_ptr->suspend();
    }

    // 恢复回放
    auto playback_controller_ptr = playback_controller_wptr_.lock();
    if (playback_controller_ptr)
    {
        playback_controller_ptr->resume();
    }
}
