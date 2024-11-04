#include "bglineitem.h"
#include <QPen>
#include <QPainter>

#include "csmeasurebaseitem.h"
#include <QStaticText>

CSMeasureBaseItem::CSMeasureBaseItem(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info)
	: QGraphicsLineItem(parent)
	, m_item(info)
{
	this->setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);
	m_pTextItem = new QGraphicsTextItem(this);
	m_pTextItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	QFont ft;
	ft.setFamily("宋体");
	ft.setPixelSize(22);
	m_pTextItem->setFont(ft);
	m_pTextItem->setDefaultTextColor(Qt::green);
}

CMeasureLineManage::TMeasureLineInfo CSMeasureBaseItem::getItemInfo()
{
	return m_item;
}

CMeasureLineManage::TMeasureLineType CSMeasureBaseItem::getMeasureType()
{
	return m_item.nType;
}

void CSMeasureBaseItem::updateMeasureInfo(const CMeasureLineManage::TMeasureLineInfo& ptInfo)
{
	m_item = ptInfo;
}

int CSMeasureBaseItem::getMeasureIndex()
{
	return m_item.nIndex;
}

QRectF CSMeasureBaseItem::boundingRect()
{
	return QGraphicsLineItem::boundingRect();
}

QPainterPath CSMeasureBaseItem::shape() const
{
	QPainterPath prect;
	for (auto item : m_item.vctPoint)
	{
		item -= m_ptOffset;
		QRectF rect;
		rect.setLeft(item.x() - Width / 2);
		rect.setTop(item.y() - Height / 2);
		rect.setWidth(Width);
		rect.setHeight(Height);
		prect.addRect(rect);
	}
	return prect;
}

void CSMeasureBaseItem::setMeasureModeType(CMeasureLineManage::TMeasureModeType mode)
{
	m_modetype = mode;
}


void CSMeasureBaseItem::setImageRect(QRectF rc)
{
	m_ImageRect = rc;
}

void CSMeasureBaseItem::setImageOffset(QPoint ptOffset)
{
	m_ptOffset = ptOffset;
}

void CSMeasureBaseItem::setSelectedStatus(bool bSelected)
{
	m_bSelected = bSelected;
}

bool CSMeasureBaseItem::getSelectedStatus()
{
	return m_bSelected;
}
