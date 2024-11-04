#include "thumbnaillabel.h"

#include <QPainter>
#include <QFontMetrics>
#include <QFont>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStaticText>

#include "RMAGlobalFunc.h"

ThumbnailLabel::ThumbnailLabel(QWidget *parent)
    : QLabel(parent)
{
    setMouseTracking(true);
}

ThumbnailLabel::ThumbnailLabel(const RMAImage &img, QWidget *parent)
    : QLabel(parent)
{
    setMouseTracking(true);
    setThumbnailImage(img);
}

void ThumbnailLabel::setThumbnailImage(const RMAImage &img)
{
    thumb_image_ = img;
    MakePaintBuffer();
}

RMAImage ThumbnailLabel::getThumbnailImage() const
{
    return thumb_image_;
}

void ThumbnailLabel::clearThumbnailImage()
{
    thumb_image_ = RMAImage();
    MakePaintBuffer();
}

void ThumbnailLabel::setSelected(bool bselected)
{
    if(bselected == bselected_ || !bused_)
        return;
    if(bselected)
    {
		emit selected(thumb_image_);
    }
	bselected_ = bselected;
	MakePaintBuffer();
}

bool ThumbnailLabel::isSelected() const
{
    return bselected_;
}

void ThumbnailLabel::setLabelNo(uint no)
{
    label_number_ = no;
}

uint ThumbnailLabel::getLabelNo() const
{
    return label_number_;
}

void ThumbnailLabel::setUsed(bool bused)
{
	bused_ = bused;
	MakePaintBuffer();
}

bool ThumbnailLabel::isUsed() const
{
	return bused_;
}

void ThumbnailLabel::setLoadStatus(bool bfinished)
{
	bload_finished_ = bfinished;
	if (thumb_image_.Empty())
	{
		MakePaintBuffer();
	}
}

bool ThumbnailLabel::isLoaded() const
{
	return bload_finished_;
}

void ThumbnailLabel::mouseReleaseEvent(QMouseEvent *event)
{
    Q_ASSERT(event);

    QLabel::mouseReleaseEvent(event);

	if (bselected_)
		return;
	setSelected(true);
}

void ThumbnailLabel::paintEvent(QPaintEvent *event)
{
    Q_ASSERT(event);


    QPainter painter(this);

    painter.drawPixmap(0, 0, width(), height(), paint_buffer_);

	QLabel::paintEvent(event);

}

void ThumbnailLabel::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(event);
    MakePaintBuffer();
	QLabel::resizeEvent(event);
}

void ThumbnailLabel::MakePaintBuffer()
{
	paint_buffer_ = QPixmap(size());
    QPainter painter(&paint_buffer_);
    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);//文字和图形抗锯齿
 
	if(!bused_)
    {
        paint_buffer_.fill(Qt::white);
    }
    else
    {
		QRect buffer_rc(paint_buffer_.rect());
		int flags = Qt::TextWordWrap;
		QFontMetrics metrics = painter.fontMetrics();
		if (thumb_image_.Empty() || PIXEL_FMT_YUV == thumb_image_.GetPixelFormat())
		{
			paint_buffer_.fill(Qt::white);

			if (bload_finished_)//只有加载完成后图像仍为空才叠加加载失败字样
			{
				painter.setPen(kBorderColor);
				QString text = QString(QObject::tr("Failed to load"));
				QRect bounding_rect = metrics.boundingRect(buffer_rc, flags, text);
				bounding_rect.moveTo(QPoint(buffer_rc.right() - bounding_rect.width() - kFrameNoTextPadding, buffer_rc.y() + kFrameNoTextPadding));
				if (buffer_rc.contains(bounding_rect))
				{
					painter.drawText(bounding_rect, flags, text);
				}
			}
		}
		else
		{
			paint_buffer_.fill(Qt::black);

			//绘制图像
			QImage thumb_img;
			RMAIMAGE2QIMAGE(thumb_image_, thumb_img);
			thumb_img = thumb_img.rgbSwapped().copy();

			QPixmap thumb_pix = QPixmap::fromImage(thumb_img);
			QSize paint_buffer_size = paint_buffer_.size();
			thumb_pix = thumb_pix.scaled(paint_buffer_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QRect thumb_rc(0, 0, thumb_pix.width(), thumb_pix.height());
			thumb_rc.moveCenter(paint_buffer_.rect().center());
			painter.drawPixmap(thumb_rc, thumb_pix);

			//绘制文字
			int freme_idx = thumb_image_.GetAbsoluteFrameIndex();
			QString text = QString("%1: %2").arg(QObject::tr("Frame No")).arg(freme_idx);

			QRect bounding_rect = metrics.boundingRect(buffer_rc, flags, text);
			bounding_rect.moveTo(QPoint(buffer_rc.right() - bounding_rect.width() - kFrameNoTextPadding, buffer_rc.y() + kFrameNoTextPadding));
			if (buffer_rc.contains(bounding_rect))
			{
				QString frame_no = QString(QObject::tr("<font color=#FF0000>%1</font>")).arg(freme_idx);
				text = QString("<font color=#FFFFFF>%1: </font>%2").arg(QObject::tr("Frame No")).arg(frame_no);
				painter.drawStaticText(bounding_rect.topLeft(), text);
			}
		}

		if (bselected_)
		{
			QPen pen(kBorderColor);
			pen.setWidth(kBorder);
			painter.setPen(pen);
			painter.drawRect(buffer_rc);
		}
    }
    update();
}

