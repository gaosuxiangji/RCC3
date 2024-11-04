#include "remotemainwidget.h"
#include "ui_remotemainwidget.h"

#include "Device/devicemanager.h"
#include "Device/device.h"
#include "PlayerStructer.h"
#include "Video/Export/exportutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/setroiwidget.h"
#include "Device/playbackplayercontroller.h"
#include "Video/VideoUtils/videoutils.h"
#include "LogRunner.h"

using namespace FHJD_LOG;

RemoteMainWidget::RemoteMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RemoteMainWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

    initUi();

	DeviceManager & device_manager = DeviceManager::instance();
	connect(&device_manager, &DeviceManager::deviceConnected, this, &RemoteMainWidget::onDeviceConnected);
	connect(&device_manager, &DeviceManager::deviceDisconnected, this, &RemoteMainWidget::onDeviceDisconnected);
}

RemoteMainWidget::~RemoteMainWidget()
{
    delete ui;
}

void RemoteMainWidget::updateVideoList()
{
	ui->widgetCtrl->updateVideoList();
}

std::shared_ptr<PlaybackPlayerController> RemoteMainWidget::getPlaybackPlayerController() const
{
	return std::dynamic_pointer_cast<PlaybackPlayerController>(ui->widgetPlayer->getPlayerController());
}

void RemoteMainWidget::setFullScreen(bool benable)
{
	ui->widgetPlayer->setFullScreen(benable);
}

void RemoteMainWidget::setFocusPoint(const QPoint &pt)
{
	ui->widgetPlayer->setFocusPoint(pt);
}

void RemoteMainWidget::setRoi(const QRect &roi)
{
    ui->widgetPlayer->setRoi(roi);
}

void RemoteMainWidget::onDeviceConnected(const QString & device_ip)
{
	auto device_ptr = DeviceManager::instance().getDevice(device_ip);
	if (device_ptr)
	{
		ui->widgetCtrl->setDevice(device_ptr);
	}
	
}

void RemoteMainWidget::onDeviceDisconnected(const QString & device_ip)
{
	auto device_ptr = DeviceManager::instance().getDevice(device_ip);
	if (device_ptr)
	{
		ui->widgetCtrl->setDevice(QSharedPointer<Device>());
	}
	
}


void RemoteMainWidget::setCurrentVideo(const QVariant & video_id)
{	
	ui->widgetCtrl->setCurrrentVideo(video_id);
}

void RemoteMainWidget::pause()
{
	ui->widgetPlayer->slotPause();
}

void RemoteMainWidget::slotMeasureLine(bool benabled)
{
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);
	if (benabled)
		ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_MEASURE_LINE);
}

void RemoteMainWidget::slotMeasurePoint(bool benabled)
{
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);
	if (benabled)
		ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_MEASURE_POINT);
}

void RemoteMainWidget::slotSanpshot(const QVariant& id, const RMAImage& img)
{
	if (img.Empty() || !id.isValid())
		return;

	VideoItem video_item = VideoItemManager::instance().getVideoItem(id);
	if (!video_item.isValid())
	{
		return;
	}

	QString filepath = ExportUtils::getSnapshotPath(video_item);
	bool ok = ExportUtils::snapshot(filepath, img);
	if (ok)
	{
		UIUtils::showInfoMsgBox(this, tr("Snapshot Saved."));
	}
	else
	{
		UIUtils::showInfoMsgBox(this, tr("Out of disk space."));
	}
}

void RemoteMainWidget::slotRoiChanged(const QVariant& id, const QRect& rc)
{
	if (!id.isValid())
		return;
	//切换到roi设置页面
	ui->stackedWidget->setCurrentWidget(ui->widgetRoi);
	ui->widgetRoi->VideoRioChanged(id, rc);
	
}

void RemoteMainWidget::slotRoiChangeFinished(bool b_applyed, const QVariant& id, const QRect& roi)
{
	if (!id.isValid())
	{
		return;
	}
	QRect correct_roi(roi);

	if (b_applyed)
	{
		//获取校正后的矩形
		auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(id));
		if (!device_ptr)
		{
			return;
		}
		auto video_segment_ptr = device_ptr->getVideoSegment(VideoUtils::parseVideoSegmentId(id));
		if (!video_segment_ptr)
		{
			return;
		}
		QRect video_rc(0, 0, video_segment_ptr->rect.width, video_segment_ptr->rect.height);
		VideoUtils::correctRoi(correct_roi, video_rc);

		VideoItem video = VideoItemManager::instance().getVideoItem(id);
		if (video.isValid())
		{
			video.setProperty(VideoItem::PropType::Roi, correct_roi);
			VideoItemManager::instance().setVideoItem(id, video);
		}
		else
		{
			video = VideoItem(id);
			video.setProperty(VideoItem::PropType::Roi, correct_roi);
			VideoItemManager::instance().addVideoItem(video);
		}
	}

	ui->widgetPlayer->setRoi(correct_roi);

	//播放器设为空闲模式
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);

	//切换回到控制面板
	ui->stackedWidget->setCurrentWidget(ui->widgetCtrl);
}

