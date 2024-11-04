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
		Release,//û�н�����ק
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

	// ROI��������Լ��
	enum RoiConstraintCondition
	{
		kNoConstraint,				// �޻�������Լ��
		kVerCenterConstraint,		// ��ֱ��������Լ�� Y�᷽�������ĵĶԳ�
		kHorCenterConstraint,		// ˮƽ��������Լ�� X�᷽�������ĵĶԳ�
		kCenterConstraint,			// ���ĵ��������Լ�� �����ĵ�Գ�
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

	// ����ͼ�������Χ����Ҫ��Ϊ�˷�ֹ�༭���򳬳�ͼ����ʹ��
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
	//�϶���ʼλ��
	QPointF m_startPos;
	//��ǰ��ק�ĵ�
	DragType m_drag_type;
	//������ɫ
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