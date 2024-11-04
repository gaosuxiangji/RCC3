#include "csexportpreviewplayersliderwidget.h"
#include "ui_csexportpreviewplayersliderwidget.h"
#include <QPainter>
#include <QString>
#include <QResizeEvent>
#include <QDebug>
#include <thread>
#include <atomic>
#include <QTimer>
#include "Video/VideoUtils/videoutils.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#define ENABLE_HOT_AREA_DOUBLE 0

CSExportPreviewPlayerSliderWidget::CSExportPreviewPlayerSliderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSExportPreviewPlayerSliderWidget),
    slider_left_img(QPixmap(":/image/image/export_preview_slider-left.png")),
    slider_right_img(QPixmap(":/image/image/export_preview_slider-right.png")),
    slider_pos_img(QPixmap(":/image/image/export_preview_slider-pos.png")),
	trigger_frame_img(QPixmap(":/image/image/export_preview_trigger.png"))
{
    ui->setupUi(this);

	slider_left_img=slider_left_img.scaledToWidth(playerRangeIconWidth);
	slider_right_img=slider_right_img.scaledToWidth(playerRangeIconWidth);

	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &CSExportPreviewPlayerSliderWidget::slotHandleMouseMoveEvent);
	timer->start(kMouseMoveHandlePeriod);

// 	QMatrix matrix;
// 	matrix.rotate(180);
// 	slider_pos_img=slider_pos_img.transformed(matrix, Qt::SmoothTransformation);
}

CSExportPreviewPlayerSliderWidget::~CSExportPreviewPlayerSliderWidget()
{
    delete ui;
}

