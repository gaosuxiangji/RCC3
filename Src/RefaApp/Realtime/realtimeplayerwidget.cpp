#include "realtimeplayerwidget.h"
#include "ui_realtimeplayerwidget.h"

#include <PlayerStructer.h>
#include <RMAImage.h>
#include <MediaInfo.h>
#include <PlayerControllerInterface.h>
#include <QDateTime>
#include <QFileInfo>
#include <QImage>
#include <QStyle>
#include <QTimer>

#include "LogRunner.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "System/SystemSettings/systemsettingsmanager.h"

using namespace FHJD_LOG;

RealtimePlayerWidget::RealtimePlayerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealtimePlayerWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

    initUi();
    initBinding();
}

RealtimePlayerWidget::~RealtimePlayerWidget()
{
    delete ui;
}


void RealtimePlayerWidget::setPlayerWorkStatus(PLAYER_WORK_MODE mode)
{
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (p)
	{
		p->SetPlayerWorkStatus(mode);
	}
}

PLAYER_WORK_MODE RealtimePlayerWidget::getPlayerWorkStatus() const
{
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (p)
	{
		return p->GetPlayerWorkStatus();
	}
	return WORK_MODE_IDLE;
}

void RealtimePlayerWidget::initUi()
{
    ui->widgetPlayer0->SetPlayCtrlAndToolWidgetVisible(false);
    ui->widgetPlayer0->SetVideoNameLabelVisible(false);
	ui->widgetPlayer0->SetVideoInProj(true);

    ui->widgetPlayer1->SetPlayCtrlAndToolWidgetVisible(false);
    ui->widgetPlayer1->SetVideoNameLabelVisible(false);
	ui->widgetPlayer1->SetVideoInProj(true);

    ui->widgetTitleBar->setVisible(false);
	ui->widgetTitleBar->setTitle(UIExplorer::instance().getProductName());
	ui->widgetTitleBar->setIcon(QIcon(":/images/logo.ico"));

	slotSetFocusWidget(ui->widgetPlayer0);

	ui->widgetToolBar->setVisible(false);
	toolbar_visible_timer_ = new QTimer(this);
	toolbar_visible_timer_->setInterval(kToolbarVisibleTimer_);
	connect(toolbar_visible_timer_, &QTimer::timeout, this, [this] {ui->widgetToolBar->setVisible(false); });

	installEventFilter(this);
}

void RealtimePlayerWidget::initBinding()
{
	//标题栏信号槽绑定
	connect(ui->widgetTitleBar, &PlayerTitleBar::fullScreen, this, &RealtimePlayerWidget::fullScreenTriggered);

    //工具栏信号槽绑定
    connect(ui->widgetToolBar, &VideoToolBar::zoomInTriggered, this, &RealtimePlayerWidget::zoomIn);
    connect(ui->widgetToolBar, &VideoToolBar::zoomOutTriggered, this, &RealtimePlayerWidget::zoomOut);
    connect(ui->widgetToolBar, &VideoToolBar::zoomToOriginalTriggered, this, &RealtimePlayerWidget::zoomToOriginal);
    connect(ui->widgetToolBar, &VideoToolBar::zoomToFitTriggered, this, &RealtimePlayerWidget::zoomToFit);
    connect(ui->widgetToolBar, &VideoToolBar::focusLineTriggered, this, &RealtimePlayerWidget::setFocusPointVisible);
    connect(ui->widgetToolBar, &VideoToolBar::fullScreenTriggered, this, &RealtimePlayerWidget::fullScreenTriggered);
    connect(ui->widgetToolBar, &VideoToolBar::roiSelectionTriggered, this, &RealtimePlayerWidget::setRoiSelectionStart);
    connect(ui->widgetToolBar, &VideoToolBar::snapshotTriggered, this, &RealtimePlayerWidget::snapshot);
	connect(ui->widgetToolBar, &VideoToolBar::buttonClicked, this, &RealtimePlayerWidget::startToolbarVisibleTimer);

    //播放器信号槽绑定
    connect(ui->widgetPlayer0, &CWidgetVideoPlaySingle::sigFocusWidget, this, &RealtimePlayerWidget::slotSetFocusWidget);
    connect(ui->widgetPlayer1, &CWidgetVideoPlaySingle::sigFocusWidget, this, &RealtimePlayerWidget::slotSetFocusWidget);
    connect(ui->widgetPlayer0, &CWidgetVideoPlaySingle::sigRoiChanged, this, &RealtimePlayerWidget::slotRoiChanged);
    connect(ui->widgetPlayer1, &CWidgetVideoPlaySingle::sigRoiChanged, this, &RealtimePlayerWidget::slotRoiChanged);
    connect(ui->widgetPlayer0, &CWidgetVideoPlaySingle::sigCustomCrossLineCenterChanged, this, &RealtimePlayerWidget::slotFocusPointChanged);
    connect(ui->widgetPlayer1, &CWidgetVideoPlaySingle::sigCustomCrossLineCenterChanged, this, &RealtimePlayerWidget::slotFocusPointChanged);
	connect(ui->widgetPlayer0, &CWidgetVideoPlaySingle::sigUpdateFrame, this, &RealtimePlayerWidget::slotUpdateFrame);
	connect(ui->widgetPlayer1, &CWidgetVideoPlaySingle::sigUpdateFrame, this, &RealtimePlayerWidget::slotUpdateFrame);
}

