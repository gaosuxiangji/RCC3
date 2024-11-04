#ifndef DEVICESEARCHWIDGET_H
#define DEVICESEARCHWIDGET_H

#include <QDialog>
#include <QPointer>
#include <QTimer>


#include "Device/devicemanager.h"
#include "devicesearchthread.h"


namespace Ui {
class DeviceSearchWidget;
}

/**
 * @brief 设备搜索进度模态对话框
 * 使用方法:
 * 需要搜索设备时调用startSearchDevices(),搜索结束后返回搜索到的设备数量
 * 设备详细信息存放在devicemanager中
 * 	DeviceSearchWidget search_widget;
 *  slotSearchFinished(search_widget.startSearchDevices());
 */
class DeviceSearchWidget : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSearchWidget(QWidget *parent = 0);
    ~DeviceSearchWidget();

    /**
     * @brief 开始进行设备搜索
     * @return 搜索到的设备数量
     */
	int startSearchDevices();



protected:
	void closeEvent(QCloseEvent *) override;

private slots:
	void onProgressChanged(int progress);

	void onSearchFinished(int device_count);

	void onTimer();

private:	
	void initUI();

private:
	int device_count_{ 0 };

	QTimer *timer;

	int cur_progress_{ 0 };

	QPointer<DeviceManager> dev_manager_ptr_;

	QPointer<DeviceSearchThread> dev_search_thread_ptr_;

    Ui::DeviceSearchWidget *ui;
};

#endif // DEVICESEARCHWIDGET_H
