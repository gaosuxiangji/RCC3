#include "remotectrlwidget.h"
#include "ui_remotectrlwidget.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoUtils/videoutils.h"
#include "Video/Export/deviceexportingwidget.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
#include "Device/deviceutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/PathUtils/pathutils.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include "LogRunner.h"

using namespace FHJD_LOG;

RemoteCtrlWidget::RemoteCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteCtrlWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

	widgets_to_enable_
		<< ui->widgetThumbnailVisible
		<< ui->widgetCameraSelection
		<< ui->widgetLuminanceContrast
		<< ui->widgetOsdVisible
		<< ui->widgetStreamType
		<< ui->widgetVideoFormat
		<< ui->widgetOtherSettings
		;

	InitUI();
}

RemoteCtrlWidget::~RemoteCtrlWidget()
{
    delete ui;
}

void RemoteCtrlWidget::setDevice(QSharedPointer<Device> device_ptr)
{
	auto old_device_ptr = device_wptr_.lock();
	if (old_device_ptr != device_ptr)
	{
		device_wptr_ = device_ptr;

		if (old_device_ptr)
		{
			// 解除旧设备的信号槽关联
			disconnect(old_device_ptr.data(), 0, this, 0);
		}

		// 增加新设备的信号槽关联
		connect(device_ptr.data(), &Device::stateChanged, this, &RemoteCtrlWidget::onDeviceStateChanged);
		connect(device_ptr.data(), &Device::errorOccurred, this, &RemoteCtrlWidget::onDeviceErrorOccurred);
		connect(device_ptr.data(), &Device::propertyChanged, this, &RemoteCtrlWidget::onDevicePropertyChanged);
		connect(device_ptr.data(), &Device::videoSegmentListUpdated, this, &RemoteCtrlWidget::onVideoSegmentListUpdated);


		updateVideoSettingUI();
		onVideoSegmentListUpdated();
	}
}

QSharedPointer<Device> RemoteCtrlWidget::getDevice() const
{
    return device_wptr_.lock();
}

QSharedPointer<Device> RemoteCtrlWidget::getLeftDevice() const
{
	return left_device_wptr_.lock();
}

QSharedPointer<Device> RemoteCtrlWidget::getRightDevice() const
{
	return right_device_wptr_.lock();
}

void RemoteCtrlWidget::setCurrrentVideo(const QVariant &video_id)
{
	//转到视频列表界面
	toVideoSettingRootWidget();

	//选中视频
	ui->widgetVideoList->setCurrentItem(video_id);
}


QVariant RemoteCtrlWidget::getCurrentVideo() const
{
	return ui->widgetVideoList->getCurrentItem();
}

void RemoteCtrlWidget::updateVideoList()
{
	if (needs_update_video_list_)
	{
		updateVideoListUI();

		needs_update_video_list_ = false;
	}
}

void RemoteCtrlWidget::updateMeasureCtrl()
{
	ui->widgetCalibrationParam->SetVideoExistsFlag(getCurrentVideo().isValid());
}

void RemoteCtrlWidget::videoLoaded()
{
	ui->widgetVideoList->onMouseClickedEnabled();
}

void RemoteCtrlWidget::toVideoSettingRootWidget()
{
	ui->stackedWidgetVideoSetting->setCurrentWidget(ui->pageVideoSettingRoot);
}

void RemoteCtrlWidget::toCameraSelectionWidget()
{
	ui->stackedWidgetVideoSetting->setCurrentWidget(ui->pageCameraSelecttion);
}

void RemoteCtrlWidget::toOtherSettingsWidget()
{
	ui->stackedWidgetVideoSetting->setCurrentWidget(ui->pageOtherSettings);
	toOtherSettingsWidgetRootPage();
}

void RemoteCtrlWidget::toOtherSettingsWidgetRootPage()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageOtherSettingsRoot);
}

void RemoteCtrlWidget::toLuminanceContrastWidget()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageLuminanceContrast);
}

void RemoteCtrlWidget::toStreamTypeWidget()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageStreamType);
}