CWidgetVideoPlaySingle *RealtimePlayerWidget::getFocusPlayer() const
{
    switch (current_focus_player_no_)
    {
    case FOCUS_PLAYER_ZERO:
        return ui->widgetPlayer0;
    case FOCUS_PLAYER_ONE:
        return ui->widgetPlayer1;
    default:
        break;
    }
    return nullptr;
}

void RealtimePlayerWidget::setFocusPoint(const QPoint &pt)
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
        p->SetCustomCrossLineCenter(pt);
}

void RealtimePlayerWidget::setRoi(const QRect &roi)
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (!p)
		return;
    p->SetRoi(roi);
	p->SetRoiVisible(true);
}

QString RealtimePlayerWidget::getCurrentPlayerIp() const
{
    auto current_player_ptr = getFocusPlayer();
    if (!current_player_ptr)
    {
        return QString();
    }

    auto current_player_ctrl_ptr = current_player_ptr->GetActiveVideoPlayerCtrl();
    if (!current_player_ctrl_ptr)
    {
        return QString();
    }

    auto video_info_ptr = current_player_ctrl_ptr->GetVideoInfo();
    if (!video_info_ptr)
    {
        return QString();
    }

    return video_info_ptr->GetCameraIP();
}

void RealtimePlayerWidget::addVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr, const QVariant video_mark)
{
    if (!ui->widgetPlayer0->GetActiveVideoPlayerCtrl())
    {
		addVideo(ui->widgetPlayer0, player_controller_ptr);
		player_id_map_[FOCUS_PLAYER_ZERO] = { video_mark, true };
    }
    else if (!ui->widgetPlayer1->GetActiveVideoPlayerCtrl())
    {
		addVideo(ui->widgetPlayer1, player_controller_ptr);
		player_id_map_[FOCUS_PLAYER_ONE] = { video_mark, true };
    }
	updatePlayersVisible();
}

void RealtimePlayerWidget::addVideo(CWidgetVideoPlaySingle *pplayer, std::shared_ptr<PlayerControllerInterface> player_ctrl)
{
	if (!pplayer || !player_ctrl)
		return;

	pplayer->AddVideo({ player_ctrl });
	pplayer->SetVideoInProj(true);
	pplayer->SetTranslationEnable(true);
	pplayer->slotSetCalibrationMode(CALI_MODE_UNKNOWN);
	pplayer->SetOsdVisible(true);
	slotSetFocusWidget(pplayer);
}

void RealtimePlayerWidget::removeVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr)
{
    if (ui->widgetPlayer0->GetActiveVideoPlayerCtrl() == player_controller_ptr)
    {
        ui->widgetPlayer0->CloseVideo();
		player_id_map_[FOCUS_PLAYER_ZERO] = {};
    }
    else if (ui->widgetPlayer1->GetActiveVideoPlayerCtrl() == player_controller_ptr)
    {
        ui->widgetPlayer1->CloseVideo();
		player_id_map_[FOCUS_PLAYER_ONE] = {};
    }

	updatePlayersVisible();

	updateToolBar();
}

