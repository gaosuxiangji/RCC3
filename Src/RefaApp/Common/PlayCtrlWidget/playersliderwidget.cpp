#include "playersliderwidget.h"
#include "ui_playersliderwidget.h"
#include <QPainter>
#include <QString>
#include <QResizeEvent>
#include <QDebug>
#include <thread>
#include <atomic>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

PlayerSliderWidget::PlayerSliderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerSliderWidget),
    slider_left_img(QPixmap(":/playctrl/images/slider-left.png")),
    slider_right_img(QPixmap(":/playctrl/images/slider-right.png")),
    slider_pos_img(QPixmap(":/playctrl/images/slider-pos.png")),
	trigger_frame_img(QPixmap(":/playctrl/images/trigger.png"))
{
    ui->setupUi(this);

	slider_left_img=slider_left_img.scaledToWidth(playerRangeIconWidth);
	slider_right_img=slider_right_img.scaledToWidth(playerRangeIconWidth);

// 	QMatrix matrix;
// 	matrix.rotate(180);
// 	slider_pos_img=slider_pos_img.transformed(matrix, Qt::SmoothTransformation);

#if 0   //测试用
	SetTotalFrameCnt(1000);
#endif

}

PlayerSliderWidget::~PlayerSliderWidget()
{
    delete ui;
}

void PlayerSliderWidget::SetTotalFrameCnt(uint64_t total)
{
	totalFrameCnt = total;
	validPlayerRangeBeginFrameIndex = 0;
	validPlayerRangeEndFrameIndex = total-1;
	validTotalFrameCnt = total;
	curFrameIndex = 0;

	MakePaintBuffer();
}

void PlayerSliderWidget::GetPlayerRange(uint64_t& begin, uint64_t& end)
{
	begin = validPlayerRangeBeginFrameIndex;
	end = validPlayerRangeEndFrameIndex;
}

bool PlayerSliderWidget::SetPlayerRange(uint64_t begin, uint64_t end)
{
	if (!(begin < totalFrameCnt && end < totalFrameCnt && begin <= end))
	{
		return false;
	}

	if (curFrameIndex<begin || curFrameIndex>end)
	{
		curFrameIndex = begin;
	}

	validPlayerRangeBeginFrameIndex = begin;
	validPlayerRangeEndFrameIndex = end;

	MakePaintBuffer();

	return true;
}

void PlayerSliderWidget::SetTriggerFrame(uint64_t trigger)
{
	triggerFrameIndex = trigger;
	isTriggerFrameExists = true;

	MakePaintBuffer();
}

void PlayerSliderWidget::ClearTriggerFrame()
{
	isTriggerFrameExists = false;

	MakePaintBuffer();
}

void PlayerSliderWidget::SetEnabled(bool enable)
{
	this->setEnabled(enable);
	enabled = enable;

	MakePaintBuffer();
}

void PlayerSliderWidget::SetPlayerBusyFlag(bool busy)
{
	isBusy = busy;
}

void PlayerSliderWidget::SetPlayerPausedFlag(bool pause)
{
	isPlayerPaused = pause;
}

void PlayerSliderWidget::SetCurFrameIndex(uint64_t frameIndex)
{
	if (HIT_NONE == curHitIconType)
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

void PlayerSliderWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    //painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);//文字和图形抗锯齿

	if (isFirstPaint)
	{
		MakePaintBuffer();
		isFirstPaint = false;
	}

	painter.drawPixmap(0, 0, paintBuffer);
}

void PlayerSliderWidget::resizeEvent(QResizeEvent *event)
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

void PlayerSliderWidget::mousePressEvent(QMouseEvent *event)
{
	QWidget::mousePressEvent(event);

	if (isBusy)
	{
		return;
	}

	QPoint pos = event->pos();
	curHitIconType = HitIconDetect(pos);
	MakePaintBuffer();
}

void PlayerSliderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	QWidget::mouseReleaseEvent(event);

	if (isBusy)
	{
		return;
	}

	switch (curHitIconType)
	{
	case PlayerSliderWidget::HIT_NONE:
		break;
	case PlayerSliderWidget::HIT_ICON_PLAYER_RANGE_LEFT:
		emit sigPlayerBeginRangeChanged(validPlayerRangeBeginFrameIndex);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(20));//等待设置完成
		emit sigSeekFrame(curFrameIndex);
		break;
	case PlayerSliderWidget::HIT_ICON_PLAYER_RANGE_RIGHT:
		emit sigPlayerEndRangeChanged(validPlayerRangeEndFrameIndex);
		boost::this_thread::sleep_for(boost::chrono::milliseconds(20));//等待设置完成
		emit sigSeekFrame(curFrameIndex);
		break;
	case PlayerSliderWidget::HIT_ICON_PLAYER_POS:
		emit sigSeekFrame(curFrameIndex);
		break;
	case PlayerSliderWidget::HIT_ICON_TRIGGER_FRAME:
		curFrameIndex = triggerFrameIndex;
		emit sigSeekFrame(curFrameIndex);
		break;
	default:
		break;
	}

	curHitIconType = HIT_NONE;
	MakePaintBuffer();
}

void PlayerSliderWidget::mouseMoveEvent(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);
	if (isBusy)
	{
		return;
	}

	QRect rc = this->rect();
	QPoint pt = event->pos();
	uint64_t frameIndex = curFrameIndex;
	if (event->pos().x() - playerRangeIconWidth < 0)
	{
		frameIndex = 0;
	}
	else
	{
		frameIndex = (event->pos().x() - playerRangeIconWidth) / (rc.width() - 2.f*playerRangeIconWidth)*totalFrameCnt;
	}

	//边界限制
	if (frameIndex<0)
	{
		frameIndex = 0;
	}
	else if (frameIndex >= totalFrameCnt)
	{
		frameIndex = totalFrameCnt - 1;
	}

	switch (curHitIconType)
	{
	case HIT_ICON_PLAYER_RANGE_LEFT:
		if (frameIndex >= 0 && frameIndex < validPlayerRangeEndFrameIndex)
		{
			validPlayerRangeBeginFrameIndex = frameIndex;
			validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;

			curFrameIndex = validPlayerRangeBeginFrameIndex;

			if (triggerFrameIndex >= validPlayerRangeBeginFrameIndex && triggerFrameIndex <= validPlayerRangeEndFrameIndex)
			{
				isTriggerFrameActice = true;
			}
			else
			{
				isTriggerFrameActice = false;
			}
		}
		break;
	case HIT_ICON_PLAYER_RANGE_RIGHT:
		if (frameIndex > validPlayerRangeBeginFrameIndex && frameIndex <= (totalFrameCnt-1))
		{
			validPlayerRangeEndFrameIndex = frameIndex;
			validTotalFrameCnt = validPlayerRangeEndFrameIndex - validPlayerRangeBeginFrameIndex + 1;

				curFrameIndex = validPlayerRangeEndFrameIndex;

			if (triggerFrameIndex >= validPlayerRangeBeginFrameIndex && triggerFrameIndex <= validPlayerRangeEndFrameIndex)
			{
				isTriggerFrameActice = true;
			}
			else
			{
				isTriggerFrameActice = false;
			}
		}
		break;
	case HIT_ICON_PLAYER_POS:
		if (frameIndex >= validPlayerRangeBeginFrameIndex && frameIndex <= validPlayerRangeEndFrameIndex)
		{
			curFrameIndex = frameIndex;
			//MakeaintBuffer();
			/*qDebug() << "seek frame frameIndex: " << frameIndex;*/
			//emit sigSeekFrame(frameIndex);
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
}

PlayerSliderWidget::HitIconType PlayerSliderWidget::HitIconDetect(const QPoint& mousePos) const
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
	else if (isTriggerFrameExists && isTriggerFrameActice && touchTriggerFrameRect.contains(mousePos))
	{
		type = HIT_ICON_TRIGGER_FRAME;
	}

	return type;
}

