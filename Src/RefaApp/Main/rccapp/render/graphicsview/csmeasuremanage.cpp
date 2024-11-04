#include "csmeasuremanage.h"
#include <QVector>
#include <QDebug>
#include <QMenu>
#include <QSpinBox>
#include <QWidgetAction>
#include <QPen>
#include <QPainter>
#include <QPolygonF>

CSMeasureItemManage::CSMeasureItemManage()
{
}

void CSMeasureItemManage::setMeasureItemVisible(bool visible)
{
}

void CSMeasureItemManage::setImageRect(QRectF rc)
{
	m_ImageRect = rc;
	for (auto item : m_mesure_item_list)
	{
		if (item)
		{
			item->setImageRect(rc);
		}
	}
}

void CSMeasureItemManage::setImageOffset(QPoint ptOffset)
{
	m_ptOffset = ptOffset;
	for (auto item : m_mesure_item_list)
	{
		if (item)
		{
			item->setImageOffset(ptOffset);
		}
	}
}

void CSMeasureItemManage::setSelectedItem(int nIndex)
{
	for (auto item : m_mesure_item_list)
	{
		if (item)
		{
			CMeasureLineManage::TMeasureLineInfo info = item->getItemInfo();
			if (info.nIndex == nIndex)
			{
				item->setSelectedStatus(true);
			}
			else
			{
				if (item->getSelectedStatus())
				{
					item->setSelectedStatus(false);
				}
			}
		}
	}
}

void CSMeasureItemManage::setCalibration(CMeasureLineManage::TMeasureModeType mode)
{
	m_modeCalibration = mode;
}

void CSMeasureItemManage::setCalibrationPoints(QList<QPoint> vctPoints)
{
	m_vctCalibration = vctPoints;
}

QList<QPoint> CSMeasureItemManage::getCalibrationPoints()
{
	return m_vctCalibration;
}

QRectF CSMeasureItemManage::boundingRect() const
{
	return m_ImageRect;
	//return QRectF{0, 0, 1920, 1080};
}