void RealtimePlayerWidget::zoomIn()
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
        p->on_ZoomInBtn_clicked();
}

void RealtimePlayerWidget::zoomOut()
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
        p->on_ZoomOutBtn_clicked();
}

void RealtimePlayerWidget::zoomToOriginal()
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
        p->on_OriginalSizeBtn_clicked();
}

void RealtimePlayerWidget::zoomToFit()
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
        p->on_FitViewBtn_clicked();
}

void RealtimePlayerWidget::setFocusPointVisible(bool bvisible)
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (p)
	{
		p->SetCustomCrossLineVisible(bvisible);
		emit focusPointVisible(getPlayerIP(p), bvisible);
	}
	ui->widgetToolBar->setFocusLineChecked(bvisible);
}

void RealtimePlayerWidget::setFullScreen(bool benabled)
{
    ui->widgetToolBar->setVisible(!benabled);
    if(benabled)
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

void RealtimePlayerWidget::setRoiSelectionStart()
{
    CWidgetVideoPlaySingle *p = getFocusPlayer();
    if(p)
    {
        p->SetRoiVisible(/*benabled*/true);
        p->SetPlayerWorkStatus(WORK_MODE_SELECTION_ROI);
		QString ip = getPlayerIP(p);
		if (!ip.isEmpty())
		{
			emit roiChanged(ip, p->GetRoi());
		}
    }
}

void RealtimePlayerWidget::setRoiSelectionEnabled(bool benabled, const QVariant & player_id)
{
	for (auto &e : player_id_map_)
	{
		if (player_id == e.first)
		{
			e.second = benabled;
			ui->widgetToolBar->setRoiSelectionEnabled(benabled);
		}
	}
}

void RealtimePlayerWidget::snapshot()
{
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (!p)
		return;
	auto img = getCurImage();
    if(!img.Empty())
    {
		emit saveSnapshot(getPlayerIP(p), img);
    }
}

void RealtimePlayerWidget::slotSetFocusWidget(CWidgetVideoPlaySingle *player)
{
    if(!player)
        return;
	const int focus_player_no_last = current_focus_player_no_;
    CWidgetVideoPlaySingle *pother = nullptr;
    if(player == ui->widgetPlayer0)
    {
        current_focus_player_no_ = FOCUS_PLAYER_ZERO;
        pother = ui->widgetPlayer1;
    }
    else if(player == ui->widgetPlayer1)
    {
        current_focus_player_no_ = FOCUS_PLAYER_ONE;
        pother = ui->widgetPlayer0;
    }
	updateToolBar();

	//选中播放器未发生变化时不修改
	if (focus_player_no_last == current_focus_player_no_)
		return;
    player->setStyleSheet("QFrame#frame{border : 2px solid red}");
    if(pother)
    {
        pother->setStyleSheet("QFrame#frame{border : none}");
    }
	emit focusPlayerChanged(getPlayerIP(player));
}

void RealtimePlayerWidget::slotRoiChanged(const QRect &roi)
{
    CWidgetVideoPlaySingle *p = dynamic_cast<CWidgetVideoPlaySingle*>(sender());
    if(!p)
        return;

    QString ip = getPlayerIP(p);
    if(!ip.isEmpty())
    {
        emit roiChanged(ip, roi);
    }
}

void RealtimePlayerWidget::slotFocusPointChanged(const QPoint &pt)
{
    CWidgetVideoPlaySingle *p = dynamic_cast<CWidgetVideoPlaySingle*>(sender());
    if(!p)
        return;
    QString ip = getPlayerIP(p);
    if(!ip.isEmpty())
    {
        emit focusPointChanged(ip, pt);
    }
}

void RealtimePlayerWidget::slotUpdateFrame(int64_t idx)
{
	const CWidgetVideoPlaySingle *p = dynamic_cast<CWidgetVideoPlaySingle*>(sender());
	if (!p)
		return;
	if (p == getFocusPlayer())
	{
		updateToolBar();
	}
}

void RealtimePlayerWidget::startToolbarVisibleTimer()
{
	if (ui->widgetToolBar->isVisible())
	{
		toolbar_visible_timer_->start();
	}
}

void RealtimePlayerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_ASSERT(event);

	//空闲状态下双击全屏
	auto player = getFocusPlayer();
	if (player && player->GetPlayerWorkStatus() == PLAYER_WORK_MODE::WORK_MODE_IDLE)
	{
		emit fullScreenTriggered(!ui->widgetTitleBar->isVisible());
	}

	QWidget::mouseDoubleClickEvent(event);
}

