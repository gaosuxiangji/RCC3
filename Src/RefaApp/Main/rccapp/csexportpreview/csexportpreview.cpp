#include "csexportpreview.h"
#include "ui_csexportpreview.h"
#include "Playback/csplaybackcontroller.h"
#include "Video/Export/csdlgdevicevideoexport.h"
#include "Video/preexportcolorcorrectrollback.h"
#include "../../../Common/UIUtils/uiutils.h"
#include "DoubleValidator.h"
#include "Main/rccapp/csrccapp.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QCheckBox>
#include <QCloseEvent>
#include <QSpinBox>
#include "../../csrccapptranslator.h"
#include "Common/UIExplorer/uiexplorer.h"
#ifndef _WINDOWS
#include "UtilityNS/UtilityNS.h"
#endif

CSExportPreview::CSExportPreview(int32_t index, VideoItem video_item, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSExportPreview),
	m_iPlaySpeed(1),
	m_targetScoring(new CSTargetScoring(this)),
	m_displaySetting(new CSDisplaySetting(this)),
	m_videoItem(video_item),
	m_playerController(new CSPlaybackController(video_item)),
	m_targetScoringControl(new CSCtrl::CSTargerScoring(video_item)),
	m_strIp(""),
	m_current_idx(index)
{
	m_strIp = VideoUtils::parseDeviceIp(video_item.getId());
	m_devicePtr = DeviceManager::instance().getDevice(m_strIp);

    ui->setupUi(this);
	qRegisterMetaType<RccFrameInfo>("RccFrameInfo");
	Init();
	ConnectSignalAndSlot();
	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->setFocus();
	}
	//在线程中初始化播放状态和加载缩略图
	//m_InitThumnailsAndPlayStateThrd = std::thread(&CSExportPreview::InitThumnailsAndPlayState, this);
	InitThumnailsAndPlayState();
	UpadateDisplaySetting(m_strIp);

	// 缩略图加载结束的回调
	auto thumbnailLoadFinishCallback = [this](bool ok) {
		thumbnailLoadingFinished(ok);
	};

	m_playerController->setCrtlCallback(thumbnailLoadFinishCallback);

	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetPlayPauseStatus(true);
	}
}

CSExportPreview::~CSExportPreview()
{
    delete ui;
	if (m_devicePtr)
	{
		m_devicePtr->SetSdiState(HSC_SDI_STOP);
	}
	if (m_pSecondScreen)
	{
		m_pSecondScreen->hide();
	}
	if (m_playerController)
	{
		delete m_playerController;
		m_playerController = nullptr;
	}
	if (m_InitThumnailsAndPlayStateThrd.joinable()) m_InitThumnailsAndPlayStateThrd.join();
 }

void CSExportPreview::SlotEndFrame(const int64_t frameNo)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		ui->spinBox_skip_frame_value->setEnabled(false);
		int64_t uiNew = m_playerController->GetStartFrameNo();
		m_playerController->SwitchRange(uiNew, frameNo);
		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetPlayerRange(uiNew, frameNo);
		}

#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		if (m_pThumbnailManage)
		{
			m_pThumbnailManage->updateSelectedThumbnail(uiNew);
		}
#endif
		UpadateSdiRange(uiNew, frameNo);
		m_uiOldStart = uiNew;
		m_uiOldEnd = frameNo;
	}
}

void CSExportPreview::SlotImageUpdate(const RccFrameInfo & image)
{
	if (!m_bForbidSwitchBox)
	{
		ui->video_swtch_comboBox->setEnabled(true);
	}
	
	RccFrameInfo currentImg = image;
	m_bOpenClosePopup = true;
	if (m_player)
	{
		if (m_bGridVisible)
		{
			if (m_targetScoringControl)
			{
				m_targetScoringControl->DrawGridAndBurstPoint(currentImg.image, m_bGridVisible, currentImg.playback_info.frame_no);
			}
		}
		
		m_player->SlotUpdateImage(currentImg);
	}

	//IsPreviewingThumbnail 正在预览缩略图或者关键帧时，播放控制模块不响应
	if (!m_playerController->IsPreviewingThumbnail() && m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetCurFrameIndex(currentImg.playback_info.frame_no);
	}

	m_currentImg = currentImg.image;
	m_currentImgInfo = currentImg;
}

void CSExportPreview::SlotShowImgInBigScreen(const int index)
{
	if (m_playerController)
	{
		if (PS_PLAY == m_playerController->GetState())
		{
			m_bOldPlayState = true;
			SwitchDisplayBtnStatus(false);

			{
				m_playerController->SwitchState(PS_PAUSE);
				m_bVideoStated = false;
			}

			if (m_player)
			{
				m_player->SetExportDisplayBtnStatus(true);
			}
		}
		
		m_playerController->PreviewSingleThumbnail(index);
	}
}

void CSExportPreview::SlotUapdateFrameRange(const qint64 iStart, const qint64 iEnd)
{
	m_bOpenClosePopup = true;
	ui->spinBox_skip_frame_value->setEnabled(false);
	if (m_playerController)
	{
		m_playerController->SwitchRange(iStart, iEnd);
	}

	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetPlayerRange(iStart, iEnd);
	}

	UpadateSdiRange(iStart, iEnd);
	m_uiOldStart = iStart;
	m_uiOldEnd = iEnd;	
}

void CSExportPreview::SlotKeyFrameToBigScreen()
{
	if (m_playerController)
	{
		SwitchDisplayBtnStatus(false);

		{
			m_playerController->SwitchState(PS_PAUSE);
			m_bVideoStated = false;
		}

		if (m_player)
		{
			m_player->SetExportDisplayBtnStatus(true);
		}
		m_playerController->PreviewKeyFrame();
		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetCurFrameIndex(m_playerController->GetCurrentFrameNo());
		}
	}
}

void CSExportPreview::SlotSeekFrame(uint64_t curFrameIndex)
{
	if (m_playerController)
	{
		m_playerController->SeekFrame(curFrameIndex);
	}
}

void CSExportPreview::SlotSliderBeginFrame(const int64_t frameNo)
{
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
	if (m_pThumbnailManage)
	{
		m_pThumbnailManage->updateSelectedThumbnail(frameNo);
	}
#endif

	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		int64_t uiNew = m_playerController->GetEndFrameNo();
		if ((frameNo <= m_playerController->GetEndFrameNo()) && (frameNo != m_playerController->GetStartFrameNo())) {
			ui->spinBox_skip_frame_value->setEnabled(false);
			m_playerController->SwitchRange(frameNo, uiNew);
		}
		
		UpadateSdiRange(frameNo, uiNew);
		m_uiOldEnd = uiNew;
	}
	m_uiOldStart = frameNo;
}

void CSExportPreview::SlotSliderEndFrame(const int64_t frameNo)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		int64_t uiNew = m_playerController->GetStartFrameNo();
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		if (m_pThumbnailManage)
		{
			m_pThumbnailManage->updateSelectedThumbnail(uiNew);
		}
#endif
		if ((frameNo != m_playerController->GetEndFrameNo()) && (frameNo >= m_playerController->GetStartFrameNo())) {
			ui->spinBox_skip_frame_value->setEnabled(false);
			m_playerController->SwitchRange(uiNew, frameNo);
		}

		UpadateSdiRange(uiNew, frameNo);
		m_uiOldStart = uiNew;
		m_uiOldEnd = frameNo;
	}
}

void CSExportPreview::SlotMouseMoveOnSlider(uint64_t frameNo)
{
	if (m_playerController)
	{
		uint64_t uiNewStart = 0;
		uint64_t uiNewEnd = 0;
		m_pExportPreviewPlayCtr->GetPlayerRange(uiNewStart, uiNewEnd);
		if (m_uiOldStart != uiNewStart || m_uiOldEnd != uiNewEnd)
		{
			ui->spinBox_skip_frame_value->setEnabled(false);
			m_playerController->SwitchRange(uiNewStart, uiNewEnd, false);
			m_uiOldStart = uiNewStart;
			m_uiOldEnd = uiNewEnd;
			UpadateSdiRange(m_uiOldStart, m_uiOldEnd);
		}
		if (frameNo < uiNewStart)
		{
			frameNo = uiNewStart;
		}
		if (frameNo > uiNewEnd)
		{
			frameNo = uiNewEnd;
		}
		m_playerController->PreviewThumbnails(frameNo);
	}
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
	if (m_pThumbnailManage)
	{
		m_pThumbnailManage->updateSelectedThumbnail(frameNo);
	}
#endif
}

void CSExportPreview::SlotMouseMoveOutSlider()
{
	if (m_playerController)
	{
		m_playerController->ResetThumbnails();
	}
}

void CSExportPreview::SetVideoInfo(VideoInfoType type, const QString & info)
{

	switch (type)
	{
	case CSExportPreview::CAMERA_NAME:
	{
		QFontMetrics fontWidth(ui->camera_value_label->font());
		QString elideNote = fontWidth.elidedText(info, Qt::ElideRight, 160);
		ui->camera_value_label->setText(elideNote);
		ui->camera_value_label->setToolTip(info);
		break;
	}	
	case CSExportPreview::RECORD_TIME:
	{
		QFontMetrics fontWidth(ui->record_time_value_label->font());
		ui->record_time_value_label->setText(info);
		ui->record_time_value_label->setToolTip(info);
		break;
	}
	case CSExportPreview::TIME_LENGTH:
		ui->time_length_value_label->setText(info);
		break;
	case CSExportPreview::RATE:
		ui->rate_value_label->setText(info);
		break;
	case CSExportPreview::RESOLUTION:
		ui->resolution_value_label->setText(info);
		break;
	case CSExportPreview::DEPTH:
		ui->depth_value_label->setText(info);
		break;
	case CSExportPreview::TOTAL_FRAME:
		ui->total_frame_value_label->setText(info);
		break;
	case CSExportPreview::EXPOSURE_TIME:
		ui->exposure_time_value_label->setText(info);
		break;
	default:
		break;
	}
}

