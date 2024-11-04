#include "playbackplayerwidget.h"
#include "ui_playbackplayerwidget.h"
#include <RMAImage.h>
#include <RMAGlobalFunc.h>
#include <MediaInfo.h>
#include <ISPUtil.h>
#include <PlayerControllerInterface.h>
#include <CalibrationTypeDefine.h>
#include <PlayerParams.h>
#include <QEvent>
#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QFontMetrics>
#include <QFont>
#include <QStaticText>
#include <qmath.h>
#include <QTimer>

#include "Video/VideoItemManager/videoitemmanager.h"
#include "Measurer/basicmeasurer.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Device/playbackplayercontroller.h"
#include "LogRunner.h"
#include "Common/UIExplorer/uiexplorer.h"

using namespace FHJD_LOG;

PlaybackPlayerWidget::PlaybackPlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlaybackPlayerWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

	initUi();
	initBinding();
}

PlaybackPlayerWidget::~PlaybackPlayerWidget()
{
	quitPlayCtrlThread();
    delete ui;
}

void PlaybackPlayerWidget::addVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr, const QVariant& video_mark)
{
	if (ui->widgetPlayer->GetActiveVideoPlayerCtrl())
	{
		ui->widgetPlayer->CloseVideo();
		updateCloseVideoUi();
	}
	if (!player_controller_ptr || !video_mark.isValid())
		return;
	ui->widgetPlayer->AddVideo({ player_controller_ptr });
	cur_video_mark_ = video_mark;
	player_controller_ptr->EnableLoopPlay(true);
	bupdated_add_video_ui_ = false;

	quitPlayCtrlThread();
	playback_ctrl_thread_.resetPlayerController(player_controller_ptr);

	//先加载缩略图再设置范围等参数
	auto pctrl = std::dynamic_pointer_cast<PlaybackPlayerController>(player_controller_ptr);
	if (pctrl)
	{
		updateThumbnailCount(0, player_controller_ptr->GetPlayerParams()->GetTotalFrameCnt_Absolute());
		connect(pctrl.get(), &PlaybackPlayerController::thumbnailUpdated, this, &PlaybackPlayerWidget::setThumbnailImage);
		connect(pctrl.get(), &PlaybackPlayerController::thumbnailLoadingFinished, this, &PlaybackPlayerWidget::thumbnailLoadingFinished);

		if (ui->widgetThumbnail->isVisible())
		{
			startLoadingThumbnails();
		}
		else
		{
			updateAddVideoUi(player_controller_ptr->GetPlayerParams()->GetTotalFrameCnt_Absolute());
			ui->widgetThumbnail->setLoadThumbnailStatus(true);
			slotPlayerCtrlWidgetEnabled(true);
			emit pctrl->thumbnailLoadingFinished(false);
		}
	}
	else
	{
		updateAddVideoUi(player_controller_ptr->GetPlayerParams()->GetTotalFrameCnt_Absolute());
		slotPlayerCtrlWidgetEnabled(true);
	}
}

void PlaybackPlayerWidget::removeVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr)
{
	if (player_controller_ptr)
	{
		ui->widgetPlayer->RemoveVideo(player_controller_ptr);
		auto pctrl = std::dynamic_pointer_cast<PlaybackPlayerController>(player_controller_ptr);
		if (pctrl)
		{
			pctrl->disconnect(this);
		}
		updateCloseVideoUi();
	}
	else
	{
		removeVideo();
	}
}

void PlaybackPlayerWidget::removeVideo()
{
	auto player_controller_ptr = getPlaybackPlayerController();
	if (player_controller_ptr)
	{
		player_controller_ptr->disconnect(this);
	}

	ui->widgetPlayer->CloseVideo();
	updateCloseVideoUi();
}

QVariant PlaybackPlayerWidget::getCurrentVideoId() const
{
	return cur_video_mark_;
}

void PlaybackPlayerWidget::setFocusPoint(const QPoint &pt)
{
	ui->widgetPlayer->SetCustomCrossLineCenter(pt);
}

void PlaybackPlayerWidget::setRoi(const QRect &roi)
{
	ui->widgetPlayer->SetRoi(roi);
	ui->widgetPlayer->SetRoiVisible(true);
}

void PlaybackPlayerWidget::setTriggerFrame(uint64_t frame_index)
{
	ui->widgetPlayCtrl->SetTriggerFrame(frame_index);
}

void PlaybackPlayerWidget::setPlayerWorkStatus(PLAYER_WORK_MODE mode)
{
	ui->widgetPlayer->SetPlayerWorkStatus(mode);
}

PLAYER_WORK_MODE PlaybackPlayerWidget::getPlayerWorkStatus() const
{
	return ui->widgetPlayer->GetPlayerWorkStatus();
}

void PlaybackPlayerWidget::clearMeasureFeatures()
{
	ui->widgetPlayer->slotClearFeature();
}

