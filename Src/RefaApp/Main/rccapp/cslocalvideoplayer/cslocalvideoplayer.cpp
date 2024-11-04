#include "cslocalvideoplayer.h"
#include <QHBoxLayout>
#include "../VideoFile/IVideoFileReader.h"
#include "../VideoFile/VideoFileUtils.h"
#include <QDateTime>
#include <QStyle>
#include <QFileInfo>
#include <QIntValidator>
CSLocalVideoPlayer::CSLocalVideoPlayer( QWidget *parent)
	: QDialog(parent)
	, m_bColorRevert(false)
	, m_Timer(new QTimer(this))
	, m_iCurrentFrame(0)
	, m_bPause(false)
	, m_iPlaySpeed(1)
	, m_iContrast(0)
	, m_iLuminance(0)
{
	ui.setupUi(this);
	InitUI();
	ConnectSignalSlot();
	InitPlayerLabelsMap();
	//InitVideoPlayer();
}

CSLocalVideoPlayer::~CSLocalVideoPlayer()
{
	if (nullptr != m_pVideoFileReader)
	{
		m_pVideoFileReader->Close();
	}

	if (m_Timer->isActive())
	{
		m_Timer->stop();
	}
}

bool CSLocalVideoPlayer::LoadFile(const QString& filepath)
{
	bool bRet = true;

	do 
	{
		m_videoFormat = VIDEO_RHVD;
		std::wstring wstrFilePath = filepath.toStdWString();
		if (!VideoFileUtils::GetVideoFormat(wstrFilePath, m_videoFormat))
		{
			bRet = false;
			break;
		}

		m_pVideoFileReader.reset(VideoFileUtils::GetVideoFileReader(m_videoFormat));
		if (nullptr == m_pVideoFileReader)
		{
			bRet = false;
			break;
		}

		if (!m_pVideoFileReader->Open(wstrFilePath))
		{
			bRet = false;
			break;
		}

		if (!m_pVideoFileReader->GetVideoSegmentInfo(m_infoVideo))
		{
			m_pVideoFileReader->Close();
			bRet = false;
			break;
		}

		{
			QFileInfo fileinfo(filepath);
			m_infoVideo.time_stamp = fileinfo.lastModified().toTime_t();
		}
	
		if (VIDEO_AVI == m_videoFormat || VIDEO_MP4 == m_videoFormat)
		{
			ui.VideoTimeLabel->setText(tr("Export Time:"));
			ui.TimeLenLabel->setText(tr("Display Time:"));
			ui.FrameRateLabel->setText(tr("Display Frame Rate:"));
		}
	} while (false);

	if (bRet)
	{
		InitMainWindowInfo();
	}

	return bRet;
}

void CSLocalVideoPlayer::InitUI()
{
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
	this->setMinimumSize(1180,860);//设置最小尺寸，大小由采集软件窗口尺寸得到

	//播放状态单选控制
	m_DisplayStatusGroup = new QButtonGroup(this);
	m_DisplayStatusGroup->addButton(ui.PlayBtn, 0);
	m_DisplayStatusGroup->addButton(ui.PauseBtn, 1);

	//倍速单选控制
	m_SpeedGroup = new QButtonGroup(this);
	m_SpeedGroup->addButton(ui.Speed_1X_Btn, 0);
	m_SpeedGroup->addButton(ui.Speed_2X_Btn, 1);
	m_SpeedGroup->addButton(ui.Speed_4X_Btn, 2);
	m_SpeedGroup->addButton(ui.Speed_8X_Btn, 3);
	m_SpeedGroup->addButton(ui.Speed_16X_Btn, 4);

	ui.PlayPauseBtn->setObjectName("commonBtn");
	ui.PlayBtn->setObjectName("commonBtn");
	ui.PauseBtn->setObjectName("commonBtn");

	ui.PlayPauseBtn->setIcon(QPixmap(":/image/image/pause.png"));
	ui.PlayPauseBtn->setToolTip(tr("Pause"));

	// Add by Juwc -- 2022/7/6 添加"播放"和"暂停"按钮Tooltip
	ui.PlayBtn->setToolTip(tr("Play"));
	ui.PauseBtn->setToolTip(tr("Pause"));
	ui.PlayBtn->setHidden(true);
	ui.PauseBtn->setHidden(true);

	ui.Speed_1X_Btn->setObjectName("commonBtn");
	ui.Speed_2X_Btn->setObjectName("commonBtn");
	ui.Speed_4X_Btn->setObjectName("commonBtn");
	ui.Speed_8X_Btn->setObjectName("commonBtn");
	ui.Speed_16X_Btn->setObjectName("commonBtn");
}

