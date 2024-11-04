#include "setroiwidget.h"
#include "ui_setroiwidget.h"

#include "Device/device.h"
#include "Device/devicemanager.h"
#include "UIUtils/uiutils.h"
#include "Video/VideoItem/videoitem.h"
#include "Video/VideoItemManager/videoitemmanager.h"

SetRoiWidget::SetRoiWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetRoiWidget)
{
    ui->setupUi(this);
}

SetRoiWidget::~SetRoiWidget()
{
    delete ui;
}

void SetRoiWidget::setCurInterfaceType(CurInterfaceType type)
{
	cur_type_ = type;
	switch (type)
	{
	case RealtimeType:	
		ui->label_title->setText(QString("%1 >> %2").arg(tr("Realtime Image")).arg(tr("Image ROI")));
		break;
	case RemoteTpye:
		ui->label_title->setText(QString("%1 >> %2").arg(tr("Record Playback ")).arg(tr("Image ROI")));
		break;
	case LocalType:
		ui->label_title->setText(QString("%1 >> %2").arg(tr("Local Playback")).arg(tr("Image ROI")));
		break;
	default:

		break;
	}
}

void SetRoiWidget::DeviceRoiChanged(const QString &ip, const QRect &roi)
{
	cur_device_ip_ = ip;
	dataRefresh(roi);
}

void SetRoiWidget::VideoRioChanged(const QVariant& id, const QRect& roi)
{
	cur_video_id_ = id;
	dataRefresh(roi);
}

void SetRoiWidget::on_pushButton_apply_clicked()
{
	if (cur_roi_.width()<width_min||cur_roi_.height()<height_min)
	{
		QString str = QString(tr("The ROI is smaller than %1*%2, can't take effect.").arg(width_min).arg(height_min));
		UIUtils::showWarnMsgBox(this, str);
		return;
	}

	switch (cur_type_)
	{
	case RealtimeType:
		emit sigDeviceRoiChangeFinished(true, cur_device_ip_, cur_roi_);
		break;
	case RemoteTpye:
	case LocalType:
		emit sigVideoRoiChangeFinished(true, cur_video_id_, cur_roi_);
		break;
	default:
		break;
	}
}

void SetRoiWidget::on_pushButton_cancel_clicked()
{
	switch (cur_type_)
	{
	case RealtimeType:
		emit sigDeviceRoiChangeFinished(false, cur_device_ip_, getDeviceOldRoi(cur_device_ip_));
		break;
	case RemoteTpye:
	case LocalType:
		emit sigVideoRoiChangeFinished(false, cur_video_id_, getVideoOldRoi(cur_video_id_));
		break;
	default:
		break;
	}
}

void SetRoiWidget::dataRefresh(QRect roi)
{
	cur_roi_ = roi;

	//刷新当前显示数据
	ui->lineEdit_X->setText(QString::number(roi.topLeft().x()));
	ui->lineEdit_Y->setText(QString::number(roi.topLeft().y()));
	ui->lineEdit_Width->setText(QString::number(roi.width()));
	ui->lineEdit_Height->setText(QString::number(roi.height()));
}

QRect SetRoiWidget::getDeviceOldRoi(QString ip)
{
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (device_ptr)
	{
		return device_ptr->getProperty(Device::PropRoi).toRect();
	}
	return QRect();
}

QRect SetRoiWidget::getVideoOldRoi(QVariant id)
{
	VideoItem video = VideoItemManager::instance().getVideoItem(id);
	if (video.isValid())
	{
		return video.getProperty(VideoItem::PropType::Roi).toRect();
	}
	return QRect();
}