void PlaybackPlayerWidget::setThumbnailVisible(bool bvisible)
{
	ui->widgetThumbnail->setVisible(bvisible);
	startLoadingThumbnails();
}

void PlaybackPlayerWidget::startLoadingThumbnails()
{
	auto playbcak_ctrl_ptr = getPlaybackPlayerController();
	if (playbcak_ctrl_ptr && !playbcak_ctrl_ptr->isThumbnailsLoaded())
	{
		ui->widgetThumbnail->setLoadThumbnailStatus(false);
		slotPlayerCtrlWidgetEnabled(false);
		playbcak_ctrl_ptr->startLoadingThumbnails(ui->widgetThumbnail->getThumbnailCount());
	}
}

void PlaybackPlayerWidget::setThumbnailImage(int thumbnail_index, const RMAImage & thumbnail)
{
	if (thumbnail.Empty() || thumbnail_index < 0)
		return;
	QApplication::processEvents();
	ui->widgetThumbnail->setThumbnail(thumbnail_index, thumbnail);
}

void PlaybackPlayerWidget::thumbnailLoadingFinished(bool ok)
{
	if (!bupdated_add_video_ui_)
	{
		auto pctrl = getPlaybackPlayerController();
		if (pctrl)
		{
			updateAddVideoUi(pctrl->GetPlayerParams()->GetTotalFrameCnt_Absolute());
		}
	}
	ui->widgetThumbnail->setLoadThumbnailStatus(true);
	slotPlayerCtrlWidgetEnabled(true);
}

void PlaybackPlayerWidget::onSelectedThumbnail(uint thumbnail_index, const RMAImage& img)
{
	if (thumbnail_index >= ui->widgetThumbnail->getThumbnailCount())
		return;
	slotPause();

	auto pctrl = getPlaybackPlayerController();
	if (pctrl)
	{
		pctrl->setCurrentThumbnail(thumbnail_index);
	}
}

void PlaybackPlayerWidget::updateThumbnails()
{
	auto pctrl = getPlaybackPlayerController();
	if (pctrl)
	{
		pctrl->updateThumbnailsLuminanceAndContrast();
	}
}

RMAImage PlaybackPlayerWidget::getCurrentFrame()
{
	std::shared_ptr<PlayerControllerInterface> pctrl = ui->widgetPlayer->GetActiveVideoPlayerCtrl();
	if (!pctrl)
		return RMAImage();
	VIDEO_ID id = pctrl->GetVideoInfo()->GetVideoId();
	RMAImage img;
	FRAME_INDEX idx = -1;
	ui->widgetPlayer->GetCurImage(id, img, idx);
	return qMove(img);
}

bool PlaybackPlayerWidget::isPaused() const
{
	return ui->widgetPlayer->isPaused();
}

void PlaybackPlayerWidget::setFocusPointVisible(bool bvisible)
{
	ui->widgetPlayer->SetCustomCrossLineVisible(bvisible);

	auto video_item = getVideoItem();
	if (video_item.isValid())
	{
		video_item.setProperty(VideoItem::FocusPointVisible, bvisible);
		setVideoItem(video_item);
	}
}

void PlaybackPlayerWidget::setFullScreen(bool benabled)
{
	ui->widgetToolBar->setVisible(!benabled);
	if (benabled)
	{
		ui->widgetTitleBar->setVisible(true);
		setWindowFlags(Qt::Window);
		showFullScreen();
	}
	else
	{
		ui->widgetTitleBar->setVisible(false);
		setWindowFlags(Qt::Widget);
		showNormal();
	}
}

void PlaybackPlayerWidget::setRoiSelectionEnabled(/*bool benabled*/)
{
	ui->widgetPlayer->SetRoiVisible(true);
	setPlayerWorkStatus(WORK_MODE_SELECTION_ROI);
	emit roiChanged(cur_video_mark_, ui->widgetPlayer->GetRoi());
}

void PlaybackPlayerWidget::snapshot()
{
	if (!cur_video_mark_.isValid())
		return;
	RMAImage img = getCurrentFrame();
	if (!img.Empty())
	{
		paintMeasureFeaturesToImage(img);
		emit saveSnapshot(cur_video_mark_, img);
	}
}

void PlaybackPlayerWidget::setOSDVisible(bool bvisible)
{
	ui->widgetPlayer->SetOsdVisible(bvisible);
}

void PlaybackPlayerWidget::setLuminanceAndContrast(const int luminance, const int contrast)
{
	std::shared_ptr<PlayerControllerInterface> pctrl = ui->widgetPlayer->GetActiveVideoPlayerCtrl();
	if (!pctrl)
		return;
	auto playbcak_ctrl_ptr = std::dynamic_pointer_cast<PlaybackPlayerController>(pctrl);
	if (!playbcak_ctrl_ptr)
	{
		auto pisp = pctrl->GetISPHandle();
		if (pisp)
		{
			pisp->SetIlluminate(luminance);
			pisp->SetContrast(contrast);
		}
	}
	else
	{
		playbcak_ctrl_ptr->setLuminanceAndContrast(luminance, contrast);
		if (isPaused())
		{
			updateCurrentFrame();
		}
	}
}