void CSExportPreview::Init()
{
	auto frame_head_type = m_devicePtr->getFrameHeadType();
	ui->display_widget->setContentsMargins(0, 0, 0, 0);
	m_player = new CPlayerViewBase(20, frame_head_type, ui->display_widget);
	m_player->setContentsMargins(0, 0, 0, 0);
	CSRccApp* main_window = nullptr;
	for (QWidget* widget : qApp->topLevelWidgets())
	{
		if (QMainWindow * mainWin = qobject_cast<QMainWindow*>(widget))
		{
			main_window = dynamic_cast<CSRccApp*>(mainWin);
			break;
		}
	}
	if (main_window)
	{
		auto pVideoPlayer = main_window->GetViewManagerPtr()->getSelectView();
		if (pVideoPlayer)
		{
			m_player->SetAntiAliasing(pVideoPlayer->GetAntiAliasingStatus());
		}
	}
	InitUI();
	UapdateVideoInfo(m_videoItem);
	if (m_playerController && m_pExportPreviewPlayCtr)
	{
		if (-1 != m_playerController->GetKeyFrameNo())
		{
			m_pExportPreviewPlayCtr->SetTriggerFrame(m_playerController->GetKeyFrameNo());
		}
	}
}

void CSExportPreview::InitThumnailsAndPlayState()
{
	if (m_playerController)
	{
		m_playerController->StartLoadThumbnails();
	}
}

void CSExportPreview::InitUI()
{
	setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
	setWindowTitle(tr("Playback"));

	ui->tabWidget->addTab(m_displaySetting,tr("Display Setting"));
	ui->tabWidget->addTab(m_targetScoring, tr("Target Scoring Setting"));
	m_targetScoring->setFixedWidth(400);
	m_displaySetting->setFixedWidth(400);

	QHBoxLayout* displayLayout = new QHBoxLayout(ui->display_widget);
	displayLayout->setContentsMargins(0, 0, 0, 0);
	ui->display_widget->setLayout(displayLayout);
	displayLayout->addWidget(m_player);

	m_leftFrameBtn = new FrameButton(ui->thumbnail_control_widget);
	//m_leftFrameBtn->setObjectName("FrameBtn");
	m_leftFrameBtn->setEnabled(false);
	m_leftFrameBtn->setFixedSize(20, ui->thumbnail_control_widget->height()-4);
	m_leftFrameBtn->setText("<");
	m_leftFrameBtn->setAutoDefault(false);
	m_rightFrameBtn = new FrameButton(ui->thumbnail_control_widget);
	//m_rightFrameBtn->setObjectName("FrameBtn");
	m_rightFrameBtn->setEnabled(false);
	m_rightFrameBtn->setFixedSize(20, ui->thumbnail_control_widget->height()-4);
	m_rightFrameBtn->setText(">");
	m_rightFrameBtn->setAutoDefault(false);

	m_hThumbnailManageLayout = new QHBoxLayout(ui->thumbnail_control_widget);
	m_hThumbnailManageLayout->setContentsMargins(0, 2, 0, 2);
	m_hThumbnailManageLayout->setSpacing(1);
	ui->thumbnail_control_widget->setLayout(m_hThumbnailManageLayout);
	m_pThumbnailManage = new CSThumbnailManage(ui->thumbnail_control_widget);
	m_hThumbnailManageLayout->addWidget(m_leftFrameBtn);
	m_hThumbnailManageLayout->addWidget(m_pThumbnailManage);
	m_hThumbnailManageLayout->addWidget(m_rightFrameBtn);

    QHBoxLayout* hSliderLayout = new QHBoxLayout(ui->slider_control_widget);
	hSliderLayout->setContentsMargins(0, 0, 0, 0);
	hSliderLayout->addSpacing(0);
    ui->slider_control_widget->setLayout(hSliderLayout);
    m_pExportPreviewPlayCtr = new CSExportPreviewPlayCtrl(ui->slider_control_widget);
    hSliderLayout->addWidget(m_pExportPreviewPlayCtr);

	ui->key_frame_value_label->setObjectName("DisplayCtrlText");

	ui->spinBox_skip_frame_value->setValue(0);
	ui->spinBox_skip_frame_value->setRange(0, VALID_MAX);
	ui->spinBox_skip_frame_value->setEnabled(false);
	this->setFocusPolicy(Qt::ClickFocus);
	UpadateDisplayCtrlStatus(false);
}

void CSExportPreview::ConnectSignalAndSlot()
{
// 	bool ok = connect(m_SpeedGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), \
// 		[=](int id) {SlotSpeedBtnClicked(id); });
	// [2022/9/8 rgq]: 在QButtonGroup中QPushButton不能响应左右键及Tab键
	auto ok = connect(m_player, &CPlayerViewBase::SignalKeyFrame, this, &CSExportPreview::SlotKeyFrame);
	ok = connect(m_player, &CPlayerViewBase::SignalBeginFrame, this, &CSExportPreview::SlotBeginFrame);
	ok = connect(m_player, &CPlayerViewBase::SignalEndFrame, this, &CSExportPreview::SlotEndFrame);
	ok = connect(m_player, &CPlayerViewBase::SignalMousePressPointF, this, &CSExportPreview::SlotMousePressPointF/*, Qt::QueuedConnection*/);
	ok = connect(m_player, &CPlayerViewBase::SignalCloseManualSelect, this, &CSExportPreview::SlotCloseManualSelect, Qt::QueuedConnection);
	ok = connect(m_playerController, &CSPlaybackController::ImageUpdate, this, &CSExportPreview::SlotImageUpdate);
	ok = connect(m_playerController, &CSPlaybackController::thumbnailUpdated, m_pThumbnailManage, &CSThumbnailManage::SlotThumbnailUpdated);
	ok = connect(m_playerController, &CSPlaybackController::signalHighlightThumbnail, m_pThumbnailManage, &CSThumbnailManage::SlotHighlightThumbnail);
	ok = connect(m_playerController, &CSPlaybackController::signalStartLoadingThumbnailTimers, this, &CSExportPreview::SlotThumbnailLoadingStarted);
	ok = connect(m_playerController, &CSPlaybackController::thumbnailLoadingFinished, this, &CSExportPreview::SlotThumbnailLoadingFinished);
	ok = connect(m_playerController, &CSPlaybackController::signalRangeStackChange, this, &CSExportPreview::slotRangeStackChange);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalShowImgInBigScreen, this, &CSExportPreview::SlotShowImgInBigScreen);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalKeyFrame, this, &CSExportPreview::SlotKeyFrame);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalBeginFrame, this, &CSExportPreview::SlotBeginFrame);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalEndFrame, this, &CSExportPreview::SlotEndFrame);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalUapdateFrameRange, this, &CSExportPreview::SlotUapdateFrameRange,Qt::QueuedConnection);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalMoveOutThumbnailArea, this, &CSExportPreview::SlotMoveOutThumbnailArea);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalSingleThumbnailClicked, this, &CSExportPreview::SlotSingleThumbnailClicked);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalFrameStepStatus, this, &CSExportPreview::SlotFrameStepStatus, Qt::QueuedConnection);
	ok = connect(m_pThumbnailManage, &CSThumbnailManage::SignalThumbnailRange, this, &CSExportPreview::SlotThumbnailRange, Qt::QueuedConnection);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::sigPlayerBeginRangeChanged, this, &CSExportPreview::SlotSliderBeginFrame);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::sigPlayerEndRangeChanged, this, &CSExportPreview::SlotSliderEndFrame);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalKeyFrameToBigScreen, this, &CSExportPreview::SlotKeyFrameToBigScreen);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::sigSeekFrame, this, &CSExportPreview::SlotSeekFrame);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalMouseMoveOnSlider, this, &CSExportPreview::SlotMouseMoveOnSlider);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalMouseMoveOutSlider, this, &CSExportPreview::SlotMouseMoveOutSlider);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalDisplayControl, this, &CSExportPreview::SlotDisplayControl);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalSwitchFrameAndMs, this, &CSExportPreview::SlotSwitchFrameAndMs);
	ok = connect(m_pExportPreviewPlayCtr, &CSExportPreviewPlayCtrl::SignalSliderMouseClicked, this, &CSExportPreview::SlotSingleThumbnailClicked);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalRevertCheckBoxStatusChanged, this, &CSExportPreview::SlotRevertCheckBoxStatusChanged);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalLuminanceChanged, this, &CSExportPreview::SlotLuminanceChanged,Qt::QueuedConnection);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalContrastChanged, this, &CSExportPreview::SlotContrastChanged, Qt::QueuedConnection);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiForwardStep, this, &CSExportPreview::SlotSdiForwardStep); 
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiBackwardStep, this, &CSExportPreview::SlotSdiBackwardStep); 
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiDisplayControl, this, &CSExportPreview::SlotSdiDisplayControl);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiSwitchControl, this, &CSExportPreview::SlotSdiSwitchControl);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiFpsResolsListIndex, this, &CSExportPreview::SlotSdiFpsResolsListIndex);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiSpeedValue, this, &CSExportPreview::SlotSdiSpeedValue);
	ok = connect(m_displaySetting, &CSDisplaySetting::SignalSdiSecondScreen, this, &CSExportPreview::SlotSdiSecondScreen);
	ok = connect(m_targetScoring, &CSTargetScoring::SignalMeasuringGridVisible, this, &CSExportPreview::SlotMeasuringGridVisible);
	ok = connect(m_targetScoring, &CSTargetScoring::SignalPositionSelect, this, &CSExportPreview::SlotPositionSelect);
	ok = connect(m_targetScoring, &CSTargetScoring::SignalTargetSelect, this, &CSExportPreview::SlotTargetSelect);
	ok = connect(m_targetScoring, &CSTargetScoring::SignalOverlaImageVisible, this, &CSExportPreview::SlotOverlaImageVisible);
	ok = connect(m_targetScoring, &CSTargetScoring::SignalGridIntervalChanged, this, &CSExportPreview::SlotGridIntervalChanged);
	ok = connect(m_targetScoringControl, &CSCtrl::CSTargerScoring::SignalTargetScoringInfoChanged, this, &CSExportPreview::SlotTargetScoringInfoChanged);
	ok = connect(m_leftFrameBtn, &FrameButton::SignalEnterFrameBtn, m_pThumbnailManage, &CSThumbnailManage::SlotMouseInLeftBtn, Qt::QueuedConnection);
	ok = connect(m_leftFrameBtn, &FrameButton::SignalLeaveFrameBtn, m_pThumbnailManage, &CSThumbnailManage::SlotMouseOutLeftBtn, Qt::QueuedConnection);
	ok = connect(m_rightFrameBtn, &FrameButton::SignalEnterFrameBtn, m_pThumbnailManage, &CSThumbnailManage::SlotMouseInRightBtn, Qt::QueuedConnection);
	ok = connect(m_rightFrameBtn, &FrameButton::SignalLeaveFrameBtn, m_pThumbnailManage, &CSThumbnailManage::SlotMouseOutRightBtn, Qt::QueuedConnection);
	ok = connect(ui->snap_btn, &QPushButton::pressed, this, &CSExportPreview::slotSnapBtnPressed, Qt::QueuedConnection);

	Q_UNUSED(ok);
}

