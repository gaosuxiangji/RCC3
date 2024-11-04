#include "csmeasurecenterdistanceitem.h"
#include <QStaticText>
#include <QDebug>

CSMeasureCenterDistanceItem::CSMeasureCenterDistanceItem(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info)
	:CSMeasureBaseItem(parent, info)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void CSMeasureCenterDistanceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

		if (m_item.vctPoint.size() >= 6)
		{
			double dbRadius1 = -1;
			QPointF ptCenter1 = CMeasureLineManage::GetRadiusOfThreePoint(m_item.vctPoint[0] - m_ptOffset, m_item.vctPoint[1] - m_ptOffset, m_item.vctPoint[2] - m_ptOffset, dbRadius1);
			if (dbRadius1 > 0)
			{
				painter->drawEllipse(ptCenter1, dbRadius1, dbRadius1);
				painter->drawEllipse(ptCenter1, m_pointSize, m_pointSize);

				QPainterPath pPath1;
				double dbPathRadius = dbRadius1;
				if (dbPathRadius > 1) {
					dbPathRadius -= 1;
				}
				else if (dbPathRadius > 0.5) {
					dbPathRadius -= 0.5;
				}
				pPath1.addEllipse(ptCenter1, dbPathRadius, dbPathRadius);
				QPainterPath pPath2;
				pPath2.addEllipse(ptCenter1, dbPathRadius + 2, dbPathRadius + 2);
				m_pPath = pPath2 - pPath1;
				QRectF rect;
				rect.setLeft(ptCenter1.x() - Width / 2);
				rect.setTop(ptCenter1.y() - Height / 2);
				rect.setWidth(Width);
				rect.setHeight(Height);
				m_pPath.addRect(rect);

				double dbRadius2 = -1;
				QPointF ptCenter2 = CMeasureLineManage::GetRadiusOfThreePoint(m_item.vctPoint[3] - m_ptOffset, m_item.vctPoint[4] - m_ptOffset, m_item.vctPoint[5] - m_ptOffset, dbRadius2);
				if (dbRadius2 > 0)
				{
					painter->drawEllipse(ptCenter2, dbRadius2, dbRadius2);
					painter->drawEllipse(ptCenter2, 4, 4);
					painter->drawLine(ptCenter1, ptCenter2);

					QPainterPath pPath1;
					double dbPathRadius = dbRadius2;
					if (dbPathRadius > 1) {
						dbPathRadius -= 1;
					}
					else if (dbPathRadius > 0.5) {
						dbPathRadius -= 0.5;
					}
					pPath1.addEllipse(ptCenter2, dbPathRadius, dbPathRadius);
					QPainterPath pPath2;
					pPath2.addEllipse(ptCenter2, dbPathRadius + 2, dbPathRadius + 2);
					m_pPath.addPath(pPath2 - pPath1);
					QVector<QPointF> vctPolyPoints;
					vctPolyPoints.append(ptCenter1);
					vctPolyPoints.append(ptCenter2);
					QPolygonF pLine(vctPolyPoints);
					QPainterPathStroker stroker;
					stroker.setWidth(2);
					QPainterPath linePath;
					linePath.addPolygon(pLine);
					m_pPath.addPath(stroker.createStroke(linePath));
					QRectF rect;
					rect.setLeft(ptCenter2.x() - Width / 2);
					rect.setTop(ptCenter2.y() - Height / 2);
					rect.setWidth(Width);
					rect.setHeight(Height);
					m_pPath.addRect(rect);
				}
			}
		}
		painter->restore();
	}
}

void CSMeasureCenterDistanceItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_nSelected_Index = -1;
	if (m_modetype == CMeasureLineManage::MMT_Modify) {
		if (event->button() == Qt::LeftButton)
		{
			QPointF m_startPos = event->pos();//鼠标左击时，获取当前鼠标在图片中的坐标，

			for (int i = 0; i < m_item.vctPoint.size(); ++i) {
				if (getFeaturePointRect(m_item.vctPoint[i] - m_ptOffset).contains(m_startPos)) {
					setCursor(Qt::PointingHandCursor);
					m_nSelected_Index = i;
				}
			}
		}
	}
	if (event->button() == Qt::LeftButton) {
		emit signalMeasureItemClicked(m_item.nIndex);
	}
}

void CSMeasureCenterDistanceItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
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

void CSMeasureCenterDistanceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

QRectF CSMeasureCenterDistanceItem::boundingRect() const
{
	QRectF rc = QGraphicsLineItem::boundingRect();
	return rc;
}

QRectF CSMeasureCenterDistanceItem::getFeaturePointRect(const QPointF& featurePt)
{
	return QRectF(featurePt.x() - Width / 2.0, featurePt.y() - Height / 2.0, Width, Height);
}

QPainterPath CSMeasureCenterDistanceItem::shape() const
{
	QPainterPath prect;
	prect.addPath(m_pPath);
	prect.addPath(CSMeasureBaseItem::shape());
	return prect;
}