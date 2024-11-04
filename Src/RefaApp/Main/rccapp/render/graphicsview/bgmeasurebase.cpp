#include "bgmeasurebase.h"

BGMeasureBase::BGMeasureBase()
{
	this->setFlags(QGraphicsItem::ItemIsSelectable |
		QGraphicsItem::ItemIsMovable |
		QGraphicsItem::ItemIsFocusable);
}