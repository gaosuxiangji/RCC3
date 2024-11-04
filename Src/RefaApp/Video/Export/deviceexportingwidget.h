#ifndef DEVICEEXPORTINGWIDGET_H
#define DEVICEEXPORTINGWIDGET_H

#include <QDialog>
#include <QList>
#include <QMap>
#include <thread>

#include "Video/VideoItem/videoitem.h"
#include "HscExportHeader.h"


namespace Ui {
class DeviceExportingWidget;
}

class DeviceExportingWidget : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceExportingWidget(QWidget *parent = 0);
    ~DeviceExportingWidget();

	/**
	*@brief 开始导出
	*@param video_items 导出视频项列表
	**/
	void startExport(const QList<VideoItem> &video_items);

	/**
	*@brief 导出进度回调
	*@param handle 设备句柄
	*@param value 导出进度
	*@param state 导出状态
	*@note 用来发出进度信号,跨线程更新界面
	**/
	void progressCallback(DeviceHandle handle, uint32_t value, uint32_t state);

public slots:
	/**
	* @brief 停止导出
	*/
	void stopExport();

signals:
	/**
	*@brief 导出进度变化信号
	*@param value 导出进度
	*@param state 导出状态
	**/
	void sigProgressChanged(uint32_t value, uint32_t state);

protected:
	void closeEvent(QCloseEvent *) override;

private slots:
	void onProgressChanged(uint32_t value, uint32_t state);

private:
	void initUI();

	void exportVideo(const VideoItem & video_item);

	/**
	*@brief 绘制水印
	*@param image_data 图像数据指针
	*@param timestamp 时间戳指针
	*@param frame_no 帧号
	*@param width 宽度
	*@param height 高度
	*@param channel 通道数
	*@note 导出由HscExport模块做，这里作为回调来叠加水印
	**/
	void paintWatermark(uint8_t * image_data, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel);

private:
    Ui::DeviceExportingWidget *ui;

	int current_progress_{0};

	QList<VideoItem> video_items_;

	VideoItem current_video_item_;

	int current_video_index_{ 0 };

	bool bexport_finished_{ false };//是否完成导出，导出完成和导出出错时设为true
};

#endif // DEVICEEXPORTINGWIDGET_H

// 全局导出进度回调
void __stdcall callback(DeviceHandle handle, uint32_t value, uint32_t state, void *user_ptr);