void PlaybackPlayerWidget::on_toolButtonRestoreFullScreen_clicked()
{
	emit fullScreenTriggered(false);
}

void PlaybackPlayerWidget::setPlayerBegin(uint64_t begin)
{
	clearThumbnailSelect();
	playback_ctrl_thread_.setPlayerBegin(begin);
	setPlayOperation(PLAY_OPT_SET_PLAYER_BEGIN);
	auto item = getVideoItem();
	if (!item.isValid())
		return;
	item.setProperty(VideoItem::BeginFrameIndex, begin);
	setVideoItem(item);
}

void PlaybackPlayerWidget::setPlayerEnd(uint64_t end)
{
	clearThumbnailSelect();
	playback_ctrl_thread_.setPlayerEnd(end);
	setPlayOperation(PLAY_OPT_SET_PLAYER_END);
	auto item = getVideoItem();
	if (!item.isValid())
		return;
	item.setProperty(VideoItem::EndFrameIndex, end);
	setVideoItem(item);
}

void PlaybackPlayerWidget::setPlayerRange(uint64_t begin, uint64_t end)
{
	clearThumbnailSelect();
	playback_ctrl_thread_.setPlayerRange(begin, end);
	setPlayOperation(PLAY_OPT_SET_PLAYER_RANGE);

	auto item = getVideoItem();
	if (!item.isValid())
		return;
	item.setProperty(VideoItem::BeginFrameIndex, begin);
	item.setProperty(VideoItem::EndFrameIndex, end);
	setVideoItem(item);
}

void PlaybackPlayerWidget::setCurrentFrame(uint64_t frameIndex)
{
	clearThumbnailSelect();
	playback_ctrl_thread_.setCurrentFrameNo(frameIndex);
	setPlayOperation(PLAY_OPT_SET_CURRENT_FRAME);
}

void PlaybackPlayerWidget::backwardPlay()
{
	clearThumbnailSelect();
	ui->widgetPlayer->SetPlayStatus(false);
	setPlayOperation(PLAY_OPT_BACKWARD_PLAY);
}

void PlaybackPlayerWidget::forwardPlay()
{
	clearThumbnailSelect();
	ui->widgetPlayer->SetPlayStatus(false);
	setPlayOperation(PLAY_OPT_FORWARD_PALY);
}

void PlaybackPlayerWidget::pause()
{
	ui->widgetPlayer->SetPlayStatus(true);
    setPlayOperation(PLAY_OPT_PAUSE);
}

void PlaybackPlayerWidget::fastForwardPlay()
{
	clearThumbnailSelect();
	ui->widgetPlayer->SetPlayStatus(false);
	setPlayOperation(PLAY_OPT_FAST_FORWARD);
}

void PlaybackPlayerWidget::fastBackwardPlay()
{
	clearThumbnailSelect();
	ui->widgetPlayer->SetPlayStatus(false);
	setPlayOperation(PLAY_OPT_FAST_BACKWARD);
}

void PlaybackPlayerWidget::backward()
{
	clearThumbnailSelect();
	setPlayOperation(PLAY_OPT_BACKWARD);
}

void PlaybackPlayerWidget::forward()
{
	clearThumbnailSelect();
	setPlayOperation(PLAY_OPT_FORWARD);
}

void PlaybackPlayerWidget::stop()
{
	ui->widgetPlayCtrl->blockSignals(true);
	slotPause();
	ui->widgetPlayCtrl->blockSignals(false);
	setPlayOperation(PLAY_OPT_STOP);
}

void PlaybackPlayerWidget::initUi()
{
	ui->widgetPlayer->SetPlayCtrlAndToolWidgetVisible(false);
	ui->widgetPlayer->SetVideoNameLabelVisible(false);
	ui->widgetPlayer->SetOsdVisible(true);
	ui->widgetPlayer->setNeedPauseWorkMode({ WORK_MODE_MEASURE_POINT, WORK_MODE_MEASURE_LINE });
	ui->widgetPlayer->SetVideoInProj(true);
	ui->widgetPlayer->controlPauseEnabled(false);
	ui->widgetPlayer->SetPlayStatus(true);

	switchLanguage();//切换语言

	//设置标题栏
	setFullScreen(false);//设置为非全屏
	ui->widgetTitleBar->setTitle(UIExplorer::instance().getProductName());
	ui->widgetTitleBar->setIcon(QIcon(":/images/logo.ico"));

	setThumbnailVisible(false);//缩略图默认不显示

	// 初始禁用工具栏
	ui->widgetToolBar->setEnabled(false);

	ui->widgetToolBar->setVisible(false);
	toolbar_visible_timer_ = new QTimer(this);
	toolbar_visible_timer_->setInterval(kToolbarVisibleTimer_);
	connect(toolbar_visible_timer_, &QTimer::timeout, this, [this] {ui->widgetToolBar->setVisible(false); });

	installEventFilter(this);
}

