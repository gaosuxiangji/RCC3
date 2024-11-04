#include "csexportpreviewplayctrl.h"
#include "ui_csexportpreviewplayctrl.h"
CSExportPreviewPlayCtrl::CSExportPreviewPlayCtrl(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSExportPreviewPlayCtrl)
{
    ui->setupUi(this);
	InitUI();
	ConnectSignalAndSlot();

	//默认状态使能
	SetEnabled(true);
}

CSExportPreviewPlayCtrl::~CSExportPreviewPlayCtrl()
{
    delete ui;
}

void CSExportPreviewPlayCtrl::SetTotalFrameCnt(uint64_t total, bool bForceFlush)
{
	if (0 >= total)
	{
		return;
	}
	m_total = total;
	m_currentIndex = 0;
	QString strTotalLabel = m_bFrame ? QString::number(total, 'f', 1) : \
		QString::number(VideoUtils::frameIdToMs(total, m_iFrameRate), 'f', 1);
	ui->total_frame_label->setText(strTotalLabel);
	if (total > 0)
	{
		QString strEdit = m_bFrame ? QString::number(1, 'f', 1) :\
			QString::number(VideoUtils::frameIdToMs(1, m_iFrameRate), 'f', 1);
		ui->current_frame_lineEdit->setText(strEdit);
	}
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetTotalFrameCnt(total, bForceFlush);
	}
}

void CSExportPreviewPlayCtrl::SetFrameRate(const qint64 & fps)
{
	m_iFrameRate = fps;
	m_pExportPreviewPlayerSliderWidget->SetFrameRate(fps);
}

bool CSExportPreviewPlayCtrl::SetPlayerRange(uint64_t begin, uint64_t end)
{
	bool bRet = false;
	if (m_pExportPreviewPlayerSliderWidget)
	{
		bRet = m_pExportPreviewPlayerSliderWidget->SetPlayerRange(begin, end);
		if (bRet)
		{
			if (m_currentIndex<begin || m_currentIndex>end)
			{
				m_currentIndex = begin;
			}
			
			QString strEdit = m_bFrame ? QString::number(m_currentIndex +1, 'f', 1) : \
				QString::number(VideoUtils::frameIdToMs(m_currentIndex +1, m_iFrameRate), 'f', 1);
			ui->current_frame_lineEdit->setText(strEdit);
		}
		return bRet;
	}
	return bRet;
}

void CSExportPreviewPlayCtrl::GetPlayerRange(uint64_t & begin, uint64_t & end)
{
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->GetPlayerRange(begin, end);
	}
}

void CSExportPreviewPlayCtrl::SetTriggerFrame(uint64_t trigger)
{
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetTriggerFrame(trigger);
	}
}

void CSExportPreviewPlayCtrl::SetPlayerPausedFlag(bool pause)
{
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetPlayerBusyFlag(pause);
	}
}

void CSExportPreviewPlayCtrl::SetCurFrameIndex(uint64_t frameIndex)
{
	m_currentIndex = frameIndex;
	uint64_t uiframeIndex =  frameIndex + 1;
	QString strEdit = m_bFrame ? QString::number(uiframeIndex, 'f', 1) : \
		QString::number(VideoUtils::frameIdToMs(uiframeIndex, m_iFrameRate), 'f', 1);
	ui->current_frame_lineEdit->setText(strEdit);
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetCurFrameIndex(frameIndex);
	}
}

void CSExportPreviewPlayCtrl::SetEnabled(bool enable)
{
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetEnabled(enable);
	}
	ui->play_pause_btn->setEnabled(enable);
	ui->current_frame_lineEdit->setEnabled(enable);
	ui->frame_switch_comboBox->setEnabled(enable);
}

void CSExportPreviewPlayCtrl::SetPlayPauseStatus(bool bPlay)
{
	ui->play_pause_btn->setChecked(bPlay);
	bPlay ? ui->play_pause_btn->setToolTip(tr("Pause")):ui->play_pause_btn->setToolTip(tr("Play"));
}

void CSExportPreviewPlayCtrl::SetThumbnailRange(const uint64_t & iStart, const uint64_t & iEnd)
{
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetThumbnailRange(iStart, iEnd);
	}
}

void CSExportPreviewPlayCtrl::resetSwitchFrameAndMs()
{
	ui->frame_switch_comboBox->setCurrentIndex(0);
}

void CSExportPreviewPlayCtrl::InitUI()
{
	QHBoxLayout* hLayout = new QHBoxLayout(ui->slider_widget);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addSpacing(0);
	ui->slider_widget->setLayout(hLayout);
	m_pExportPreviewPlayerSliderWidget = new CSExportPreviewPlayerSliderWidget(ui->slider_widget);
	hLayout->addWidget(m_pExportPreviewPlayerSliderWidget);

	ui->play_pause_btn->setObjectName("PlayPauseControl");
	ui->play_pause_btn->setToolTip(tr("Play"));

	QStringList frameSwitchList;
	frameSwitchList<< tr("Frame") << "ms";
	ui->frame_switch_comboBox->addItems(frameSwitchList);

	QRegExp regExp("(^[0-9]+\.?[0]?)|([0-9]?)$");
	ui->current_frame_lineEdit->setValidator(new QRegExpValidator(regExp, this));
}