void CSMeasureItemManage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QPen pen;
	pen.setWidth(2);
	pen.setCosmetic(true);
	painter->setClipRect(m_ImageRect);
	switch (m_modeCalibration)
	{
	case CMeasureLineManage::MMT_Add:
	case CMeasureLineManage::MMT_Modify:
	case CMeasureLineManage::MMT_Show:
		if (m_vctCalibration.size() > 0) {
			painter->save();
			pen.setColor(Qt::green);
			painter->setPen(pen);
			QBrush brush;
			brush.setStyle(Qt::SolidPattern);
			brush.setColor(Qt::green);
			painter->setBrush(brush);
			QVector<QPointF> vctLinePoint;
			for (auto feature_pt : m_vctCalibration) {
				feature_pt -= m_ptOffset;
				painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
				vctLinePoint.append(feature_pt);
			}
			painter->restore();

			painter->save();
			pen.setColor(Qt::red);
			painter->setPen(pen);
			if (vctLinePoint.size() > 0)
			{
				if (vctLinePoint.size() < 2)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
				}
				QPolygonF pPolyon(vctLinePoint);
				painter->drawPolyline(pPolyon);
			}
			if (vctLinePoint.size() > 1)
			{
				for (int i = 0; i < 2; ++i)
				{
					QPointF ptsrc = vctLinePoint[i % 2];
					QPointF ptdst = vctLinePoint[(i + 1) % 2];
					QLineF line(ptsrc, ptdst);
					if (line.length() > 0.001)
					{
						double dbAngle = acos(line.dx() / line.length());
						if (line.dy() >= 0)
						{
							dbAngle = 2 * M_PI - dbAngle;
						}
						QPointF  destArrowP1 = ptdst + QPointF(sin(dbAngle - M_PI / 3)*kARROWSIZE, cos(dbAngle - M_PI / 3)*kARROWSIZE);
						QPointF  destArrowP2 = ptdst + QPointF(sin(dbAngle - M_PI + M_PI / 3)*kARROWSIZE, cos(dbAngle - M_PI + M_PI / 3)*kARROWSIZE);
						QPointF  destArrowP12 = (destArrowP1 + destArrowP2) / 2;
						QPointF  destArrowP3 = (destArrowP12 + ptdst) / 2;
						QPolygonF arrowHead;
						arrowHead << ptdst << destArrowP1 << destArrowP3 << destArrowP2;
						painter->drawPolygon(arrowHead);
					}
				}
			}
			painter->restore();
		}
		break;
	default:
		break;
	}
	if (m_measureMode == CMeasureLineManage::MMT_Add)
	{
		switch (m_measureType)
		{
		case CMeasureLineManage::MLT_TWO_POINT:
		case CMeasureLineManage::MLT_MORE_POINT:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					QPolygonF pPolyon(vctLinePoint);
					painter->drawPolyline(pPolyon);
				}
				painter->restore();
			}
		}
			break;
		case CMeasureLineManage::MLT_TWO_CALIBRATION:
			break;
		case CMeasureLineManage::MLT_ANGLE_3:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					QPolygonF pPolyon(vctLinePoint);
					painter->drawPolyline(pPolyon);
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_ANGLE_4:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					if (vctLinePoint.size() > 3)
					{
						painter->drawLine(vctLinePoint[0], vctLinePoint[1]);
						painter->drawLine(vctLinePoint[2], vctLinePoint[3]);
					}
					else if (vctLinePoint.size() > 1)
					{
						painter->drawLine(vctLinePoint[0], vctLinePoint[1]);
					}
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_RADIUS:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					if (vctLinePoint.size() == 3)
					{
						double dbRadius = -1;
						QPointF ptCenter = CMeasureLineManage::GetRadiusOfThreePoint(vctLinePoint[0].toPoint(), vctLinePoint[1].toPoint(), vctLinePoint[2].toPoint(), dbRadius);
						if (dbRadius > 0)
						{
							painter->drawEllipse(ptCenter, dbRadius, dbRadius);

							painter->drawEllipse(ptCenter, 4, 4);
							QPointF ptVerter = ptCenter;
							ptVerter.setX(ptCenter.x() + dbRadius);
							painter->drawLine(ptCenter, ptVerter);
						}
					}
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_DIAMETER:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					if (vctLinePoint.size() == 3)
					{
						double dbRadius = -1;
						QPointF ptCenter = CMeasureLineManage::GetRadiusOfThreePoint(vctLinePoint[0].toPoint(), vctLinePoint[1].toPoint(), vctLinePoint[2].toPoint(), dbRadius);
						if (dbRadius > 0)
						{
							painter->drawEllipse(ptCenter, dbRadius, dbRadius);

							painter->drawEllipse(ptCenter, 4, 4);
							QPointF ptVerter1 = ptCenter;
							QPointF ptVerter2 = ptCenter;
							ptVerter1.setX(ptCenter.x() - dbRadius);
							ptVerter2.setX(ptCenter.x() + dbRadius);
							painter->drawLine(ptVerter1, ptVerter2);
						}
					}
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_CENTER_DISTANCE:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					if (vctLinePoint.size() >= 3)
					{
						double dbRadius1 = -1;
						QPointF ptCenter1 = CMeasureLineManage::GetRadiusOfThreePoint(vctLinePoint[0].toPoint(), vctLinePoint[1].toPoint(), vctLinePoint[2].toPoint(), dbRadius1);
						if (dbRadius1 > 0)
						{
							painter->drawEllipse(ptCenter1, dbRadius1, dbRadius1);

							painter->drawEllipse(ptCenter1, 4, 4);
							if (vctLinePoint.size() >= 6)
							{
								double dbRadius2 = -1;
								QPointF ptCenter2 = CMeasureLineManage::GetRadiusOfThreePoint(vctLinePoint[3].toPoint(), vctLinePoint[4].toPoint(), vctLinePoint[5].toPoint(), dbRadius2);
								if (dbRadius2 > 0)
								{
									painter->drawEllipse(ptCenter2, dbRadius2, dbRadius2);
									painter->drawEllipse(ptCenter2, m_pointSize, m_pointSize);
									painter->drawLine(ptCenter1, ptCenter2);
								}
							}
						}
					}
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_DIMENSION:
		{
			if (!m_curPt.isNull()) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				painter->drawEllipse(m_curPt, m_pointSize, m_pointSize);
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_AREA_CENTER:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					if (vctLinePoint.size() >= 3)
					{
						double dbRadius = -1;
						QPointF ptCenter = CMeasureLineManage::GetRadiusOfThreePoint(vctLinePoint[0].toPoint(), vctLinePoint[1].toPoint(), vctLinePoint[2].toPoint(), dbRadius);
						if (dbRadius > 0)
						{
							brush.setColor(m_areaColor);
							painter->setBrush(brush);
							painter->drawEllipse(ptCenter, dbRadius, dbRadius);
						}
					}
				}
				painter->restore();
			}
			break;
		}
		case CMeasureLineManage::MLT_AREA_POLYGON:
		{
			if (m_vctAddPoint.size() > 0) {
				painter->save();
				pen.setColor(m_pointColor);
				painter->setPen(pen);
				QBrush brush;
				brush.setStyle(Qt::SolidPattern);
				brush.setColor(m_pointColor);
				painter->setBrush(brush);
				QVector<QPointF> vctLinePoint;
				for (auto feature_pt : m_vctAddPoint) {
					painter->drawEllipse(feature_pt, m_pointSize, m_pointSize);
					vctLinePoint.append(feature_pt);
				}
				painter->restore();

				painter->save();
				pen.setColor(m_lineColor);
				painter->setPen(pen);
				if (vctLinePoint.size() > 0)
				{
					if (!m_curPt.isNull())
					{
						vctLinePoint.append(m_curPt);
					}
					brush.setColor(m_areaColor);
					painter->setBrush(brush);
					QPolygonF pPolyon(vctLinePoint);
					painter->drawConvexPolygon(pPolyon);
				}
				painter->restore();
			}
		}
		default:
			break;
		}
	}
}

