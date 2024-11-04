#pragma once

#include "csmeasurebaseitem.h"
#include "csmeasurelineitem.h"
#include "csmeasureangle3item.h"
#include "csmeasureangle4item.h"
#include "csmeasureradiusitem.h"
#include "csmeasurediameteritem.h"
#include "csmeasurecenterdistanceitem.h"
#include "csmeasuredimensionitem.h"
#include "csmeasurearearadiusitem.h"
#include "csmeasureareapolygonitem.h"
#include <QObject>
#include <QGraphicsItem>

class CSMeasureItemManage : public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	CSMeasureItemManage();
	~CSMeasureItemManage() {}
public:
	void initMeasureItem(QList<CMeasureLineManage::TMeasureLineInfo> vctInfo);
	void addMeasureItem(CMeasureLineManage::TMeasureLineInfo info, bool bSelected = false);
	void updateMeasureItem(CMeasureLineManage::TMeasureLineInfo info);
	void deleteMeasureItem(QList<int> vctIndex);

	void clearMeasureItem();
	void setImageRect(QRectF rc);
	void setImageOffset(QPoint ptOffset);

	void setCalibration(CMeasureLineManage::TMeasureModeType mode);
	void setCalibrationPoints(QList<QPoint> vctPoints);
	QList<QPoint> getCalibrationPoints();

	void setSelectedItem(int nIndex);
public slots:
// 	void pushPointSlot(const QPoint& pt, bool cancel);
// 	void measureLineItemSlot(uint32_t index, const PointInfo& item);
// 	void pushMultiPtSlot(const QPoint& pt, bool feature, bool cancel);
// 	void multiMeasureLineItemSlot(uint32_t index, const MultiPointInfo& item);

	void slotPressOnePt(const QPoint& pt);
	void slotMoveOnePt(const QPoint& pt);
	void slotReleassMouse();

	void slotMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo& ptInfo);
	void slotMeasureItemClicked(const int nIndex);

signals:
// 	void updateMeasureItemInfoSignal(const PointInfo& item);
// 	void updateMultiMeasureItemInfoSignal(const MultiPointInfo& item);
	void signalAddItemForPoint(QList<QPoint> vctPoint);
	void signalMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo& ptInfo);
	void signalItemPointsMove(QList<QPoint> vctPoint);
	void signalMeasureItemClicked(const int nIndex);
public:
	void updateMeasureModeType(CMeasureLineManage::TMeasureModeType mode);
	void updateMeasureType(CMeasureLineManage::TMeasureLineType mode);
// 	void updateMeasureItemInfo(const PointInfo& item);
// 	void deleteMeasureLines(const QList<int> vctLine);
 	void setMeasureItemVisible(bool visible);
protected:
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
	QList<QSharedPointer<CSMeasureBaseItem>> m_mesure_item_list{};
	CMeasureLineManage::TMeasureLineType m_measureType{ CMeasureLineManage::MLT_MORE_POINT };
	CMeasureLineManage::TMeasureModeType m_measureMode{ CMeasureLineManage::MMT_Show };
	QList<QPoint> m_vctAddPoint;					// 添加的点
	QPoint m_curPt{};								// 当前的点
	QRectF m_ImageRect{}; 							// 图像区域
	QPoint m_ptOffset{ 0, 0 };						// 偏移点，主要为ROI的x,y
	QList<QPoint> m_vctCalibration;					// 两点标定
	CMeasureLineManage::TMeasureModeType m_modeCalibration;						// 标定类型
	const int kARROWSIZE{ 14 };						// 标定箭头大小
	QColor m_pointColor{ Qt::red };					// 点颜色
	QColor m_editpointColor{ Qt::red };				// 编辑框颜色
	QColor m_lineColor{ Qt::green };				// 绘制线颜色
	QColor m_fontColor{ Qt::red };					// 字体颜色
	QColor m_areaColor{ QColor(0,255, 0, 32) };		// 面积计算填充颜色
	const int m_pointSize = 4;						// 点大小
};
