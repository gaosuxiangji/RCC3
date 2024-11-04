#include "bgimageitem.h"
#include <QGraphicsSceneMouseEvent>
#

BImageItem::BImageItem()
{
	setFlags(QGraphicsItem::ItemIsMovable);
}

BImageItem::~BImageItem()
{

}

void BImageItem::updateImage(const QImage& img)
{
	{
		std::lock_guard<std::mutex> lock(m_background_image_lock);
		m_background_image = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
	}
	update();
	//setPixmap(QPixmap::fromImage(img));
}

void BImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	std::lock_guard<std::mutex> lock(m_background_image_lock);
	QSize sz = m_background_image.size();
	QRectF imgRC(0, 0, sz.width(), sz.height());

	if (!m_background_image.isNull()) {
		painter->drawImage(QPoint(0, 0), m_background_image, imgRC);
	}
}

void BImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsItem::mouseMoveEvent(event);
}