void CSMeasureItemManage::slotPressOnePt(const QPoint& pt)
{
	if (!m_ImageRect.contains(pt))
	{
		return;
	}
	switch (m_measureType)
	{
	case CMeasureLineManage::MLT_TWO_POINT:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 1)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
	}
		break;
	case CMeasureLineManage::MLT_MORE_POINT:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint[m_vctAddPoint.size() - 1] == pt)
		{
			break;
		}
		m_vctAddPoint.append(pt);
	}
		break;
	case CMeasureLineManage::MLT_TWO_CALIBRATION:
		if (m_modeCalibration == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Modify)
		{
			if (m_vctCalibration.size() < 2)
			{
				if (m_vctCalibration.size() > 0 && m_vctCalibration.contains(pt + m_ptOffset))
				{
					break;
				}
				m_vctCalibration.append(pt + m_ptOffset);
			}
			else
			{
				m_vctCalibration.clear();
				m_vctCalibration.append(pt + m_ptOffset);
			}
			if (m_vctCalibration.size() >= 2)
			{
				emit signalAddItemForPoint(m_vctCalibration);
			}
		}
		break;
	case CMeasureLineManage::MLT_ANGLE_3:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt + m_ptOffset))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 2)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
		break;
	}
	case CMeasureLineManage::MLT_ANGLE_4:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 3)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
		break;
	}
	case CMeasureLineManage::MLT_RADIUS:
	case CMeasureLineManage::MLT_DIAMETER:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 2)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
		break;
	}
	case CMeasureLineManage::MLT_CENTER_DISTANCE:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 5)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
		break;
	}
	case CMeasureLineManage::MLT_DIMENSION:
	{
		m_vctAddPoint.append(pt);
		emit signalAddItemForPoint(m_vctAddPoint);
		break;
	}
	case CMeasureLineManage::MLT_AREA_CENTER:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint.contains(pt))
		{
			break;
		}
		m_vctAddPoint.append(pt);
		if (m_vctAddPoint.size() > 2)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
		}
		break;
	}
	case CMeasureLineManage::MLT_AREA_POLYGON:
	{
		if (m_vctAddPoint.size() > 0 && m_vctAddPoint[m_vctAddPoint.size() - 1] == pt)
		{
			break;
		}
		m_vctAddPoint.append(pt);
	}
	break;
	default:
		break;
	}
}

void CSMeasureItemManage::slotMoveOnePt(const QPoint& pt)
{
	m_curPt = pt;
	if (m_modeCalibration == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Modify)
	{
		if (!m_curPt.isNull() && m_vctCalibration.size() == 1)
		{
			if (m_ImageRect.contains(m_curPt))
			{
				QList<QPoint> vctPoints;
				vctPoints.append(m_vctCalibration[0] - m_ptOffset);
				vctPoints.append(m_curPt - m_ptOffset);
				emit signalItemPointsMove(vctPoints);
			}
		}
	}
}

void CSMeasureItemManage::slotReleassMouse()
{
    qDebug() << "CSMeasureItemManage::slotReleassMouse: " << (int)m_measureType;
	switch (m_measureType)
	{
	case CMeasureLineManage::MLT_MORE_POINT:
	{
		if (m_vctAddPoint.size() > 1)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
			m_vctAddPoint.clear();
		}
	}
	break;
	case CMeasureLineManage::MLT_AREA_POLYGON:
	{
		if (m_vctAddPoint.size() > 2)
		{
			emit signalAddItemForPoint(m_vctAddPoint);
			m_vctAddPoint.clear();
		}
	}
	break;
	case CMeasureLineManage::MLT_TWO_CALIBRATION:
	{
		if (m_modeCalibration != CMeasureLineManage::MMT_Normal && m_modeCalibration != CMeasureLineManage::MMT_Show)
		{
			if (m_vctCalibration.size() < 2)
			{
				m_vctCalibration.clear();
			}
			emit signalAddItemForPoint(m_vctCalibration);
		}
		break;
	}
	default:
		break;
	}
}

