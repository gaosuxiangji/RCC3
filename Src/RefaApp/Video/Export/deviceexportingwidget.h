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
	*@brief ��ʼ����
	*@param video_items ������Ƶ���б�
	**/
	void startExport(const QList<VideoItem> &video_items);

	/**
	*@brief �������Ȼص�
	*@param handle �豸���
	*@param value ��������
	*@param state ����״̬
	*@note �������������ź�,���̸߳��½���
	**/
	void progressCallback(DeviceHandle handle, uint32_t value, uint32_t state);

public slots:
	/**
	* @brief ֹͣ����
	*/
	void stopExport();

signals:
	/**
	*@brief �������ȱ仯�ź�
	*@param value ��������
	*@param state ����״̬
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
	*@brief ����ˮӡ
	*@param image_data ͼ������ָ��
	*@param timestamp ʱ���ָ��
	*@param frame_no ֡��
	*@param width ���
	*@param height �߶�
	*@param channel ͨ����
	*@note ������HscExportģ������������Ϊ�ص�������ˮӡ
	**/
	void paintWatermark(uint8_t * image_data, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel);

private:
    Ui::DeviceExportingWidget *ui;

	int current_progress_{0};

	QList<VideoItem> video_items_;

	VideoItem current_video_item_;

	int current_video_index_{ 0 };

	bool bexport_finished_{ false };//�Ƿ���ɵ�����������ɺ͵�������ʱ��Ϊtrue
};

#endif // DEVICEEXPORTINGWIDGET_H

// ȫ�ֵ������Ȼص�
void __stdcall callback(DeviceHandle handle, uint32_t value, uint32_t state, void *user_ptr);


