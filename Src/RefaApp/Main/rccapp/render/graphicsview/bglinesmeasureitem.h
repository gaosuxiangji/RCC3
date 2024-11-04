#pragma once
#include <QGraphicsPolygonItem>
#include <QObject>
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

typedef struct MultiPointInfo
{
	QPolygonF multi_pt_feature_vec{};
	uint32_t multi_point_index{ 0 };   //Ë÷Òý
	QString strName;    // Ãû³Æ
	qreal qrLength;    // ¾àÀë
	uint32_t measure_type;
}MultiPointInfo;

class BGLinesMeasureItem : public QObject, public QGraphicsPolygonItem
{
	Q_OBJECT
public:
	enum class DragType
	{
		Release,
		Feature_Pt,
	};

	BGLinesMeasureItem(QGraphicsLineItem* parent, MultiPointInfo p);
	~BGLinesMeasureItem() {}
	void setFeatureRectVisible(bool bVisible);
	MultiPointInfo getItemInfo();
	void updatePointInfo(const MultiPointInfo& ptInfo);
	void setLineVisible(bool bVisible);
signals:
	void updateMultiMeasureInfoSignal(const MultiPointInfo& ptInfo);
protected:
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	QPainterPath shape() const override;
private:
	QRectF getFeaturePointRect(const QPointF& featurePt);
private:
	MultiPointInfo m_multi_pt_info{};
	bool m_line_visible{ true };
	bool m_feature_visible{ false };
	const double Width = 30.0;
	const double Height = 30.0;
	DragType m_drag_type{ DragType::Release };
	QPointF m_startPos{};
	int m_feature_pt_index{ -1 };
};