void PlaybackPlayerWidget::initBinding()
{
	//标题栏信号槽绑定
	connect(ui->widgetTitleBar, &PlayerTitleBar::fullScreen, this, &PlaybackPlayerWidget::fullScreenTriggered);

	//工具栏信号槽绑定
	connect(ui->widgetToolBar, &VideoToolBar::zoomInTriggered, ui->widgetPlayer, &CWidgetVideoPlaySingle::on_ZoomInBtn_clicked);
	connect(ui->widgetToolBar, &VideoToolBar::zoomOutTriggered, ui->widgetPlayer, &CWidgetVideoPlaySingle::on_ZoomOutBtn_clicked);
	connect(ui->widgetToolBar, &VideoToolBar::zoomToOriginalTriggered, ui->widgetPlayer, &CWidgetVideoPlaySingle::on_OriginalSizeBtn_clicked);
	connect(ui->widgetToolBar, &VideoToolBar::zoomToFitTriggered, ui->widgetPlayer, &CWidgetVideoPlaySingle::on_FitViewBtn_clicked);
	connect(ui->widgetToolBar, &VideoToolBar::focusLineTriggered, this, &PlaybackPlayerWidget::setFocusPointVisible);
	connect(ui->widgetToolBar, &VideoToolBar::fullScreenTriggered, this, &PlaybackPlayerWidget::fullScreenTriggered);
	connect(ui->widgetToolBar, &VideoToolBar::roiSelectionTriggered, this, &PlaybackPlayerWidget::setRoiSelectionEnabled);
	connect(ui->widgetToolBar, &VideoToolBar::snapshotTriggered, this, &PlaybackPlayerWidget::snapshot);
	connect(ui->widgetToolBar, &VideoToolBar::buttonClicked, this, &PlaybackPlayerWidget::startToolbarVisibleTimer);

	//播放器信号槽绑定
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigRoiChanged, this, &PlaybackPlayerWidget::slotRoiChanged);
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigCustomCrossLineCenterChanged, this, &PlaybackPlayerWidget::slotFocusPointChanged);
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigUpdateFrame, ui->widgetPlayCtrl, &PlayCtrlWidget::SetCurFrameIndex);
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigUpdateFrame, this, &PlaybackPlayerWidget::slotUpdatedFrame);
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigPause, this, &PlaybackPlayerWidget::slotPause);
	connect(ui->widgetPlayer, &CWidgetVideoPlaySingle::sigFeatureAdded, this, &PlaybackPlayerWidget::slotFeatureAdded);

	//播放器控制模块信号槽绑定
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigPlayerBeginRangeChanged, this, &PlaybackPlayerWidget::setPlayerBegin);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigPlayerEndRangeChanged, this, &PlaybackPlayerWidget::setPlayerEnd);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigSeekFrame, this, &PlaybackPlayerWidget::setCurrentFrame);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigBackwardPlay, this, &PlaybackPlayerWidget::backwardPlay);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigPause, this, &PlaybackPlayerWidget::pause);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigForwardPlay, this, &PlaybackPlayerWidget::forwardPlay);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigFastBackwardPlay, this, &PlaybackPlayerWidget::fastBackwardPlay);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigBackward, this, &PlaybackPlayerWidget::backward);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigForward, this, &PlaybackPlayerWidget::forward);
	connect(ui->widgetPlayCtrl, &PlayCtrlWidget::sigFastForwardPlay, this, &PlaybackPlayerWidget::fastForwardPlay);

	//缩略图信号
	connect(ui->widgetThumbnail, &WidgetThumbnail::selectedImageChanged, this, &PlaybackPlayerWidget::onSelectedThumbnail);

	//其他
	connect(&playback_ctrl_thread_, &PlaybackPlayerCtrlThread::started, this, &PlaybackPlayerWidget::slotPlayCtrlThreadStarted);
	connect(&playback_ctrl_thread_, &PlaybackPlayerCtrlThread::finished, this, &PlaybackPlayerWidget::slotPlayCtrlThreadFinished);
}

void PlaybackPlayerWidget::addMeasureFeature(const VideoItem &item)
{
	if (!item.isValid())
		return;
	auto feature_list = item.getFeatureItems();
	for each (auto feature in feature_list)
	{
		if (!feature.isValid())
			continue;
		ui->widgetPlayer->slotAddFeature(feature.getId().toInt(), feature.getValue());
		ui->widgetPlayer->slotMeasureResult(feature.getId().toInt(), feature.getUserData());
	}
}

