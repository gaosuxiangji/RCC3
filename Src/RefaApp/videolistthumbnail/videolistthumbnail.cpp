#include "videolistthumbnail.h"
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgcodecs/legacy/constants_c.h>

VideoListThumbnail::VideoListThumbnail(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setFocus(Qt::FocusReason::NoFocusReason);
	initUI();
}

void VideoListThumbnail::setThumbnailInfo(const QPoint& pt, const ThumbnailInfo& info)
{
	move(pt.x() - width()/2, pt.y()+30);
	ui.image_label->setPixmap(QPixmap{});
	QImage input_src{};
	if (!info.thumbnail_mat.empty())
	{
		if (1 == info.thumbnail_mat.channels()) {
			input_src = QImage((const unsigned char*)info.thumbnail_mat.data, \
				info.thumbnail_mat.cols, info.thumbnail_mat.rows, info.thumbnail_mat.step, QImage::Format_Grayscale8);
		}
		else if (3 == info.thumbnail_mat.channels()) {
			cvtColor(info.thumbnail_mat, info.thumbnail_mat, CV_BGR2RGB);
			input_src = QImage((const unsigned char*)info.thumbnail_mat.data, \
				info.thumbnail_mat.cols, info.thumbnail_mat.rows, info.thumbnail_mat.step, QImage::Format_RGB888);
		}
		QPixmap pixmap = QPixmap::fromImage(input_src);
		QPixmap pixmap_scaled = pixmap.scaled(ui.image_label->width(), ui.image_label->height(), Qt::KeepAspectRatio);
		ui.image_label->setPixmap(pixmap_scaled);
	}

	if (!info.camera_name.isEmpty()) {
		ui.name_key_label->setText(tr("Camera Name:"));
		ui.name_value_label->setText(info.camera_name);
		m_bVideo = false;
	}

	if (!info.video_name.isEmpty()) {
		ui.name_key_label->setText(tr("Video Name:"));
		ui.name_value_label->setText(info.video_name);
		m_bVideo = true;
	}

	ui.time_value_label->setText(info.record_time);
	ui.size_value_label->setText(info.video_size);
}

void VideoListThumbnail::initUI()
{
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowFlags(Qt::WindowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	ui.name_key_label->setText(tr("Video Name:"));
	ui.time_key_label->setText(tr("Record Time:"));
	ui.size_key_label->setText(tr("Video Size(MB):"));
}

void VideoListThumbnail::mousePressEvent(QMouseEvent *event)
{
	hide();
	setAttribute(Qt::WA_NoMouseReplay);
	QWidget::mousePressEvent(event);
}

void VideoListThumbnail::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui.retranslateUi(this);
		if (m_bVideo) {
			ui.name_key_label->setText(tr("Video Name:"));
		}
		else {
			ui.name_key_label->setText(tr("Camera Name:"));

		}
		ui.time_key_label->setText(tr("Record Time:"));
		ui.size_key_label->setText(tr("Video Size(MB):"));
	}

	QWidget::changeEvent(event);
}
