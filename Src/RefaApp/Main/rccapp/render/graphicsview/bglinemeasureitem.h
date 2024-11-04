#pragma once

#include <QObject>
#include <QAbstractGraphicsShapeItem>
#include <QPointF>
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QCursor>
#include <QKeyEvent>
#include <QList>
#include <QGraphicsLineItem>
#include <QObject>

typedef struct PointInfo
{
	QPointF start_point{};
	QPointF end_point{};
	uint32_t point_index{ 0 };    //单点索引
	QString strName;    // 名称
	qreal qrLength;    // 距离
	uint32_t measure_type;
}PointInfo;

class BGLineMeasureItem : public QObject, public QGraphicsLineItem
{
	Q_OBJECT

public:
	enum class DragType
	{
		Release,
		Line_Begin,
		Line_End,
	};

	enum PointType {
		Vertex,
	};

	BGLineMeasureItem(QGraphicsLineItem* parent, PointInfo p);
	~BGLineMeasureItem() {}

	void setFeatureRectVisible(bool bVisible);
	PointInfo getItemInfo();
	void updatePointInfo(const PointInfo& ptInfo);
	void setLineVisible(bool bVisible);
signals:
	void updateMeasureInfoSignal(const PointInfo& ptInfo);
protected:
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	QPainterPath shape() const override;
private:
	QRectF getStartRect();
	QRectF getEndRect();
private:
	PointInfo m_item{};
	bool m_feature_visible{ false };
	bool m_line_visible{ true };

	QPointF m_startPos{};
	DragType m_drag_type{ DragType::Release };
	const double Width = 50.0;
	const double Height = 50.0;
};