void PlayerSliderWidget::refreshTouchRect()
{
	//左播放范围按钮热区
	touch_slider_left_img_rect = slider_left_img_rect;
	touch_slider_left_img_rect.setWidth(slider_left_img_rect.width() * 2);
	touch_slider_left_img_rect.moveLeft(slider_left_img_rect.left()-slider_left_img_rect.width()*0.5);
	touch_slider_left_img_rect.setHeight(slider_left_img_rect.height() * 2);
	touch_slider_left_img_rect.moveBottom(slider_left_img_rect.bottom() + slider_left_img_rect.height()*0.5);

	//右播放范围按钮热区
	touch_slider_right_img_rect = slider_right_img_rect;
	touch_slider_right_img_rect.setWidth(slider_right_img_rect.width() * 2);
	touch_slider_right_img_rect.moveLeft(slider_right_img_rect.left() - slider_right_img_rect.width()*0.5);
	touch_slider_right_img_rect.setHeight(slider_right_img_rect.height() * 2);
	touch_slider_right_img_rect.moveBottom(slider_right_img_rect.bottom() + slider_right_img_rect.height()*0.5);

	//播放位置按钮热区
	touchPlayerPosIconRect = playerPosIconRect;
	touchPlayerPosIconRect.setWidth(playerPosIconRect.width() * 2);
	touchPlayerPosIconRect.moveLeft(playerPosIconRect.left() - playerPosIconRect.width()*0.5);
	touchPlayerPosIconRect.setHeight(playerPosIconRect.height() * 2);
	touchPlayerPosIconRect.moveBottom(playerPosIconRect.bottom() + playerPosIconRect.height()*0.5);

	//触发帧按钮热区
	touchTriggerFrameRect = triggerFrameRect;
	touchTriggerFrameRect.setWidth(triggerFrameRect.width() * 2);
	touchTriggerFrameRect.moveLeft(triggerFrameRect.left() - triggerFrameRect.width()*0.5);
	touchTriggerFrameRect.setHeight(triggerFrameRect.height() * 2);
	touchTriggerFrameRect.moveBottom(triggerFrameRect.bottom() + triggerFrameRect.height()*0.5);
}

void PlayerSliderWidget::SetImageBackgroundColor(const QPixmap& src, QImage& dst, const QColor& oldBackground, const QColor& newBackgroundColor) const
{
	dst = src.toImage();

	for (int i = 0; i < dst.height(); i++)
	{
		for (int j = 0; j < dst.width(); j++)
		{
			if (dst.pixelColor(QPoint(j, i)) == oldBackground)
			{
				dst.setPixelColor(QPoint(j, i), newBackgroundColor);
			}
		}
	}
}

