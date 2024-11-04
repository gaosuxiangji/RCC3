#include "csmeasuredimensionitem.h"
#include <QStaticText>
#include <QDebug>

CSMeasureDimensionItem::CSMeasureDimensionItem(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info)
	:CSMeasureBaseItem(parent, info)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
}

void CSMeasureDimensionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
		if (m_bSelected){
			brush.setColor(Qt::red);
			pen.setColor(Qt::red);
		}
		else {
			brush.setColor(Qt::darkRed);
			pen.setColor(Qt::darkRed);
		}
		painter->setBrush(brush);
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
	}
}

void CSMeasureDimensionItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
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
	if (event->button() == Qt::LeftButton){
		emit signalMeasureItemClicked(m_item.nIndex);
	}
}

void CSMeasureDimensionItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
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

void CSMeasureDimensionItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
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

QRectF CSMeasureDimensionItem::boundingRect() const
{
	QRectF rc = QGraphicsLineItem::boundingRect();
	return rc;
}

QRectF CSMeasureDimensionItem::getFeaturePointRect(const QPointF& featurePt)
{
	return QRectF(featurePt.x() - Width / 2.0, featurePt.y() - Height / 2.0, Width, Height);
}
