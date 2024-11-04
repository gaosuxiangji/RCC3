#include "deviceexportingwidget.h"
#include "ui_deviceexportingwidget.h"

#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
#include "exportutils.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoUtils/videoutils.h"
#include "Device/devicemanager.h"
#include "Device/device.h"
#include "HscExportHeader.h"
#include "Device/deviceutils.h"
#include "RMAGlobalFunc.h"

#include <QDebug>
#include <QCloseEvent>

bool b_exporting_{ false };

DeviceExportingWidget::DeviceExportingWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceExportingWidget)
{
    ui->setupUi(this);

	initUI();

	connect(this, &DeviceExportingWidget::sigProgressChanged, this, &DeviceExportingWidget::onProgressChanged, Qt::QueuedConnection);
}

DeviceExportingWidget::~DeviceExportingWidget()
{
    delete ui;
}

void DeviceExportingWidget::startExport(const QList<VideoItem> &video_items)
{
	b_exporting_ = true;
	bexport_finished_ = false;
	if (video_items.isEmpty())
	{
		return;
	}
	video_items_ = video_items;

	current_progress_ = 0;

	int min = 0;
	int max = 0;

	//计算导出总进度
	for (auto video_item : video_items)
	{
		max += (video_item.getEndFrameIndex() - video_item.getBeginFrameIndex() + 1);
	}

	ui->progressBar->setRange(min, max);
	ui->progressBar->setValue(0);
	float percent = 0;
	ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
	ui->labelTip->setText(tr("Exporting..."));

	//批量导出
	exportVideo(video_items.first());

	ui->pushButtonClose->setEnabled(true);

	exec();
}

void DeviceExportingWidget::closeEvent(QCloseEvent *event)
{
	if(!bexport_finished_)
	{
		QString title = tr("Break Export?");
		QString content = tr("After clicking OK, the system will automatically stop the export.");
		QString msg = QString("%1\r\n%2").arg(title).arg(content);
		if (!UIUtils::showQuestionMsgBox(this, msg, 1))
		{
			event->ignore();
			return;
		}
	}

	b_exporting_ = false;
	stopExport();

	event->accept();
}

void DeviceExportingWidget::onProgressChanged(uint32_t value, uint32_t state)
{
	if (current_video_index_ < video_items_.size())
	{
		auto video_item = video_items_.at(current_video_index_);
		if (video_item.isValid())
		{
			value -= video_item.getProperty(VideoItem::PropType::BeginFrameIndex).toInt();
		}
	}
	int cur_value = current_progress_ + value;
	//导出进度刷新
	ui->progressBar->setValue(cur_value);

	float percent = float(cur_value *100) / ui->progressBar->maximum();
	ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));

	//导出错误提示
	if (state == HSC_EOF)
	{
		current_progress_ += value;
		current_video_index_++;
		if (current_video_index_ < video_items_.size())
		{
			exportVideo(video_items_.at(current_video_index_));
		}
		else
		{
			bexport_finished_ = true;
			close();
		}
	}
	else if (state == HSC_OK)
	{
		return;
	}
	else
	{
		bexport_finished_ = true;

		//导出错误
		QString msg = QString("%1%2").arg(tr("Export Error :")).arg(QString("0x%1").arg(state, sizeof(state)*2, 16, QChar('0')));
		UIUtils::showInfoMsgBox(this, msg);
		close();
	}

}

void DeviceExportingWidget::initUI()
{
	// 设置标题
	setWindowTitle(UIExplorer::instance().getProductName());

	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui->pushButtonClose->setEnabled(false);

	connect(ui->pushButtonClose, &QPushButton::clicked, this, &DeviceExportingWidget::close);
}

