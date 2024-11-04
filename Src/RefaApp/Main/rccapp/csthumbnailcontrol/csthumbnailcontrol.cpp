#include "csthumbnailcontrol.h"
#include "ui_csthumbnailcontrol.h"
#include <QContextMenuEvent>
#include <QStyle>
#include <QPainter>

CSThumbnailControl::CSThumbnailControl(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSThumbnailControl),
	m_iIndex(index)
{
    ui->setupUi(this);
	Init();
}

CSThumbnailControl::~CSThumbnailControl()
{
    delete ui;
}

void CSThumbnailControl::DrawBorderLine(bool moveIn)
{
	if (!m_imgCurrent.isNull())
	{
		if (move_in_ != moveIn)
		{
			move_in_ = moveIn;
			update();
			emit SignalUpdateShowHotWidget(m_iIndex, move_in_);
		}
	}
#if TRUN_ON_FLOAT_THUMBNAIL
	else if (m_iIndex == constFloatControlIndex)
	{
		if (move_in_ != moveIn)
		{
			move_in_ = moveIn;
			emit SignalUpdateShowHotWidget(m_iIndex, move_in_);
		}
	}
#endif
}

void CSThumbnailControl::UpdateThumbnail(int thumbnail_index, const RccFrameInfo & thumbnail_image)
{
	m_iThumbnailIndex = thumbnail_index;
	//UadateBackground(true);
	m_imgFrameNo = thumbnail_image.playback_info.frame_no;
	m_bKeyFrame = thumbnail_image.playback_info.is_key_frame;
	m_bStartFrame = (thumbnail_image.playback_info.frame_no == thumbnail_image.playback_info.start_frame_no) ? true : false;
	m_bEndFrame = (thumbnail_image.playback_info.frame_no == thumbnail_image.playback_info.end_frame_no) ? true : false;
	thumbnail_state_ = thumbnail_image.playback_info.thumb_nail_state;
	MouseRightClicked();
	switch (thumbnail_image.playback_info.thumb_nail_state)
	{
	case PlaybackInfo::ThumbNailState::TN_VOID:
		m_bEmptyImage = true;
		ui->frame_info_label->setText(tr(" No Image"));
		SetThumbnail(QImage{});
		SetLoadInfo(None1);
		break;
	case PlaybackInfo::ThumbNailState::TN_NORMAL:
		SetFrameInfo(thumbnail_image.playback_info.is_key_frame, m_imgFrameNo);
		SetLoadInfo(None1);
		SetThumbnail(thumbnail_image.image);
		
		break;
	case PlaybackInfo::ThumbNailState::TN_LOADING:
		m_bEmptyImage = true;
		SetThumbnail(QImage{});//加载时也不显示以往的图像
		SetFrameInfo(thumbnail_image.playback_info.is_key_frame, m_imgFrameNo);
		SetLoadInfo(Loading);
		break;
	case PlaybackInfo::ThumbNailState::TN_FAIL:
		SetFrameInfo(thumbnail_image.playback_info.is_key_frame, m_imgFrameNo);
		SetLoadInfo(LoadFail);
		break;
	default:
		break;
	}
}

void CSThumbnailControl::Init()
{
	InitUI();
}

void CSThumbnailControl::InitUI()
{
	//去除标题框
	setWindowFlags(Qt::FramelessWindowHint);
	ui->frame_info_label->setObjectName("FrameInfo");
	//ui->image_show_label->setObjectName("ImageInfo");
	ui->image_show_label->setObjectName("LoadingInfo");
	ui->widget->setObjectName("MainWidgetMoveOut");
	m_Graph = new QGraphicsDropShadowEffect(this);
}

void CSThumbnailControl::SetFrameInfo(bool bKey, const qint64 frameNo)
{
	QString strInfo{};
	if (bKey)
	{
		strInfo = tr("Frame") + QString::number(frameNo+1) + tr("[Key Frame]");
	}
	else
	{
		strInfo = tr("Frame") + QString::number(frameNo+1);
	}
	ui->frame_info_label->setText(strInfo);
}

void CSThumbnailControl::SetThumbnail(const QImage & img)
{
	m_imgCurrent = img;

	if (!m_imgCurrent.isNull())
	{
		QPixmap pix = QPixmap::fromImage(m_imgCurrent);
		QPixmap npix = pix.scaled(ui->image_show_label->width(), ui->image_show_label->height(), Qt::KeepAspectRatio);
		//CSLOG_INFO("show Thumbnail - SetThumbnail: index={}, no={}, width={}, height={}.", m_iThumbnailIndex, m_imgFrameNo, npix.width(), npix.height());
		ui->image_show_label->setPixmap(npix);
	}
	else if (m_bEmptyImage)
	{
		m_bEmptyImage = false;
		QPixmap null{};
		//CSLOG_INFO("reset Thumbnail - SetThumbnail: index={}, no={}.", m_iThumbnailIndex, m_imgFrameNo);

		ui->image_show_label->setPixmap(null);
	}

	if (m_imgCurrent.isNull()) CSLOG_WARN("SetThumbnail m_imgCurrent is nullptr");
}