void CSExportPreview::SetVideoList(const QString& strIp)
{
	QStringList strList{};
	QList<VideoItem> current_video_items = VideoItemManager::instance().findVideoItems(VideoItem::Remote, strIp);
	qSort(current_video_items.begin(), current_video_items.end(), compareList);
	int nCurrent = -1;
	for (int i=0; i<current_video_items.size(); ++i)
	{
		strList << current_video_items[i].getName();
		int vid = VideoUtils::parseVideoSegmentId(current_video_items[i].getId());
		m_mapVideoList.insert(current_video_items[i].getName(), vid);
		ui->video_swtch_comboBox->addItem(current_video_items[i].getName(), QVariant::fromValue(vid));
		if (vid == m_nSwitchBoxOldVid)
		{
			nCurrent = i;
		}
	}
	ui->video_swtch_comboBox->setCurrentIndex(nCurrent);

	//刷新显示像素位深范围
	QVariantList list;
	m_devicePtr->getSupportedProperties(Device::PropDisplayBitDepth, list);
	int cur_dbd = m_devicePtr->getProperty(Device::PropDisplayBitDepth).toInt();
	for(int i = 0; i<list.size(); ++i)
	{
		ui->comboBox_display_bpp->addItem(DeviceUtils::getDisplayBitDepthText(list.at(i).toInt()), list.at(i).toInt() );
		if (cur_dbd == list.at(i).toInt())
		{
			ui->comboBox_display_bpp->setCurrentIndex(i);
		}
	}
	
}

void CSExportPreview::UapdateVideoInfo(const VideoItem & item)
{
	m_strIp = VideoUtils::parseDeviceIp(item.getId()); 
	m_strSwitchBoxDefaultText = item.getName();
	m_strSwitchBoxOldText = item.getName();
	m_nSwitchBoxOldVid = VideoUtils::parseVideoSegmentId(item.getId());

	UpdateTargetSetting();
	SetVideoList(m_strIp);
	SetVideoInfo(VideoInfoType::CAMERA_NAME, getCameraName(m_strIp));
	SetVideoInfo(VideoInfoType::RECORD_TIME, paraseShootTimeInfo(item));
	SetVideoInfo(VideoInfoType::TIME_LENGTH, item.getTimeLength()+("ms"));
	SetVideoInfo(VideoInfoType::RATE, item.getFrameRate());
	SetVideoInfo(VideoInfoType::RESOLUTION, item.getResolution());
	SetVideoInfo(VideoInfoType::DEPTH, QString::number( item.getValidBitsPerPixel()) + "bit");
	SetVideoInfo(VideoInfoType::TOTAL_FRAME, item.getTotalFrame());
	SetVideoInfo(VideoInfoType::EXPOSURE_TIME, QString::number(item.getExposureTime()) + " (100ns)");
	m_fps = item.getFPS();
	if (m_pExportPreviewPlayCtr && m_playerController)
	{
		m_pExportPreviewPlayCtr->SetTotalFrameCnt(m_playerController->GetVideoTotalFrameCount());
		UpadateDisplayCtrlStatus(true);
		//m_pExportPreviewPlayCtr->SetEnabled(true);
		m_pExportPreviewPlayCtr->SetFrameRate(m_fps);
		SetKeyFrameLabelValue(m_playerController->GetKeyFrameNo());
		m_uiOldEnd = m_playerController->GetVideoTotalFrameCount()-1;
	}
	m_uiFrameTotal = item.getTotalFrame().toLongLong();
	SetEditRange();
	ui->reset_pushButton->setEnabled(false);
}

void CSExportPreview::ReloadVideoInfo(const VideoItem & item)
{
	UpdateTargetSetting();
	
	SetVideoInfo(VideoInfoType::CAMERA_NAME, getCameraName(m_strIp));
	SetVideoInfo(VideoInfoType::RECORD_TIME, paraseShootTimeInfo(item));
	SetVideoInfo(VideoInfoType::TIME_LENGTH, item.getTimeLength() + ("ms"));
	SetVideoInfo(VideoInfoType::RATE, item.getFrameRate());
	SetVideoInfo(VideoInfoType::RESOLUTION, item.getResolution());
	SetVideoInfo(VideoInfoType::DEPTH, QString::number(item.getValidBitsPerPixel()) + "bit");
	SetVideoInfo(VideoInfoType::TOTAL_FRAME, item.getTotalFrame());
	SetVideoInfo(VideoInfoType::EXPOSURE_TIME, QString::number(item.getExposureTime()) + " (100ns)");
	m_fps = item.getFPS();
	m_bVideoStated = true;
	if (m_pExportPreviewPlayCtr && m_playerController)
	{
		m_pExportPreviewPlayCtr->SetTotalFrameCnt(m_playerController->GetVideoTotalFrameCount(), true);
		//m_pExportPreviewPlayCtr->SetEnabled(true);
		m_pExportPreviewPlayCtr->SetFrameRate(m_fps);
		SetKeyFrameLabelValue(m_playerController->GetKeyFrameNo());
		m_uiOldEnd = m_playerController->GetVideoTotalFrameCount() - 1;
	}
	m_uiFrameTotal = item.getTotalFrame().toLongLong();
	SetEditRange();
	ui->reset_pushButton->setEnabled(false);
}

void CSExportPreview::SwitchDisplayBtnStatus(const bool bPlay)
{
	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetPlayPauseStatus(bPlay);
	}
}

void CSExportPreview::UpadateDisplaySetting(const QString & deviceIp)
{
	if (m_playerController && m_displaySetting)
	{
		m_displaySetting->SetColorRevertCheckBoxStatus(m_playerController->isAntiColorEnable());
		m_displaySetting->SetluminanceValue(m_playerController->GetLuminance());
		m_displaySetting->SetContrastValue(m_playerController->GetContrast());
	}
	//获取到当前视频对应的设备
	m_devicePtr = DeviceManager::instance().getDevice(m_strIp);
	auto connect_type = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
	bool ok = connect(m_devicePtr.data(), &Device::networkErrorOccurred, this, &CSExportPreview::SlotErrorOccurred, connect_type);
	Q_UNUSED(ok);
	if (m_devicePtr && m_displaySetting)
	{
		if (m_devicePtr->IsSdiPlaybackCtrlSupported())
		{
			//分辨率&播放帧率
			QVariantList sdiVarList;
			m_devicePtr->getSupportedProperties(Device::PropType::PropSdiFpsResols, sdiVarList);
			QList<QString> sdiList;
			for (auto &item : sdiVarList)
			{
				QString strItem = DeviceUtils::getSdiFpsResolsText(item.toInt());
				sdiList.push_back(strItem);
			}
			m_displaySetting->ClearSdiFpsResolsList();
			m_displaySetting->SetSdiFpsResolsList(sdiList);

			//播放倍速列表
			QStringList speedsList{};
			m_devicePtr->GetSupportedSdiPlaySpeeds(speedsList);
			m_displaySetting->ClearSdiDisplaySpeedsList();
			m_displaySetting->SetSdiDisplaySpeeds(speedsList);

			//设置当前分辨率&播放帧率列表
			m_displaySetting->SetCurrentSdiFpsResol(m_devicePtr->getProperty(Device::PropType::PropSdiFpsResols).toInt());
			
			//设置当前播放倍速
			uint32_t start_frame_no{0};
			uint32_t end_frame_no{0};
			double interval{0};
			HscResult ret = m_devicePtr->GetSdiRange(start_frame_no, end_frame_no, interval);
			if (HSC_OK == ret)
			{
				m_displaySetting->SetCurrentSdiDisplaySpeed(QString::number(interval,'f',QLocale::FloatingPointShortest));
			}
		}
		else
		{
			m_displaySetting->SDIVisible(false);
		}
	}
}

