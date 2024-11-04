#pragma once

#include <QWidget>
#include <QtWidgets/QDialog>
#include "ui_videolistthumbnail.h"
#include <opencv2/opencv.hpp>

class VideoListThumbnail : public QWidget
{
	Q_OBJECT
public:
	typedef struct ThumbnailInfo {
		QString video_name{};
		QString camera_name{};
		QString record_time{};
		QString video_size{};
		cv::Mat thumbnail_mat{};
	}ThumbnailInfo;
public:
	VideoListThumbnail(QWidget *parent = Q_NULLPTR);
	void setThumbnailInfo(const QPoint& pt, const ThumbnailInfo& info);
private:
	void initUI();
	virtual void mousePressEvent(QMouseEvent *event) override;

	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
private:
	Ui::VideoListThumbnailClass ui;

	bool m_bVideo{ false };
};