bool RealtimePlayerWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this)
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			auto mouse_event = dynamic_cast<QMouseEvent*>(event);
			const auto mode = getPlayerWorkStatus();
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

QString RealtimePlayerWidget::getPlayerIP(CWidgetVideoPlaySingle *p) const
 {
     if (p && p->GetActiveVideoPlayerCtrl() && p->GetActiveVideoPlayerCtrl()->GetVideoInfo())
     {
         return p->GetActiveVideoPlayerCtrl()->GetVideoInfo()->GetCameraIP();
     }
     return QString();
 }

QString RealtimePlayerWidget::getPlayerIP(int player_no) const
{
	if (FOCUS_PLAYER_ZERO == player_no)
	{
		return getPlayerIP(ui->widgetPlayer0);
	}
	else if(FOCUS_PLAYER_ONE == player_no)
	{
		return getPlayerIP(ui->widgetPlayer1);
	}
	return QString();
}

void RealtimePlayerWidget::updateToolBar()
{
	bool benabled = false;
	auto player = getFocusPlayer();
	if (player)
	{
		if (!getCurImage().Empty())
		{
			benabled = true;
		}
	}

	if (!benabled)
	{		
		ui->widgetToolBar->setFocusLineChecked(false);
		ui->widgetToolBar->setEnabled(false);
	}
	else
	{
		// 工具栏联动
		ui->widgetToolBar->setFocusLineChecked(player->isCustomCrossLineVisible());
		ui->widgetToolBar->setEnabled(benabled);
		ui->widgetToolBar->setRoiSelectionEnabled(isEnabledSelctionRoi());
    }
}

void RealtimePlayerWidget::updatePlayersVisible()
{
	//没有两个视频的话则只显示一个播放器,没有视频的话只显示默认播放器
	bool player0_visible = (bool)ui->widgetPlayer0->GetActiveVideoPlayerCtrl();
	bool player1_visible = (bool)ui->widgetPlayer1->GetActiveVideoPlayerCtrl();

	ui->widgetPlayer0->setVisible(player0_visible);
	ui->widgetPlayer1->setVisible(player1_visible);
	if (!player0_visible && !player1_visible)
	{
		ui->widgetPlayer0->setVisible(true);
	}
}

RMAImage RealtimePlayerWidget::getCurImage()
{
	RMAImage img;
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (!p)
		return img;
	std::shared_ptr<PlayerControllerInterface> pctrl = p->GetActiveVideoPlayerCtrl();
	if (!pctrl)
		return img;
	VIDEO_ID id = pctrl->GetVideoInfo()->GetVideoId();
	FRAME_INDEX frame_no = -1;
	p->GetCurImage(id, img, frame_no);
	return qMove(img);
}

bool RealtimePlayerWidget::isEnabledSelctionRoi() const
{
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (p)
	{
		const QVariant id = getPlayerIP(p);
		for (auto e : player_id_map_)
		{
			if (id == e.first)
			{
				return e.second;
				break;
			}
		}
	}
	return false;
}

void RealtimePlayerWidget::on_toolButtonRestoreFullScreen_clicked()
{
	emit fullScreenTriggered(false);
}

void RealtimePlayerWidget::setOSDVisible(bool bvisible)
{
	CWidgetVideoPlaySingle *p = getFocusPlayer();
	if (!p)
		return;
	p->SetOsdVisible(bvisible);
}