void CSExportPreview::UpdateTargetSetting()
{
	if (m_targetScoringControl && m_targetScoring)
	{
		if (m_targetScoringControl->AllowsTargetScoring())
		{
			int iMin{};
			int iMax{};
			m_targetScoringControl->GetGridIntervalRange(iMin, iMax);
			m_targetScoring->SetGridSliderIntervalRange(iMin, iMax);
			m_targetScoring->SetGridSliderInterval(m_targetScoringControl->GetGridInterval());
			if ((-1==m_targetScoringControl->GetBurstPoint().x()) && (-1==m_targetScoringControl->GetBurstPoint().y()))
			{
				m_targetScoring->SetScoringTargetResultExit(false);
			}
			else
			{
				QPointF fallPoint{};
				float dis{};
				if (m_targetScoringControl->GetTargetScoringInfo(fallPoint, dis)==CSCtrl::CSTargerScoring::TSR_OK)
				{
					m_targetScoring->SetScoringTargetResultExit(true);
					m_targetScoring->SetTargetScoringInfo(fallPoint,dis);
				}
			}
		}
		else
		{
			m_targetScoring->UpdateTargetScoringSettingStatus(false);
			m_targetScoring->SetCalFileExit(false);
		}
	}
}

void CSExportPreview::SetKeyFrameLabelValue(const int64_t keyFrameNo)
{
	QString strKey{};
	if (m_bFrame)
	{
		strKey = tr("The") + QString(" ") + QString::number(keyFrameNo + 1) + QString(" ") + tr("Frame");
	}
	else
	{
		QString strMs = QString::number(VideoUtils::frameIdToMs(keyFrameNo+1, m_fps), 'f', 1);
		strKey = tr("The") + QString(" ") + strMs + QString(" ") + QString("ms");
	}
	ui->key_frame_value_label->setText(strKey);
}

void CSExportPreview::ForbidSwitchVideoCombobox(const bool bForbid)
{
	ui->video_swtch_comboBox->setDisabled(bForbid);
	m_bForbidSwitchBox = bForbid;
}

void CSExportPreview::ForbidDisplayWindowControls(const bool bForbid)
{
	m_bKeyboardUsed = !bForbid;
	ui->video_swtch_comboBox->setDisabled(bForbid);
	ui->tabWidget->setTabEnabled(0, !bForbid);
	m_pThumbnailManage->setDisabled(bForbid);
	m_pExportPreviewPlayCtr->setDisabled(bForbid);//进度条置灰
	ui->reset_pushButton->setDisabled(bForbid);
	ui->lastTime_pushButton->setDisabled(bForbid && m_bEnableLastButton);
	ui->export_pushButton->setDisabled(bForbid);
}

void CSExportPreview::UpadateDisplayCtrlStatus(bool bUsed)
{
	m_bKeyboardUsed = bUsed;
	ui->reset_pushButton->setEnabled(bUsed);
	ui->lastTime_pushButton->setEnabled(bUsed && m_bEnableLastButton);
	ui->export_pushButton->setEnabled(bUsed);
	ui->doubleSpinBox_begin->setEnabled(bUsed);
	ui->doubleSpinBox_end->setEnabled(bUsed);
}

void CSExportPreview::UpadateSdiRange(const int64_t & iStartFrameNo, const int64_t & iEndFrameNo)
{
	UpadateRange(iStartFrameNo, iEndFrameNo);
	if (m_devicePtr && !m_devicePtr->IsSdiPlaybackCtrlSupported())  return;
	if (m_devicePtr)
	{
		uint32_t start_frame_no{};
		uint32_t end_frame_no{};
		double interval{};
		HscResult ret = m_devicePtr->GetSdiRange(start_frame_no, end_frame_no, interval);
		if (HSC_OK == ret)
		{
			m_devicePtr->ApplySdiRange(iStartFrameNo, iEndFrameNo, interval);
		}
	}
}

void CSExportPreview::UpadateRange(const int64_t & iStartFrameNo, const int64_t & iEndFrameNo)
{//to do: 区间联动，暂时放在UpadateSdiRange接口内
	if (m_bFrame)
	{
		m_begin_old_value = iStartFrameNo + 1;
		m_end_old_value = iEndFrameNo + 1;
		ui->doubleSpinBox_begin->setValue(iStartFrameNo + 1);
		ui->doubleSpinBox_end->setValue(iEndFrameNo + 1);
	}
	else
	{
		m_end_old_value = VideoUtils::frameIdToMs(iEndFrameNo + 1, m_fps);
		ui->doubleSpinBox_begin->setValue(VideoUtils::frameIdToMs(iStartFrameNo + 1, m_fps));
		ui->doubleSpinBox_end->setValue(m_end_old_value);
	}
	UpdateSkipFrameInfo();
}

void CSExportPreview::closeEvent(QCloseEvent * e)
{
	if (m_bOpenClosePopup)
	{
		QMessageBox infoBox(this);
		infoBox.setWindowTitle(POPUP_TITLE);
		infoBox.addButton(tr("OK"), QMessageBox::YesRole);
		infoBox.addButton(tr("NO"), QMessageBox::NoRole);
		infoBox.setText(tr("Close the playback interface?\nAfter clicking OK,the current operation will not be saved\nand the playback \
interface will be closed"));
		if (1 == infoBox.exec())
		{
			e->ignore();
			return;
		}
	}
	//关闭回放界面之前先停止全部回放操作
	m_playerController->DisableGetFrame(true);
	m_bInit = false;
	m_strSwitchBoxDefaultText = QString{};
	m_nSwitchBoxOldVid = -1;
}

void CSExportPreview::keyPressEvent(QKeyEvent * ev)
{
	if (m_bKeyboardUsed)
	{
		if (ev->key() == Qt::Key_Left)
		{
			on_forward_frame_btn_clicked();
		}
		else if (ev->key() == Qt::Key_Right)
		{
			on_backward_frame_btn_clicked();
		}
	}
}

bool CSExportPreview::event(QEvent *e)
{
	if (e){
		auto type = e->type();
		if (type == QEvent::Show)
		{
			emit signalExportpreviewShowMain(true);
		}
		else if (type == QEvent::Hide)
		{
			if (m_nSwitchBoxOldVid > -1)
			{
				emit signalExportpreviewShowMain(false);
			}
			else {
				emit signalExportpreviewShowMain(true);
			}
		}
	}
	return QDialog::event(e);
}

QString CSExportPreview::paraseShootTimeInfo(const VideoItem & item)
{
	QString strTime = item.getTimeStamp();
	if (!strTime.isEmpty())
	{
		QStringList strList = strTime.split("-");
		if (!strList[2].isEmpty())
		{
			QStringList strList_ = strList[2].split(" ");
			// Modify by Juwc -- 2022/7/6
			//strTime = strList[0] + tr("year") + strList[1] + tr("month") + strList_[0]\
				+ tr("day") + strList_[1];
			strTime = strList[0] + "/" + strList[1] + "/" + strList_[0]\
				+ " " + strList_[1];
		}
	}
	return strTime;
}

void CSExportPreview::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	m_leftFrameBtn->setFixedSize(20, ui->thumbnail_control_widget->height()-4);
	m_rightFrameBtn->setFixedSize(20, ui->thumbnail_control_widget->height()-4);
}

void CSExportPreview::SlotKeyFrame(const int64_t frameNo)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		m_playerController->SetKeyFrameNo(frameNo);
	}

	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetTriggerFrame(frameNo);
	}
	SetKeyFrameLabelValue(frameNo);

	if(m_playerController)
	{
		int64_t nKeyFrame = m_playerController->GetOriginKeyFrameNo();
		if (nKeyFrame == frameNo)
		{
			ui->reset_pushButton->setEnabled(false);
		}
		else
		{
			ui->reset_pushButton->setEnabled(true);
		}
	}
	else
	{
		ui->reset_pushButton->setEnabled(true);
	}
}

void CSExportPreview::SlotBeginFrame(const int64_t frameNo)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		ui->spinBox_skip_frame_value->setEnabled(false);
		int64_t uiNew = m_playerController->GetEndFrameNo();
		m_playerController->SwitchRange(frameNo, uiNew);
		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetPlayerRange(frameNo, uiNew);
		}

#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
		if (m_pThumbnailManage)
		{
			m_pThumbnailManage->updateSelectedThumbnail(frameNo);
		}
#endif
		UpadateSdiRange(frameNo, uiNew);
		m_uiOldEnd = uiNew;
		m_uiOldStart = frameNo;
	}
}

void CSExportPreview::SlotSpeedAllClicked()
{
}

void CSExportPreview::SlotSpeedBtnClicked(int index)
{
}

void CSExportPreview::on_spinBox_skip_frame_value_editingFinished()
{
	int value = ui->spinBox_skip_frame_value->value();
	setPalySpeed(value);
}