void PlaybackPlayerWidget::addPlayerLegends()
{
	auto video_item = getVideoItem();
	if (!video_item.isValid())
		return;

	//设置roi
	QVariant var = video_item.getProperty(VideoItem::Roi);
	if (var.isValid())
	{
		setRoi(var.toRect());
	}
	else
	{
		auto pctrl = ui->widgetPlayer->GetActiveVideoPlayerCtrl();
		if (pctrl)
		{
			auto pvideo_info = pctrl->GetVideoInfo();
			if (pvideo_info)
			{
				setRoi(QRect(0, 0, pvideo_info->GetWidth(), pvideo_info->GetHeight()));
			}
		}
	}

	//设置窗口中心线
	var = video_item.getProperty(VideoItem::FocusPoint);
	if (var.isValid())
	{
		setFocusPoint(var.toPoint());
	}

	addMeasureFeature(video_item);
}

void PlaybackPlayerWidget::updateAddVideoUi(uint64_t frame_cnt)
{
	if (bupdated_add_video_ui_)
	{
		//添加视频后已经刷新过界面，不再重复刷新
		return;
	}
	bupdated_add_video_ui_ = true;

	ui->widgetPlayer->SetTranslationEnable(true);
	ui->widgetPlayer->SetVideoInProj(true);
	ui->widgetPlayer->slotSetCalibrationMode(CALI_MODE_CHESSBOARD_HOMOGRAPHY);
    ui->widgetPlayer->SetProjSettingMode(true);
	ui->widgetPlayer->setNeedPauseWorkMode({ WORK_MODE_MEASURE_POINT, WORK_MODE_MEASURE_LINE });
	ui->widgetPlayer->controlPauseEnabled(false);
	ui->widgetPlayCtrl->SetTotalFrameCnt(frame_cnt);

	auto video_item = getVideoItem();
	if (!video_item.isValid())
	{
		ui->widgetPlayCtrl->SetCurFrameIndex(0);
		return;
	}

	//设置播放范围和当前帧
	QVariant  var = video_item.getProperty(VideoItem::BeginFrameIndex);
	qint64 begin = 0, end = frame_cnt - 1;
	if (var.isValid())
		begin = var.toLongLong();
	var = video_item.getProperty(VideoItem::EndFrameIndex);
	if (var.isValid())
		end = var.toLongLong();
	if (begin > end)
		qSwap(begin, end);
	if (begin < 0)
		begin = 0;
	if (end >= frame_cnt)
		end = frame_cnt - 1;
	setPlayerRange(begin, end);
	ui->widgetPlayCtrl->SetPlayerRange(begin, end);//设置范围需要在设置当前帧之前，否则会出错
	setCurrentFrame(begin);
	ui->widgetPlayCtrl->SetCurFrameIndex(begin);
	
	//设置亮度对比度
	int lum = 50, contrast = 50;
	var = video_item.getProperty(VideoItem::Luminance);
	if (var.isValid())
		lum = var.toInt();
	var = video_item.getProperty(VideoItem::Contrast);
	if (var.isValid())
		contrast = var.toInt();
	setLuminanceAndContrast(lum, contrast);
	//设置osd是否显示
	bool bvisible = true;
	var = video_item.getProperty(VideoItem::OsdVisible);
	if (var.isValid())
		bvisible = var.toBool();
	setOSDVisible(bvisible);

	//设置特征
	addPlayerLegends();
}

void PlaybackPlayerWidget::updateCloseVideoUi()
{
	quitPlayCtrlThread();
	playback_ctrl_thread_.resetPlayerController(nullptr);
	slotPlayerCtrlWidgetEnabled(false);
	setPlayerWorkStatus(WORK_MODE_IDLE);
	ui->widgetPlayer->SetPlayStatus(true);
	ui->widgetPlayCtrl->SetTotalFrameCnt(0);
	ui->widgetPlayCtrl->SetCurFrameIndex(0);
	ui->widgetPlayCtrl->SetPlayerRange(0, 0);
	ui->widgetPlayCtrl->ClearTriggerFrame();
	ui->widgetPlayCtrl->setPause();
	ui->widgetToolBar->setFocusLineChecked(false);
	cur_video_mark_.clear();

	RMAImage img;
	img.SetOSD(QStringList() << QString(" "));//没有图像时不显示osd信息，为屏蔽水印信息，填入一个空格符
	ui->widgetPlayer->SetImage(img);

    ui->widgetToolBar->setEnabled(false);

	ui->widgetThumbnail->clearThumbnail();//清空缩略图
	ui->widgetThumbnail->setLoadThumbnailStatus(true);

	bupdated_add_video_ui_ = false;
}

QString PlaybackPlayerWidget::getPlayerIPOrPath() const
{
	auto pctrl = ui->widgetPlayer->GetActiveVideoPlayerCtrl();
	if (!pctrl)
		return QString();
	auto pvideo_info = pctrl->GetVideoInfo();
	if (!pvideo_info)
		return QString();
	QString str = pvideo_info->GetCameraIP();
	if (str.isEmpty())
	{
		str = pvideo_info->GetFilePath().front();
	}
	return str;
}