void RemoteCtrlWidget::toVideoformatWidget()
{
	ui->stackedWidgetOtherSettings->setCurrentWidget(ui->pageVideoFormat);
}

void RemoteCtrlWidget::setThumbnailVisible(bool b_visible)
{
	emit sigSetThumbnailVisible(b_visible);

	qDebug() << "Thumbnail Visible:" << b_visible;
}

void RemoteCtrlWidget::setLeftCameraSelection(bool b_selected)
{
	left_camera_old_select_ = b_selected;
	left_camera_select_ = b_selected;
	onVideoSegmentListUpdated();// 设置播放器列表可以更新
	updateVideoSettingUI();
}

void RemoteCtrlWidget::setRightCameraSelection(bool b_selected)
{	
	right_camera_old_select_ = b_selected;
	right_camera_select_ = b_selected;
	onVideoSegmentListUpdated();// 设置播放器列表可以更新
	updateVideoSettingUI();
}

void RemoteCtrlWidget::setOsdVisible(bool b_visible)
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		cur_item.setOsdVisible(b_visible);
		VideoItemManager::instance().setVideoItem(cur_item.getId(), cur_item);

	}
	updateOsdUI();
	qDebug() << "OSD visible:" << b_visible;

	//更新其他设置显示内容
	setOtherSettingsText();
}

void RemoteCtrlWidget::setStreamType(const QVariant & stream_id)
{
	
	StreamType type = StreamType(stream_id.toInt());

	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		cur_item.setProperty(VideoItem::PropType::StreamType, type);
		VideoItemManager::instance().setVideoItem(cur_item.getId(), cur_item);

	}
	updateStreamTypeUI();

	//更新其他设置显示内容
	setOtherSettingsText();
}

void RemoteCtrlWidget::setVideoFormat(const QVariant & format_id)
{
	VideoFormat format = VideoFormat(format_id.toInt());

	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		cur_item.setProperty(VideoItem::PropType::VideoFormat, format);
		VideoItemManager::instance().setVideoItem(cur_item.getId(), cur_item);

	}
	updateVideoFormatUI();

	//更新其他设置显示内容
	setOtherSettingsText();
}

void RemoteCtrlWidget::setOtherSettingsText()
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		QString text = QString("%1, %2, %3").arg(
			QString("%1:%2").arg(ui->widgetOsdVisible->text(), cur_item.isOsdVisible() ? tr("Opened") : tr("Closed")),
			DeviceUtils::getStreamTypeText(cur_item.getProperty(VideoItem::PropType::StreamType).toInt()),
			DeviceUtils::getVideoFormatText(cur_item.getProperty(VideoItem::PropType::VideoFormat).toInt())
		);
		ui->widgetOtherSettings->setCurrentText(text);
	}
}

void RemoteCtrlWidget::onDeviceStateChanged()
{
	
}

void RemoteCtrlWidget::onDevicePropertyChanged()
{
	
}

void RemoteCtrlWidget::onDeviceErrorOccurred()
{

}

void RemoteCtrlWidget::onCurrentVideoItemChanged(const QVariant & video_id)
{
	current_video_id_ = video_id;

	//切换视频时阻塞信号，仅更新界面，不需要通知通知上层
	blockSignals(true);
	//刷新其他设置界面
	updateOtherSettingsUI();
	blockSignals(false);

	emit sigCurrentVideoItemChanged(video_id);
}

void RemoteCtrlWidget::onSelectedVideoItemsChanged(const QVariantList & video_id_list)
{
	//导出按钮刷新
	updateExportButtonUI();
	//测量面板刷新
	updateMeasureCtrl();
}

void RemoteCtrlWidget::onVideoItemsRemoved(const QVariantList & video_id_list)
{
	for (auto video_id : video_id_list)
	{
		auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(video_id));
		device_ptr->removeVideoSegment(VideoUtils::parseVideoSegmentId(video_id));
	}

	//删除完成后刷新一次即可
	updateVideoListUI();

	//导出按钮刷新
	updateExportButtonUI();
	//测量面板刷新
	updateMeasureCtrl();
}