void CSExportPreviewPlayerSliderWidget::SetTotalFrameCnt(uint64_t total, bool bForceFlush)
{
	if ((validTotalFrameCnt != total) || bForceFlush)
	{
		totalFrameCnt = total;
		validPlayerRangeBeginFrameIndex = 0;
		validPlayerRangeEndFrameIndex = total - 1;
		validTotalFrameCnt = total;
		curFrameIndex = 0;

		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::SetFrameRate(const qint64 & fps)
{
	m_iFrameRate = fps;
}

void CSExportPreviewPlayerSliderWidget::GetPlayerRange(uint64_t& begin, uint64_t& end)
{
	begin = validPlayerRangeBeginFrameIndex;
	end = validPlayerRangeEndFrameIndex;
}

bool CSExportPreviewPlayerSliderWidget::SetPlayerRange(uint64_t begin, uint64_t end)
{
	if ((validPlayerRangeBeginFrameIndex != begin) || (validPlayerRangeEndFrameIndex != end))
	{
		if (!(begin < totalFrameCnt && end < totalFrameCnt && begin <= end))
		{
			return false;
		}

		if (curFrameIndex<begin || curFrameIndex>end)
		{
			curFrameIndex = begin;
		}

		emit sigSeekFrame(curFrameIndex);

		validPlayerRangeBeginFrameIndex = begin;
		validPlayerRangeEndFrameIndex = end;
		validTotalFrameCnt = end - begin + 1;

		MakePaintBuffer();
	}

	return true;
}

void CSExportPreviewPlayerSliderWidget::SetTriggerFrame(uint64_t trigger)
{
	//if (triggerFrameIndex != trigger)
	{
		triggerFrameIndex = trigger;
		isTriggerFrameExists = true;

		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::ClearTriggerFrame()
{
	if (isTriggerFrameExists)
	{
		isTriggerFrameExists = false;

		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::SetEnabled(bool enable)
{
	if (enabled != enable)
	{
		this->setEnabled(enable);
		enabled = enable;

		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::SetPlayerBusyFlag(bool busy)
{
	isBusy = busy;
}

void CSExportPreviewPlayerSliderWidget::SetPlayerPausedFlag(bool pause)
{
	isPlayerPaused = pause;
}

void CSExportPreviewPlayerSliderWidget::SetCurFrameIndex(uint64_t frameIndex)
{
	if (curFrameIndex != frameIndex)
	{
		if (HIT_NONE == curHitIconType || HIT_ICON_TRIGGER_FRAME == curHitIconType)
		{
			curFrameIndex = frameIndex;
		}

		MakePaintBuffer();
		//update();

		/*if (isPlayerPaused)
		{*/
		emit sigReachMinValidFrameIndex(curFrameIndex == validPlayerRangeBeginFrameIndex);
		emit sigReachMaxValidFrameIndex(curFrameIndex == validPlayerRangeEndFrameIndex);
		//}
	}
}

void CSExportPreviewPlayerSliderWidget::SwitchFrameAndMs(const bool bFrame)
{
	if (m_bFrame != bFrame)
	{
		m_bFrame = bFrame;
		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::SetThumbnailRange(const uint64_t & iStart, const uint64_t & iEnd)
{
	if (m_thumbnailRangeBeginFrameIndex != iStart || m_thumbnailRangeEndFrameIndex != iEnd)
	{
		m_thumbnailRangeBeginFrameIndex = iStart;
		m_thumbnailRangeEndFrameIndex = iEnd;
		MakePaintBuffer();
	}
}

void CSExportPreviewPlayerSliderWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    //painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);//文字和图形抗锯齿

	if (isFirstPaint)
	{
		MakePaintBuffer(false);
		isFirstPaint = false;
	}

	painter.drawPixmap(0, 0, paintBuffer);
}

void CSExportPreviewPlayerSliderWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	QSize newSize = event->size();

	int playerPosIconHeight = newSize.height()*playerPosIconStretch;//进度条距离界面两端的距离，用于放置播放范围图标
	slider_pos_img = slider_pos_img.scaledToHeight(playerPosIconHeight);

	int triggerFrameIconHeight = newSize.height()*playerTriggerFrameIconStretch;
	trigger_frame_img = trigger_frame_img.scaledToHeight(triggerFrameIconHeight);

	paintBuffer_backup = QPixmap(this->size());
	paintBuffer_backup.fill(this->palette().window().color());
	MakePaintBuffer();
}

void CSExportPreviewPlayerSliderWidget::mousePressEvent(QMouseEvent *event)
{
	m_bPressed = true;
	QWidget::mousePressEvent(event);

	if (isBusy || 0 >= totalFrameCnt)
	{
		return;
	}

	QPoint pos = event->pos();
	curHitIconType = HitIconDetect(pos);
	MakePaintBuffer();
}

void CSExportPreviewPlayerSliderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_bPressed = false;
	QWidget::mouseReleaseEvent(event);

	if (isBusy || 0 >= totalFrameCnt)
	{
		return;
	}

	switch (curHitIconType)
	{
	case CSExportPreviewPlayerSliderWidget::HIT_NONE:
		break;
	case CSExportPreviewPlayerSliderWidget::HIT_ICON_PLAYER_RANGE_LEFT:
	{
		m_bSwtiching = true;
		emit sigPlayerBeginRangeChanged(validPlayerRangeBeginFrameIndex);
		m_bSwtiching = false;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(20));//等待设置完成
		emit sigSeekFrame(curFrameIndex);
	}
		break;
	case CSExportPreviewPlayerSliderWidget::HIT_ICON_PLAYER_RANGE_RIGHT:
		m_bSwtiching = true;
		emit sigPlayerEndRangeChanged(validPlayerRangeEndFrameIndex);
		m_bSwtiching = false;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(20));//等待设置完成
		emit sigSeekFrame(validPlayerRangeBeginFrameIndex);
		break;
	case CSExportPreviewPlayerSliderWidget::HIT_ICON_PLAYER_POS:
		emit sigSeekFrame(curFrameIndex);
		break;
	case CSExportPreviewPlayerSliderWidget::HIT_ICON_TRIGGER_FRAME:
		emit SignalKeyFrameToBigScreen();
		break;
	default:
		break;
	}

	QRect rc = this->rect();
	QPoint pt = event->pos();
	//点击进度条区域，跳转
	if (pt.x() >= m_sliderRect.left() && pt.x() <= m_sliderRect.right() && pt.y() >= m_sliderRect.top() && pt.y() <= m_sliderRect.bottom())
	{
		uint64_t frameIndex = curFrameIndex;

		if (event->pos().x() - playerRangeIconWidth < 0)
		{
			frameIndex = 0;
		}
		else
		{
			int64_t nIndex = (event->pos().x() - playerRangeIconWidth) / (rc.width() - 2.f*playerRangeIconWidth)*totalFrameCnt;
			if (nIndex < 0)
			{
				nIndex = 0;
			}
			frameIndex = nIndex;
		}

		//边界限制
		if (frameIndex >= totalFrameCnt)
		{
			frameIndex = totalFrameCnt - 1;
		}
		emit SignalSliderMouseClicked(frameIndex);
	}

	curHitIconType = HIT_NONE;
	MakePaintBuffer();
}

void CSExportPreviewPlayerSliderWidget::mouseMoveEvent(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);
	if (isBusy)
	{
		return;
	}

	if (mouse_move_event_.isNull())
	{
		mouse_move_event_.reset(new QMouseEvent(*event));
	}
	else
	{
		*mouse_move_event_ = *event;
	}

	QPoint pos = event->pos();
	if (isTriggerFrameExists && touchTriggerFrameRect.contains(pos))
	{
		m_key_tip_visible = true;
	}
	else {
		m_key_tip_visible = false;
	}
	MakePaintBuffer();
}

void CSExportPreviewPlayerSliderWidget::slotHandleMouseMoveEvent()
{
	if (mouse_move_event_ && !mouse_move_event_->isAccepted())
	{
		if (!m_bSwtiching)
		{
			handleMouseMoveEvent(mouse_move_event_.data());
		}
		
		mouse_move_event_->accept();
	}
}

CSExportPreviewPlayerSliderWidget::HitIconType CSExportPreviewPlayerSliderWidget::HitIconDetect(const QPoint& mousePos) const
{
	HitIconType type = HIT_NONE;
	if (touch_slider_left_img_rect.contains(mousePos))
	{
		type = HIT_ICON_PLAYER_RANGE_LEFT;
	}
	else if (touch_slider_right_img_rect.contains(mousePos))
	{
		type = HIT_ICON_PLAYER_RANGE_RIGHT;
	}
	else if (touchPlayerPosIconRect.contains(mousePos))
	{
		type = HIT_ICON_PLAYER_POS;
	}
	else if (isTriggerFrameExists && touchTriggerFrameRect.contains(mousePos))
	{
		type = HIT_ICON_TRIGGER_FRAME;
	}

	return type;
}

void CSExportPreviewPlayerSliderWidget::refreshTouchRect()
{
	//左播放范围按钮热区
	touch_slider_left_img_rect = slider_left_img_rect;
#if ENABLE_HOT_AREA_DOUBLE
	touch_slider_left_img_rect.setWidth(slider_left_img_rect.width() * 2);
	touch_slider_left_img_rect.moveLeft(slider_left_img_rect.left() - slider_left_img_rect.width()*0.5);
	touch_slider_left_img_rect.setHeight(slider_left_img_rect.height() * 2);
	touch_slider_left_img_rect.moveBottom(slider_left_img_rect.bottom());
#endif

	//右播放范围按钮热区
	touch_slider_right_img_rect = slider_right_img_rect;
#if ENABLE_HOT_AREA_DOUBLE
	touch_slider_right_img_rect.setWidth(slider_right_img_rect.width() * 2);
	touch_slider_right_img_rect.moveLeft(slider_right_img_rect.left() - slider_right_img_rect.width()*0.5);
	touch_slider_right_img_rect.setHeight(slider_right_img_rect.height() * 2);
	touch_slider_right_img_rect.moveBottom(slider_right_img_rect.bottom());
#endif

	//播放位置按钮热区
	touchPlayerPosIconRect = playerPosIconRect;
#if ENABLE_HOT_AREA_DOUBLE
	touchPlayerPosIconRect.setWidth(playerPosIconRect.width() * 2);
	touchPlayerPosIconRect.moveLeft(playerPosIconRect.left() - playerPosIconRect.width()*0.5);
	touchPlayerPosIconRect.setHeight(playerPosIconRect.height() * 2);
	touchPlayerPosIconRect.moveBottom(playerPosIconRect.bottom());
#endif

	//关键帧按钮热区
	touchTriggerFrameRect = triggerFrameRect;
#if ENABLE_HOT_AREA_DOUBLE
	touchTriggerFrameRect.setWidth(triggerFrameRect.width() * 2);
	touchTriggerFrameRect.moveLeft(triggerFrameRect.left() - triggerFrameRect.width()*0.5);
	touchTriggerFrameRect.setHeight(triggerFrameRect.height() * 2);
	touchTriggerFrameRect.moveBottom(triggerFrameRect.bottom());
#endif
}

void CSExportPreviewPlayerSliderWidget::SetImageBackgroundColor(const QPixmap& src, QImage& dst, const QColor& oldBackground, const QColor& newBackgroundColor) const
{
	dst = src.toImage();

	for (int i = 0; i < dst.height(); i++)
	{
		for (int j = 0; j < dst.width(); j++)
		{
			//if (dst.pixelColor(QPoint(j, i)) == oldBackground)
			{
				dst.setPixelColor(QPoint(j, i), newBackgroundColor);
			}
		}
	}
}

void CSExportPreviewPlayerSliderWidget::MakePaintBuffer(bool needs_repaint)
{
	paintBuffer = paintBuffer_backup;
	
	QPainter painter(&paintBuffer);
 	painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);//文字和图形抗锯齿

	QRect rc = this->rect();

	QFont font = this->font();
	int nSize = font.pointSize();
	if (nSize <= 0)
	{
		font.setPointSize(9);
		this->setFont(font);
	}
	font.setPointSize(font.pointSize() <= 1 ? font.pointSize() : font.pointSize() - 1);
	painter.setFont(font);

	QString totalRangeText = QString::number(validTotalFrameCnt);
	totalRangeText = m_bFrame ? QString::number(validTotalFrameCnt) : \
		QString::number(VideoUtils::frameIdToMs(validTotalFrameCnt, m_iFrameRate), 'f', 1);
	uint64_t uiDrawTotal = totalFrameCnt - 1;
	QString playerRangeLeftText = QString::number(validPlayerRangeBeginFrameIndex);
	if (totalFrameCnt > 0)
	{
		playerRangeLeftText = m_bFrame ? QString::number(validPlayerRangeBeginFrameIndex+1) : \
			QString::number(VideoUtils::frameIdToMs(validPlayerRangeBeginFrameIndex + 1, m_iFrameRate), 'f', 1);
	}

	QString playerRangeRightText = QString::number(validPlayerRangeEndFrameIndex);
	if (totalFrameCnt > 0)
	{
		playerRangeRightText = m_bFrame ? QString::number(validPlayerRangeEndFrameIndex+1) : \
			QString::number(VideoUtils::frameIdToMs(validPlayerRangeEndFrameIndex+1, m_iFrameRate), 'f', 1);
	}

	//绘制播放范围信息
	QString playerRangeText = tr("begin/end:") + QString("%1-%2").arg(playerRangeLeftText).arg(playerRangeRightText) + (m_bFrame ? \
		tr(" Frame") : QString("ms")) + QString(",") + tr("Length:") + totalRangeText + (m_bFrame ? tr(" Frame") : QString("ms"));

	int playerRangeTextHeight = rc.height()*playerRangeTextStretch;
	QRectF playerRangeTextRect(QPointF(0,6), QSizeF(rc.width(), playerRangeTextHeight));
	painter.drawText(playerRangeTextRect, Qt::AlignCenter, playerRangeText);

	//绘制左播放范围图标
	float validPlayerRangeBeginPos = playerRangeIconWidth;
	if (totalFrameCnt != 0 && totalFrameCnt != 1)
	{
		validPlayerRangeBeginPos = (rc.width() - 2.f* playerRangeIconWidth)*validPlayerRangeBeginFrameIndex / uiDrawTotal + playerRangeIconWidth;
	}
	slider_left_img_rect = QRectF(QPointF(), slider_left_img.size());
	float playerRangeIconBottom = rc.height()*(playerRangeTextStretch + playerRangeIconStretch);
	slider_left_img_rect.moveBottomRight(QPointF(validPlayerRangeBeginPos, playerRangeIconBottom));
	if (enabled)
	{
		//if (curHitIconType == HIT_ICON_PLAYER_RANGE_LEFT)
		//{
		//	if (slider_left_img_pressed.isNull() || slider_left_img_pressed.size() != slider_left_img.size())
		//	{
		//		SetImageBackgroundColor(slider_left_img, slider_left_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
		//	}
		//	painter.drawImage(slider_left_img_rect.topLeft(), slider_left_img_pressed);//按下样式
		//}
		//else
		{
			painter.drawPixmap(slider_left_img_rect.topLeft(), slider_left_img);//正常样式
		}
	}
	else
	{
		if (slider_left_img_disabled.isNull() || slider_left_img_disabled.size() != slider_left_img.size())
		{
			SetImageBackgroundColor(slider_left_img, slider_left_img_disabled, iconBackgroundColor, Qt::lightGray);
		}
		painter.drawImage(slider_left_img_rect.topLeft(), slider_left_img_disabled);//置灰样式
	}
	
	//绘制右播放范围图标
	float validPlayerRangeEndPos = rc.width() - playerRangeIconWidth;
	if (totalFrameCnt != 0 && totalFrameCnt != 1)
	{
		validPlayerRangeEndPos = (rc.width() - 2.f* playerRangeIconWidth)*validPlayerRangeEndFrameIndex / uiDrawTotal + playerRangeIconWidth;
	}
	slider_right_img_rect = QRectF(QPointF(), slider_right_img.size());
	slider_right_img_rect.moveBottomLeft(QPointF(validPlayerRangeEndPos, playerRangeIconBottom));
	if (enabled)
	{
		//if (curHitIconType == HIT_ICON_PLAYER_RANGE_RIGHT)
		//{
		//	if (slider_right_img_pressed.isNull() || slider_right_img_pressed.size() != slider_right_img.size())
		//	{
		//		SetImageBackgroundColor(slider_right_img, slider_right_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
		//	}
		//	painter.drawImage(slider_right_img_rect.topLeft(), slider_right_img_pressed);//按下样式
		//}
		//else
		{
			painter.drawPixmap(slider_right_img_rect.topLeft(), slider_right_img);//正常样式
		}
	}
	else
	{
		if (slider_right_img_disabled.isNull() || slider_right_img_disabled.size() != slider_right_img.size())
		{
			SetImageBackgroundColor(slider_right_img, slider_right_img_disabled, iconBackgroundColor, Qt::lightGray);
		}
		painter.drawImage(slider_right_img_rect.topLeft(), slider_right_img_disabled);//置灰样式
	}

	QPen pen(Qt::gray, 1);
	painter.setPen(pen);
	
	//绘制播放范围上直线
	QPointF validPlayerRangeLineTopLeft(validPlayerRangeBeginPos, PLAY_RANGE_TOP_LINE);
	QPointF validPlayerRangeLineTopRight(validPlayerRangeEndPos, PLAY_RANGE_TOP_LINE);
	painter.drawLine(validPlayerRangeLineTopLeft, validPlayerRangeLineTopRight);

	//绘制播放范围左直线
	QPointF validPlayerRangeLineBottomLeft(validPlayerRangeBeginPos, playerRangeIconBottom);
	painter.drawLine(validPlayerRangeLineTopLeft, validPlayerRangeLineBottomLeft);

	//绘制播放范围右直线
	QPointF validPlayerRangeLineBottomRight(validPlayerRangeEndPos, playerRangeIconBottom);
	painter.drawLine(validPlayerRangeLineTopRight, validPlayerRangeLineBottomRight);

	//绘制进度条
	QRectF sliderRect(playerRangeIconWidth, playerRangeIconBottom, rc.width() - 2.f*playerRangeIconWidth, rc.height()*playerSliderStretch+10);
	painter.drawRect(sliderRect);
	QRectF validSliderRect(QPointF(validPlayerRangeBeginPos, sliderRect.top()), QPointF(validPlayerRangeEndPos, sliderRect.bottom()));
	m_sliderRect = validSliderRect;
	if (enabled)
	{
		painter.fillRect(validSliderRect, iconBackgroundColor);
	}
	else
	{
		painter.fillRect(validSliderRect, Qt::lightGray);
	}

// 	float thumbnailRangeBeginPos = playerRangeIconWidth;
// 	float thumbnailRangeEndPos = rc.width() - playerRangeIconWidth;
// 	if (totalFrameCnt != 0 && totalFrameCnt != 1)
// 	{
// 		int nthumbBegin = m_thumbnailRangeBeginFrameIndex;
// 		if (nthumbBegin < validPlayerRangeBeginFrameIndex)
// 		{
// 			nthumbBegin = validPlayerRangeBeginFrameIndex;
// 		}
// 		int nthumbEnd = m_thumbnailRangeEndFrameIndex + 1;
// 		if (nthumbEnd > validPlayerRangeEndFrameIndex)
// 		{
// 			nthumbEnd = validPlayerRangeEndFrameIndex;
// 		}
// 		if (nthumbBegin > validPlayerRangeEndFrameIndex)
// 		{
// 			nthumbBegin = nthumbEnd;
// 		}
// 		if (nthumbEnd < validPlayerRangeBeginFrameIndex)
// 		{
// 			nthumbEnd = nthumbBegin;
// 		}
// 		thumbnailRangeBeginPos = (rc.width() - 2.f* playerRangeIconWidth)*nthumbBegin / uiDrawTotal + playerRangeIconWidth;
// 
// 		thumbnailRangeEndPos = (rc.width() - 2.f* playerRangeIconWidth)*nthumbEnd / uiDrawTotal + playerRangeIconWidth;
// 	}
// 
// 	// 绘制缩略图范围
// 	QRectF thumbnailRect(QPointF(thumbnailRangeBeginPos, sliderRect.top()+5), QPointF(thumbnailRangeEndPos, sliderRect.bottom()-5));
// 	painter.fillRect(thumbnailRect, Qt::gray);
	
	//绘制播放位置图标
	//qDebug() << "pain current frameIndex: " << curFrameIndex;
	//图标移动位置不超过进度条区域
	float curPlayerPos = playerRangeIconWidth + slider_pos_img.size().width() / 2.0f;
	if (totalFrameCnt != 0 && totalFrameCnt != 1)
	{
		curPlayerPos = (rc.width() - 2.f* playerRangeIconWidth)*curFrameIndex / uiDrawTotal + playerRangeIconWidth;
	}
	playerPosIconRect = QRectF(QPointF(), slider_pos_img.size());
	playerPosIconRect.moveCenter(QPointF(curPlayerPos, sliderRect.center().y()));
	if (enabled)
	{
		//if (curHitIconType == HIT_ICON_PLAYER_POS)
		//{
		//	if (slider_pos_img_pressed.isNull() || slider_pos_img_pressed.size() != slider_pos_img.size())
		//	{
		//		SetImageBackgroundColor(slider_pos_img, slider_pos_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
		//	}
		//	painter.drawImage(playerPosIconRect.topLeft(), slider_pos_img_pressed);//按下样式
		//}
		//else
		{
			painter.drawPixmap(playerPosIconRect.topLeft(), slider_pos_img);//正常样式
		}
	}
	else
	{
		if (slider_pos_img_disabled.isNull() || slider_pos_img_disabled.size() != slider_pos_img.size())
		{
			SetImageBackgroundColor(slider_pos_img, slider_pos_img_disabled, iconBackgroundColor, Qt::lightGray);
		}
		painter.drawImage(playerPosIconRect.topLeft(), slider_pos_img_disabled);
	}
	
	//绘制关键帧图标
	if (isTriggerFrameExists)
	{
		triggerFrameRect = QRectF(QPointF(), trigger_frame_img.size());
		float triggerPos = playerRangeIconWidth;
		if (totalFrameCnt != 0 && totalFrameCnt != 1)
		{
			triggerPos = (rc.width() - 2.f* playerRangeIconWidth)*triggerFrameIndex / (totalFrameCnt - 1) + playerRangeIconWidth;
		}
		triggerFrameRect.moveCenter(QPointF(triggerPos, playerPosIconRect.bottom()));
		triggerFrameRect.moveTop(playerPosIconRect.bottom());
		if (enabled)
		{
			//if (curHitIconType == HIT_ICON_TRIGGER_FRAME)
			//{
			//	if (trigger_frame_img_pressed.isNull() || trigger_frame_img_pressed.size() != trigger_frame_img.size())
			//	{
			//		SetImageBackgroundColor(trigger_frame_img, trigger_frame_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
			//	}
			//	painter.drawImage(triggerFrameRect.topLeft(), trigger_frame_img_pressed);//按下样式
			//}
			//else
			{
				painter.drawPixmap(triggerFrameRect.topLeft(), trigger_frame_img);//正常样式
			}
		}
		else
		{
			if (trigger_frame_img_disabled.isNull() || trigger_frame_img_disabled.size() != trigger_frame_img.size())
			{
				SetImageBackgroundColor(trigger_frame_img, trigger_frame_img_disabled, iconBackgroundColor, Qt::lightGray);
			}
			painter.drawImage(triggerFrameRect.topLeft(), trigger_frame_img_disabled);
		}
		if (m_key_tip_visible) {
			painter.save();
			QPen pen(Qt::black, 1);
			QFont ft;
			ft.setFamily("Microsoft YaHei UI");
			ft.setPixelSize(12);
			painter.setFont(ft);
			painter.setPen(pen);
			if (triggerFrameRect.x() < (this->rect().width() / 2.0)) {
				painter.drawText(triggerFrameRect.bottomRight(), tr("Key Frame"));
			}
			else
			{
				QPointF tip_pt = QPointF(triggerFrameRect.left() - triggerFrameRect.width()*1.5, triggerFrameRect.bottom()+5);
				painter.drawText(tip_pt, tr("Key Frame"));
			}
			painter.restore();
		}
	}

	refreshTouchRect();

	if (needs_repaint)
	{
		update();
	}
}

void CSExportPreviewPlayerSliderWidget::handleMouseMoveEvent(QMouseEvent* mouse_event)
{
	QRect rc = this->rect();
	uint64_t frameIndex = curFrameIndex;

	QPoint pos = mouse_event->pos();
	if (pos.x() - playerRangeIconWidth < 0)
	{
		frameIndex = 0;
	}
	else
	{
		int64_t nIndex = (pos.x() - playerRangeIconWidth) / (rc.width() - 2.f*playerRangeIconWidth)*totalFrameCnt;
		if (nIndex < 0)
		{
			nIndex = 0;
		}
		frameIndex = nIndex;
	}

	//边界限制
	if (frameIndex >= totalFrameCnt)
	{
		frameIndex = totalFrameCnt - 1;
	}

	switch (curHitIconType)
	{
	case HIT_ICON_PLAYER_RANGE_LEFT:
		if (frameIndex <= validPlayerRangeEndFrameIndex)
		{
			validPlayerRangeBeginFrameIndex = frameIndex;
			validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;

			curFrameIndex = validPlayerRangeBeginFrameIndex;
		}
		else if(frameIndex >= validPlayerRangeEndFrameIndex && totalFrameCnt > 1)
		{
			if (validPlayerRangeBeginFrameIndex != validPlayerRangeEndFrameIndex)
			{
				validPlayerRangeBeginFrameIndex = validPlayerRangeEndFrameIndex;
				validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;

				curFrameIndex = validPlayerRangeBeginFrameIndex;
			}
		}
		break;
	case HIT_ICON_PLAYER_RANGE_RIGHT:
		if (frameIndex >= validPlayerRangeBeginFrameIndex && frameIndex <= (totalFrameCnt - 1))
		{
			validPlayerRangeEndFrameIndex = frameIndex;
			validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;
			curFrameIndex = validPlayerRangeEndFrameIndex;
		}
		else if (frameIndex <= validPlayerRangeBeginFrameIndex && totalFrameCnt >= 1)
		{
			if (validPlayerRangeBeginFrameIndex != validPlayerRangeEndFrameIndex)
			{
				validPlayerRangeEndFrameIndex = validPlayerRangeBeginFrameIndex;
				validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;

				curFrameIndex = validPlayerRangeEndFrameIndex;
			}
		}
		break;
	case HIT_ICON_PLAYER_POS:
		if (frameIndex >= validPlayerRangeBeginFrameIndex && frameIndex <= validPlayerRangeEndFrameIndex)
		{
			curFrameIndex = frameIndex;
		}
		break;
	default:
		break;
	}

	if (curHitIconType != HIT_NONE)
	{
		MakePaintBuffer();
		//update();
	}

	if (!m_bPressed)
	{
		auto hitType = HitIconDetect(pos);
		if (m_sliderRect.contains(pos) || (hitType != HIT_NONE && hitType != HIT_ICON_TRIGGER_FRAME))
		{
			setCursor(Qt::PointingHandCursor);
			emit SignalMouseMoveOnSlider(frameIndex);
		}
		else
		{
			setCursor(Qt::ArrowCursor);
		}
	}
}
