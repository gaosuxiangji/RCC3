#include "csmeasureangle3item.h"
#include <QStaticText>
#include <QDebug>

CSMeasureAngle3Item::CSMeasureAngle3Item(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info)
	:CSMeasureBaseItem(parent, info)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void CSMeasureAngle3Item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
			QPolygonF pPolyon(vctPoints);
			painter->drawPolyline(pPolyon);
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
		double dbAngle = CMeasureLineManage::GetAngleOfThreePoint(m_item.vctPoint[0], m_item.vctPoint[2], m_item.vctPoint[1]);
		QPoint ptEnd;
		ptEnd.setX(m_item.vctPoint[1].x() + 100);
		ptEnd.setY(m_item.vctPoint[1].y());
		double dbStartAngle = CMeasureLineManage::GetAngleOfThreePoint(m_item.vctPoint[0], ptEnd, m_item.vctPoint[1]);
		double dbFirstLen = CMeasureLineManage::PointDistance(m_item.vctPoint[0], m_item.vctPoint[1]);
		double dbSecondLen = CMeasureLineManage::PointDistance(m_item.vctPoint[2], m_item.vctPoint[1]);
		double dbLen = dbFirstLen > dbSecondLen ? dbSecondLen : dbFirstLen;

		int nX = m_item.vctPoint[1].x() - m_ptOffset.x() - dbLen / 2;
		int nY = m_item.vctPoint[1].y() - m_ptOffset.y() - dbLen / 2;
		int nW = dbLen;
		int nH = dbLen;
		painter->drawArc(nX, nY, nW, nH, dbStartAngle * 16, -dbAngle * 16);
#if MEASURE_DRAW_VALUE
		QString strText = QString::number(dbAngle, 10, 3);
		strText += QStringLiteral("°");

		double dbCenterAX = dbStartAngle - dbAngle / 2;
		double dbExrendLenX = std::cos(M_PI / 180 * dbCenterAX)*(dbLen / 2);
		double dbExrendLenY = std::sin(M_PI / 180 * dbCenterAX)*(dbLen / 2);

		int nX0 = m_item.vctPoint[1].x() + dbExrendLenX;
		int nY0 = m_item.vctPoint[1].y() - dbExrendLenY;
		int nW0 = dbExrendLenX * 2;
		int nH0 = dbExrendLenY * 2;
		//painter->drawRect(nX0, nY0, nW0, nH0);
		painter->drawText(nX0 + 5, nY0 - 5, strText);
#endif
		painter->restore();
	}
}

void CSMeasureAngle3Item::mousePressEvent(QGraphicsSceneMouseEvent* event)
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

void CSMeasureAngle3Item::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_modetype == CMeasureLineManage::MMT_Modify)
	{
		if (m_nSelected_Index >= 0)
		{
			QPointF curPT = event->pos();
			if (m_ImageRect.contains(curPT))
			{
				m_item.vctPoint[m_nSelected_Index] = curPT.toPoint()+ m_ptOffset;
			}
		}
	}
}

void CSMeasureAngle3Item::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

QRectF CSMeasureAngle3Item::boundingRect() const
{
	QRectF rc = QGraphicsLineItem::boundingRect();
	return rc;
}

QRectF CSMeasureAngle3Item::getFeaturePointRect(const QPointF& featurePt)
{
	return QRectF(featurePt.x() - Width / 2.0, featurePt.y() - Height / 2.0, Width, Height);
}

QPainterPath CSMeasureAngle3Item::shape() const
{
	QPainterPath prect;
	prect.addPath(CSMeasureBaseItem::shape());
	int nSize = m_item.vctPoint.size();
	for (int i = 1; i < nSize; i++) {
		QVector<QPointF> vctPolyPoints;
		vctPolyPoints.append(m_item.vctPoint[i - 1] - m_ptOffset);
		vctPolyPoints.append(m_item.vctPoint[i] - m_ptOffset);
		QPolygonF pLine(vctPolyPoints);
		QPainterPathStroker stroker;
		stroker.setWidth(2);
		QPainterPath linePath;
		linePath.addPolygon(pLine);
		prect.addPath(stroker.createStroke(linePath));
	}
	return prect;
}