void PlaybackPlayerWidget::setPlayOperation(PlayOperationType opt)
{
	//若线程正在运行则等待3s，3s后仍在运行直接退出上一步操作，进行下一步
	if (playback_ctrl_thread_.isRunning())
	{
		QElapsedTimer timer;
		timer.start();
		const int timeout = 3000;
		bool btimeout = true;
		do 
		{
			if (playback_ctrl_thread_.isFinished())
			{
				btimeout = false;
				break;
			}
			QThread::msleep(100);
		} while (!timer.hasExpired(timeout));

		if (btimeout)
		{
			playback_ctrl_thread_.terminate();
			playback_ctrl_thread_.wait();
		}
	}
    playback_ctrl_thread_.setPlayOperation(opt);
	playback_ctrl_thread_.start();
}

void PlaybackPlayerWidget::quitPlayCtrlThread()
{
	if (playback_ctrl_thread_.isRunning())
		playback_ctrl_thread_.terminate();
}

VideoItem PlaybackPlayerWidget::getVideoItem() const
{
	if (!cur_video_mark_.isValid())
		return VideoItem();
	auto video_item = VideoItemManager::instance().getVideoItem(cur_video_mark_);
	if (!video_item.isValid())
	{
		video_item = VideoItem(cur_video_mark_);
		VideoItemManager::instance().addVideoItem(video_item);
	}
	return std::move(video_item);
}

void PlaybackPlayerWidget::setVideoItem(const VideoItem &item)
{
	if (!item.isValid() || !cur_video_mark_.isValid())
		return;
	VideoItemManager::instance().setVideoItem(cur_video_mark_, item);
}

std::shared_ptr<PlaybackPlayerController> PlaybackPlayerWidget::getPlaybackPlayerController() const
{
	if (!ui->widgetPlayer->GetActiveVideoPlayerCtrl())
	{
		return std::shared_ptr<PlaybackPlayerController>();
	}
	return std::dynamic_pointer_cast<PlaybackPlayerController>(ui->widgetPlayer->GetActiveVideoPlayerCtrl());
}

void PlaybackPlayerWidget::updateThumbnailCount(int begin, int end)
{
	if (end - begin + 1 < ui->widgetThumbnail->getThumbnailCountMax())
	{
		ui->widgetThumbnail->setThumbnailCount(end - begin + 1);
	}
	else
	{
		ui->widgetThumbnail->setThumbnailCount(ui->widgetThumbnail->getThumbnailCountMax());
	}
}

void PlaybackPlayerWidget::paintMeasureFeaturesToImage(RMAImage & image) const
{
	if (image.Empty())
	{
		return;
	}

	auto video_item = getVideoItem();
	if (!video_item.isValid())
	{
		return;
	}

	auto feature_list = video_item.getFeatureItems();
	if (feature_list.isEmpty())
	{
		return;
	}

	//绘制测量元素
	cv::Mat mat;
	RMAIMAGE2CVMAT(image, mat);
	if (image.GetPixelFormat() == PIXEL_FMT_GRAY8)
	{
		cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGR);
	}
	else if (image.GetPixelFormat() == PIXEL_FMT_YUV)
	{
		cv::cvtColor(mat, mat, cv::COLOR_YUV2BGR_NV21);
	}

	QImage img;
	CVMAT2QIMAGE(mat, img);
	if (image.GetPixelFormat() == PIXEL_FMT_BGR24)
	{
		img = img.rgbSwapped().copy();
	}

	QPainter painter(&img);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	QRect buffer_rc = img.rect();
	for each (auto feature in feature_list)
	{
		if (!feature.isValid() || !feature.getValue().isValid())
			continue;

		paintMeasureFeature(painter, feature.getValue(), feature.getUserData(), buffer_rc);
	}

	image = QImage2RMAImage(img);
}

