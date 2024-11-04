#include "bgrectitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#define ENABLE_INNER_OFFSET 0

RectangleGraphicsItem::RectangleGraphicsItem(QGraphicsItem* parent)
	: QGraphicsPolygonItem(parent),
	m_drag_type(DragType::Release)
{
	setFlag(QGraphicsItem::ItemIsFocusable, false);
	setFlag(QGraphicsItem::ItemIsMovable);
	setAcceptDrops(true);
	setAcceptHoverEvents(true);

	m_line_color = Qt::red;
	m_listFeatureRect.append(QRectF{});
	m_listFeatureRect.append(QRectF{});
	m_listFeatureRect.append(QRectF{});
	m_listFeatureRect.append(QRectF{});
}

void RectangleGraphicsItem::setPainterColor(const QColor& color)
{
	m_line_color = color;
}

void RectangleGraphicsItem::setRoiRect(const QRect& rect)
{
	if (4 == m_current_feature_vec.size()) {
		m_current_feature_vec[0] = rect.topLeft();
		m_current_feature_vec[1] = rect.topRight() + QPoint(1, 0);
		m_current_feature_vec[2] = rect.bottomRight() + QPoint(1, 1);
		m_current_feature_vec[3] = rect.bottomLeft() + QPoint(0, 1);
	}
	else {
		m_current_feature_vec.append(rect.topLeft());
		m_current_feature_vec.append(rect.topRight() + QPoint(1, 0));
		m_current_feature_vec.append(rect.bottomRight() + QPoint(1, 1));
		m_current_feature_vec.append(rect.bottomLeft() + QPoint(0, 1));
	}
#if	ENABLE_INNER_OFFSET
	for (auto & item_p : m_current_feature_vec)
	{
		item_p -= m_ptOffset;
	}
#endif
}

void RectangleGraphicsItem::setRoiType(const Device::RoiTypes& type)
{
	m_roi_type = type;
}

QRectF RectangleGraphicsItem::getRoiRect()
{
	//if(m_current_feature_vec.size()>2)
	//	return QRect(m_current_feature_vec[0].toPoint(), m_current_feature_vec[2].toPoint());
	return m_current_feature_vec.boundingRect();
}

QRectF RectangleGraphicsItem::getRealRoiRect()
{
	auto _roi = getRoiRect();
#if	ENABLE_INNER_OFFSET
	_roi.moveTo(m_ptOffset);
#endif
	return _roi;
}

void RectangleGraphicsItem::setRoiVisible(bool bVisible)
{
	m_roi_visible = bVisible;
}

void RectangleGraphicsItem::setFeaturePointVisible(bool bVisible)
{
	if (m_feature_visible != bVisible) {
		m_listFeatureRect.clear();
		m_listFeatureRect.append(QRectF{});
		m_listFeatureRect.append(QRectF{});
		m_listFeatureRect.append(QRectF{});
		m_listFeatureRect.append(QRectF{});
		m_feature_visible = bVisible;
		this->update();
	}
	this->setFlag(ItemIsFocusable, bVisible);
	this->setFlag(ItemIsSelectable, bVisible);
	this->setFlag(ItemIsMovable, bVisible);
}

bool RectangleGraphicsItem::getFeaturePointVisible()
{
	return m_feature_visible;
}

void RectangleGraphicsItem::setImageOffset(QPoint ptOffset)
{
#if	ENABLE_INNER_OFFSET
	if (m_ptOffset != ptOffset) {
		auto target_offset = m_ptOffset - ptOffset;
		for (auto & item_p : m_current_feature_vec)
		{
			item_p += target_offset;
		}
		m_ptOffset = ptOffset;
	}
#endif
}

void RectangleGraphicsItem::setRoiConstraintCondition(int nRoiConstraintCondition)
{
	m_nRoiConstrainCondition = (RoiConstraintCondition)nRoiConstraintCondition;
}

QRectF RectangleGraphicsItem::boundingRect() const
{
	return m_current_feature_vec.boundingRect();
	//return QGraphicsPolygonItem::boundingRect();
}

