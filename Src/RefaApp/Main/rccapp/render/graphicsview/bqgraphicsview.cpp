#include "bqgraphicsview.h"
#include <qgraphicsitem.h>
#include <QDebug>
#include "Common/UIUtils/UIHelper.h"
#define ENABLE_VIEW_DRAW_OSD 1
BQGraphicsView::BQGraphicsView(QWidget *parent) : QGraphicsView(parent)
{
    // 隐藏水平/竖直滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
	InitViewLabel();
}

void BQGraphicsView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
	if (m_bTranslation && (event->buttons() & Qt::LeftButton))
	{
		QPointF offsetPos = event->pos() - m_pos_anchor;
		if (m_is_mouse_pressed && this->scene()->property("enable_drag_scene").toBool()) {
			//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
			//centerOn(m_center_anchor - offsetPos);

			horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - m_pos_anchor.x()));
			verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - m_pos_anchor.y()));
			m_pos_anchor = event->pos();

			//QPointF delta = mapToScene(event->pos()) - mapToScene(this->m_pos_anchor);
			////调用平移方法
			//delta *= this->transform().m11();
			//this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
			//this->centerOn(this->mapToScene(QPoint(this->viewport()->rect().width() / 2 - delta.x(),
			//	this->viewport()->rect().height() / 2 - delta.y())));
			//this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
			////lastMousePos是MyGraphicsView的私有成员变量，用以记录每次的事件结束时候的鼠标位置
			//this->m_pos_anchor = event->pos();
		}
	}
}


void BQGraphicsView::wheelEvent(QWheelEvent *event)
{
	if (m_wheel_disabled)
		return;
#if 1
	auto graphicsView = this;
	QPointF pos = graphicsView->mapToScene(event->pos());
	auto oldAnchor = graphicsView->transformationAnchor();
	graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	if (event->delta()>0)
	{
		double _scale = qAbs(graphicsView->transform().m11() + graphicsView->transform().m12()) * 100;
		if (qFuzzyCompare(_scale, maxScale)) return;
		if (_scale >= maxScale) return;
		double _zoomScale = qMin(zoomScale, maxScale / _scale);
		graphicsView->scale(_zoomScale, _zoomScale);
	}
	else
	{
		double _scale = qAbs(graphicsView->transform().m11() + graphicsView->transform().m12()) * 100;
		if (qFuzzyCompare(_scale, this->minScale)) return;
		if (_scale <= this->minScale) return;
		double _zoomScale = qMin(zoomScale, _scale / minScale);
		graphicsView->scale(1 / _zoomScale, 1 / _zoomScale);
	}
	graphicsView->setTransformationAnchor(oldAnchor);
	isFitView = false;
#endif // 1

#if 0
	if(m_wheel_disabled)
		return;
	QPointF cursorPoint = event->pos();
	QPointF scene_pos = mapToScene(QPoint(cursorPoint.x(), cursorPoint.y()));

	qreal view_width = viewport()->width();
	qreal view_height = viewport()->height();

	qreal hScale = cursorPoint.x() / view_width;
	qreal vScale = cursorPoint.y() / view_height;

	if ((event->delta() > 0) && m_allow_wheel_up) {
		scale(m_scale_coef_const, m_scale_coef_const);
	}
	else if (event->delta() < 0 && m_allow_wheel_down) {
		scale(1 / m_scale_coef_const, 1 / m_scale_coef_const);
	}
	else {
		return;
	}

	QPointF view_point = matrix().map(scene_pos);
	horizontalScrollBar()->setValue(int(view_point.x() - view_width*hScale));
	verticalScrollBar()->setValue(int(view_point.y() - view_height*vScale));
#endif // 0

	emit sigUpdateCoefficient();
}

void BQGraphicsView::mousePressEvent(QMouseEvent *event)
{
	QGraphicsView::mousePressEvent(event);
	if (m_wheel_disabled)
		return;
	setDragMode(QGraphicsView::ScrollHandDrag);
	if (this->scene() == nullptr)
	{
		return;
	}
	// 记录鼠标按下时的中心点坐标
	m_center_anchor = mapToScene(event->pos()) - event->pos() + QPointF(width() / 2, height() / 2);
	// 记录当前鼠标在view中的位置，用来在mouseMove事件中计算偏移
	// 此处不将view坐标转换成scene坐标的原因是优化性能，在move的过程中会产生抖动
	m_pos_anchor = event->pos();
	if(!this->scene()->focusItem())
		m_is_mouse_pressed = true;
}

void BQGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	QGraphicsView::mouseReleaseEvent(event);
	setDragMode(QGraphicsView::NoDrag);
	if (m_wheel_disabled)
		return;
	
	m_is_mouse_pressed = false;
}