void PlayerSliderWidget::MakePaintBuffer()
{
	paintBuffer = paintBuffer_backup;
	
	QPainter painter(&paintBuffer);
	painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);//文字和图形抗锯齿

	QRect rc = this->rect();

	QFont font = this->font();
	font.setPointSize(font.pointSize() <= 1 ? font.pointSize() : font.pointSize() - 1);
	painter.setFont(font);

	//绘制播放范围信息
	QString playerRangeText = tr("begin:%1,end:%2,total:%3").arg(0 == totalFrameCnt ? 0 : validPlayerRangeBeginFrameIndex + 1).arg(0 == totalFrameCnt ? 0 : validPlayerRangeEndFrameIndex + 1).arg(validTotalFrameCnt);
	int playerRangeTextHeight = rc.height()*playerRangeTextStretch;
	QRectF playerRangeTextRect(QPointF(), QSizeF(rc.width(), playerRangeTextHeight));
	painter.drawText(playerRangeTextRect, Qt::AlignCenter, playerRangeText);
	
	//绘制左播放范围图标
	float validPlayerRangeBeginPos = (rc.width() - 2.f* playerRangeIconWidth)*validPlayerRangeBeginFrameIndex / (totalFrameCnt - 1) + playerRangeIconWidth;
	slider_left_img_rect = QRectF(QPointF(), slider_left_img.size());
	float playerRangeIconBottom = rc.height()*(playerRangeTextStretch + playerRangeIconStretch);
	slider_left_img_rect.moveBottomRight(QPointF(validPlayerRangeBeginPos, playerRangeIconBottom));
	if (enabled)
	{
		if (curHitIconType == HIT_ICON_PLAYER_RANGE_LEFT)
		{
			if (slider_left_img_pressed.isNull() || slider_left_img_pressed.size() != slider_left_img.size())
			{
				SetImageBackgroundColor(slider_left_img, slider_left_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
			}
			painter.drawImage(slider_left_img_rect.topLeft(), slider_left_img_pressed);//按下样式
		}
		else
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
	if (totalFrameCnt != 0)
	{
		validPlayerRangeEndPos = (rc.width() - 2.f* playerRangeIconWidth)*validPlayerRangeEndFrameIndex / (totalFrameCnt - 1) + playerRangeIconWidth;
	}
	slider_right_img_rect = QRectF(QPointF(), slider_right_img.size());
	slider_right_img_rect.moveBottomLeft(QPointF(validPlayerRangeEndPos, playerRangeIconBottom));
	if (enabled)
	{
		if (curHitIconType == HIT_ICON_PLAYER_RANGE_RIGHT)
		{
			if (slider_right_img_pressed.isNull() || slider_right_img_pressed.size() != slider_right_img.size())
			{
				SetImageBackgroundColor(slider_right_img, slider_right_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
			}
			painter.drawImage(slider_right_img_rect.topLeft(), slider_right_img_pressed);//按下样式
		}
		else
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
	QPointF validPlayerRangeLineTopLeft(validPlayerRangeBeginPos, playerRangeTextRect.bottom());
	QPointF validPlayerRangeLineTopRight(validPlayerRangeEndPos, playerRangeTextRect.bottom());
	painter.drawLine(validPlayerRangeLineTopLeft, validPlayerRangeLineTopRight);

	//绘制播放范围左直线
	QPointF validPlayerRangeLineBottomLeft(validPlayerRangeBeginPos, playerRangeIconBottom);
	painter.drawLine(validPlayerRangeLineTopLeft, validPlayerRangeLineBottomLeft);

	//绘制播放范围右直线
	QPointF validPlayerRangeLineBottomRight(validPlayerRangeEndPos, playerRangeIconBottom);
	painter.drawLine(validPlayerRangeLineTopRight, validPlayerRangeLineBottomRight);

	//绘制进度条
	QRectF sliderRect(playerRangeIconWidth, playerRangeIconBottom, rc.width() - 2.f*playerRangeIconWidth, rc.height()*playerSliderStretch);
	painter.drawRect(sliderRect);
	QRectF validSliderRect(QPointF(validPlayerRangeBeginPos, sliderRect.top()), QPointF(validPlayerRangeEndPos, sliderRect.bottom()));
	if (enabled)
	{
		painter.fillRect(validSliderRect, iconBackgroundColor);
	}
	else
	{
		painter.fillRect(validSliderRect, Qt::lightGray);
	}
	
	//绘制播放位置图标
	//qDebug() << "pain current frameIndex: " << curFrameIndex;
	float curPlayerPos = (rc.width() - 2.f* playerRangeIconWidth)*curFrameIndex / (totalFrameCnt - 1) + playerRangeIconWidth;
	playerPosIconRect = QRectF(QPointF(), slider_pos_img.size());
	playerPosIconRect.moveCenter(QPointF(curPlayerPos, sliderRect.bottom()));
	playerPosIconRect.moveTop(sliderRect.bottom());
	if (enabled)
	{
		if (curHitIconType == HIT_ICON_PLAYER_POS)
		{
			if (slider_pos_img_pressed.isNull() || slider_pos_img_pressed.size() != slider_pos_img.size())
			{
				SetImageBackgroundColor(slider_pos_img, slider_pos_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
			}
			painter.drawImage(playerPosIconRect.topLeft(), slider_pos_img_pressed);//按下样式
		}
		else
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
	
	//绘制触发帧图标
	if (isTriggerFrameExists)
	{
		triggerFrameRect = QRectF(QPointF(), trigger_frame_img.size());
		float triggerPos = (rc.width() - 2.f* playerRangeIconWidth)*triggerFrameIndex / (totalFrameCnt - 1) + playerRangeIconWidth;
		triggerFrameRect.moveCenter(QPointF(triggerPos, playerPosIconRect.bottom()));
		triggerFrameRect.moveTop(playerPosIconRect.bottom());
		if (enabled && isTriggerFrameActice)
		{
			if (curHitIconType == HIT_ICON_TRIGGER_FRAME)
			{
				if (trigger_frame_img_pressed.isNull() || trigger_frame_img_pressed.size() != trigger_frame_img.size())
				{
					SetImageBackgroundColor(trigger_frame_img, trigger_frame_img_pressed, iconBackgroundColor, iconPressedBackgroundColor);
				}
				painter.drawImage(triggerFrameRect.topLeft(), trigger_frame_img_pressed);//按下样式
			}
			else
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
	}

	refreshTouchRect();
	update();
}