void CSExportPreview::setPalySpeed(int nSpeed)
{
	m_iPlaySpeed = nSpeed + 1;
	if (m_iPlaySpeed >= m_uiOldEnd - m_uiOldStart) {
		m_iPlaySpeed = m_uiOldEnd - m_uiOldStart;
		int nUiValue = m_iPlaySpeed;
		if (nUiValue > 0)
		{
			nUiValue--;
		}
		else
		{
			nUiValue = 0;
		}
		ui->spinBox_skip_frame_value->setValue(nUiValue);
	}
	if (m_iPlaySpeed < 1) m_iPlaySpeed = 1;
	if (m_playerController && m_iPlaySpeed > 0)
	{
		if (m_playerController->GetSpeed() != m_iPlaySpeed)
		{
			m_playerController->SwitchSpeed(m_iPlaySpeed);
		}
		//m_playerController->SkipFrame(m_iPlaySpeed);
	}
}

void CSExportPreview::SlotDisplayControl(bool bPlay)
{
	if (!bPlay)
	{
		m_pExportPreviewPlayCtr->setFocus();
	}

	PlayState state = bPlay ? PS_PLAY : PS_PAUSE;
	if (m_playerController)
	{
		if (PS_PLAY == state && m_iPlaySpeed != m_playerController->GetSpeed())
		{
			m_playerController->SwitchSpeed(m_iPlaySpeed);
		}
		m_playerController->SwitchState(state);
		m_bVideoStated = bPlay;
	}

	if (m_player)
	{
		m_player->SetExportDisplayBtnStatus(!bPlay);
	}
}

void CSExportPreview::SlotRevertCheckBoxStatusChanged(bool bChecked)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		m_playerController->SetAntiColorEnable(bChecked);
	}
}

void CSExportPreview::SlotLuminanceChanged(const int value)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		m_playerController->SetLuminance(value);
	}
}

void CSExportPreview::SlotContrastChanged(const int value)
{
	m_bOpenClosePopup = true;
	if (m_playerController)
	{
		m_playerController->SetContrast(value);
	}
}

void CSExportPreview::SlotSdiForwardStep()
{
	if (m_devicePtr)
	{
		m_devicePtr->SdiBackward();
	}
}

void CSExportPreview::SlotSdiBackwardStep()
{
	if (m_devicePtr)
	{
		m_devicePtr->SdiForward();
	}
}

void CSExportPreview::SlotSdiDisplayControl(bool bPlay)
{
	ForbidSwitchVideoCombobox(bPlay);
	if (m_devicePtr)
	{
		HscSDIState state = bPlay ? HSC_SDI_PLAY : HSC_SDI_PAUSE;
		m_devicePtr->SetSdiState(state);
	}
}

void CSExportPreview::SlotSdiSwitchControl(bool bOpened, const double dSpeed)
{
	m_bOpenClosePopup = true;
	do 
	{
		if (m_devicePtr)
		{
			if (bOpened)
			{
				QMessageBox warningBox(this);
				warningBox.setWindowTitle(POPUP_TITLE);
				warningBox.addButton(tr("OK"), QMessageBox::YesRole);
				DeviceState currentState = m_devicePtr->getState();
				if ((DeviceState::Unconnected == currentState)|| (DeviceState::Connecting == currentState)\
					|| (DeviceState::Disconnecting == currentState) || (DeviceState::Disconnected == currentState)\
					|| (DeviceState::Reconnecting == currentState))
				{
					warningBox.setIcon(QMessageBox::Critical);
					warningBox.setText(CAMERA_DISCONNECT);
					warningBox.exec();
					if (m_displaySetting)
					{
						m_displaySetting->SetSdiCurrentStatus(false);
						m_displaySetting->SetSdiButtonStatus(false);
					}
					break;
				}

				if (!m_bSdiboxForbid)
				{
					if (m_playerController && (dSpeed > (m_playerController->GetEndFrameNo() - m_playerController->GetStartFrameNo())))
					{
						QMessageBox msgBox(this);
						QString strMsg = tr("The total length of SDI playback is less than\nthe current set playback speed!");
						msgBox.setWindowTitle(POPUP_TITLE);
						msgBox.setText(strMsg);
						msgBox.setIcon(QMessageBox::Information);
						msgBox.addButton(tr("OK"), QMessageBox::YesRole);
						m_popupSdiCheckBox = new QCheckBox(&msgBox);
						connect(m_popupSdiCheckBox, &QCheckBox::clicked, this, &CSExportPreview::SlotPopupSdiCheckBox);
						m_popupSdiCheckBox->setText(NOT_TIP);
						msgBox.setCheckBox(m_popupSdiCheckBox);
						int ret = msgBox.exec();
						m_bSdiboxForbid = m_bSdiBoxChecked ? true : false;
						m_bSdiBoxChecked = false;
					}
				}
			}
			
			HscSDIState state = bOpened ? HSC_SDI_PLAY : HSC_SDI_STOP;
			HscResult ret = m_devicePtr->SetSdiState(state);
			if (HSC_OK == ret)
			{
				if (m_displaySetting)
				{
					m_displaySetting->SetSdiCurrentStatus(bOpened);
					m_displaySetting->SetSdiButtonStatus(bOpened);
				}
			}
			else
			{
				QMessageBox warningBox(this);
				warningBox.setWindowTitle(POPUP_TITLE);
				warningBox.addButton(tr("OK"), QMessageBox::YesRole);
				warningBox.setIcon(QMessageBox::Information);
				warningBox.setText(tr("This video clip has not been obtained\nand cannot be opened!"));
				warningBox.exec();
				if (m_displaySetting)
				{
					m_displaySetting->SetSdiCurrentStatus(false);
					m_displaySetting->SetSdiButtonStatus(false);
				}
				break;
			}
		}
	} while (false);
}

void CSExportPreview::SlotSdiFpsResolsListIndex(const int index)
{
	if (m_devicePtr)
	{
		m_devicePtr->setProperty(Device::PropType::PropSdiFpsResols, QVariant::fromValue(index));
	}
}

void CSExportPreview::SlotSdiSpeedValue(const double value)
{
	if (m_devicePtr && m_playerController)
	{
		m_devicePtr->ApplySdiRange(m_playerController->GetStartFrameNo(), m_playerController->GetEndFrameNo(), value);
	}
}

void CSExportPreview::SlotSdiSecondScreen(bool bOpen)
{
	m_bOpenClosePopup = true;
	do
	{
		if (nullptr == m_pSecondScreen)
		{
			if(!m_currentImgInfo.image.isNull()){
				auto frame_head_type = m_devicePtr->getFrameHeadType();
				m_pSecondScreen = new CSSecondScreen(m_currentImgInfo, frame_head_type ,this);
				connect(m_pSecondScreen, &CSSecondScreen::SecondScreenCloseSignal, this, &CSExportPreview::OnSecondScreenClose);
			}
			else {
				QMessageBox warningBox(this);
				warningBox.setIcon(QMessageBox::Information);
				warningBox.setWindowTitle(POPUP_TITLE);
				warningBox.addButton(tr("OK"), QMessageBox::YesRole);
				warningBox.setText(tr("The image information cannot be obtained,\nand the screen projection cannot be turned on!"));
				warningBox.exec();
				if (m_displaySetting)
				{
					m_displaySetting->SetSecondScreenBtnStatus(false);
				}
				break;
			}
		}

		if (!bOpen)
		{
			if (m_pSecondScreen)
			{
				disconnect(m_pSecondScreen, &CSSecondScreen::SecondScreenCloseSignal, this, &CSExportPreview::OnSecondScreenClose);
				delete m_pSecondScreen;
				m_pSecondScreen = nullptr;
			}
		}

		if (m_pSecondScreen)
		{
			if (bOpen)
			{
				QMessageBox warningBox(this);
				warningBox.setWindowTitle(POPUP_TITLE);
				warningBox.addButton(tr("OK"), QMessageBox::YesRole);
				if (m_devicePtr)
				{
					DeviceState currentState = m_devicePtr->getState();
					if ((DeviceState::Unconnected == currentState) || (DeviceState::Connecting == currentState)\
						|| (DeviceState::Disconnecting == currentState) || (DeviceState::Disconnected == currentState)\
						|| (DeviceState::Reconnecting == currentState))
					{
						warningBox.setIcon(QMessageBox::Critical);
						warningBox.setText(CAMERA_DISCONNECT);
						warningBox.exec();
						if (m_displaySetting)
						{
							m_displaySetting->SetSecondScreenBtnStatus(false);
						}
						break;
					}
					else if (m_currentImg.isNull())
					{
						warningBox.setIcon(QMessageBox::Information);
						warningBox.setText(tr("The image information cannot be obtained,\nand the screen projection cannot be turned on!"));
						warningBox.exec();
						if (m_displaySetting)
						{
							m_displaySetting->SetSecondScreenBtnStatus(false);
						}
						break;
					}
				}
			}

			//成功开启后再禁用切换视频下拉框
			ForbidSwitchVideoCombobox(bOpen);

			if (bOpen)
			{
				//投屏显示在副屏
				QDesktopWidget* desktop = QApplication::desktop();//获取屏幕对象
				int screenNum = desktop->screenCount();//获取屏幕个数
				int mainScreenID = desktop->primaryScreen();//获取主屏幕索引
				if (screenNum > 1)
				{
					m_pSecondScreen->setGeometry(desktop->availableGeometry(mainScreenID + 1));
				}
				else
				{
					QMessageBox infoBox(this);
					infoBox.setWindowTitle(POPUP_TITLE);
					infoBox.addButton(tr("OK"), QMessageBox::YesRole);
					infoBox.addButton(tr("NO"), QMessageBox::NoRole);
					infoBox.setText(tr("The system does not detect the large screen,\nwhether to continue to display the screen?"));
					int iRet = infoBox.exec();
					if (1 == iRet)
					{
						if (m_displaySetting)
						{
							m_displaySetting->SetSecondScreenBtnStatus(false);
						}
						//退出信号进入此接口，即启用切换视频下拉框
						ForbidSwitchVideoCombobox(false);
						break;
					}
				}
			}
			
			auto connect_type = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);
			bool ok = connect(m_playerController, &CSPlaybackController::ImageUpdate, m_pSecondScreen, &CSSecondScreen::SlotImageUpdate, connect_type);
			Q_UNUSED(ok);
			bOpen ? m_pSecondScreen->show() : m_pSecondScreen->hide();
			m_pSecondScreen->setWindowState(Qt::WindowMaximized);
		}
		//退出信号进入此接口，即启用切换视频下拉框
		if (!bOpen)
		{
			ForbidSwitchVideoCombobox(bOpen);
		}
	} while (false);
}