void PlaybackPlayerWidget::paintMeasureFeature(QPainter & painter, const QVariant & feature_data, const QVariant & user_data, const QRect & buffer_rc) const
{
	if (!feature_data.isValid() || !user_data.isValid())
		return;

	QPoint center;

	//保存绘制点/线段前的painter参数
	painter.save();
	static const QColor color(Qt::red);
	static const int pt_radius = 3;//绘制点的半径
	static const int thickness = 2;//绘制线段宽度
	painter.setBrush(color);
	if (feature_data.type() == QVariant::Point)
	{
		center = feature_data.toPoint();
		painter.drawEllipse(center, pt_radius, pt_radius);
	}
	else if (feature_data.type() == QVariant::Line)
	{
		QLine line = feature_data.toLine();
		center = QPoint((line.p1().x() + line.p2().x()) / 2, (line.p1().y() + line.p2().y()) / 2);

		QPen pen(color);
		pen.setWidth(thickness);
		painter.setPen(pen);
		painter.drawLine(line.p1(), line.p2());
		painter.drawEllipse(line.p1(), pt_radius, pt_radius);
		painter.drawEllipse(line.p2(), pt_radius, pt_radius);
	}
	painter.restore();//还原至绘制点/线段前的painter参数

	QString text;
	if (user_data.type() == QVariant::StringList)
	{
		QStringList list = user_data.toStringList();
		if (list.isEmpty())
			return;
		text = list.join(QString("<br/>"));
	}
	else if (user_data.type() == QVariant::String)
		text = user_data.toString();
	if (text.isEmpty())
		return;

	static const int flags = Qt::TextWordWrap;//设置文字支持换行

	QRect bounding_rect = painter.fontMetrics().boundingRect(buffer_rc, flags, text);
	bounding_rect = getPaintTextRect(bounding_rect, buffer_rc, center);
	if (bounding_rect.isEmpty())
	{
		return;
	}

	painter.save();//保存绘制文字前的painter参数

	//绘制矩形框
	painter.fillRect(bounding_rect, Qt::white);

	//绘制文字
	painter.setPen(Qt::black);
	painter.drawText(bounding_rect, flags, text);

	//绘制矩形框外边界
	painter.setPen(Qt::gray);
	painter.drawRect(bounding_rect);

	//绘制线段
	if (!bounding_rect.contains(center))
	{
		painter.drawLine(center, getPaintTextLineEndPoint(center, bounding_rect));
	}
	else
	{
		//如果调整后中点仍在矩形中且绘制点，则重新绘制点，防止其被覆盖
		if (feature_data.type() == QVariant::Point)
		{
			painter.drawEllipse(center, pt_radius, pt_radius);
		}
	}

	painter.restore();//还原至绘制文字前的painter参数
}

QRect PlaybackPlayerWidget::getPaintTextRect(const QRect & text_rc, const QRect & buffer_rc, const QPoint & point) const
{
	if (text_rc.width() > buffer_rc.width() || text_rc.height() > buffer_rc.height())
		return QRect();
	static const int kRectXOffset{ 20 };//文字x方向偏移
	static const int kFontRectPadding{ 2 };//文字矩形边界padding

	QRect out_rc = text_rc;
	const QPoint buffer_center = buffer_rc.center();
	out_rc.adjust(-kFontRectPadding, 0, kFontRectPadding, 0);
	QPoint offset_point(-kRectXOffset, out_rc.height());
	out_rc.moveCenter(point + offset_point);
	if (text_rc.contains(out_rc))
		return out_rc;

	if (point.x() > buffer_center.x())
		offset_point.setX(kRectXOffset);
	if (point.y() > buffer_center.y())
		offset_point.setY(-out_rc.height());
	out_rc.moveCenter(point + offset_point);

	if (buffer_rc.left() > out_rc.left())
		out_rc.moveLeft(buffer_rc.left() + kFontRectPadding);
	if (buffer_rc.top() > out_rc.top())
		out_rc.moveTop(buffer_rc.top() + kFontRectPadding);
	if (buffer_rc.right() < out_rc.right())
		out_rc.moveRight(buffer_rc.right() - kFontRectPadding);
	if (buffer_rc.bottom() < out_rc.bottom())
		out_rc.moveBottom(buffer_rc.bottom() - kFontRectPadding);

	return out_rc;
}

QPoint PlaybackPlayerWidget::getPaintTextLineEndPoint(const QPoint & begin_pt, const QRect & text_rc) const
{
	QList<QPoint> pt_list;
	pt_list << (text_rc.topLeft() + text_rc.topRight()) / 2
		//<< (text_rc.topLeft() + text_rc.bottomLeft()) / 2
		//<< (text_rc.topRight() + text_rc.bottomRight()) / 2
		<< (text_rc.bottomLeft() + text_rc.bottomRight()) / 2
		;
	
	float min_distance = std::numeric_limits<float>::max();
	float distance = min_distance;
	QPoint end_pt = begin_pt;
	for (const auto & pt : pt_list)
	{
		distance = getDistance(begin_pt, pt);
		if (distance < min_distance)
		{
			min_distance = distance;
			end_pt = pt;
		}
	}

	return qMove(end_pt);
}

float PlaybackPlayerWidget::getDistance(const QPoint & p1, const QPoint & p2) const
{
	return qSqrt(qPow(p1.x() - p2.x(), 2) + qPow(p1.y() - p2.y(), 2));
}

RMAImage PlaybackPlayerWidget::QImage2RMAImage(QImage src_image) const
{
	if (src_image.isNull())
	{
		return RMAImage();
	}

	RMAImage rmaImage;
	do {
			switch ((src_image).format()) 
			{
			case QImage::Format_RGB32:
				{
					QImage image = src_image.convertToFormat(QImage::Format_RGB888);
					rmaImage = RMAImage(image.width(), image.height(), PIXEL_FMT_BGR24, image.bits());
				}
					break;
			case QImage::Format_RGB888:
				{
					QImage image = src_image;
					rmaImage = RMAImage(image.width(), image.height(), PIXEL_FMT_BGR24, image.bits());
				}
					break;
			case QImage::Format_Indexed8:
			case QImage::Format_Grayscale8:
					rmaImage = RMAImage(src_image.width(), src_image.height(), PIXEL_FMT_GRAY8, src_image.bits());
					break;
			default:
					break;
			}
	} while (0);

	return rmaImage;
}

