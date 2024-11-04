#ifndef MSGLISTENER_H
#define MSGLISTENER_H

#include "HscViewBase.h"
#include "device.h"

/**
 * @brief 消息监听器类
 */
class MsgListener : public CHscViewBase
{
public:
    MsgListener(Device *device_ptr) : device_ptr_(device_ptr)
    {

    }

    virtual void Update(HscEvent* msg, void* parameters) override
    {
        if (device_ptr_)
        {
            device_ptr_->Update(msg, parameters);
        }
    }

private:
    Device* device_ptr_;
};

#endif // MSGLISTENER_H