void CSExportPreview::OnSecondScreenClose()
{
	// 设置投屏按钮状态
	if (m_displaySetting)
		m_displaySetting->SetSecondScreenBtnStatus(false);

	if (m_pSecondScreen)
	{
		disconnect(m_pSecondScreen, &CSSecondScreen::SecondScreenCloseSignal, this, &CSExportPreview::OnSecondScreenClose);
		disconnect(m_playerController, &CSPlaybackController::ImageUpdate, m_pSecondScreen, &CSSecondScreen::SlotImageUpdate);

		delete m_pSecondScreen;
		m_pSecondScreen = nullptr;
	}

	// 启用切换视频下拉框
	ForbidSwitchVideoCombobox(false);
}

void CSExportPreview::SlotMeasuringGridVisible(const bool bShow)
{
	m_bGridVisible = bShow;
	if (m_playerController)
	{
		m_playerController->RefreshImage();
	}
}

void CSExportPreview::SlotPositionSelect()
{
	ForbidDisplayWindowControls(true);
	if (m_playerController)
	{
		if (PS_PLAY == m_playerController->GetState())
		{
			m_playerController->SeekFrame(m_playerController->GetKeyFrameNo());

			{
				m_playerController->SwitchState(PS_PAUSE);
				m_bVideoStated = false;
			}
		}
		m_bPosManualSelectClicked = true;
	}

	if (m_player)
	{
		m_player->setCursor(Qt::CrossCursor);
		m_player->SetManualSelectCloseBtnVisible(true);
		m_player->SetExportDisplayBtnStatus(true);
	}
}

void CSExportPreview::SlotTargetSelect()
{
	ForbidDisplayWindowControls(true);
	if (m_playerController)
	{
		if (PS_PLAY == m_playerController->GetState())
		{
			m_playerController->SwitchState(PS_PAUSE);
			m_bVideoStated = false;
		}
		m_bTargetManualSelectClicked = true;
	}

	if (m_player)
	{
		m_player->setCursor(Qt::CrossCursor);
		m_player->SetManualSelectCloseBtnVisible(true);
		m_player->SetExportDisplayBtnStatus(true);
	}
}

void CSExportPreview::SlotMousePressPointF(const QPointF & point)
{
	m_bOpenClosePopup = true;
	if (m_targetScoringControl && m_playerController)
	{
		if (m_bPosManualSelectClicked)
		{
			m_targetScoringControl->SetCenterPoint(point);
			m_playerController->RefreshImage();
		}

		if (m_bTargetManualSelectClicked)
		{
			m_targetScoringControl->SetBurstPoint(point);
			m_playerController->RefreshImage();
		}
	}
}

void CSExportPreview::SlotOverlaImageVisible(const bool bShow)
{
	if (m_player)
	{
		if (bShow)
		{
			if (m_targetScoringControl && m_targetScoring)
			{
				if ((-1 == m_targetScoringControl->GetBurstPoint().x()) && (-1 == m_targetScoringControl->GetBurstPoint().y()))
				{
					m_targetScoring->SetScoringTargetResultExit(false);
				}
				else
				{
					QPointF fallPoint{};
					float dis{};
					m_targetScoringControl->GetTargetScoringInfo(fallPoint, dis);
					m_player->SetTargetScoringInfo(bShow, fallPoint, dis);
				}
			}
		}
		else
		{
			m_player->SetTargetScoringInfo(bShow, QPointF{}, double{});
		}
	}
	
}

void CSExportPreview::SlotCloseManualSelect()
{
	ForbidDisplayWindowControls(false);
	if (m_bPosManualSelectClicked)
	{
		m_bPosManualSelectClicked = false;
	}

	if (m_bTargetManualSelectClicked)
	{
		m_bTargetManualSelectClicked = false;
	}

	if (m_targetScoring)
	{
		m_targetScoring->SetPosManualSelectBtnEnabled(true);
		m_targetScoring->SetTargetManualSelectBtnEnabled(true);
	}

	if (m_playerController)
	{
		m_playerController->SwitchState(PS_PLAY);
		m_bVideoStated = true;
	}
}

void CSExportPreview::SlotGridIntervalChanged(const int interval)
{
	if (m_targetScoringControl && m_playerController)
	{
		m_targetScoringControl->SetGridInterval(interval);
		m_playerController->RefreshImage();
	}
}

void CSExportPreview::SlotTargetScoringInfoChanged()
{
	if (m_targetScoringControl && m_targetScoring)
	{
		if ((-1 == m_targetScoringControl->GetBurstPoint().x()) && (-1 == m_targetScoringControl->GetBurstPoint().y()))
		{
			m_targetScoring->SetScoringTargetResultExit(false);
		}
		else
		{
			QPointF fallPoint{};
			float dis{};
			m_targetScoringControl->GetTargetScoringInfo(fallPoint, dis);
			m_targetScoring->SetScoringTargetResultExit(true);
			m_targetScoring->SetTargetScoringInfo(fallPoint, dis);
		}
	}
}

void CSExportPreview::SlotMoveOutThumbnailArea()
{
	if (m_playerController)
	{
		if (!m_bOldPlayState)
		{
			m_playerController->SeekFrame(m_playerController->GetCurrentFrameNo());
		}
		
		if (m_bOldPlayState && (PlayState::PS_PLAY != m_playerController->GetState()))
		{
			{
				m_playerController->SwitchState(PlayState::PS_PLAY);
				m_bVideoStated = true;
			}

			if (m_player)
			{
				m_player->SetExportDisplayBtnStatus(false);
			}
			if (m_pExportPreviewPlayCtr)
			{
				m_pExportPreviewPlayCtr->SetPlayPauseStatus(true);
			}
			m_bOldPlayState = false;
		}
	}
}

void CSExportPreview::SlotSingleThumbnailClicked(const uint64_t & frameNo)
{
	if (m_playerController)
	{
		m_playerController->SeekFrame(frameNo);
	}

	if (m_pExportPreviewPlayCtr)
	{
		//m_pExportPreviewPlayCtr->SetCurFrameIndex(frameNo);
	}
}

void CSExportPreview::SlotSwitchFrameAndMs(const bool bFrame)
{
	m_bFrame = bFrame;
	if (m_playerController)
	{
		SetKeyFrameLabelValue(m_playerController->GetKeyFrameNo());
	}
	SetEditRange();
}

void CSExportPreview::SlotPopupSwitchCheckBox(bool bChecked)
{
	m_bSwictchBoxChecked = bChecked;
}

void CSExportPreview::SlotPopupSdiCheckBox(bool bChecked)
{
	m_bSdiBoxChecked = bChecked;
}

void CSExportPreview::SlotErrorOccurred(quint64 error)
{
	QMessageBox warningBox(this);
	warningBox.setWindowTitle(POPUP_TITLE);
	warningBox.addButton(tr("OK"), QMessageBox::YesRole);
	warningBox.setIcon(QMessageBox::Critical);
	warningBox.setText(CAMERA_DISCONNECT);
	warningBox.exec();
}

void CSExportPreview::SlotFrameStepStatus(int type, bool enable)
{
	if (1 == type)
	{
		m_leftFrameBtn->setEnabled(enable);
	}
	else if (2 == type)
	{
		m_rightFrameBtn->setEnabled(enable);
	}
}

void CSExportPreview::SlotThumbnailRange(const uint64_t & iStart, const uint64_t & iEnd)
{
	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetThumbnailRange(iStart, iEnd);
	}
}

void CSExportPreview::SlotThumbnailLoadingStarted()
{
	enableSpeedCtrlBtns(false);
}

void CSExportPreview::thumbnailLoadingFinished(bool ok)
{
	if (m_playerController)
	{
		if (m_bVideoStated)
		{
			if (m_uiOldEnd - m_uiOldStart < m_iPlaySpeed)
			{
				m_iPlaySpeed = m_uiOldEnd - m_uiOldStart;
				m_playerController->setSpeed(m_iPlaySpeed);
			}

			m_playerController->SwitchState(PS_PLAY);

		}

		/*if (m_pThumbnailManage)
		{
			m_pThumbnailManage->updateSelectedThumbnail(m_playerController->GetStartFrameNo());
		}*/
	}
}