void CSThumbnailControl::SetLoadInfo(const LoadStatus status)
{
	if (!m_imgCurrent.isNull())
	{
		SetThumbnail(QImage{});
	}
	
	switch (status)
	{
	case Loading:
		//UadateBackground(false);
		ui->image_show_label->setText(tr("Loading..."));
		break;
	case LoadFail:
		//UadateBackground(false);
		ui->image_show_label->setText(tr("Load Fail"));
		break;
	case None1:
		//UadateBackground(true);
		ui->image_show_label->setText("");
		break;
	default:
		break;
	}
}

void CSThumbnailControl::contextMenuEvent(QContextMenuEvent * event)
{
	if (!m_imgCurrent.isNull())
	{
		if (!m_rightMenu)
		{
			MouseRightClicked();
		}
		m_rightMenu->exec(event->globalPos());
	}
}

void CSThumbnailControl::MouseRightClicked()
{
	m_rightMenu = new QMenu(this);
	QAction* actKeyFrame = new QAction(tr("Set Key Frame"), this);
	QAction* actBeginPos = new QAction(tr("Set Begin Position"), this);
	QAction* actEndPos = new QAction(tr("Set End Position"), this);

	if (m_bKeyFrame)
	{
		actKeyFrame->setDisabled(true);
	}

	if (m_bStartFrame)
	{
		actBeginPos->setDisabled(true);
	}

	if (m_bEndFrame)
	{
		actEndPos->setDisabled(true);
	}

	connect(actKeyFrame, &QAction::triggered, this, [this] {

		emit SignalKeyFrame(m_imgFrameNo);
	});
	connect(actBeginPos, &QAction::triggered, this, [this] {
		emit SignalBeginFrame(m_imgFrameNo);
	});
	connect(actEndPos, &QAction::triggered, this, [this] {
		emit SignalEndFrame(m_imgFrameNo);
	});

	m_rightMenu->addAction(actKeyFrame);
	m_rightMenu->addAction(actBeginPos);
	m_rightMenu->addAction(actEndPos);
}

void CSThumbnailControl::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	painter.save();
	bool bShowBorder = move_in_;
#if TRUN_ON_FLOAT_THUMBNAIL
	if (m_iIndex == constFloatControlIndex)
	{
		bShowBorder = true;
	}
#endif
	QColor border_color = bShowBorder ? QColor(Qt::red) : QColor(0, 0, 0, 0);
	int border_width = 2;
	painter.setPen(QPen(QBrush(border_color), border_width));

	painter.drawRect(rect().adjusted(border_width, border_width, -border_width, -border_width));

	painter.restore();
}

void CSThumbnailControl::enterEvent(QEvent * event)
{
	setCursor(Qt::PointingHandCursor);

#if TRUN_ON_FLOAT_THUMBNAIL
	if (!m_imgCurrent.isNull() && (-1 != m_iThumbnailIndex) && (constFloatControlIndex != m_iIndex))
	{
		DrawBorderLine(true);
		emit SignalShowImgInBigScreen(m_iThumbnailIndex);
	}
	else if (constFloatControlIndex == m_iIndex)
	{
		DrawBorderLine(true);
		if (-1 != m_iThumbnailIndex)
		{
			emit SignalShowImgInBigScreen(m_iThumbnailIndex);
		}
	}
#else
	if (!m_imgCurrent.isNull() && (-1 != m_iThumbnailIndex))
	{
		DrawBorderLine(true);
		emit SignalShowImgInBigScreen(m_iThumbnailIndex);
	}
#endif
}

void CSThumbnailControl::leaveEvent(QEvent * event)
{
	setCursor(Qt::ArrowCursor);
	DrawBorderLine(false);

#if TRUN_ON_FLOAT_THUMBNAIL
	if (m_iIndex == constFloatControlIndex)
	{
		emit SignalMoveOutThumbnailArea();
	}
#endif
}

void CSThumbnailControl::UadateBackground(bool bSucess)
{
	style()->unpolish(ui->image_show_label);
	bSucess ? ui->image_show_label->setObjectName("ImageInfo") : ui->image_show_label->setObjectName("LoadingInfo");
	style()->polish(ui->image_show_label);
	update();
}

void CSThumbnailControl::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		emit SignalSingleThumbnailClicked(m_imgFrameNo);
	}
}

void CSThumbnailControl::resizeEvent(QResizeEvent *event)
{
	if (ui->image_show_label && (!m_imgCurrent.isNull())) {
		QPixmap pix = QPixmap::fromImage(m_imgCurrent);
		QPixmap npix = pix.scaled(ui->image_show_label->width(), ui->image_show_label->height(), Qt::KeepAspectRatio);
		ui->image_show_label->setPixmap(npix);
	}
}