QRectF RectangleGraphicsItem::getFeaturePointRect(RectangleGraphicsItem::DragType type, const QPointF& featurePt)
{
	switch (RectangleGraphicsItem::DragType(type))
	{
	case RectangleGraphicsItem::DragType::LeftTop:
	{
		return QRectF(featurePt.x(), featurePt.y(), Width, Height);
	}
	case RectangleGraphicsItem::DragType::RightTop:
	{
		return QRectF(featurePt.x()- Width, featurePt.y(), Width, Height);
	}
	case RectangleGraphicsItem::DragType::RightBottom:
	{
		return QRectF(featurePt.x()- Width, featurePt.y()- Height, Width, Height);
	}
	case RectangleGraphicsItem::DragType::LeftBottom:
	{
		return QRectF(featurePt.x(), featurePt.y()- Height, Width, Height);
	}
	default:
		return QRectF{};
	}
	
}

QPointF RectangleGraphicsItem::getFeaturePoint(RectangleGraphicsItem::DragType type, const QPointF& featurePt, RotateAngle nAngle)
{
	switch (RectangleGraphicsItem::DragType(type))
	{
	case RectangleGraphicsItem::DragType::LeftTop:
	{
		int xoffset = 0, yoffset = 0;
		switch (nAngle)
		{
		case RectangleGraphicsItem::RotateAngle::RA_0:
			break;
		case RectangleGraphicsItem::RotateAngle::RA_90:
			xoffset = -Width;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_180:
			xoffset = -Width;
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_270:
			yoffset = -Height;
			break;
		default:
			break;
		}
		return QPointF(featurePt.x() + xoffset, featurePt.y() + yoffset);
	}
	case RectangleGraphicsItem::DragType::RightTop:
	{
		int xoffset = 0, yoffset = 0;
		switch (nAngle)
		{
		case RectangleGraphicsItem::RotateAngle::RA_0:
			xoffset = -Width;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_90:
			xoffset = -Width;
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_180:
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_270:
			break;
		default:
			break;
		}
		return QPointF(featurePt.x() + xoffset, featurePt.y() + yoffset);
		return QPointF(featurePt.x() - Width, featurePt.y());
	}
	case RectangleGraphicsItem::DragType::RightBottom:
	{
		int xoffset = 0, yoffset = 0;
		switch (nAngle)
		{
		case RectangleGraphicsItem::RotateAngle::RA_0:
			xoffset = -Width;
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_90:
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_180:
			break;
		case RectangleGraphicsItem::RotateAngle::RA_270:
			xoffset = -Width;
			break;
		default:
			break;
		}
		return QPointF(featurePt.x() + xoffset, featurePt.y() + yoffset);
		return QPointF(featurePt.x() - Width, featurePt.y() - Height);
	}
	case RectangleGraphicsItem::DragType::LeftBottom:
	{
		int xoffset = 0, yoffset = 0;
		switch (nAngle)
		{
		case RectangleGraphicsItem::RotateAngle::RA_0:
			yoffset = -Height;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_90:
			break;
		case RectangleGraphicsItem::RotateAngle::RA_180:
			xoffset = -Width;
			break;
		case RectangleGraphicsItem::RotateAngle::RA_270:
			xoffset = -Width;
			yoffset = -Height;
			break;
		default:
			break;
		}
		return QPointF(featurePt.x() + xoffset, featurePt.y() + yoffset);
		return QPointF(featurePt.x(), featurePt.y() - Height);
	}
	default:
		return QPointF{};
	}
}

void RectangleGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QPen pen;
	pen.setWidth(2);
	if (m_roi_visible) {
		setPolygon(m_current_feature_vec);

		QPen pen;
		pen.setWidth(2);
		pen.setCosmetic(true);// 将保持线宽 不被缩放
		painter->save();
		pen.setColor(m_line_color);
		painter->setPen(pen);
		painter->drawPolygon(m_current_feature_vec);
		painter->restore();

		if (m_feature_visible) {
			painter->save();
			pen.setColor(Qt::yellow);
			painter->setPen(pen);

			QBrush brush;
			brush.setColor(Qt::transparent);
			painter->setBrush(brush);
#if 1
			m_transform  = painter->transform();
			painter->resetTransform();
			auto nAngle = getTransformRotateAngle(m_transform);
			for (int i = 0; i < m_current_feature_vec.size(); ++i) {
				auto rect = QRectF(0, 0, 20, 20);
				QPointF pt = m_transform.map(m_current_feature_vec[i]);
				rect.moveTo(getFeaturePoint(RectangleGraphicsItem::DragType(i + 1), pt, nAngle));
				painter->drawRect(rect);
				if (m_listFeatureRect.size() > i)
				{
					m_listFeatureRect[i] = rect;
				}
				else {
					m_listFeatureRect.append(rect);
				}
			}
			painter->setTransform(m_transform);
#else
			for (int i = 0; i < m_current_feature_vec.size(); ++i) {
				painter->drawRect(getFeaturePointRect(RectangleGraphicsItem::DragType(i + 1),m_current_feature_vec[i]));
			}
#endif
			painter->restore();
		}
	}
}

void RectangleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	m_drag_type = DragType::Release;
	if (m_roi_visible && m_feature_visible) {
		if (event->button() == Qt::LeftButton)
		{
			m_startPos = event->pos();//鼠标左击时，获取当前鼠标在图片中的坐标，
#if 1
			m_drag_type = DragType::SelectedDrag;
			auto curPt = m_transform.map(m_startPos);
			for (int i = 0; i < m_listFeatureRect.size() && i < 4; ++i)
			{
				if (m_listFeatureRect[i].contains(curPt))
				{
					m_drag_type = RectangleGraphicsItem::DragType(i + 1);
				}
			}
			if (m_drag_type != DragType::Release)
			{
				setCursor(Qt::PointingHandCursor);
			}
#else
			for (int i = 0; i < m_current_feature_vec.size(); ++i) {
				if (getFeaturePointRect(RectangleGraphicsItem::DragType(i + 1),m_current_feature_vec[i]).contains(m_startPos)) {
					switch (RectangleGraphicsItem::DragType(i+1))
					{
					case RectangleGraphicsItem::DragType::LeftTop:
					{
						m_drag_type = DragType::LeftTop;
						break;
					}
					case RectangleGraphicsItem::DragType::RightTop:
					{
						m_drag_type = DragType::RightTop;
						break;
					}
					case RectangleGraphicsItem::DragType::RightBottom:
					{
						m_drag_type = DragType::RightBottom;
						break;
					}
					case RectangleGraphicsItem::DragType::LeftBottom:
					{
						m_drag_type = DragType::LeftBottom;
						break;
					}
					default:
						break;
					}
					setCursor(Qt::PointingHandCursor);
				}
			}
#endif

			if (m_parent_scene) {
				m_parent_scene->refreshScene(Device::RoiTypes::kUnknownRoi);
			}
		}
	}
}

void RectangleGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_roi_visible && m_feature_visible)
	{
		const QPointF point = (event->pos() - m_startPos);

		switch (m_drag_type)
		{
		case DragType::SelectedDrag:
		{
			if (m_nRoiConstrainCondition == kVerCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
			}
			else if (m_nRoiConstrainCondition == kHorCenterConstraint)
			{
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());
			}
			else if (m_nRoiConstrainCondition == kNoConstraint)
			{
#if 0
				int nWidth = m_current_feature_vec[1].x() - m_current_feature_vec[0].x();
				int nHeight = m_current_feature_vec[3].y() - m_current_feature_vec[0].y();
				int nXMax = m_ImageRect.width() + m_ImageRect.x();
				int nYMax = m_ImageRect.height() + m_ImageRect.y();
				int nXMin = m_ImageRect.x();
				int nYMin = m_ImageRect.y();
				if (point.x() < 0)
				{
					if (m_current_feature_vec[0].x() + point.x() < nXMin)
					{
						m_current_feature_vec[0].setX(nXMin);
						m_current_feature_vec[3].setX(nXMin);
						m_current_feature_vec[1].setX(nXMin + nWidth);
						m_current_feature_vec[2].setX(nXMin + nWidth);
					}
					else
					{
						m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
						m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
						m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
						m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
					}
				}
				else
				{
					if (m_current_feature_vec[1].x() + point.x() > nXMax)
					{
						m_current_feature_vec[1].setX(nXMax);
						m_current_feature_vec[2].setX(nXMax);
						m_current_feature_vec[0].setX(nXMax - nWidth);
						m_current_feature_vec[3].setX(nXMax - nWidth);
					}
					else
					{
						m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
						m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
						m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
						m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
					}
				}
				if (point.y() < 0)
				{
					if (m_current_feature_vec[0].y() + point.y() < nYMin)
					{
						m_current_feature_vec[0].setY(nYMin);
						m_current_feature_vec[1].setY(nYMin);
						m_current_feature_vec[2].setY(nYMin + nHeight);
						m_current_feature_vec[3].setY(nYMin + nHeight);
					}
					else
					{
						m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());
						m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());
						m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
						m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());
					}
				}
				else
				{
					if (m_current_feature_vec[2].y() + point.y() > nYMax)
					{
						m_current_feature_vec[2].setY(nYMax);
						m_current_feature_vec[3].setY(nYMax);
						m_current_feature_vec[0].setY(nYMax - nHeight);
						m_current_feature_vec[1].setY(nYMax - nHeight);
					}
					else
					{
						m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());
						m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());
						m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
						m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());
					}
				}
