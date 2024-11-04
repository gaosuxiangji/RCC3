#ifndef DEVICESEARCHTHREAD_H
#define DEVICESEARCHTHREAD_H
#include <QThread>
#include <QPointer>
#include <QList>
#include <QSharedPointer>
#include "Device/devicemanager.h"
#include "Device/device.h"


/**
 * @brief 搜索线程类
 */
class DeviceSearchThread : public QThread
{
    Q_OBJECT
public:
    DeviceSearchThread();



signals:

    /**
     * @brief 搜索进度变化
     * @param value 进度值，有效范围[0,100]
     */
    void searchProcessChanged(int value);

    /**
     * @brief 搜索完成
     * @param device_count 设备个数
     */
    void searchFinished(int device_count);



private:
    void run() override;

    QPointer<DeviceManager> dev_manager_ptr_;
};

#endif // DEVICESEARCHTHREAD_H
