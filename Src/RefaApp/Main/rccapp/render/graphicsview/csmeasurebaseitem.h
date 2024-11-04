#pragma once
#include <QGraphicsItem>
#include <QWidget>
#include <QGraphicsTextItem>
#include "PointMeasure/CMeasureLineManage.h"

#define MEASURE_DRAW_VALUE 0
#define MEASURE_DRAW_TEXT_PAINT 1
class CSMeasureBaseItem : public QObject, public QGraphicsLineItem
{
	Q_OBJECT

public:
	CSMeasureBaseItem(QGraphicsItem* parent, CMeasureLineManage::TMeasureLineInfo info);
	~CSMeasureBaseItem() {}

	virtual CMeasureLineManage::TMeasureLineInfo getItemInfo();
	virtual void updateMeasureInfo(const CMeasureLineManage::TMeasureLineInfo& ptInfo);

	virtual int getMeasureIndex();

	virtual CMeasureLineManage::TMeasureLineType getMeasureType();
	
	virtual QRectF boundingRect();

	virtual QPainterPath shape() const override;

	virtual void setMeasureModeType(CMeasureLineManage::TMeasureModeType mode);

	virtual void setImageRect(QRectF rc);

	virtual void setImageOffset(QPoint ptOffset);

	virtual void setSelectedStatus(bool bSelected);

	virtual bool getSelectedStatus();
signals:
	void signalMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo& ptInfo);
	void signalMeasureItemClicked(const int nIndex);
protected:
	CMeasureLineManage::TMeasureLineInfo m_item{};

	CMeasureLineManage::TMeasureModeType m_modetype{ CMeasureLineManage::MMT_Show };
	int m_nSelected_Index{ -1 };

	QRectF m_ImageRect{};
	QPoint m_ptOffset{0, 0};
	bool m_bSelected{ false };
	QColor m_Selectedcolor{ Qt::green };
	QColor m_pointColor{ Qt::red };					// 点颜色
	QColor m_editpointColor{ Qt::red };				// 编辑框颜色
	QColor m_lineColor{ Qt::darkGreen };			// 绘制线颜色
	QColor m_fontColor{ Qt::green };				// 字体颜色
	QColor m_areaColor{ QColor(0,255, 0, 32) };		// 面积计算填充颜色
	const int m_pointSize = 4;						// 点大小
	QPainterPath m_pPath;
	QGraphicsTextItem *m_pTextItem{ nullptr };

	const double Width = 50.0;
	const double Height = 50.0;
};
