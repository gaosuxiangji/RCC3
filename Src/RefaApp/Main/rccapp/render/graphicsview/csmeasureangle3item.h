﻿#pragma once
#include "csmeasurebaseitem.h"
#include <QGraphicsPolygonItem>
#include <QObject>
#include <QPen>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>

class CSMeasureAngle3Item : public CSMeasureBaseItem
{
	Q_OBJECT
public:
	CSMeasureAngle3Item(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info);
protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	QRectF boundingRect() const;
	virtual QPainterPath shape() const override;
private:
	QRectF getFeaturePointRect(const QPointF& featurePt);
};