void CSLocalVideoPlayer::ConnectSignalSlot()
{
	bool ok = connect(m_DisplayStatusGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), \
		[=](int id) {SlotDisplayBtnClicked(id); });

	ok = connect(m_SpeedGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), \
		[=](int id) {SlotSpeedBtnClicked(id); });

	ok = connect(ui.PlayPauseBtn, &QPushButton::clicked, this, &CSLocalVideoPlayer::OnPlayPauseBtnClicked);

	ok = connect(m_Timer, &QTimer::timeout, this, &CSLocalVideoPlayer::SlotTimeout);

	Q_UNUSED(ok);
}

void CSLocalVideoPlayer::SetVideoPlayerLabelInfo(VideoPlayerLabelType type, const QString& info)
{
	
	if (m_mapPlayerLabels.end() != m_mapPlayerLabels.find(type))
	{
		if (VideoLenValue == type)
		{
			m_mapPlayerLabels[type]->setText("/" + info);
		}
		else
		{
			m_mapPlayerLabels[type]->setText(info);
		}
	}
}

void CSLocalVideoPlayer::InitPlayerLabelsMap()
{
	m_mapPlayerLabels.insert(VideoTimeValue, ui.VideoTimeValueLabel);
	m_mapPlayerLabels.insert(TimeLenValue, ui.TimeLenValueLabel);
	m_mapPlayerLabels.insert(FrameRateValue, ui.FrameRateValueLabel);
	m_mapPlayerLabels.insert(TotalFramesValue, ui.TotalFramesValueLabel);
	m_mapPlayerLabels.insert(VideoLenValue, ui.VideoLenLable);
}

void CSLocalVideoPlayer::InitVideoPlayer()
{
	m_VideoPlayer = new CPlayerViewBase(0, 1, ui.VideoWidget);
	QHBoxLayout* hLayout = new QHBoxLayout(ui.VideoWidget);
	// Add By Juwc -- 2022/7/14
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(m_VideoPlayer);
}

void CSLocalVideoPlayer::InitMainWindowInfo()
{
	InitVideoPlayer();

	//拍摄时间
	time_t recTime = m_infoVideo.time_stamp;
	QDateTime time = QDateTime::fromTime_t(recTime);
	// Modify by Juwc -- 2022/7/6
	//QString strTime = QString::number(time.date().year()) + tr("year") + QString::number(time.date().month()) +\
		tr("month") + QString::number(time.date().day()) + tr("day") + time.toString("hh:mm:ss");
	QString strTime = QString::number(time.date().year()) + "/" + QString::number(time.date().month()) + \
		"/" + QString::number(time.date().day()) + " " + time.toString("hh:mm:ss");
	SetVideoPlayerLabelInfo(VideoTimeValue, strTime);

	//时长
	QString strTimeLen = QString::number(m_infoVideo.frame_num * 1000.0 / m_infoVideo.fps,'f',2) + "ms";
	SetVideoPlayerLabelInfo(TimeLenValue, strTimeLen);

	//帧率
	SetVideoPlayerLabelInfo(FrameRateValue, QString::number(m_infoVideo.fps));

	//总帧数
	SetVideoPlayerLabelInfo(TotalFramesValue, QString::number(m_infoVideo.frame_num));
	UpdateFrameNum(0);

	//时长，显示在滑块编辑框后
	strTimeLen = QString::number(m_infoVideo.frame_num) + tr("frames");
	SetVideoPlayerLabelInfo(VideoLenValue, strTimeLen);

	int min = 0;
	int max = 100;


	// Add by Juwc 2022/7/14 隐藏视频播放控制栏
	ui.VideoPlayCtrlBox->setHidden(true);

	//对比度滑块
	ui.ContrastSlide->setRange(min, max);
	if (min <= m_infoVideo.contrast && m_infoVideo.contrast <= max)
	{
		m_iContrast = m_infoVideo.contrast;
		ui.ContrastSlide->setValue(m_infoVideo.contrast);
	}
	else
	{
		m_iContrast = (min + max) / 2;
		ui.ContrastSlide->setValue((min + max) / 2);
	}

	//亮度滑块
	ui.LumSlider->setRange(min, max);
	if (min <= m_infoVideo.luminance && m_infoVideo.luminance <= max)
	{
		m_iLuminance = m_infoVideo.luminance;
		ui.LumSlider->setValue(m_infoVideo.luminance);
	}
	else
	{
		m_iLuminance = (min + max) / 2;
		ui.LumSlider->setValue((min + max) / 2);
	}

	//帧滑块
	ui.ProcessSlider->setRange(0, m_infoVideo.frame_num - 1);

	//帧滑块对应的编辑框
	QIntValidator* intValidator = new QIntValidator(this);
	//intValidator->setRange(0, m_infoVideo.frame_num * 1000.0 / m_infoVideo.fps);
	intValidator->setRange(1, m_infoVideo.frame_num);
	ui.CurrentLineEdit->setValidator(intValidator);
	ui.CurrentLineEdit->setText(QString::number(1, 'f', 3));
	if (m_infoVideo.frame_num == 1) UpdateFrameInfo(0);
	m_Timer->start(REFRESH_CYCLE);
}

