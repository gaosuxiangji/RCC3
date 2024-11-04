#include "bglinemeasureitem.h"
#include <QStaticText>

BGLineMeasureItem::BGLineMeasureItem(QGraphicsLineItem* parent, PointInfo p)
	: QGraphicsLineItem(parent)
	, m_item(p)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void BGLineMeasureItem::setFeatureRectVisible(bool bVisible)
{
	m_feature_visible = bVisible;
}

PointInfo BGLineMeasureItem::getItemInfo()
{
	return m_item;
}

void BGLineMeasureItem::updatePointInfo(const PointInfo& ptInfo)
{
	m_item = ptInfo;
}

void BGLineMeasureItem::setLineVisible(bool bVisible)
{
	m_line_visible = bVisible;
}

QRectF BGLineMeasureItem::boundingRect() const
{
	return QGraphicsLineItem::boundingRect();
}

void BGLineMeasureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (m_line_visible) {
		QPen pen;
		pen.setWidth(2);
		setLine(QLineF(m_item.start_point, m_item.end_point));
		QBrush brush;
		painter->save();
		pen.setColor(Qt::green);
		painter->setPen(pen);

		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::green);
		painter->setBrush(brush);
		painter->drawEllipse(m_item.start_point, 4, 4);

		painter->drawEllipse(m_item.end_point, 4, 4);
		painter->restore();

		painter->save();
		pen.setColor(Qt::red);
		painter->setPen(pen);
		painter->drawLine(m_item.start_point, m_item.end_point);
		painter->restore();

		painter->save();
		QStaticText strStaticTextName;
		pen.setColor(Qt::red);
		painter->setPen(pen);
		QFont ft;
		ft.setFamily("宋体");
		ft.setPixelSize(22);
		painter->setFont(ft);
		strStaticTextName.setText(m_item.strName);
		painter->drawStaticText(m_item.start_point, strStaticTextName);

		QStaticText strStaticTextValue;
		strStaticTextValue.setText((QString("%1").arg(QString::number(m_item.qrLength, 'f', 3))));
		painter->drawStaticText(m_item.end_point, strStaticTextValue);
		painter->restore();

		if (m_feature_visible) {
			painter->save();
			pen.setColor(Qt::red);
			painter->setPen(pen);

			brush.setColor(Qt::transparent);
			painter->setBrush(brush);

			painter->drawRect(getStartRect());
			painter->drawRect(getEndRect());
			painter->restore();
		}
	}
}


void BGLineMeasureItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_feature_visible) {
		if (event->button() == Qt::LeftButton)
		{
			m_startPos = event->pos();//鼠标左击时，获取当前鼠标在图片中的坐标，
			if (getStartRect().contains(m_startPos))
			{
				setCursor(Qt::PointingHandCursor);
				m_drag_type = DragType::Line_Begin;
			}
			else if (getEndRect().contains(m_startPos))
			{
				setCursor(Qt::PointingHandCursor);
				m_drag_type = DragType::Line_End;
			}
		}
	}
}

void BGLineMeasureItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	const QPointF point = (event->pos() - m_startPos);

	switch (m_drag_type)
	{
	case DragType::Release:
		break;
	case DragType::Line_Begin:
	{
		m_item.start_point = event->pos();
		break;
	}
	case DragType::Line_End:
	{
		m_item.end_point = event->pos();
		break;
	}
	}
	m_startPos = event->pos();

}


void BGLineMeasureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	m_drag_type = DragType::Release;
	emit updateMeasureInfoSignal(m_item);
}


QPainterPath BGLineMeasureItem::shape() const
{
	QPainterPath rect;
	QPointF pt1 = QPointF(m_item.start_point.x() - 50, m_item.start_point.y() + 50);
	QPointF pt2 = QPointF(m_item.start_point.x() + 50, m_item.start_point.y() - 50);
	QPointF pt3 = QPointF(m_item.end_point.x() + 50, m_item.end_point.y() - 50);
	QPointF pt4 = QPointF(m_item.end_point.x() - 50, m_item.end_point.y() + 50);
	QPolygonF polygon;
	polygon.append(pt1);
	polygon.append(pt2);
	polygon.append(pt3);
	polygon.append(pt4);
	rect.addPolygon(polygon);
	return rect;
}

QRectF BGLineMeasureItem::getStartRect()
{
	return QRectF(m_item.start_point.x() - Width / 2.0, m_item.start_point.y() - Height / 2.0, Width, Height);
}


QRectF BGLineMeasureItem::getEndRect()
{
	return QRectF(m_item.end_point.x() - Width / 2.0, m_item.end_point.y() - Height / 2.0, Width, Height);
}