void BQGraphicsView::paintEvent(QPaintEvent * event)
{
	QGraphicsView::paintEvent(event);
#if ENABLE_VIEW_DRAW_OSD
	QPainter painter(this->viewport());
	painter.save();	
	QPen pen;
	pen.setWidth(2);
	pen.setCosmetic(true);

	QVector<QPoint> vctPoints;
	QBrush brush;
	brush.setStyle(Qt::SolidPattern);
	brush.setColor(Qt::red);
	painter.setBrush(brush);
	pen.setColor(Qt::green);
	painter.setPen(pen);
	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewIpLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewIpLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewIpLabel]->text(),
		UIHelper::LT,
		m_locationOffset);

	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewConnectStatusLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewConnectStatusLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewConnectStatusLabel]->text(),
		UIHelper::RT,
		m_locationOffset);
	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewTimestampLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewTimestampLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewTimestampLabel]->text(),
		UIHelper::LB,
		m_locationOffset);
	painter.restore();
#endif
}

void BQGraphicsView::InitViewLabel()
{
#if ENABLE_VIEW_DRAW_OSD
	for (size_t i = 0; i < ViewLabel_Max_COUNT; i++)
	{
		QLabel *pLabel = new QLabel(this);
		if (nullptr != pLabel)
		{
			pLabel->setVisible(false);
			if (ViewFrameLabel == i)
			{
				pLabel->setStyleSheet(QString::fromUtf8("background-color:white;font-size:12px;font-weight:bold;color:black;"));		
			}
			else
			{
				pLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:24px;font-family:Microsoft YaHei;font-weight:bold;color:#FF0000;"));
			}
			pLabel->setText("");
			m_mapViewLabel.insert(ViewShowLabelType(i), pLabel);
		}
	}
	for (size_t i = 0; i < ViewLabel_Max_COUNT; i++)
	{
		m_mapViewColor.insert(ViewShowLabelType(i), QColor(255, 0, 0));
	}
#endif
}

void BQGraphicsView::resizeEvent(QResizeEvent *event)
{
	AdjustViewBaseLabel(QSize(this->rect().width(), this->rect().height()));
}

void BQGraphicsView::zoomIn()
{
#if 1
	if (m_wheel_disabled)
		return;
	double _scale = qAbs(this->transform().m11() + this->transform().m12()) * 100;
	if (qFuzzyCompare(_scale, maxScale)) return;
	if (_scale >= maxScale) return;
	double _zoomScale = qMin(zoomScale, maxScale / _scale);
	auto oldAnchor = this->transformationAnchor();
	this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	this->scale(_zoomScale, _zoomScale);
	this->setTransformationAnchor(oldAnchor);
	isFitView = false;
#endif // 1

#if 0
	if (m_wheel_disabled || !m_allow_wheel_up)
		return;
	qreal viewWidth = viewport()->width();
	qreal viewHeight = viewport()->height();
	qreal hScale = m_mouse_press_point.x() / viewWidth;
	qreal vScale = m_mouse_press_point.y() / viewHeight;

	scale(m_scale_coef_const, m_scale_coef_const);

	//将scene坐标转换为放大缩小后的坐标
	QPointF viewpoint = matrix().map(m_mouse_press_point_scene);

	horizontalScrollBar()->setValue(viewpoint.x() - viewWidth*hScale);
	verticalScrollBar()->setValue(viewpoint.y() - viewHeight*vScale);
#endif // 0
	emit sigUpdateCoefficient();
}

void BQGraphicsView::zoomOut()
{
#if 1
	if (m_wheel_disabled)
		return;
	double _scale = qAbs(this->transform().m11() + this->transform().m12()) * 100;
	if (qFuzzyCompare(_scale, this->minScale)) return;
	if (_scale <= this->minScale) return;
	double _zoomScale = qMin(zoomScale, _scale / minScale);
	auto oldAnchor = this->transformationAnchor();
	this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	this->scale(1 / _zoomScale, 1 / _zoomScale);
	this->setTransformationAnchor(oldAnchor);
	isFitView = false;
#endif // 1

#if 0
	if (m_wheel_disabled || !m_allow_wheel_down)
		return;
	scale(1 / m_scale_coef_const, 1 / m_scale_coef_const);
#endif // 0

	emit sigUpdateCoefficient();
}

void BQGraphicsView::fitView(bool bg)
{
#if 1
	this->fitInView(this->scene()->sceneRect(), Qt::KeepAspectRatio);
	this->fitInView(this->scene()->sceneRect(), Qt::KeepAspectRatio);//第二遍去除滚动条的影响
	isFitView = true;
#endif // 1

#if 0
	QMatrix mat;
	
	double dx = viewport()->geometry().width() / sceneRect().width();
	double dy = viewport()->geometry().height() / sceneRect().height();
	double min_value = qMin(dx, dy);
	if (bg) {
		min_value = 0.5;
	}
	mat.scale(min_value, min_value);
	if(!bg)
		mat.rotate(m_rotate_angle);
	centerOn(QPointF(sceneRect().width() / 2, sceneRect().height() / 2));
	setMatrix(mat);
#endif
	emit sigUpdateCoefficient();
}

