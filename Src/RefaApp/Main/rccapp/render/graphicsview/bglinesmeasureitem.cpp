#include "bglinesmeasureitem.h"
#include <QStaticText>

BGLinesMeasureItem::BGLinesMeasureItem(QGraphicsLineItem* parent, MultiPointInfo p)
	:QGraphicsPolygonItem(parent)
	,m_multi_pt_info(p)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void BGLinesMeasureItem::setFeatureRectVisible(bool bVisible)
{
	m_feature_visible = bVisible;
}

MultiPointInfo BGLinesMeasureItem::getItemInfo()
{
	return m_multi_pt_info;
}

void BGLinesMeasureItem::updatePointInfo(const MultiPointInfo& ptInfo)
{
	m_multi_pt_info = ptInfo;
}

void BGLinesMeasureItem::setLineVisible(bool bVisible)
{
	m_line_visible = bVisible;
}

QRectF BGLinesMeasureItem::boundingRect() const
{
	return QGraphicsPolygonItem::boundingRect();
}

void BGLinesMeasureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_line_visible) {
		QPen pen;
		pen.setWidth(2);

		painter->save();
		pen.setColor(Qt::red);
		painter->setPen(pen);
		painter->drawPolyline(m_multi_pt_info.multi_pt_feature_vec);
		painter->restore();

		setPolygon(m_multi_pt_info.multi_pt_feature_vec);
		QBrush brush;
		painter->save();
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(Qt::green);
		painter->setBrush(brush);
		for (auto feature_pt : m_multi_pt_info.multi_pt_feature_vec) {
			painter->drawEllipse(feature_pt, 4, 4);
		}
		painter->restore();

		painter->save();
		QStaticText strStaticTextName;
		pen.setColor(Qt::red);
		painter->setPen(pen);
		QFont ft;
		ft.setFamily("宋体");
		ft.setPixelSize(22);
		painter->setFont(ft);
		strStaticTextName.setText(m_multi_pt_info.strName);
		painter->drawStaticText(m_multi_pt_info.multi_pt_feature_vec[0], strStaticTextName);

		QStaticText strStaticTextValue;
		strStaticTextValue.setText(QString("%1").arg(QString::number(m_multi_pt_info.qrLength, 'f', 3)));
		painter->drawStaticText(m_multi_pt_info.multi_pt_feature_vec[m_multi_pt_info.multi_pt_feature_vec.size()-1], strStaticTextValue);
		painter->restore();

		if (m_feature_visible) {
			painter->save();
			pen.setColor(Qt::red);
			painter->setPen(pen);

			brush.setColor(Qt::transparent);
			painter->setBrush(brush);

			for (auto featurPt : m_multi_pt_info.multi_pt_feature_vec) {
				painter->drawRect(getFeaturePointRect(featurPt));
			}
			painter->restore();
		}
	}
}

void BGLinesMeasureItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_feature_pt_index = -1;
	if (m_feature_visible) {
		if (event->button() == Qt::LeftButton)
		{
			m_startPos = event->pos();//鼠标左击时，获取当前鼠标在图片中的坐标，

			for (int i = 0; i < m_multi_pt_info.multi_pt_feature_vec.size(); ++i) {
				if (getFeaturePointRect(m_multi_pt_info.multi_pt_feature_vec[i]).contains(m_startPos)) {
					setCursor(Qt::PointingHandCursor);
					m_drag_type = DragType::Feature_Pt;
					m_feature_pt_index = i;
				}
			}
		}
	}
}

void BGLinesMeasureItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	const QPointF point = (event->pos() - m_startPos);

	switch (m_drag_type)
	{
	case DragType::Release:
		break;
	case DragType::Feature_Pt:
	{
		if (-1 != m_feature_pt_index) {
			m_multi_pt_info.multi_pt_feature_vec[m_feature_pt_index] = event->pos();
		}
		break;
	}
	}
	m_startPos = event->pos();
}

void BGLinesMeasureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	m_drag_type = DragType::Release;
	emit updateMultiMeasureInfoSignal(m_multi_pt_info);
}

QPainterPath BGLinesMeasureItem::shape() const
{
	QPainterPath rect;
	rect.addRect(m_multi_pt_info.multi_pt_feature_vec.boundingRect());
	return rect;
}

QRectF BGLinesMeasureItem::getFeaturePointRect(const QPointF& featurePt)
{
	return QRectF(featurePt.x() - Width / 2.0, featurePt.y() - Height / 2.0, Width, Height);
}