void RemoteCtrlWidget::InitUI()
{
	//视频设置界面初始化
	InitVideoSettingUI();

	//初始化测量界面消息绑定
	InitBasicMeasureUiConnect();
}

void RemoteCtrlWidget::InitVideoSettingUI()
{
	//切换到视频设置根目录 
	toVideoSettingRootWidget();

	//未连接设备,初始时部分界面置灰
	SetWidgetsEnable(widgets_to_enable_, false);

	//缩略图显示开关
	ui->widgetThumbnailVisible->setText(tr("Thumbnail Visible"));
	ui->widgetThumbnailVisible->setChecked(false);

	setThumbnailVisible(ui->widgetThumbnailVisible->isChecked());
	connect(ui->widgetThumbnailVisible, &TouchCheckWidget::clicked, this, &RemoteCtrlWidget::setThumbnailVisible);

	//选择相机
	ui->widgetCameraSelection->setTitle(tr("Camera Selection"));
	ui->widgetCameraSelection->setCurrentText(QString(tr("%1 device(s)")).arg(0));

	ui->widgetLeftCameraSelection->setText(tr("Left Camera ID"));
	ui->widgetRightCameraSelection->setText(tr("Right Camera ID"));
	ui->widgetLeftCameraSelection->setVisible(false);
	ui->widgetRightCameraSelection->setVisible(false);

	connect(ui->widgetLeftCameraSelection, &TouchCheckWidget::clicked, this, &RemoteCtrlWidget::setLeftCameraSelection);
	connect(ui->widgetRightCameraSelection, &TouchCheckWidget::clicked, this, &RemoteCtrlWidget::setRightCameraSelection);
	connect(ui->widgetCameraSelection, &TouchComboWidget::clicked, this, &RemoteCtrlWidget::toCameraSelectionWidget);
	connect(ui->pushButtonCameraSelectionBack, &QPushButton::clicked, this, &RemoteCtrlWidget::toVideoSettingRootWidget);

	//TODO:由于工控机仅支持连接1台相机，暂时隐藏相机选择按钮
	ui->widgetCameraSelection->setVisible(false);
	ui->line_10->setVisible(false);

	//视频列表
	// 当前视频项切换关联
	connect(ui->widgetVideoList, &VideoListWidget::currentItemChanged, this, &RemoteCtrlWidget::onCurrentVideoItemChanged);

	// 选中视频改变关联
	connect(ui->widgetVideoList, &VideoListWidget::selectedItemsChanged, this, &RemoteCtrlWidget::onSelectedVideoItemsChanged);

	// 删除视频响应关联
	connect(ui->widgetVideoList, &VideoListWidget::itemsRemoved, this, &RemoteCtrlWidget::onVideoItemsRemoved);

	//其他设置
	ui->widgetOtherSettings->setTitle(tr("Other Settings"));
	connect(ui->widgetOtherSettings, &TouchComboWidget::clicked, this, &RemoteCtrlWidget::toOtherSettingsWidget);
	connect(ui->pushButtonBackToVideoSettingRoot, &QPushButton::clicked, this, &RemoteCtrlWidget::toVideoSettingRootWidget);

	//亮度对比度
	ui->widgetLuminanceContrast->setTitle(tr("Luminance/Contrast"));
	ui->sliderContrast->setValue(50);
	ui->sliderLuminance->setValue(50);
	ui->labelLuminance->setText(QString("%1").arg(ui->sliderLuminance->value()));
	ui->labelContrast->setText(QString("%1").arg(ui->sliderContrast->value()));
	ui->widgetLuminanceContrast->setCurrentText(QString("%1:%2/%3:%4").arg(tr("Luminance")).arg(ui->sliderLuminance->value()).arg(tr("Contrast")).arg(ui->sliderContrast->value()));

	connect(ui->widgetLuminanceContrast, &TouchComboWidget::clicked, this, &RemoteCtrlWidget::toLuminanceContrastWidget);
	connect(ui->pushButtonLuminanceContrastBack, &QPushButton::clicked, this, &RemoteCtrlWidget::toOtherSettingsWidgetRootPage);
	connect(ui->sliderLuminance, &QSlider::sliderReleased, this, &RemoteCtrlWidget::sigLuminanceContrastUpdateFinished);
	connect(ui->sliderContrast, &QSlider::sliderReleased, this, &RemoteCtrlWidget::sigLuminanceContrastUpdateFinished);


	//水印信息开关
	ui->widgetOsdVisible->setText(tr("OSD"));
	ui->widgetOsdVisible->setChecked(true);
	connect(ui->widgetOsdVisible, &TouchCheckWidget::clicked, this, &RemoteCtrlWidget::setOsdVisible);

	//协议格式
	ui->widgetStreamType->setTitle(tr("Stream Type"));
	ui->widgetStreamType->setCurrentText(DeviceUtils::getStreamTypeText(StreamType::TYPE_RAW8));

	connect(ui->widgetStreamType, &TouchComboWidget::clicked, this, &RemoteCtrlWidget::toStreamTypeWidget);
	connect(ui->pageStreamType, &TouchOptionsWidget::backButtonClicked, this, &RemoteCtrlWidget::toOtherSettingsWidgetRootPage);
	connect(ui->pageStreamType, &TouchOptionsWidget::currentOptionChanged, this, &RemoteCtrlWidget::setStreamType);

	//保存格式
	ui->widgetVideoFormat->setTitle(tr("Video Format"));
	ui->widgetVideoFormat->setCurrentText(DeviceUtils::getVideoFormatText(VideoFormat::VIDEO_RHVD));

	connect(ui->widgetVideoFormat, &TouchComboWidget::clicked, this, &RemoteCtrlWidget::toVideoformatWidget);
	connect(ui->pageVideoFormat, &TouchOptionsWidget::backButtonClicked, this, &RemoteCtrlWidget::toOtherSettingsWidgetRootPage);
	connect(ui->pageVideoFormat, &TouchOptionsWidget::currentOptionChanged, this, &RemoteCtrlWidget::setVideoFormat);

	//设置其他设置显示内容
	setOtherSettingsText();
}