#else
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());
				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());
				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());
#endif
			}
		}
			break;
		case DragType::LeftTop:
		{
			m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
			m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());

			if (m_nRoiConstrainCondition == kVerCenterConstraint)
			{
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() - point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() - point.y());
			}
			else if (m_nRoiConstrainCondition == kHorCenterConstraint)
			{
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() - point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() - point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y());
			}
			else if (m_nRoiConstrainCondition == kCenterConstraint)
			{
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() - point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() - point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() - point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() - point.y());
			}
			else
			{
				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y());
			}
			break;
		}

		case DragType::RightTop:
		{
			m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
			m_current_feature_vec[1].setY(m_current_feature_vec[1].y() + point.y());

			if (m_nRoiConstrainCondition == kVerCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() - point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() - point.y());
			}
			else if (m_nRoiConstrainCondition == kHorCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() - point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() - point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y());
			}
			else if (m_nRoiConstrainCondition == kCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() - point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() - point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() - point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() - point.y());
			}
			else
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() + point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y());
			}
			break;
		}

		case DragType::LeftBottom:
		{
			m_current_feature_vec[3].setX(m_current_feature_vec[3].x() + point.x());
			m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());

			if (m_nRoiConstrainCondition == kVerCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() - point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() - point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
			}
			else if (m_nRoiConstrainCondition == kHorCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() - point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() - point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
			}
			else if (m_nRoiConstrainCondition == kCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() - point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() - point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() - point.y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x() - point.x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
			}
			else
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() + point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y());

				m_current_feature_vec[2].setX(m_current_feature_vec[2].x());
				m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());
			}
			break;
		}

		case DragType::RightBottom:
		{
			m_current_feature_vec[2].setX(m_current_feature_vec[2].x() + point.x());
			m_current_feature_vec[2].setY(m_current_feature_vec[2].y() + point.y());

			if (m_nRoiConstrainCondition == kVerCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() - point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() - point.y());
			}
			else if (m_nRoiConstrainCondition == kHorCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() - point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() - point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y());
			}
			else if (m_nRoiConstrainCondition == kCenterConstraint)
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x() - point.x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y() - point.y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x() - point.x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y() - point.y());
			}
			else
			{
				m_current_feature_vec[0].setX(m_current_feature_vec[0].x());
				m_current_feature_vec[0].setY(m_current_feature_vec[0].y());

				m_current_feature_vec[3].setX(m_current_feature_vec[3].x());
				m_current_feature_vec[3].setY(m_current_feature_vec[3].y() + point.y());

				m_current_feature_vec[1].setX(m_current_feature_vec[1].x() + point.x());
				m_current_feature_vec[1].setY(m_current_feature_vec[1].y());
			}

			break;
		}
		}
		m_startPos = event->pos();

		if (m_parent_scene) {
			m_parent_scene->refreshScene(Device::RoiTypes::kUnknownRoi);
		}
	}
}

void RectangleGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_roi_visible && m_feature_visible)
	{
		if (m_drag_type != DragType::Release)
		{
			if (m_parent_scene) {
				m_parent_scene->refreshScene(m_roi_type);
			}
		}
	}
	m_drag_type = DragType::Release;
}

RectangleGraphicsItem::RotateAngle RectangleGraphicsItem::getTransformRotateAngle(QTransform transform)
{
	RotateAngle nAngle = RotateAngle::RA_0;
	if (transform.m11() > 0)
	{
		// 0
		nAngle = RotateAngle::RA_0;
	}
	else if (transform.m11() < 0)
	{
		// 180
		nAngle = RotateAngle::RA_180;
	}
	else
	{
		if (transform.m12() > 0)
		{
			// 90
			nAngle = RotateAngle::RA_90;
		}
		else
		{
			// 270
			nAngle = RotateAngle::RA_270;
		}
	}
	return nAngle;
}

void RectangleGraphicsItem::setImageRect(QRectF rc)
{
	m_ImageRect = rc;
}