void CSExportPreviewPlayCtrl::ConnectSignalAndSlot()
{
	bool ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::sigPlayerBeginRangeChanged,\
		this, &CSExportPreviewPlayCtrl::sigPlayerBeginRangeChanged);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::sigPlayerEndRangeChanged,\
		this, &CSExportPreviewPlayCtrl::sigPlayerEndRangeChanged);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::SignalKeyFrameToBigScreen,\
		this, &CSExportPreviewPlayCtrl::SignalKeyFrameToBigScreen);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::sigSeekFrame, \
		this, &CSExportPreviewPlayCtrl::sigSeekFrame);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::sigSeekFrame, \
		this, &CSExportPreviewPlayCtrl::SlotSeekFrame);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::SignalMouseMoveOnSlider, \
		this, &CSExportPreviewPlayCtrl::SignalMouseMoveOnSlider);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::SignalMouseMoveOutSlider, \
		this, &CSExportPreviewPlayCtrl::SignalMouseMoveOutSlider);
	ok = connect(m_pExportPreviewPlayerSliderWidget, &CSExportPreviewPlayerSliderWidget::SignalSliderMouseClicked, \
		this, &CSExportPreviewPlayCtrl::SignalSliderMouseClicked);
	Q_UNUSED(ok);
}

void CSExportPreviewPlayCtrl::SlotSeekFrame(uint64_t curFrameIndex)
{
	curFrameIndex += 1;
	QString strEdit = m_bFrame ? QString::number(curFrameIndex, 'f', 1) : \
		QString::number(VideoUtils::frameIdToMs(curFrameIndex, m_iFrameRate), 'f', 1);
	ui->current_frame_lineEdit->setText(strEdit);
}

void CSExportPreviewPlayCtrl::on_play_pause_btn_clicked(bool checked)
{
	QString strTip = checked ? tr("Pause") : tr("Play");
	ui->play_pause_btn->setToolTip(strTip);
	emit SignalDisplayControl(checked);
}

void CSExportPreviewPlayCtrl::on_current_frame_lineEdit_editingFinished()
{
	uint64_t iRangeBegin;
	uint64_t iRangeEnd;
	GetPlayerRange(iRangeBegin, iRangeEnd);
	iRangeBegin += 1;
	iRangeEnd += 1;

	double dRangeBegin = iRangeBegin*1.0;
	double dRangeEnd = iRangeEnd*1.0;

	if (!m_bFrame)
	{
		dRangeBegin = VideoUtils::frameIdToMs(iRangeBegin, m_iFrameRate);
		dRangeEnd = VideoUtils::frameIdToMs(iRangeEnd, m_iFrameRate);
	}
	
	if (dRangeBegin > ui->current_frame_lineEdit->text().toDouble())
	{
		ui->current_frame_lineEdit->setText(QString::number(dRangeBegin, 'f', 1));
	}
	else if (dRangeEnd < ui->current_frame_lineEdit->text().toDouble())
	{
		ui->current_frame_lineEdit->setText(QString::number(dRangeEnd, 'f', 1));
	}
	double dbValue = ui->current_frame_lineEdit->text().toDouble();
	uint64_t editValue = 0;
	if (m_bFrame)
	{
		editValue = dbValue;
		m_currentIndex = editValue - 1;
	}
	else
	{
		editValue = VideoUtils::msToFrameIdOfRound(dbValue, m_iFrameRate);
		if (editValue > iRangeEnd)
		{
			editValue = iRangeEnd;
		}
		else if (editValue < iRangeBegin)
		{
			editValue = iRangeBegin;
		}
	}
	m_currentIndex = editValue - 1;
	if (m_pExportPreviewPlayerSliderWidget)
	{
		m_pExportPreviewPlayerSliderWidget->SetCurFrameIndex(m_currentIndex);
	}
	emit sigSeekFrame(m_currentIndex);
}

void CSExportPreviewPlayCtrl::on_frame_switch_comboBox_currentIndexChanged(int index)
{
	QString strTotalLabel = (index == 0) ? QString::number(m_total, 'f', 1) : \
		QString::number(VideoUtils::frameIdToMs(m_total, m_iFrameRate), 'f', 1);
	ui->total_frame_label->setText(strTotalLabel);

	QString lineEditText = ui->current_frame_lineEdit->text();
	double editValue_ = lineEditText.toDouble();

	if (0 == index)
	{
		QRegExp regExp("(^[0-9]+\.?[0]?)|([0-9]?)$");
		ui->current_frame_lineEdit->setValidator(new QRegExpValidator(regExp, this));
		m_bFrame = true;
		m_pExportPreviewPlayerSliderWidget->SwitchFrameAndMs(true);
		uint64_t editValue = VideoUtils::msToFrameIdOfRound(editValue_, m_iFrameRate);
		if (editValue >= m_total)
		{
			editValue = m_total - 1;
		}
		ui->current_frame_lineEdit->setText(QString::number(editValue, 'f', 1));
	}
	else if(1 == index)
	{
		QRegExp regExp("(^[0-9]+\.?[0-9]?)|([0-9]?)$");
		ui->current_frame_lineEdit->setValidator(new QRegExpValidator(regExp, this));
		m_bFrame = false;
		m_pExportPreviewPlayerSliderWidget->SwitchFrameAndMs(false);
		ui->current_frame_lineEdit->setText(QString::number(VideoUtils::frameIdToMs(editValue_, m_iFrameRate), 'f', 1));
	}
	emit  SignalSwitchFrameAndMs(m_bFrame);
}
