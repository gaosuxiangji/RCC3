#pragma once
#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QObject>
#include "bqgraphicsscene.h"
#include "Device/device.h"

class RectangleGraphicsItem : public QGraphicsPolygonItem
{
	enum class DragType
	{
		Release,//没有进行拖拽
		LeftTop,
		RightTop,
		RightBottom,
		LeftBottom,
		SelectedDrag,
	};

	enum class RotateAngle
	{
		RA_0,
		RA_90,
		RA_180,
		RA_270,
	};

	// ROI绘制区域约束
	enum RoiConstraintCondition
	{
		kNoConstraint,				// 无绘制区域约束
		kVerCenterConstraint,		// 垂直绘制区域约束 Y轴方向以中心的对称
		kHorCenterConstraint,		// 水平绘制区域约束 X轴方向以中心的对称
		kCenterConstraint,			// 中心点绘制区域约束 以中心点对称
	};
public:
	RectangleGraphicsItem(QGraphicsItem* parent = nullptr);
	~RectangleGraphicsItem() override = default;
	void setPainterColor(const QColor& color);
	void setRoiRect(const QRect& rect);
	void setRoiType(const Device::RoiTypes& type);
	QRectF getRoiRect();
	QRectF getRealRoiRect();
	void setParentScene(BQGraphicsScene* scene) { m_parent_scene = scene; }
	void setRoiVisible(bool bVisible);
	void setFeaturePointVisible(bool bVisible);
	bool getFeaturePointVisible();
	void setImageOffset(QPoint ptOffset);

	void setRoiConstraintCondition(int nConstraintCondition);

	// 设置图像的区域范围，主要是为了防止编辑区域超出图像外使用
	void setImageRect(QRectF rc);
protected:
	QPoint m_ptOffset{ 0, 0 };
protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
private:
	QRectF boundingRect() const override;
private:
	QRectF getFeaturePointRect(RectangleGraphicsItem::DragType type, const QPointF& featurePt);
	QPointF getFeaturePoint(RectangleGraphicsItem::DragType type, const QPointF& featurePt, RotateAngle nAngle = RotateAngle::RA_0);

	RotateAngle getTransformRotateAngle(QTransform transform);
private:
	//拖动起始位置
	QPointF m_startPos;
	//当前拖拽的点
	DragType m_drag_type;
	//画笔颜色
	QColor m_line_color;
	BQGraphicsScene* m_parent_scene{ nullptr };
	bool m_roi_visible{ false };
	bool m_feature_visible{ false };
	Device::RoiTypes m_roi_type{ Device::RoiTypes::kUnknownRoi };
	QPolygonF m_current_feature_vec{};
	const double Width = 20.0;
	const double Height = 20.0;
	RoiConstraintCondition m_nRoiConstrainCondition{ kNoConstraint };
	QList<QRectF> m_listFeatureRect;
	QTransform m_transform;
	QRectF m_ImageRect;
};