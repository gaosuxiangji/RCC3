#pragma once
#include <QGraphicsItem>
#include <mutex>
#include <QImage>
#include <QPainter>
#include <QObject>

class BImageItem : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
public:
	BImageItem();
	~BImageItem();
public:
	void updateImage(const QImage& img);
	QPointF getImgItemCenterPt() { return QPointF(boundingRect().width() / 2, boundingRect().height() / 2); }
private:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override;
private:
	std::mutex m_background_image_lock{};
	QImage m_background_image{};
	//QRect m_image_original_size{};
};