void CSExportPreview::SlotThumbnailLoadingFinished(bool ok)
{
	if (m_pThumbnailManage && m_playerController)
	{
		m_pThumbnailManage->updateSelectedThumbnail(m_playerController->GetStartFrameNo());
	}

	if (m_pExportPreviewPlayCtr)
	{
		m_pExportPreviewPlayCtr->SetEnabled(true);
	}

	if (m_playerController) {
		m_uiOldStart = m_playerController->GetStartFrameNo();
		m_uiOldEnd = m_playerController->GetEndFrameNo();
	}
	enableSpeedCtrlBtns(true);
#ifdef _WINDOWS
	sThumbnailTrack.mEndThumbnail = GetTickCount64();
#else
	sThumbnailTrack.mEndThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
	sThumbnailTrack.printLog();
	m_playerController->resetSwitchRangeFlag();
	ui->spinBox_skip_frame_value->setEnabled(true);
	if (m_uiOldEnd - m_uiOldStart < m_iPlaySpeed)
	{
		m_iPlaySpeed = m_uiOldEnd - m_uiOldStart; 
	
		ui->spinBox_skip_frame_value->blockSignals(true);
		ui->spinBox_skip_frame_value->setValue(m_iPlaySpeed - 1);
		ui->spinBox_skip_frame_value->blockSignals(false);
	}
	//if (mLastGetFrame != frame_no) CSLOG_INFO("AGCacheStream::getFrame {},{}", mLastGetFrame, frame_no);
}

void CSExportPreview::slotRangeStackChange(int nSize)
{
	if (nSize > 0)
	{
		m_bEnableLastButton = true;
	}
	else
	{
		m_bEnableLastButton = false;
	}
	ui->lastTime_pushButton->setEnabled(m_bEnableLastButton);
}

void CSExportPreview::on_doubleSpinBox_begin_editingFinished()
{
	uint64_t uiValue = m_uiOldStart;
	int64_t nValue = m_uiOldStart;
	double dbValue = ui->doubleSpinBox_begin->value();

	double dEndValue = ui->doubleSpinBox_end->value();
	if (dbValue > dEndValue) {
		ui->doubleSpinBox_begin->setValue(m_begin_old_value);
		return;
	}
	m_begin_old_value = dbValue;
	if (m_bFrame)
	{
		nValue = dbValue;
	}
	else
	{
		nValue = VideoUtils::msToFrameId(dbValue, m_fps);
	}
	nValue -= 1;
	if (nValue < 0)
	{
		nValue = 0;
	}
	uiValue = nValue;
	if (uiValue == m_uiOldStart)
	{
		uiValue++;
		if (uiValue >= m_uiFrameTotal)
		{
			uiValue = m_uiFrameTotal;
		}
		if (m_bFrame)
		{
			ui->doubleSpinBox_begin->setValue(uiValue);
		}
		else
		{
			ui->doubleSpinBox_begin->setValue(VideoUtils::frameIdToMs(uiValue, m_fps));
		}
		return;
	}
	SlotBeginFrame(uiValue);
	UpdateSkipFrameInfo();
}

void CSExportPreview::on_doubleSpinBox_end_editingFinished()
{
	uint64_t uiValue = m_uiOldEnd;
	int64_t nValue = m_uiOldEnd;
	double dbValue = ui->doubleSpinBox_end->value();
	double dBeginValue = ui->doubleSpinBox_begin->value();
	if (dbValue < dBeginValue) 
	{
		ui->doubleSpinBox_end->setValue(m_end_old_value);
		return;
	}
	m_end_old_value = dbValue;
	if (m_bFrame)
	{
		nValue = dbValue;
	}
	else
	{
		nValue = VideoUtils::msToFrameId(dbValue, m_fps);
	}
	nValue -= 1;
	if (nValue < 0)
	{
		nValue = 0;
	}
	uiValue = nValue;
	if (uiValue >= m_uiFrameTotal)
	{
		uiValue = m_uiFrameTotal - 1;
	}
	if (uiValue == m_uiOldEnd)
	{
		uiValue++;
		if (uiValue >= m_uiFrameTotal)
		{
			uiValue = m_uiFrameTotal;
		}
		if (m_bFrame)
		{
			ui->doubleSpinBox_end->setValue(uiValue);
		}
		else
		{
			ui->doubleSpinBox_end->setValue(VideoUtils::frameIdToMs(uiValue, m_fps));
		}
		return;
	}
	SlotEndFrame(uiValue);
	UpdateSkipFrameInfo();
}

void CSExportPreview::on_reset_pushButton_clicked()
{
	if (m_playerController)
	{
		m_playerController->ResetKeyFrameNo();

		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetTriggerFrame(m_playerController->GetKeyFrameNo());
		}
		SetKeyFrameLabelValue(m_playerController->GetKeyFrameNo());
		ui->reset_pushButton->setEnabled(false);
	}
}

void CSExportPreview::on_lastTime_pushButton_clicked()
{
	if (m_playerController)
	{
		m_playerController->SwitchToPreviousRange();

		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetPlayerRange(m_playerController->GetStartFrameNo(), m_playerController->GetEndFrameNo());
		}
		SetEditRange();

		if (m_pExportPreviewPlayCtr)
		{
			m_playerController->SeekFrame(m_playerController->GetStartFrameNo());
			m_pExportPreviewPlayCtr->SetCurFrameIndex(m_playerController->GetStartFrameNo());
		}
	}
}

void CSExportPreview::on_export_pushButton_clicked()
{
	if (m_playerController)
	{
		m_playerController->DisableGetFrame(true);//禁用当前获取图像相关操作
		PlayState state = m_playerController->GetState();
		if (state != PS_STOP)
		{
			m_playerController->SwitchState(PS_STOP);
		}

		bool temp = m_bVideoStated;
		m_bVideoStated = false;

		VideoItem item = m_playerController->GetVideoItem();//获取编辑之后的videoItem
		//开启导出对话框,传入需要导出的视频项,准备导出视频
		{
			CSDlgDeviceVideoExport dlg(std::make_pair(m_current_idx, item), false, this);
			dlg.exec();
		}

		m_playerController->DisableGetFrame(false);//取消禁用获取图像相关操作
		//导出结束后恢复播放状态
		m_playerController->StartLoadThumbnails();
		m_bVideoStated = temp;
	}
}

void CSExportPreview::on_video_swtch_comboBox_currentIndexChanged(const QString &arg1)
{
	do 
	{
		if (!m_bInit)
		{
			m_bInit = true;
			break;
		}

		int nCurrentVid = ui->video_swtch_comboBox->currentData().toInt();
		if (nCurrentVid == m_nSwitchBoxOldVid)
		{
			break;
		}
// 		if (m_strSwitchBoxOldText == arg1)
// 		{
// 			break;
// 		}
		int nOldIndex = -1;
		int nCount = ui->video_swtch_comboBox->count();
		for (int i = 0; i < nCount; ++i)
		{
			int nItemData = ui->video_swtch_comboBox->itemData(i).toInt();
			if (nItemData == m_nSwitchBoxOldVid)
			{
				nOldIndex = i;
				break;
			}
		}
		// [2022/8/19 rgq]: 添加是否支持预览的判断，不支持的不进入预览界面
		if (!GetEnableExport(QString::number(nCurrentVid)))
		{
			//ui->video_swtch_comboBox->setCurrentText(m_strSwitchBoxOldText);
			ui->video_swtch_comboBox->setCurrentIndex(nOldIndex);
			UIUtils::showErrorMsgBox(this, QObject::tr("The current protocol format does not support playback. Please change the protocol format and try again."));
			return;
		}

		if (!m_bSwitchBoxForbid)
		{
			QMessageBox msgBox(this);
			QString strMsg = tr("Switch open video") + QString("\"%1\"?\n").arg(arg1) + tr("After switching,the current action\
will not be saved and this video will open,") + QString("\n") + tr("continue") + QString("?");
			msgBox.setWindowTitle(POPUP_TITLE);
			msgBox.setText(strMsg);
			msgBox.addButton(tr("OK"), QMessageBox::YesRole);
			msgBox.addButton(tr("NO"), QMessageBox::NoRole);
			m_popupSwitchCheckBox = new QCheckBox(&msgBox);
			connect(m_popupSwitchCheckBox, &QCheckBox::clicked, this, &CSExportPreview::SlotPopupSwitchCheckBox);
			m_popupSwitchCheckBox->setText(NOT_TIP);
			msgBox.setCheckBox(m_popupSwitchCheckBox);
			int ret = msgBox.exec();
			if (1 == ret)
			{
				m_bSwictchBoxChecked = false;
				//ui->video_swtch_comboBox->setCurrentText(m_strSwitchBoxOldText);
				ui->video_swtch_comboBox->setCurrentIndex(nOldIndex);
				break;
			}
			m_bSwitchBoxForbid = m_bSwictchBoxChecked ? true : false;
			m_bSwictchBoxChecked = false;
		}
		m_strSwitchBoxOldText = arg1;
		m_nSwitchBoxOldVid = nCurrentVid;
		if (m_player)
		{
			m_player->setCursor(Qt::WaitCursor);
		}
		
// 		if (PlayState::PS_PLAY == m_playerController->GetState())
// 		{
// 			m_playerController->SwitchState(PlayState::PS_STOP);
// 		}

// 		QString strId{};
// 		if (m_mapVideoList.find(arg1) == m_mapVideoList.end())
// 		{
// 			break;
// 		}
// 		strId = m_strIp + QString("|%1").arg(m_mapVideoList[arg1]);
	
		VideoItem video_item = VideoItemManager::instance().getVideoItem(VideoUtils::packDeviceVideoId(m_strIp, m_nSwitchBoxOldVid));
		if (m_devicePtr)
		{
			video_item.setProperty(VideoItem::PropType::StreamType, m_devicePtr->getProperty(Device::PropStreamType));
			video_item.setProperty(VideoItem::PropType::VideoFormat, m_devicePtr->getProperty(Device::PropVideoFormat));
			video_item.setProperty(VideoItem::PropType::OsdVisible, m_devicePtr->getProperty(Device::PropWatermarkEnable));
		}
		m_videoItem = video_item;
		//参数修正,在界面内切换视频,不需要进行回滚,
		PreExportColorCorrectRollback preExport(m_devicePtr->getProcessor(), video_item, false);
// 		preExport.setRotationType(HscRotationType(video_item.getRotationType()));
// 		HscColorCorrectInfo color_correct_info = video_item.getProperty(VideoItem::PropType::ColorCorrectInfo).value<HscColorCorrectInfo>();
// 		preExport.setColorCorrectInfo(color_correct_info);
// 		preExport.setDisplayMode(HscDisplayMode(video_item.getDisplayMode()));
		if (m_displaySetting)
		{
			m_displaySetting->ResetParams();
		}

		if (m_playerController)
		{
			m_playerController->DisableGetFrame(true);
			delete m_playerController;
			m_playerController = nullptr;
		}

		if (m_targetScoringControl)
		{
			delete m_targetScoringControl;
			m_targetScoringControl = nullptr;
		}
		m_iPlaySpeed = 1;
		ui->reset_pushButton->setEnabled(false);
		m_bEnableLastButton = false;
		ui->lastTime_pushButton->setEnabled(false);
		m_current_idx = ui->video_swtch_comboBox->currentIndex();
		m_playerController = new CSPlaybackController(video_item);
		auto thumbnailLoadFinishCallback = [this](bool ok) {
			thumbnailLoadingFinished(ok);
		};

		m_playerController->setCrtlCallback(thumbnailLoadFinishCallback);

		m_targetScoringControl = new CSCtrl::CSTargerScoring(video_item);
		ReloadVideoInfo(m_videoItem);
		bool ok = connect(m_playerController, &CSPlaybackController::ImageUpdate, this, &CSExportPreview::SlotImageUpdate);
		ok = connect(m_playerController, &CSPlaybackController::thumbnailUpdated, m_pThumbnailManage, &CSThumbnailManage::SlotThumbnailUpdated);
		ok = connect(m_playerController, &CSPlaybackController::signalHighlightThumbnail, m_pThumbnailManage, &CSThumbnailManage::SlotHighlightThumbnail);
		ok = connect(m_playerController, &CSPlaybackController::signalVideoRangeChanged, m_pThumbnailManage, &CSThumbnailManage::SlotVideoRangeChanged);
		ok = connect(m_playerController, &CSPlaybackController::thumbnailLoadingFinished, this, &CSExportPreview::SlotThumbnailLoadingFinished);
		ok = connect(m_playerController, &CSPlaybackController::signalRangeStackChange, this, &CSExportPreview::slotRangeStackChange);
		ok = connect(m_targetScoringControl, &CSCtrl::CSTargerScoring::SignalTargetScoringInfoChanged, this, &CSExportPreview::SlotTargetScoringInfoChanged);
		if (m_playerController)
		{
			m_playerController->StartLoadThumbnails();
			UpadateDisplaySetting(m_strIp);
		}
		if (m_pExportPreviewPlayCtr)
		{
			m_pExportPreviewPlayCtr->SetPlayPauseStatus(true);
		}
		if (m_playerController && m_pExportPreviewPlayCtr)
		{
			int nKeyFrameNo = m_videoItem.getKeyFrameIndex();
			if (-1 != nKeyFrameNo)
			{
				m_pExportPreviewPlayCtr->SetTriggerFrame(nKeyFrameNo);
			}
			int nBeginFrameNo = m_videoItem.getBeginFrameIndex();
			if (-1 != nBeginFrameNo)
			{
				m_pExportPreviewPlayCtr->SetCurFrameIndex(nBeginFrameNo);
			}
			if (!m_bFrame)
			{
				m_pExportPreviewPlayCtr->resetSwitchFrameAndMs();
			}
		}
		ui->spinBox_skip_frame_value->setValue(0);
		if (m_player)
		{
			m_player->setCursor(Qt::ArrowCursor);
		}

	} while (false);
}