void PlaybackPlayerWidget::updateCurrentFrame()
{
	playback_ctrl_thread_.setCurrentFrameNo(getCurrentFrame().GetAbsoluteFrameIndex());
	setPlayOperation(PLAY_OPT_SET_CURRENT_FRAME);
}

void PlaybackPlayerWidget::slotRoiChanged(const QRect &roi)
{
	emit roiChanged(cur_video_mark_, roi);
}

void PlaybackPlayerWidget::slotFocusPointChanged(const QPoint &pt)
{
	auto video_item = getVideoItem();
	if (!video_item.isValid())
		return;
	video_item.setProperty(VideoItem::FocusPoint, pt);
	setVideoItem(video_item);
}

void PlaybackPlayerWidget::slotPlayerCtrlWidgetEnabled(bool benabled)
{
	ui->widgetPlayCtrl->SetEnabled(benabled);
}

void PlaybackPlayerWidget::slotPause()
{
	ui->widgetPlayCtrl->setPause();
}

void PlaybackPlayerWidget::slotFeatureAdded(LEGENDS_ID legend_id, const QVariant& feature)
{
	auto video_item = getVideoItem();
	if (!video_item.isValid() || !feature.isValid())
		return;
	QVariant user_data;
	if (feature.type() == QVariant::Point)
	{
		user_data = BasicMeasurer::mapToActual(feature.toPoint());
	}
	else if (feature.type() == QVariant::Line)
	{
		QLine line = feature.toLine();
		user_data = BasicMeasurer::actualDistance(line.p1(), line.p2());
	}
	ui->widgetPlayer->slotMeasureResult(legend_id, user_data);
	video_item.addFeatureItem(FeatureItem(legend_id, feature, user_data));
	setVideoItem(video_item);
}

void PlaybackPlayerWidget::slotPlayCtrlThreadStarted()
{
	ui->widgetPlayCtrl->SetPlayerBusyFlag(true);
}

void PlaybackPlayerWidget::slotPlayCtrlThreadFinished()
{
	ui->widgetPlayCtrl->SetPlayerBusyFlag(false);
}

void PlaybackPlayerWidget::slotUpdatedFrame(int64_t idx)
{
	if (getCurrentFrame().Empty())
	{
		ui->widgetToolBar->setEnabled(false);
	}
	else
	{
		ui->widgetToolBar->setEnabled(true);
		// 设置窗口中心线是否显示
		ui->widgetToolBar->setFocusLineChecked(ui->widgetPlayer->isCustomCrossLineVisible());
	}
}

void PlaybackPlayerWidget::clearThumbnailSelect()
{
	ui->widgetThumbnail->clearThumbnailSelect();
}

void PlaybackPlayerWidget::startToolbarVisibleTimer()
{
	if (ui->widgetToolBar->isVisible())
	{
		toolbar_visible_timer_->start();
	}
}

void PlaybackPlayerWidget::switchLanguage()
{
	QLocale::Language lang = SystemSettingsManager::instance().getLanguage();
	int lang_type = 0;
	switch (lang)
	{
	default:
		break;
	case QLocale::Language::English:
		lang_type = 1;
	}
	ui->widgetPlayer->switchLanguage(lang_type);
}

bool PlaybackPlayerWidget::isThumbnailVisible() const
{
	return ui->widgetThumbnail->isVisible();
}

std::shared_ptr<PlayerControllerInterface> PlaybackPlayerWidget::getPlayerController() const
{
	return ui->widgetPlayer->GetActiveVideoPlayerCtrl();
}

void PlaybackPlayerWidget::changeEvent(QEvent *event)
{
	Q_ASSERT(event);
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}
	QWidget::changeEvent(event);
}

void PlaybackPlayerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_ASSERT(event);

	if (ui->widgetPlayer->GetPlayerWorkStatus() == PLAYER_WORK_MODE::WORK_MODE_IDLE)
	{
		emit fullScreenTriggered(!ui->widgetTitleBar->isVisible());
	}
	QWidget::mouseDoubleClickEvent(event);
}

bool PlaybackPlayerWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this)
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			auto mouse_event = dynamic_cast<QMouseEvent*>(event);
			const auto mode = ui->widgetPlayer->GetPlayerWorkStatus();
			if (mouse_event && mode == PLAYER_WORK_MODE::WORK_MODE_IDLE && !ui->widgetToolBar->isVisible())
			{
				if (rect().contains(mouse_event->pos()))
				{
					ui->widgetToolBar->setVisible(true);
					startToolbarVisibleTimer();
					return false;
				}
			}
		}
	}
	
	return QWidget::eventFilter(watched, event);
}

