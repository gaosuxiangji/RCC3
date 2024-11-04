#pragma once
#include <QGraphicsLineItem>
#include <QObject>

class BGMeasureBase : public QObject, public QGraphicsLineItem
{
	Q_OBJECT
public:
	enum ItemType {
		Line,
		MultiLine,
	};
protected:
	BGMeasureBase();
	~BGMeasureBase() {}
};