void DeviceExportingWidget::exportVideo(const VideoItem & video_item)
{
	stopExport();//停止上一次导出

	current_video_item_ = video_item;
	//准备导出参数
	ExportVideoInfo export_info{};
	export_info.v_ID = VideoUtils::parseVideoSegmentId(video_item.getId());
	export_info.start_index = video_item.getProperty(VideoItem::PropType::BeginFrameIndex).toInt();
	export_info.end_index = video_item.getProperty(VideoItem::PropType::EndFrameIndex).toInt();
	export_info.frame_interval = video_item.getProperty(VideoItem::PropType::FrameStep).toInt();
	export_info.stream_type = StreamType(video_item.getProperty(VideoItem::PropType::StreamType).toInt());
	export_info.v_format = VideoFormat(video_item.getProperty(VideoItem::PropType::VideoFormat).toInt());
	export_info.enable_watermark = video_item.getProperty(VideoItem::PropType::OsdVisible).toBool();
	auto roi= video_item.getProperty(VideoItem::PropType::Roi).toRect();
	export_info.roi = {(uint16_t)roi.x(), (uint16_t)roi.y(), (uint16_t)roi.height(), (uint16_t)roi.width()};
	strcpy(export_info.v_path, ExportUtils::getExportVideoPath(video_item).toLocal8Bit().data());

	export_info.watermark_cb = std::bind(&DeviceExportingWidget::paintWatermark, this
		, std::placeholders::_1, std::placeholders::_2, 
		std::placeholders::_3, std::placeholders::_4, 
		std::placeholders::_5, std::placeholders::_6);
	export_info.luminance = video_item.getProperty(VideoItem::PropType::Luminance, 50).toInt();
	export_info.contrast = video_item.getProperty(VideoItem::PropType::Contrast, 50).toInt();

	//获取相机句柄
	auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(video_item.getId()));
	DeviceHandle dev_handle = device_ptr->device_handle_;

	Export(dev_handle, callback, this, export_info);
	//Export(dev_handle,callback, this, export_info);

	//更新提示
	QString tip_str = QString(tr("Exporting %1 (Total: %2)...")).arg(current_video_index_+1).arg(video_items_.count());
	ui->labelTip->setText(tip_str);


}

void DeviceExportingWidget::paintWatermark(uint8_t * image_data, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel)
{
	if (!image_data || !timestamp || width == 0 || height == 0 || channel == 0)
	{
		return;
	}

	QString frame_no_str = QString::number(frame_no);
	int frame_size = width * height * channel;
	QImage image;
	if (3 == channel)
	{
		frame_no_str = QString("<font color=#FF0000>%1</font>").arg(frame_no);
		image = QImage(image_data, width, height, width * channel, QImage::Format::Format_RGB888);
		image = image.rgbSwapped();
	}
	else if (4 == channel)
	{
		frame_no_str = QString("<font color=#FF0000>%1</font>").arg(frame_no);
		image = QImage(image_data, width, height, width * channel, QImage::Format::Format_RGBA8888);
		image = image.rgbSwapped();
	}
	else if (1 == channel)
	{
		//QImage::Format单通道设置为Format_Indexed8时不能在QImage上绘制图形
		image = QImage(image_data, width, height, width * channel, QImage::Format_Grayscale8);
	}

	if (image.isNull())
		return;

	QString text;
	text.append(QString("%1: %2<br/>").arg(QObject::tr("Frame No")).arg(frame_no_str));
	text.append(QString("%1: %2").arg(QObject::tr("Timestamp")).arg(DeviceUtils::formatTimestamp(timestamp)));
	text = QString("<font color = #FFFFFF>%1</font>").arg(text);
	ExportUtils::paintWatermark2Image(image, text);

	if (1 != channel)
	{
		image = image.rgbSwapped();
		memcpy(image_data, image.constBits(), frame_size);
	}
}

void DeviceExportingWidget::progressCallback(DeviceHandle handle, uint32_t value, uint32_t state)
{
	emit sigProgressChanged(value, state);
	qDebug() << "export Value:" << value;
}


void DeviceExportingWidget::stopExport()
{
	auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(current_video_item_.getId()));
	if (!device_ptr.isNull())
	{
		DeviceHandle dev_handle = device_ptr->device_handle_;
		StopExport(dev_handle);
	}
}

void __stdcall callback(DeviceHandle handle, uint32_t value, uint32_t state, void *user_ptr)
{
	if (b_exporting_ && user_ptr)
	{
		DeviceExportingWidget * widget = static_cast<DeviceExportingWidget*>(user_ptr);
		widget->progressCallback(handle, value, state);
	}
}


