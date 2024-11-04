#ifndef ACQUIRETHREAD_H
#define ACQUIRETHREAD_H

#include <QObject>
#include <QThread>
#include "device.h"

/**
* @brief 采集线程
*/
class AcquireThread : public QThread
{
	Q_OBJECT

public:
	AcquireThread(QObject *parent = nullptr) : QThread(parent)
	{

	}

	void run() override {
		Device *device = qobject_cast<Device*>(parent());
		if (device)
		{
			device->doAcquire();
		}
	}
};

#endif // ACQUIRETHREAD_H
