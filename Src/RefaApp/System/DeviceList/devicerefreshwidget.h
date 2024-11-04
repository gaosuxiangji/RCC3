#ifndef DEVICEREFRESHWIDGET_H
#define DEVICEREFRESHWIDGET_H

#include <QWidget>
#include <QPointer>

#include "Device/devicemanager.h"
#include "devicesearchthread.h"

namespace Ui {
class DeviceRefreshWidget;
}

class DeviceRefreshWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceRefreshWidget(QWidget *parent = 0);
    ~DeviceRefreshWidget();

    /**
     * @brief 开始进行设备搜索
     * @return 搜索到的设备数量
     */
	void startSearchDevices();
signals:
	void signalRefreshSearchFinished(const int devicecount);
protected:
	void closeEvent(QCloseEvent *) override;

private slots:
	void onSearchFinished(int device_count);
private:	
	void initUI();

private:
	QPointer<DeviceManager> dev_manager_ptr_;
	QPointer<DeviceSearchThread> dev_search_thread_ptr_;
};

#endif // DEVICEREFRESHWIDGET_H
