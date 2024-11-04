#include "localmainwidget.h"
#include "ui_localmainwidget.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "PlayerController.h"
#include "PlayerStructer.h"
#include "Video/Export/exportutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/setroiwidget.h"
#include "LogRunner.h"
#include "Video/VideoUtils/videoutils.h"

using namespace FHJD_LOG;

LocalMainWidget::LocalMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalMainWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

	initUi();
}

LocalMainWidget::~LocalMainWidget()
{
    delete ui;
}

void LocalMainWidget::setFullScreen(bool benable)
{
	ui->widgetPlayer->setFullScreen(benable);
}

void LocalMainWidget::setFocusPoint(const QPoint &pt)
{
	ui->widgetPlayer->setFocusPoint(pt);
}

void LocalMainWidget::setRoi(const QRect &roi)
{
	ui->widgetPlayer->setRoi(roi);
}

void LocalMainWidget::onCurrentVideoItemChanged(const QVariant & video_id)
{
	if (video_id.isValid())
	{
		std::shared_ptr<PlayerController> player_ctrl_ptr(new PlayerController);
		player_ctrl_ptr->OpenVideo(video_id.toString());
		ui->widgetPlayer->addVideo(player_ctrl_ptr, video_id);
	}
	else
	{
		//视频列表中最后一个视频被删除时会发送空的video_id,此时移除播放器中已经存在的视频
		ui->widgetPlayer->removeVideo();	
	}
}

void LocalMainWidget::slotRequestCurrentFrame()
{
	emit sigSendCurrentFrame(ui->widgetPlayer->getCurrentFrame());
}

void LocalMainWidget::slotBeginCalibrate()
{
	RMAImage img;
	if (!ui->widgetPlayer->isPaused())
	{
		ui->widgetPlayer->slotPause();
	}
	else
	{
		img = ui->widgetPlayer->getCurrentFrame();
	}
	ui->widgetCtrl->slotReceiveCurrentFrame(img);
}

void LocalMainWidget::slotRoiChanged(const QVariant& id, const QRect& rc)
{
	if (!id.isValid())
		return;
	//切换到roi设置页面
	ui->stackedWidget->setCurrentWidget(ui->widgetRoi);
	ui->widgetRoi->VideoRioChanged(id, rc);
}

void LocalMainWidget::slotRoiChangeFinished(bool b_applyed, const QVariant& id, const QRect& roi)
{
	if (!id.isValid())
	{
		return;
	}
	QRect correct_roi(roi);

	if (b_applyed)
	{
		//获取校正后的矩形
		auto image = ui->widgetPlayer->getCurrentFrame();
		if (image.Empty())
		{
			return;
		}
		QRect video_rc(0, 0, image.GetWidth(), image.GetHeight());
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

void LocalMainWidget::slotMeasureLine(bool benabled)
{
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);
	if(benabled)
		ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_MEASURE_LINE);
}

void LocalMainWidget::slotMeasurePoint(bool benabled)
{
	ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_IDLE);
	if (benabled)
		ui->widgetPlayer->setPlayerWorkStatus(WORK_MODE_MEASURE_POINT);
}

void LocalMainWidget::slotSanpshot(const QVariant& id, const RMAImage& img)
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

void LocalMainWidget::initUi()
{
	initPlayerUi();
	initCtrlUi();
}

void LocalMainWidget::initPlayerUi()
{
	connect(ui->widgetPlayer, &PlaybackPlayerWidget::fullScreenTriggered, this, &LocalMainWidget::fullScreen);
	connect(ui->widgetPlayer, &PlaybackPlayerWidget::roiChanged, this, &LocalMainWidget::slotRoiChanged/*roiChanged*/);
	//connect(ui->widgetPlayer, &PlaybackPlayerWidget::focusPointChanged, this, &LocalMainWidget::focusPointChanged);
}

void LocalMainWidget::initCtrlUi()
{
	connect(ui->widgetCtrl, &LocalCtrlWidget::currentVideoItemChanged, this, &LocalMainWidget::onCurrentVideoItemChanged);

    connect(ui->widgetCtrl, &LocalCtrlWidget::osdVisibleChanged, ui->widgetPlayer, &PlaybackPlayerWidget::setOSDVisible);
    connect(ui->widgetCtrl, &LocalCtrlWidget::luminanceAndContrastChanged, ui->widgetPlayer, &PlaybackPlayerWidget::setLuminanceAndContrast);

	connect(this, &LocalMainWidget::sigCalibrationVideoExists, ui->widgetCtrl, &LocalCtrlWidget::slotCalibrationVideoExists);
	connect(ui->widgetCtrl, &LocalCtrlWidget::sigRequestCurrentFrame, this, &LocalMainWidget::slotRequestCurrentFrame);
	connect(this, &LocalMainWidget::sigSendCurrentFrame, ui->widgetCtrl, &LocalCtrlWidget::slotReceiveCurrentFrame);
	connect(ui->widgetCtrl, &LocalCtrlWidget::sigBeginCalibrate, this, &LocalMainWidget::slotBeginCalibrate);

	connect(ui->widgetCtrl, &LocalCtrlWidget::sigMeasureLine, this, &LocalMainWidget::slotMeasureLine);
	connect(ui->widgetCtrl, &LocalCtrlWidget::sigMeasurePoint, this, &LocalMainWidget::slotMeasurePoint);
	connect(ui->widgetCtrl, &LocalCtrlWidget::sigClearMeasureFeatures, ui->widgetPlayer, &PlaybackPlayerWidget::clearMeasureFeatures);


	connect(ui->widgetPlayer, &PlaybackPlayerWidget::saveSnapshot, this, &LocalMainWidget::slotSanpshot);

	ui->stackedWidget->setCurrentWidget(ui->widgetCtrl);
	ui->widgetRoi->setCurInterfaceType(LocalType);
	
	connect(ui->widgetRoi, &SetRoiWidget::sigVideoRoiChangeFinished, this, &LocalMainWidget::slotRoiChangeFinished);
}
