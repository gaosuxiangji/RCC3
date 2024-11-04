#include "csmeasureareapolygonitem.h"
#include <QStaticText>
#include <QDebug>

CSMeasureAreaPolygonItem::CSMeasureAreaPolygonItem(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info)
	:CSMeasureBaseItem(parent, info)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void CSMeasureAreaPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_modetype == CMeasureLineManage::MMT_Show || m_modetype == CMeasureLineManage::MMT_Add || m_modetype == CMeasureLineManage::MMT_Modify) {
		QPen pen;
		pen.setWidth(2);
		pen.setCosmetic(true);
		painter->setClipRect(m_ImageRect);

		QVector<QPoint> vctPoints;
		QBrush brush;
		painter->save();
		brush.setStyle(Qt::SolidPattern);
		brush.setColor(m_pointColor);
		painter->setBrush(brush);
		pen.setColor(m_pointColor);
		painter->setPen(pen);
		for (auto feature_pt : m_item.vctPoint) {
			feature_pt -= m_ptOffset;
			painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
			vctPoints.append(feature_pt);
		}
		painter->restore();

		if (vctPoints.size() > 1)
		{
			painter->save();
			if (m_bSelected)
			{
				pen.setColor(m_Selectedcolor);
			}
			else
			{
				pen.setColor(m_lineColor);
			}
			painter->setPen(pen);
			brush.setColor(QColor(0, 255, 0, 64));
			painter->setBrush(brush);
			QPolygonF pPolyon(vctPoints);
			painter->drawPolyline(pPolyon);
			painter->drawConvexPolygon(pPolyon);
			painter->restore();
		}

		painter->save();
#if MEASURE_DRAW_TEXT_PAINT
		auto old_transform = painter->transform();
		painter->resetTransform();
		QStaticText strStaticTextName;
		pen.setColor(m_fontColor);
		painter->setPen(pen);
		QFont ft;
		ft.setFamily("宋体");
		ft.setPixelSize(22);
		painter->setFont(ft);
		strStaticTextName.setText(m_item.strName);
		auto pt = old_transform.map(m_item.vctPoint[0] - m_ptOffset);
		painter->drawStaticText(pt, strStaticTextName);
		painter->setTransform(old_transform);
#else
		if (m_pTextItem)
		{
			m_pTextItem->setPlainText(m_item.strName);
			m_pTextItem->setPos(m_item.vctPoint[0] - m_ptOffset);
		}
#endif

#if MEASURE_DRAW_VALUE
		QStaticText strStaticTextValue;
		strStaticTextValue.setText(QString("%1").arg(QString::number(m_item.qrLength, 'f', 3)));
		painter->drawStaticText(m_item.vctPoint[m_item.vctPoint.size() - 1] - m_ptOffset, strStaticTextValue);
#endif

		painter->restore();

		if (m_modetype == CMeasureLineManage::MMT_Modify) {
			painter->save();
			pen.setColor(m_editpointColor);
			painter->setPen(pen);

			brush.setColor(Qt::transparent);
			painter->setBrush(brush);

			for (auto featurPt : m_item.vctPoint) {
				featurPt -= m_ptOffset;
				painter->drawRect(getFeaturePointRect(featurPt));
			}
			painter->restore();
		}
	}
}

void CSMeasureAreaPolygonItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_nSelected_Index = -1;
	if (m_modetype == CMeasureLineManage::MMT_Modify) {
		if (event->button() == Qt::LeftButton)
		{
			QPointF m_startPos = event->pos();//鼠标左击时，获取当前鼠标在图片中的坐标，

			for (int i = 0; i < m_item.vctPoint.size(); ++i) {
				if (getFeaturePointRect(m_item.vctPoint[i]-m_ptOffset).contains(m_startPos)) {
					setCursor(Qt::PointingHandCursor);
					m_nSelected_Index = i;
					break;
				}
			}
		}
	}
	if (event->button() == Qt::LeftButton) {
		emit signalMeasureItemClicked(m_item.nIndex);
	}
}

void CSMeasureAreaPolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_modetype == CMeasureLineManage::MMT_Modify)
	{
		if (m_nSelected_Index >= 0)
		{
			QPointF curPT = event->pos();
			if (m_ImageRect.contains(curPT))
			{
				m_item.vctPoint[m_nSelected_Index] = curPT.toPoint() + m_ptOffset;
			}
		}
	}
}

void CSMeasureAreaPolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_modetype == CMeasureLineManage::MMT_Modify)
	{
		if (m_nSelected_Index >= 0)
		{
			emit signalMeasureItemModified(m_item);
		}
	}
	m_nSelected_Index = -1;
}

QRectF CSMeasureAreaPolygonItem::boundingRect() const
{
	QRectF rc = QGraphicsLineItem::boundingRect();
	return rc;
}

QPainterPath CSMeasureAreaPolygonItem::shape() const
{
	QPainterPath prect;
	int nSize = m_item.vctPoint.size();
	if (nSize > 0){
		auto item = m_item.vctPoint[0];
		item -= m_ptOffset;
		QRectF rect;
		rect.setLeft(item.x() - Width / 2);
		rect.setTop(item.y() - Height / 2);
		rect.setWidth(Width);
		rect.setHeight(Height);
		prect.addRect(rect);
	}
	for (int i = 1; i < nSize; i++) {
		auto item = m_item.vctPoint[i];
		item -= m_ptOffset;
		QRectF rect;
		rect.setLeft(item.x() - Width / 2);
		rect.setTop(item.y() - Height / 2);
		rect.setWidth(Width);
		rect.setHeight(Height);
		prect.addRect(rect);
		QVector<QPointF> vctPolyPoints;
		vctPolyPoints.append(m_item.vctPoint[i - 1]);
		vctPolyPoints.append(m_item.vctPoint[i]);
		QPolygonF pLine(vctPolyPoints);
		QPainterPathStroker stroker;
		stroker.setWidth(2);
		QPainterPath linePath;
		linePath.addPolygon(pLine);
		prect.addPath(stroker.createStroke(linePath));
	}
	return prect;
}

QRectF CSMeasureAreaPolygonItem::getFeaturePointRect(const QPointF& featurePt)
{
	return QRectF(featurePt.x() - Width / 2.0, featurePt.y() - Height / 2.0, Width, Height);
}
