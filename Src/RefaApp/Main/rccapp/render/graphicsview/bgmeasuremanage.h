#pragma once

#include <QObject>
#include "bglinemeasureitem.h"
#include "bgmeasurebase.h"
#include "bglinesmeasureitem.h"

class LineMeasureItemManage : public BGMeasureBase
{
	Q_OBJECT

	enum TMeasureModeType											// 测量模式类型
	{
		MMT_Normal,													// 正常类型，不在测量模式 不显示测量线
		MMT_Show,													// 显示测量线
		MMT_Add,													// 测量模式添加
		MMT_Modify,													// 测量模式修改
	};
public:
	LineMeasureItemManage();
	~LineMeasureItemManage() {}
public slots:
	void pushPointSlot(const QPoint& pt, bool cancel);
	void measureLineItemSlot(uint32_t index, const PointInfo& item);
	void pushMultiPtSlot(const QPoint& pt, bool feature, bool cancel);
	void multiMeasureLineItemSlot(uint32_t index, const MultiPointInfo& item);
signals:
	void updateMeasureItemInfoSignal(const PointInfo& item);
	void updateMultiMeasureItemInfoSignal(const MultiPointInfo& item);
public:
	void updateMeasureModeType(int type);
	void updateMeasureItemInfo(const PointInfo& item);
	void deleteMeasureLines(const QList<int> vctLine);
	void setMeasureItemVisible(bool visible);
	void updateMultiMeasureModeType(int type);
	void updateMultiMeasureItemInfo(const MultiPointInfo& item);
	void deleteMultiMeasureLines(const QList<int> vctLine);
	void setMultiMeasureItemVisible(bool visible);
protected:
	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
private:
	bool isLineMeasureItemExist(uint32_t index, const PointInfo& item);
	bool isMultiLineMeasureItemExist(uint32_t index, const MultiPointInfo& item);
private:
	QPolygonF m_point_vec{};
	bool m_draw_point_cancel{ false };
	QList<QSharedPointer<BGLineMeasureItem>> m_mesure_item_list{};

	QPolygonF m_multi_pt_feature_vec{};
	QPolygonF m_multi_pt_common_vec{};
	QList<QSharedPointer<BGLinesMeasureItem>> m_multi_mesure_item_list{};
};
