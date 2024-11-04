#include "playctrlwidget.h"
#include "ui_playctrlwidget.h"
#include <QDebug>

//#define ENABLE_BACKWARD_PLAY   //使能后向播放功能，包括正常速度播放和倍速播放

PlayCtrlWidget::PlayCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayCtrlWidget)
{
    ui->setupUi(this);

	SetEnabled(false);
	
	InitBinding();
}

PlayCtrlWidget::~PlayCtrlWidget()
{
    delete ui;
}

void PlayCtrlWidget::InitBinding()
{
	connect(ui->widgetSelectionSlider, &PlayerSliderWidget::sigPlayerBeginRangeChanged, this, &PlayCtrlWidget::slotPlayerBeginRangeChanged);
	connect(ui->widgetSelectionSlider, &PlayerSliderWidget::sigPlayerEndRangeChanged, this, &PlayCtrlWidget::slotPlayerEndRangeChanged);
	connect(ui->widgetSelectionSlider, &PlayerSliderWidget::sigSeekFrame, this, &PlayCtrlWidget::slotSeekFrame);

	connect(ui->widgetSelectionSlider, &PlayerSliderWidget::sigReachMinValidFrameIndex, this, &PlayCtrlWidget::slotReachMinValidFrameIndex);
	connect(ui->widgetSelectionSlider, &PlayerSliderWidget::sigReachMaxValidFrameIndex, this, &PlayCtrlWidget::slotReachMaxValidFrameIndex);
}

void PlayCtrlWidget::GetPlayerRange(uint64_t& begin, uint64_t& end)
{
	ui->widgetSelectionSlider->GetPlayerRange(begin, end);
}

void PlayCtrlWidget::SetTriggerFrame(uint64_t trigger)
{
	ui->widgetSelectionSlider->SetTriggerFrame(trigger);
}

void PlayCtrlWidget::ClearTriggerFrame()
{
	ui->widgetSelectionSlider->ClearTriggerFrame();
}

void PlayCtrlWidget::setPause()
{
	if(!isPlayerPaused)
		on_toolButtonPause_clicked();
}

void PlayCtrlWidget::on_toolButtonBackwardPlay_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	isPlayerPaused = false;
	ui->widgetSelectionSlider->SetPlayerPausedFlag(isPlayerPaused);

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(false);
#endif
	ui->toolButtonPause->setEnabled(true);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

    emit sigBackwardPlay();
}

void PlayCtrlWidget::on_toolButtonPause_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	isPlayerPaused = true;
	ui->widgetSelectionSlider->SetPlayerPausedFlag(isPlayerPaused);

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(false);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

	emit sigPause();
}

void PlayCtrlWidget::on_toolButtonForwardPlay_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	isPlayerPaused = false;
	ui->widgetSelectionSlider->SetPlayerPausedFlag(isPlayerPaused);

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(true);
	ui->toolButtonForwardPlay->setEnabled(false);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

	emit sigForwardPlay();
}

void PlayCtrlWidget::on_toolButtonFastBackwardPlay_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	isPlayerPaused = false;
	ui->widgetSelectionSlider->SetPlayerPausedFlag(isPlayerPaused);

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(true);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(false);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

	emit sigFastBackwardPlay();
}

void PlayCtrlWidget::on_toolButtonBackward_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(false);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

	emit sigBackward();
}

void PlayCtrlWidget::on_toolButtonForward_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(false);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(true);

	emit sigForward();
}

void PlayCtrlWidget::on_toolButtonFastForwardPlay_clicked(bool checked)
{
	Q_UNUSED(checked);

	if (isBusy)
	{
		return;
	}

	isPlayerPaused = false;
	ui->widgetSelectionSlider->SetPlayerPausedFlag(isPlayerPaused);

	//设置按钮使能状态
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonPause->setEnabled(true);
	ui->toolButtonForwardPlay->setEnabled(true);
#ifdef ENABLE_BACKWARD_PLAY
	ui->toolButtonFastBackwardPlay->setEnabled(true);
#endif
	ui->toolButtonBackward->setEnabled(true);
	ui->toolButtonForward->setEnabled(true);
	ui->toolButtonFastForwardPlay->setEnabled(false);

	emit sigFastForwardPlay();
}

void PlayCtrlWidget::slotPlayerBeginRangeChanged(uint64_t begin)
{
	if (isPlayerPaused)
	{
		qDebug() << "Now video is paused!";
	}
	else
	{
		qDebug() << "Now video is not paused!";
	}

	if (!isPlayerPaused)
	{
		on_toolButtonPause_clicked();

		qDebug() << "Video paused!";
	}

	emit sigPlayerBeginRangeChanged(begin);
}

void PlayCtrlWidget::slotPlayerEndRangeChanged(uint64_t end)
{
	if (isPlayerPaused)
	{
		qDebug() << "Now video is paused!";
	}
	else
	{
		qDebug() << "Now video is not paused!";
	}

	if (!isPlayerPaused)
	{
		on_toolButtonPause_clicked();

		qDebug() << "Video paused!";
	}

	emit sigPlayerEndRangeChanged(end);
}

void PlayCtrlWidget::slotSeekFrame(uint64_t frameIndex)
{
	if (!isPlayerPaused)
	{
		on_toolButtonPause_clicked();
	}

	emit sigSeekFrame(frameIndex);
}

void PlayCtrlWidget::slotReachMinValidFrameIndex(bool reach)
{
	ui->toolButtonBackward->setDisabled(reach);
}

void PlayCtrlWidget::slotReachMaxValidFrameIndex(bool reach)
{
	ui->toolButtonForward->setDisabled(reach);
}

void PlayCtrlWidget::SetEnabled(bool enable)
{
	ui->widgetSelectionSlider->SetEnabled(enable);
	this->setEnabled(enable);
}

void PlayCtrlWidget::SetPlayerBusyFlag(bool busy)
{
	ui->widgetSelectionSlider->SetPlayerBusyFlag(busy);
	isBusy = busy;
}

bool PlayCtrlWidget::SetPlayerRange(uint64_t begin, uint64_t end)
{
	return ui->widgetSelectionSlider->SetPlayerRange(begin, end);
}

void PlayCtrlWidget::SetTotalFrameCnt(uint64_t total)
{
	ui->widgetSelectionSlider->SetTotalFrameCnt(total);
}

void PlayCtrlWidget::SetCurFrameIndex(uint64_t frameIndex)
{
	ui->widgetSelectionSlider->SetCurFrameIndex(frameIndex);
}