void RemoteCtrlWidget::updateVideoSettingUI()
{

    if (DeviceManager::instance().getConnectedDeviceCount() == 0)//没有设备，返回设置主页并且置灰
	{
		toVideoSettingRootWidget();

		//相机选择
		updateCameraSelectionUI();

		//视频列表刷新
		updateVideoListUI();

		SetWidgetsEnable(widgets_to_enable_, false);
		return;
	}

	//有设备，刷新参数界面
	SetWidgetsEnable(widgets_to_enable_, true);

	//缩略图使能
	setThumbnailVisible(ui->widgetThumbnailVisible->isChecked());

	//相机选择
	updateCameraSelectionUI();

	//视频列表刷新
	updateVideoList();

	//刷新其他设置界面
	updateOtherSettingsUI();

	//导出按钮刷新
	updateExportButtonUI();

	//测量面板刷新
	updateMeasureCtrl();

}

void RemoteCtrlWidget::updateCameraSelectionUI()
{
	//刷新左右相机
	QList<QSharedPointer<Device>> device_list;
	DeviceManager::instance().getConnectedDevices(device_list);

	if (device_list.isEmpty())
	{
		ui->widgetLeftCameraSelection->setVisible(false);
		ui->widgetRightCameraSelection->setVisible(false);
		left_camera_select_ = false;
		right_camera_select_ = false;
		return;
	}
	else if (device_list.count() == 1)//设置左相机 隐藏右相机
	{
		ui->widgetLeftCameraSelection->setVisible(true);
		ui->widgetRightCameraSelection->setVisible(false);
		left_camera_select_ = left_camera_old_select_;
		right_camera_select_ = false;

		auto left_device_ptr = device_list.first();
		left_device_wptr_ = left_device_ptr;
		ui->widgetLeftCameraSelection->setText(left_device_ptr->getIp());
		ui->widgetLeftCameraSelection->setChecked(left_camera_select_);
		
	}
	else if (device_list.count() == 2)//设置双相机
	{
		ui->widgetLeftCameraSelection->setVisible(true);
		ui->widgetRightCameraSelection->setVisible(true);
		left_camera_select_ = left_camera_old_select_;
		right_camera_select_ = right_camera_old_select_;

		auto left_device_ptr = device_list.first();
		left_device_wptr_ = left_device_ptr;
		ui->widgetLeftCameraSelection->setText(left_device_ptr->getIp());
		ui->widgetLeftCameraSelection->setChecked(left_camera_select_);

		auto right_device_ptr = device_list.at(1);
		right_device_wptr_ = right_device_ptr;
		ui->widgetRightCameraSelection->setText(right_device_ptr->getIp());
		ui->widgetRightCameraSelection->setChecked(right_camera_select_);

	}
	else
	{
		//异常情况
		return;
	}

	ui->widgetCameraSelection->setCurrentText(QString(tr("%1 device(s)")).arg(left_camera_select_ + right_camera_select_));

}

