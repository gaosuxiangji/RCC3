#include "devicesearchthread.h"



DeviceSearchThread::DeviceSearchThread()
{
    dev_manager_ptr_ = &DeviceManager::instance();

    //信号跨线程传递
    connect(dev_manager_ptr_.data(), &DeviceManager::searchProcessChanged,this ,&DeviceSearchThread::searchProcessChanged);

    connect(dev_manager_ptr_.data(), &DeviceManager::searchFinished, this,  & DeviceSearchThread::searchFinished);

}


void DeviceSearchThread::run()
{
    dev_manager_ptr_->search();
}
