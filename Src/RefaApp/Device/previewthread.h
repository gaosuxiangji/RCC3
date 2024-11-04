#ifndef PREVIEWTHREAD_H
#define PREVIEWTHREAD_H

#include <QObject>
#include <QThread>
#include "device.h"

/**
* @brief ‘§¿¿œﬂ≥Ã
*/
class PreviewThread : public QThread
{
	Q_OBJECT

public:
	PreviewThread(QObject *parent = nullptr) : QThread(parent)
	{

	}

	void run() override {
		Device *device = qobject_cast<Device*>(parent());
		if (device)
		{
			device->doPreview();
		}
	}
};

#endif // PREVIEWTHREAD_H