/**
*@brief 视频列表按拍摄时间排序
*@param item1 视频项1的信息
*@param item2 视频项2的信息
*@return true-视频项1采集时间晚于视频项2
*@note 给qSort函数排序使用，视频项1采集时间<视频项2采集时间表示升序
**/
bool videoNameCompare(const VideoListItem & item1, const VideoListItem &item2)
{
	quint64 time1 = VideoUtils::parseVideoTimestampFromVideoName(item1.name);
	quint64 time2 = VideoUtils::parseVideoTimestampFromVideoName(item2.name);
	return time1 < time2;
}

void RemoteCtrlWidget::updateVideoListUI()
{
	// 只有在窗口显示时才刷新视频列表，防止出现一直导出预览的情况
	if (!this->isVisible())
	{
		return;
	}
	const auto cur_selected_video = ui->widgetVideoList->getCurrentItem();
	ui->widgetVideoList->clear();
	QList<VideoItem> left_video_item_list;
	QList<VideoItem> right_video_item_list;

	//分别获取左右相机的视频
	if (left_camera_select_)
	{
		left_video_item_list = VideoItemManager::instance().findVideoItems(VideoItem::Remote, getLeftDevice()->getIp());
	}
	if (right_camera_select_)
	{
		right_video_item_list = VideoItemManager::instance().findVideoItems(VideoItem::Remote, getRightDevice()->getIp());
	}

	QList<VideoListItem> video_list_item_list;
	int cur_selected_video_index = -1;

	for (auto video_item : left_video_item_list)
	{
		VideoListItem video_list_item;
		video_list_item.id = video_item.getId();
		video_list_item.name = video_item.getName();
		video_list_item.ip = VideoUtils::parseDeviceIp(video_item.getId());
		video_list_item_list << video_list_item;

		if (cur_selected_video == video_list_item.id)
		{
			cur_selected_video_index = video_list_item_list.size() - 1;
		}
	}

	for (auto video_item : right_video_item_list)
	{
		VideoListItem video_list_item;
		video_list_item.id = video_item.getId();
		video_list_item.name = video_item.getName();
		video_list_item.ip = VideoUtils::parseDeviceIp(video_item.getId());
		video_list_item_list << video_list_item;

		if (cur_selected_video == video_list_item.id)
		{
			cur_selected_video_index = video_list_item_list.size() - 1;
		}
	}
	
	//视频项排序
	qSort(video_list_item_list.begin(), video_list_item_list.end(), videoNameCompare);

	ui->widgetVideoList->addItems(video_list_item_list);

	if (!video_list_item_list.isEmpty())
	{
		if (cur_selected_video.isValid() && cur_selected_video_index >= 0 && !badded_new_video_segment_)
		{
			// 已选中视频未被删除时跳转至已选中视频
			ui->widgetVideoList->setCurrentItem(cur_selected_video);
		}
		else
		{
			// 默认选中最后一个
			ui->widgetVideoList->setCurrentItem(video_list_item_list.last().id);
			badded_new_video_segment_ = false;
		}
	}
}