void CSMeasureItemManage::slotMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo & ptInfo)
{
	emit signalMeasureItemModified(ptInfo);
}

void CSMeasureItemManage::slotMeasureItemClicked(const int nIndex)
{
	emit signalMeasureItemClicked(nIndex);
}

void CSMeasureItemManage::updateMeasureModeType(CMeasureLineManage::TMeasureModeType mode)
{
	m_measureMode = mode;
	if (m_measureMode == CMeasureLineManage::MMT_Show)
	{
		m_vctAddPoint.clear();
		m_curPt = QPoint{};
	}
	for (auto item : m_mesure_item_list)
	{
		if (item)
		{
			item->setMeasureModeType(mode);
		}
	}
}

void CSMeasureItemManage::updateMeasureType(CMeasureLineManage::TMeasureLineType mode)
{
	m_measureType = mode;
	m_vctAddPoint.clear();
	m_curPt = QPoint{};
}

void CSMeasureItemManage::initMeasureItem(QList<CMeasureLineManage::TMeasureLineInfo> vctInfo)
{
	for (auto temp : vctInfo)
	{
		addMeasureItem(temp);
	}
}

void CSMeasureItemManage::addMeasureItem(CMeasureLineManage::TMeasureLineInfo info, bool bSelected)
{
	switch (info.nType)
	{
	case CMeasureLineManage::MLT_TWO_POINT:
	case CMeasureLineManage::MLT_MORE_POINT:
	{
		QSharedPointer<CSMeasureLineItem> item = QSharedPointer<CSMeasureLineItem>(new CSMeasureLineItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_TWO_CALIBRATION:
	{
		break;
	}
	case CMeasureLineManage::MLT_ANGLE_3:
	{
		QSharedPointer<CSMeasureAngle3Item> item = QSharedPointer<CSMeasureAngle3Item>(new CSMeasureAngle3Item(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_ANGLE_4:
	{
		QSharedPointer<CSMeasureAngle4Item> item = QSharedPointer<CSMeasureAngle4Item>(new CSMeasureAngle4Item(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_RADIUS:
	{
		QSharedPointer<CSMeasureRadiusItem> item = QSharedPointer<CSMeasureRadiusItem>(new CSMeasureRadiusItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_DIAMETER:
	{
		QSharedPointer<CSMeasureDiameterItem> item = QSharedPointer<CSMeasureDiameterItem>(new CSMeasureDiameterItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_CENTER_DISTANCE:
	{
		QSharedPointer<CSMeasureCenterDistanceItem> item = QSharedPointer<CSMeasureCenterDistanceItem>(new CSMeasureCenterDistanceItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_DIMENSION:
	{
		QSharedPointer<CSMeasureDimensionItem> item = QSharedPointer<CSMeasureDimensionItem>(new CSMeasureDimensionItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_AREA_CENTER:
	{
		QSharedPointer<CSMeasureAreaRadiusItem> item = QSharedPointer<CSMeasureAreaRadiusItem>(new CSMeasureAreaRadiusItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	case CMeasureLineManage::MLT_AREA_POLYGON:
	{
		QSharedPointer<CSMeasureAreaPolygonItem> item = QSharedPointer<CSMeasureAreaPolygonItem>(new CSMeasureAreaPolygonItem(this, info));
		item->setImageRect(m_ImageRect);
		item->setImageOffset(m_ptOffset);
		if (bSelected)
		{
			item->setSelectedStatus(true);
		}
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemModified, this, &CSMeasureItemManage::slotMeasureItemModified);
		connect(item.data(), &CSMeasureBaseItem::signalMeasureItemClicked, this, &CSMeasureItemManage::slotMeasureItemClicked);
		m_mesure_item_list.append(item);
		break;
	}
	default:
		break;
	}
}

void CSMeasureItemManage::updateMeasureItem(CMeasureLineManage::TMeasureLineInfo info)
{
	for (auto item : m_mesure_item_list)
	{
		if (item->getMeasureIndex() == info.nIndex)
		{
			item->updateMeasureInfo(info);
			break;
		}
	}
}

void CSMeasureItemManage::deleteMeasureItem(QList<int> vctIndex)
{
	for (auto nIndex: vctIndex)
	{
		int i = 0;
		for (auto item : m_mesure_item_list)
		{
			if (item->getMeasureIndex() == nIndex)
			{
				m_mesure_item_list.removeAt(i);
				break;
			}
			i++;
		}
	}
}

void CSMeasureItemManage::clearMeasureItem()
{
	m_mesure_item_list.clear();
}