void RemoteMainWidget::onCurrentVideoItemChanged(const QVariant &video_id)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	setEnabled(false);

	do
	{
		if (!video_id.isValid())
		{
			ui->widgetPlayer->removeVideo(getPlaybackPlayerController());
			break;
		}

		auto device_ptr = DeviceManager::instance().getDevice(VideoUtils::parseDeviceIp(video_id));
		if (!device_ptr)
		{
			break;
		}

		auto video_segment_ptr = device_ptr->getVideoSegment(VideoUtils::parseVideoSegmentId(video_id));
		if (!video_segment_ptr)
		{
			break;
		}

		if (ui->widgetPlayer->getCurrentVideoId() == video_id)
		{
			break;
		}

		ui->widgetPlayer->removeVideo();
		std::shared_ptr<PlaybackPlayerController> player_ctrl_ptr(new PlaybackPlayerController(device_ptr, video_segment_ptr));
		connect(player_ctrl_ptr.get(), &PlaybackPlayerController::thumbnailLoadingFinished, this, &RemoteMainWidget::onVideoLoaded);
		ui->widgetPlayer->addVideo(player_ctrl_ptr, video_id);

		return;//执行成功后直接返回，失败才继续

	} while (0);

	onVideoLoaded();
}

void RemoteMainWidget::onVideoLoaded(bool bok/* = true*/)
{
	setEnabled(true);
	ui->widgetCtrl->videoLoaded();
	QApplication::restoreOverrideCursor();
}

void RemoteMainWidget::initUi()
{
    initPlayerUi();
    initCtrlUi();
}

void RemoteMainWidget::initPlayerUi()
{
    connect(ui->widgetPlayer, &PlaybackPlayerWidget::fullScreenTriggered, this, &RemoteMainWidget::fullScreen);
    connect(ui->widgetPlayer, &PlaybackPlayerWidget::roiChanged, this, &RemoteMainWidget::slotRoiChanged/*roiChanged*/);
    //connect(ui->widgetPlayer, &PlaybackPlayerWidget::focusPointChanged, this, &RemoteMainWidget::focusPointChanged);
    connect(ui->widgetPlayer, &PlaybackPlayerWidget::saveSnapshot, this, &RemoteMainWidget::slotSanpshot);
}

void RemoteMainWidget::initCtrlUi()
{
    connect(ui->widgetCtrl, &RemoteCtrlWidget::sigCurrentVideoItemChanged, this, &RemoteMainWidget::onCurrentVideoItemChanged);
	connect(ui->widgetCtrl, &RemoteCtrlWidget::sigSetOsdVisible, ui->widgetPlayer, &PlaybackPlayerWidget::setOSDVisible);
	connect(ui->widgetCtrl, &RemoteCtrlWidget::sigSetLuminanceContrast, ui->widgetPlayer, &PlaybackPlayerWidget::setLuminanceAndContrast);
	connect(ui->widgetCtrl, &RemoteCtrlWidget::sigLuminanceContrastUpdateFinished, ui->widgetPlayer, &PlaybackPlayerWidget::updateThumbnails);
	connect(ui->widgetCtrl, &RemoteCtrlWidget::sigSetThumbnailVisible, ui->widgetPlayer, &PlaybackPlayerWidget::setThumbnailVisible);
	connect(ui->widgetCtrl, &RemoteCtrlWidget::sigStartExport, ui->widgetPlayer, &PlaybackPlayerWidget::stop);

    connect(ui->widgetCtrl, &RemoteCtrlWidget::sigMeasureLine, this, &RemoteMainWidget::slotMeasureLine);
    connect(ui->widgetCtrl, &RemoteCtrlWidget::sigMeasurePoint, this, &RemoteMainWidget::slotMeasurePoint);
    connect(ui->widgetCtrl, &RemoteCtrlWidget::sigClearCurrentMeasureModeFeatures, ui->widgetPlayer, &PlaybackPlayerWidget::clearMeasureFeatures);

	ui->stackedWidget->setCurrentWidget(ui->widgetCtrl);
	ui->widgetRoi->setCurInterfaceType(RemoteTpye);

	connect(ui->widgetRoi, &SetRoiWidget::sigVideoRoiChangeFinished, this, &RemoteMainWidget::slotRoiChangeFinished);
}