void RemoteCtrlWidget::updateOtherSettingsUI()
{
	//亮度对比度刷新
	updateLuminanceContrastUI();

	//osd
	updateOsdUI();

	//协议格式
	updateStreamTypeUI();

	//视频格式
	//updateVideoFormatUI();

	//更新其他设置显示内容
	setOtherSettingsText();
}

void RemoteCtrlWidget::updateLuminanceContrastUI()
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		ui->sliderLuminance->setValue(cur_item.getLuminance());
		ui->sliderContrast->setValue(cur_item.getContrast());
		emit sigSetLuminanceContrast(cur_item.getLuminance(), cur_item.getContrast());
	}
	ui->widgetLuminanceContrast->setEnabled(cur_item.isValid());
}

void RemoteCtrlWidget::updateOsdUI()
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		ui->widgetOsdVisible->setChecked(cur_item.isOsdVisible());
		emit sigSetOsdVisible(cur_item.isOsdVisible());
	}
    ui->widgetOsdVisible->setEnabled(cur_item.isValid());
}

void RemoteCtrlWidget::updateStreamTypeUI()
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		//清除
		ui->pageStreamType->clearOptions();

		//添加支持选项
		QVariantList stream_type_var_list;
		auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(cur_item.getId()));
		if (device_ptr.isNull())
		{
			return;
		}
		device_ptr->getSupportedProperties(Device::PropStreamType, stream_type_var_list);
		for (auto stream_type_var : stream_type_var_list)
		{
			ui->pageStreamType->addOption(stream_type_var, DeviceUtils::getStreamTypeText(stream_type_var.toInt()));
		}

		//选中按钮 更新显示
		QVariant cur_type = cur_item.getProperty(VideoItem::PropType::StreamType);
		if (!cur_type.isValid())
		{
			cur_type = StreamType(stream_type_var_list.first().toInt());
			setStreamType(cur_type);
			return;
		}
		ui->pageStreamType->setCurrentOption(cur_type);
		ui->widgetStreamType->setCurrentText(ui->pageStreamType->getCurrentOptionText());

		//同步更新视频格式选项
		updateVideoFormatUI();
	}
	ui->widgetStreamType->setEnabled(cur_item.isValid());

}

void RemoteCtrlWidget::updateVideoFormatUI()
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		//清除
		ui->pageVideoFormat->clearOptions();

		//添加支持选项
		QVariant cur_stream = cur_item.getProperty(VideoItem::PropType::StreamType, StreamType(ui->pageStreamType->getCurrentOptionValue().toInt()));
		QVariantList format_var_list = getSupportedVideoFormat(cur_stream);
		for (auto format_var : format_var_list)
		{
			ui->pageVideoFormat->addOption(format_var, DeviceUtils::getVideoFormatText(format_var.toInt()));
		}
		
		//选中按钮 更新显示
		QVariant cur_format = cur_item.getProperty(VideoItem::PropType::VideoFormat);
		if (!cur_format.isValid())
		{
			cur_format = VideoFormat::VIDEO_AVI;
			setVideoFormat(cur_format);
			return;
		}
		ui->pageVideoFormat->setCurrentOption(cur_format);
		ui->widgetVideoFormat->setCurrentText(ui->pageVideoFormat->getCurrentOptionText());
	}
    ui->widgetVideoFormat->setEnabled(cur_item.isValid());
}

QVariantList RemoteCtrlWidget::getSupportedVideoFormat(QVariant cur_stream)
{
	QVariantList values;
	StreamType stream_type = StreamType(cur_stream.toInt());
	if (stream_type == TYPE_RAW8)
	{
		values << VIDEO_RHVD;
	}

	values << VIDEO_AVI;
	if (stream_type != TYPE_H264)
	{
		values << VIDEO_MP4;
		values << VIDEO_BMP;
		values << VIDEO_JPG;
		values << VIDEO_TIF;
	}

	return values;
}

void RemoteCtrlWidget::updateExportButtonUI()
{
	ui->pushButtonExport->setEnabled(!ui->widgetVideoList->getSelectedItems().empty());
}