void CSLocalVideoPlayer::UpdateFrameInfo(int iFrame)
{
	do 
	{
		if (iFrame < 0 || iFrame >= m_infoVideo.frame_num)
		{
			break;
		}

		if ((nullptr == m_pVideoFileReader) || (nullptr == m_VideoPlayer))
		{
			break;
		}

		cv::Mat m_matFrame;
		QImage img;
		if (m_pVideoFileReader->GetFrame(iFrame, m_matFrame, m_iContrast, m_iLuminance, m_bColorRevert))
		{
			UpdateFrameNum(iFrame);
			m_VideoPlayer->cvMat2QImage(m_matFrame, img);
			RccFrameInfo frameInfo;
			frameInfo.image = img;
			m_VideoPlayer->SlotUpdateImage(frameInfo);
		}
		else
		{
			m_matFrame.release();
		}
	} while (false);
}

void CSLocalVideoPlayer::UpdateFrameNum(int frameNum)
{
	frameNum += 1;
	QString info = QString("%1/%2").arg(frameNum).arg(m_infoVideo.frame_num);
	m_VideoPlayer->SetViewShowLabelInfo(CPlayerViewBase::ViewShowLabelType::ViewFrameLabel, QVariant::fromValue(info));
}

void CSLocalVideoPlayer::SlotDisplayBtnClicked(int index)
{
	if (ui.PlayBtn->isChecked())
	{
		m_bPause = false;
	}
	
	if (ui.PauseBtn->isChecked())
	{
		m_bPause = true;
	}

}

void CSLocalVideoPlayer::SlotTimeout()
{
	if (!m_bPause)
	{
		m_iCurrentFrame += m_iPlaySpeed;
		if (m_iCurrentFrame >= m_infoVideo.frame_num)
		{
			m_iCurrentFrame = 0;
		}
		ui.ProcessSlider->setValue(m_iCurrentFrame);
	}
}

void CSLocalVideoPlayer::OnPlayPauseBtnClicked()
{
	if (m_bPlay)
	{
		ui.PlayPauseBtn->setIcon(QPixmap(":/image/image/Display.png"));
		ui.PlayPauseBtn->setToolTip(tr("Play"));
		m_bPause = true;
		m_bPlay = false;
	}
	else
	{
		ui.PlayPauseBtn->setIcon(QPixmap(":/image/image/pause.png"));
		ui.PlayPauseBtn->setToolTip(tr("Pause"));
		m_bPause = false;
		m_bPlay = true;
	}
	ui.ProcessSlider->setFocus();
}

void CSLocalVideoPlayer::SlotSpeedBtnClicked(int index)
{
	if (ui.Speed_1X_Btn->isChecked())
	{
		m_iPlaySpeed = 1;
	}

	if (ui.Speed_2X_Btn->isChecked())
	{
		m_iPlaySpeed = 2;
	}

	if (ui.Speed_4X_Btn->isChecked())
	{
		m_iPlaySpeed = 4;
	}

	if (ui.Speed_8X_Btn->isChecked())
	{
		m_iPlaySpeed = 8;
	}

	if (ui.Speed_16X_Btn->isChecked())
	{
		m_iPlaySpeed = 16;
	}
}

void CSLocalVideoPlayer::on_ClrRevertBox_clicked(bool checked)
{
	m_bColorRevert = checked;
}

void CSLocalVideoPlayer::on_LumSlider_valueChanged(int value)
{
	m_iLuminance = value;
}

void CSLocalVideoPlayer::on_ContrastSlide_valueChanged(int value)
{
	m_iContrast = value;
}

void CSLocalVideoPlayer::on_ProcessSlider_valueChanged(int value)
{
	UpdateFrameInfo(value);
	// Modifity by Juwc -- 2022/7/14
	//ui.CurrentLineEdit->setText(QString::number(value * 1000.0 / m_infoVideo.fps, 'f', 3));
	ui.CurrentLineEdit->setText(QString::number(value+1, 'f', 3));
	m_iCurrentFrame = value;
}