bool CSExportPreview::GetEnableExport(const QString &arg1)
{
	if (m_devicePtr.isNull())
	{
		return false;
	}

	HscIntRange width_range{};
	if (!m_devicePtr->getRoiWidthRange(0, width_range))
	{
		return false;
	}

	HscIntRange height_range{};
	if (!m_devicePtr->getRoiHeightRange(0, height_range))
	{
		return false;
	}

 	QString strId{};
// 	if (m_mapVideoList.find(arg1) == m_mapVideoList.end())
// 	{
// 		return false;
// 	}
// 	strId = m_strIp + QString("|%1").arg(m_mapVideoList[arg1]);
	strId = m_strIp + QString("|%1").arg(arg1);

	VideoItem video_item = VideoItemManager::instance().getVideoItem(strId);
	QRect rect = video_item.getRoi();
	if (height_range.min > rect.height() || width_range.min > rect.width())
	{
		return false;
	}
	return true;
}

void CSExportPreview::enableSpeedCtrlBtns(bool benable)
{
	return;
}

void CSExportPreview::on_forward_frame_btn_clicked()
{
	if (m_playerController)
	{
		SwitchDisplayBtnStatus(false);
		m_playerController->PreviousFrame();
	}
}

void CSExportPreview::on_comboBox_display_bpp_currentIndexChanged(int index)
{
	if (m_devicePtr.isNull())
	{
		return ;
	}

	m_devicePtr->setProperty(Device::PropDisplayBitDepth,ui->comboBox_display_bpp->itemData(index));
}

void CSExportPreview::on_backward_frame_btn_clicked()
{
	if (m_playerController)
	{
		SwitchDisplayBtnStatus(false);
		m_playerController->NextFrame();
	}
}

void CSExportPreview::slotSnapBtnPressed()
{
	if (m_player) {
		QString msg{};
		if (m_player->Snapshot())
		{
			msg = UIExplorer::instance().getStringById("STRID_SNAPSHOT_OK");
		}
		else
		{
			msg = UIExplorer::instance().getStringById("STRID_SNAPSHOT_FAIL");
		}

		if (m_devicePtr) {
			msg += m_devicePtr->getDescription();
			msg += "\r\n";
		}
		UIUtils::showInfoMsgBox(this, msg);
	}
	
}

void FrameButton::enterEvent(QEvent * event)
{
	emit SignalEnterFrameBtn();
}

void FrameButton::leaveEvent(QEvent * event)
{
	emit SignalLeaveFrameBtn();
}

bool CSExportPreview::compareList(const VideoItem &v1, const VideoItem &v2)
{
	return v1.getTimeStamp() < v2.getTimeStamp();
}

QString CSExportPreview::getCameraName(QString strIp)
{
	QString camera_name = strIp;
	auto device_ = DeviceManager::instance().getDevice(strIp);
	if (device_)
	{
		QString strDeviceName = device_->getProperty(Device::PropType::PropName).toString();
		if (!strDeviceName.isEmpty())
		{
			camera_name = strDeviceName + QString("[%1]").arg(strIp);
		}
	}
	return camera_name;
}

void CSExportPreview::SetEditRange()
{
	uint64_t iRangeBegin;
	uint64_t iRangeEnd;
	m_pExportPreviewPlayCtr->GetPlayerRange(iRangeBegin, iRangeEnd);
	iRangeBegin += 1;
	iRangeEnd += 1;

	double dRangeBegin = iRangeBegin*1.0;
	double dRangeEnd = iRangeEnd*1.0;
	if (!m_bFrame)
	{
		ui->doubleSpinBox_begin->setDecimals(1);
		ui->doubleSpinBox_end->setDecimals(1);
		ui->doubleSpinBox_begin->setRange(VideoUtils::frameIdToMs(0, m_fps), VideoUtils::frameIdToMs(m_uiFrameTotal, m_fps));
		ui->doubleSpinBox_end->setRange(VideoUtils::frameIdToMs(0, m_fps), VideoUtils::frameIdToMs(m_uiFrameTotal, m_fps));
		dRangeBegin = VideoUtils::frameIdToMs(iRangeBegin, m_fps);
		dRangeEnd = VideoUtils::frameIdToMs(iRangeEnd, m_fps);
	}
	else
	{
		ui->doubleSpinBox_begin->setDecimals(0);
		ui->doubleSpinBox_end->setDecimals(0);
		ui->doubleSpinBox_begin->setRange(1, m_uiFrameTotal);
		ui->doubleSpinBox_end->setRange(1, m_uiFrameTotal);
	}
	ui->doubleSpinBox_begin->setValue(dRangeBegin);
	ui->doubleSpinBox_end->setValue(dRangeEnd);
}

void CSExportPreview::UpdateSkipFrameInfo()
{
	// 仅更新界面跳帧，逻辑层超出范围自己去校正
	double dbBegin = ui->doubleSpinBox_begin->value();
	double dbEnd = ui->doubleSpinBox_end->value();
	int nRangeBegin = (int)dbBegin;
	int nRangeEnd = (int)dbEnd;
	if (!m_bFrame)
	{
		nRangeBegin = VideoUtils::msToFrameId(dbBegin, m_fps);
		nRangeEnd = VideoUtils::msToFrameId(dbEnd, m_fps);
	}
	if (m_iPlaySpeed > nRangeEnd - nRangeBegin)
	{
		m_iPlaySpeed = nRangeEnd - nRangeBegin;
		if (m_iPlaySpeed < 1)
		{
			m_iPlaySpeed = 1;
		}
		ui->spinBox_skip_frame_value->blockSignals(true);
		ui->spinBox_skip_frame_value->setValue(m_iPlaySpeed - 1);
		ui->spinBox_skip_frame_value->blockSignals(false);
	}
}