VideoItem RemoteCtrlWidget::getCurVideoItem()
{
	QVariant cur_id = ui->widgetVideoList->getCurrentItem();

	if (cur_id.isValid())
	{
		return VideoItemManager::instance().getVideoItem(cur_id);
	}
	return VideoItem();
}

void RemoteCtrlWidget::InitBasicMeasureUiConnect()
{
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigMeasurePoint, this, &RemoteCtrlWidget::sigMeasurePoint);
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigMeausreLine, this, &RemoteCtrlWidget::sigMeasureLine);
	connect(ui->widgetCalibrationParam, &BasicMeasurerWidget::sigClearCurrentMeasureModeFeatures, this, &RemoteCtrlWidget::sigClearCurrentMeasureModeFeatures);
}

void RemoteCtrlWidget::SetWidgetsEnable(QList<QWidget*> widgets_to_enable, bool b_enbable)
{
	for (auto widget : widgets_to_enable)
	{
		widget->setEnabled(b_enbable);
	}
}

void RemoteCtrlWidget::on_sliderLuminance_valueChanged(int value)
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		cur_item.setLuminance(value);
		VideoItemManager::instance().setVideoItem(cur_item.getId(), cur_item);
		emit sigSetLuminanceContrast(cur_item.getLuminance(), cur_item.getContrast());
	}

	ui->labelLuminance->setText(QString("%1").arg(value));
	ui->widgetLuminanceContrast->setCurrentText(QString("%1:%2/%3:%4").arg(tr("Luminance")).arg(ui->sliderLuminance->value()).arg(tr("Contrast")).arg(ui->sliderContrast->value()));
}

void RemoteCtrlWidget::on_sliderContrast_valueChanged(int value)
{
	VideoItem cur_item = getCurVideoItem();
	if (cur_item.isValid())
	{
		cur_item.setContrast(value);
		VideoItemManager::instance().setVideoItem(cur_item.getId(), cur_item);
		emit sigSetLuminanceContrast(cur_item.getLuminance(), cur_item.getContrast());

	}

	ui->labelContrast->setText(QString("%1").arg(value));
	ui->widgetLuminanceContrast->setCurrentText(QString("%1:%2/%3:%4").arg(tr("Luminance")).arg(ui->sliderLuminance->value()).arg(tr("Contrast")).arg(ui->sliderContrast->value()));

}

void RemoteCtrlWidget::on_pushButtonExport_clicked()
{
	QString export_path;
	QString dir = SystemSettingsManager::instance().getWorkingDirectory();
	do
	{
		QString caption = tr("Export Video");
		QFileDialog::Options options = QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks;
		QString path = QFileDialog::getExistingDirectory(this, caption, dir, options);
		if (path.isEmpty())
		{
			break;
		}

		// 访问权限检查
		if (!PathUtils::isReadable(path))
		{
			dir = QFileInfo(path).absolutePath();
			continue;
		}

		export_path = path;

		break;
	} while (1);

	if (export_path.isEmpty())
	{
		return;
	}

	emit sigStartExport(); //通知外部开始导出，暂停播放

	QList<VideoItem> video_items;
	QVariantList video_id_list = ui->widgetVideoList->getSelectedItems();
	for (auto video_id : video_id_list)
	{
		VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
		video_item.setExportPath(export_path);
		video_item.setProperty(VideoItem::PropType::StreamType, StreamType(ui->pageStreamType->getCurrentOptionValue().toInt()));
		video_item.setProperty(VideoItem::PropType::VideoFormat, VideoFormat(ui->pageVideoFormat->getCurrentOptionValue().toInt()));
		video_items << video_item;
	}

	DeviceExportingWidget widget(this);
	widget.startExport(video_items);
}

void RemoteCtrlWidget::onVideoSegmentListUpdated()
{
	needs_update_video_list_ = true;//设置可以更新视频列表

	auto device_ptr = dynamic_cast<Device*>(sender());
	QVariant cur_ip = ui->widgetVideoList->getCurrentItem();
	if (device_ptr && cur_ip.isValid())
	{
		if (device_ptr->getIp() == VideoUtils::parseDeviceIp(cur_ip))
		{
			badded_new_video_segment_ = true;
		}
	}
}