void BQGraphicsView::setWheelDisalble(bool disable)
{
	m_wheel_disabled = disable;
}
#if 0

void BQGraphicsView::setWheelUpEnable(bool enable)
{
	m_allow_wheel_up = enable;
}

void BQGraphicsView::setWheelDownEnable(bool enable)
{
	m_allow_wheel_down = enable;
}
#endif // 0

void BQGraphicsView::fullView()
{
#if 1 // 修改全屏显示逻辑
	auto oldAnchor = this->transformationAnchor();
	this->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	auto transform = this->transform();
	this->setTransform(QTransform(transform.m11()>0 ? 1 : transform.m11()<0 ? -1 : 0, transform.m12()>0 ? 1 : transform.m12()<0 ? -1 : 0, transform.m13()>0 ? 1 : transform.m13()<0 ? -1 : 0,
		transform.m21()>0 ? 1 : transform.m21()<0 ? -1 : 0, transform.m22()>0 ? 1 : transform.m22()<0 ? -1 : 0, transform.m23()>0 ? 1 : transform.m23()<0 ? -1 : 0,
		transform.m31()>0 ? 1 : transform.m31()<0 ? -1 : 0, transform.m32()>0 ? 1 : transform.m32()<0 ? -1 : 0, transform.m33()>0 ? 1 : transform.m33()<0 ? -1 : 0));
	this->setTransformationAnchor(oldAnchor);

	centerOn(sceneRect().center());
	isFitView = false;
#endif // 1

#if 0
	QMatrix mat;
	mat.scale(1.0, 1.0);
	mat.rotate(m_rotate_angle);
	centerOn(QPointF(sceneRect().width() / 2, sceneRect().height() / 2));
	setMatrix(mat);
#endif // 0
	emit sigUpdateCoefficient();
}

void BQGraphicsView::setTranslation(bool bEnable)
{
	m_bTranslation = bEnable;
}

void BQGraphicsView::resetMousePress()
{
	setDragMode(QGraphicsView::NoDrag);
	m_is_mouse_pressed = false;
}


void BQGraphicsView::SetViewShowLabelInfo(ViewShowLabelType type, QVariant info)
{
	m_mapViewText[type] = info.toString();


	if (m_mapViewLabel.end() != m_mapViewLabel.find(type))
	{
		if (nullptr != m_mapViewLabel[type])
		{
			if (ViewShowLabelType::ViewFrameLabel == type)
			{
				m_mapViewLabel[type]->setVisible(false);
			}
			m_mapViewLabel[type]->setText(info.toString());
			m_mapViewLabel[type]->adjustSize();
		}
	}
	AdjustViewBaseLabel(this->size());
	update();
}

void BQGraphicsView::SetViewShowLabelColor(BQGraphicsView::ViewShowLabelType type, QVariant info)
{
	auto color = info.value<QColor>();
	m_mapViewColor[type] = color;
}

void BQGraphicsView::AdjustViewBaseLabel(const QSize& size) 
{
	for (auto item : m_mapViewLabel.toStdMap())
	{
		if (nullptr != item.second)
		{
			//更新QLabel样式
			UpdateQLabelStyle(item.first, item.second, GetLabelFontSize(size));

			switch (item.first)
			{
			case ViewIpLabel:
				item.second->move(m_locationOffset, m_locationOffset);
				break;
			case ViewConnectStatusLabel:
				if ((size.width() - item.second->width()) > 0)
				{
					item.second->move((size.width() - item.second->width() - m_locationOffset), m_locationOffset);
				}
				break;
			case ViewTimestampLabel:
				if ((size.height() - item.second->height()) > 0)
				{
					item.second->move(m_locationOffset, (size.height() - item.second->height() - m_locationOffset));
				}
				break;
			case ViewFrameLabel:
				item.second->move(0, 0);
				break;
			default:
				break;
			}
		}
	}
}

void BQGraphicsView::UpdateQLabelStyle(const ViewShowLabelType& type, QLabel* label, int fontsize)
{
	if (nullptr != label)
	{
		if (ViewShowLabelType::ViewFrameLabel == type)
		{
			label->setStyleSheet(QString::fromUtf8("background-color:white;font-size:12px;font-weight:bold;color:black;"));
		}
		else
		{
			label->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:%1px;font-family:Microsoft YaHei;font-weight:bold;color:#FF0000;").arg(fontsize));
		}
		label->adjustSize();
	}
}

int BQGraphicsView::GetLabelFontSize(const QSize& size)
{
	int fontSize = 0;

	if (m_viewBaseOriginalSize == size)
	{
		fontSize = 24;
	}
	else
	{
		double scalFactor = ((size.width() / double(m_viewBaseOriginalSize.width())) + (size.height() / double(m_viewBaseOriginalSize.height()))) / 2.0;
		fontSize = qRound(24 * scalFactor);
	}

	return fontSize;
}
