#include "bqgraphicsscene.h"
#include "qgraphicsview.h"

BQGraphicsScene::BQGraphicsScene(QObject *parent) : QGraphicsScene(parent)
{
	this->setProperty("enable_drag_scene", true);
}


void BQGraphicsScene::setMeasureModeType(CMeasureLineManage::TMeasureModeType mode)
{
	m_MeasureMode = mode;
}

void BQGraphicsScene::setCalibration(CMeasureLineManage::TMeasureModeType mode)
{
	m_modeCalibration = mode;
}

void BQGraphicsScene::clearGraphicsItem()
{
	m_measure_count = 0;
}


void BQGraphicsScene::refreshScene(const Device::RoiTypes& type)
{//解决移动roi后，事件丢失问题
	setSceneRect(0, 0, this->width()*1.00000001, this->height()*1.00000001);
	setSceneRect(0, 0, this->width() * 1 / 1.00000001, this->height() * 1 / 1.00000001);
	update();
	if (Device::RoiTypes::kUnknownRoi != type) {
		emit updateRoiInfo(type);
	}
}

void BQGraphicsScene::setCenterLinePt(const QPoint& pt)
{
	m_center_cursor_point = pt;
// 	if (m_common_param.is_acquring)
// 		m_center_cursor_point = QPoint(pt.x() - m_common_param.device_roi.x(), pt.y() - m_common_param.device_roi.y());
// 	else {
// 	}
}

void BQGraphicsScene::setCenterLineVisible(bool bvisible)
{
	m_center_line_visible = bvisible;
}

void BQGraphicsScene::setRotateValue(const int rt)
{
	m_rotate_angle = rt;
}

void BQGraphicsScene::setWheelDisalble(bool enable)
{
	m_wheel_disabled = enable;
}

void BQGraphicsScene::setCrossLineRect(const QRectF& rect)
{
	m_cross_line_rect = rect.toRect();
}

void BQGraphicsScene::setCrossLineVisible(bool bvisible)
{
	m_cross_line_visible = bvisible;
}

void BQGraphicsScene::setCommonParam(const CommonParam& param)
{
	m_common_param = param;

// 	if (m_common_param.is_acquring)
// 		m_center_cursor_point = QPoint(m_center_cursor_point.x() - m_common_param.device_roi.x(), m_center_cursor_point.y() - m_common_param.device_roi.y());
// 	else
		m_center_cursor_point = param.center_pt.toPoint();
}

void BQGraphicsScene::setWhiteBalanceBg(bool is_white_balance_bg)
{
	m_is_white_balance_bg = is_white_balance_bg;
}

void BQGraphicsScene::setBgCoef(const qreal& coef)
{
	m_bg_coef = coef;
}

void BQGraphicsScene::updateImage(QImage img)
{
	{
		std::lock_guard<std::mutex> lock(m_background_image_lock); 
		m_background_image = img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
	}
	update();
}

void BQGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
	if (m_wheel_disabled) {
		return;
	}
	if (Qt::LeftButton == event->buttons()) {
		QPointF p(event->scenePos().x(), event->scenePos().y());
// 		if (p.x() > (m_center_cursor_point.x() - 10) && p.y() > (m_center_cursor_point.y() - 10)\
// 			&& p.x() < (m_center_cursor_point.x() + 20) && p.y() < (m_center_cursor_point.y() + 20)) {
// 			m_center_rect_press = true;
// 			this->setProperty("enable_drag_scene", false);
// 			//this->views().first()->setDragMode(QGraphicsView::NoDrag);
// 		}
		if (m_center_line_visible)
		{
			auto pt = m_transform.map(p);
			if (m_rcCenterArea.contains(pt))
			{
				m_center_rect_press = true;
				this->setProperty("enable_drag_scene", false);
			}
		}

		if (m_MeasureMode == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Modify)
		{
			emit signalPressOnePt(p.toPoint());
		}
		emit signalMousePress();
	}
	else if (Qt::RightButton == event->buttons()) {
		if (m_MeasureMode == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Modify)
		{
			emit signalReleassMouse();
		}
    }
}

void BQGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseMoveEvent(event);
	if (m_wheel_disabled) {
		return;
	}
	m_cross_cursor_pt = event->scenePos();
	qreal event_x = event->scenePos().x();
	qreal event_y = event->scenePos().y();
	qreal img_x = m_image_original_size.x();
	qreal img_y = m_image_original_size.y();
	qreal img_width = m_image_original_size.width();
	qreal img_height = m_image_original_size.height();

	if (m_center_rect_press) {
		if (event_x > img_x&&event_x<(img_x + img_width) && event_y>img_y&&event_y < (img_y + img_height)) {
			m_center_cursor_point = event->scenePos().toPoint();
		}
// 		if(m_common_param.is_acquring)
// 			emit SignalCustomCrossLineCenterMovePoint(QPoint(m_center_cursor_point.x() + m_common_param.device_roi.x(), m_center_cursor_point.y() + m_common_param.device_roi.y()));
// 		else
			emit SignalCustomCrossLineCenterMovePoint(QPoint(m_center_cursor_point.x(), m_center_cursor_point.y()));
	}

	if (event_x > img_x&&event_x<(img_x + img_width) && event_y>img_y&&event_y < (img_y + img_height)) {
		m_cursor_pt = event->scenePos().toPoint();
		emit sendMousePtInSceneSignal(m_cursor_pt);
	}
	else
	{
		emit sendMousePtInSceneSignal(QPoint(-1, -1));
	}

	if (m_MeasureMode == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Add || m_modeCalibration == CMeasureLineManage::MMT_Modify)
	{
		if (Qt::NoButton == event->buttons())
		{
			emit signalMoveOnePt(event->scenePos().toPoint());
		}
	}

}

void BQGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
	if (m_current_rect != rect) {
		m_current_rect = rect;
	}
	painter->save();
	std::lock_guard<std::mutex> lock(m_background_image_lock);
	if (!m_background_image.isNull()) {
		painter->drawImage(QPoint(0, 0), m_background_image.scaled(QSize(m_background_image.width()*m_bg_coef, m_background_image.height()*m_bg_coef)));
		m_image_original_size = m_background_image.rect();
		emit imageRectSignal(m_background_image.rect());
	}
	painter->restore();

	qreal min_value = qMin(abs(m_background_image.width()*1.0 / m_cross_line_rect.width()), abs(m_background_image.height()*1.0 / m_cross_line_rect.height()));
#if 0
	drawCrossLine(painter, pen, min_value);
#endif // 0
	drawCenterLine(painter, min_value);
}

void BQGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);
	if (m_wheel_disabled) {
		return;
	}
	if (m_center_rect_press) {
// 		if(m_common_param.is_acquring)
// 			emit SignalUpdateCustomCrossLineCenterPoint(QPointF(m_center_cursor_point.x() + m_common_param.device_roi.x(), m_center_cursor_point.y() + m_common_param.device_roi.y()));
// 		else
			emit SignalUpdateCustomCrossLineCenterPoint(QPointF(m_center_cursor_point.x(), m_center_cursor_point.y()));
	}
	m_center_rect_press = false;
	this->setProperty("enable_drag_scene", true);
	//this->views().first()->setDragMode(QGraphicsView::ScrollHandDrag);
}
#if 0
void BQGraphicsScene::drawCrossLine(QPainter *painter, const QPen& pen, const qreal& min_value)
{
	if (m_cross_line_visible) {//十字中心线
		painter->save();
		painter->setPen(pen);
		if (min_value < 1) {
			painter->drawLine(QPoint(m_cross_line_rect.x(), m_cross_cursor_pt.y()), QPoint(m_cross_line_rect.width(), m_cross_cursor_pt.y()));
			painter->drawLine(QPoint(m_cross_cursor_pt.x(), m_cross_line_rect.y()), QPoint(m_cross_cursor_pt.x(), m_cross_line_rect.height()));
		}
		else {
			painter->drawLine(QPoint(m_background_image.rect().x(), m_cross_cursor_pt.y()), QPoint(m_background_image.width(), m_cross_cursor_pt.y()));
			painter->drawLine(QPoint(m_cross_cursor_pt.x(), m_background_image.rect().y()), QPoint(m_cross_cursor_pt.x(), m_background_image.height()));
		}
		painter->restore();
	}
}
#endif // 0
void BQGraphicsScene::drawCenterLine(QPainter *painter, const qreal& min_value)
{
	if (m_center_line_visible) {
		painter->save();
		QVector<qreal>dashes;
		qreal space = 4;
		dashes << 3 << space;
		QPen pen;
		pen.setDashPattern(dashes);
		pen.setColor(Qt::yellow);
		pen.setWidth(2);
		pen.setCosmetic(true);// 将保持线宽 不被缩放
		painter->setPen(pen);
#if 1
		if (m_center_cursor_point.x() >= m_background_image.rect().left() && m_center_cursor_point.x() <= m_background_image.rect().right()) {
			painter->drawLine(QPoint(m_center_cursor_point.x(), m_background_image.rect().y()), QPoint(m_center_cursor_point.x(), m_background_image.height()));
		}
		if (m_center_cursor_point.y() >= m_background_image.rect().top() && m_center_cursor_point.y() <= m_background_image.rect().bottom()) {
			painter->drawLine(QPoint(m_background_image.rect().x(), m_center_cursor_point.y()), QPoint(m_background_image.width(), m_center_cursor_point.y()));
		}
#endif // 1

#if 0
		if ((m_common_param.is_acquring&m_common_param.is_draw_h_line) || !m_common_param.is_acquring)
			painter->drawLine(QPoint(m_background_image.rect().x(), m_center_cursor_point.y()), QPoint(m_background_image.width(), m_center_cursor_point.y()));
		if ((m_common_param.is_acquring&m_common_param.is_draw_v_line) || !m_common_param.is_acquring)
			painter->drawLine(QPoint(m_center_cursor_point.x(), m_background_image.rect().y()), QPoint(m_center_cursor_point.x(), m_background_image.height()));
#endif // 0
#if 1
		m_transform = painter->transform();
		painter->resetTransform();
		auto rect = QRectF(0, 0, 2 * qCenterOffset, 2 * qCenterOffset);
		QPointF pt = m_transform.map(m_center_cursor_point);
		rect.moveTo(pt - QPointF(qCenterOffset, qCenterOffset));
		m_rcCenterArea = rect;
		auto curPt = m_transform.map(m_cursor_pt);
		if (m_rcCenterArea.contains(curPt)) {
			pen.setStyle(Qt::PenStyle::SolidLine);
			painter->setPen(pen);
			painter->drawRect(rect);
// 			qreal coef = (min_value > 1) ? (1 / min_value) : min_value;
// 			painter->drawRect(QRectF(m_center_cursor_point.x() - 5 * coef, m_center_cursor_point.y() - 5 * coef, 20 * coef, 20 * coef));
		}
		painter->setTransform(m_transform);
#endif // 1

		painter->restore();
#if 0
		if ((m_common_param.is_acquring&m_common_param.is_draw_center_rect) || !m_common_param.is_acquring) {
			painter->save();
			painter->setPen(pen);
			qreal coef = (min_value > 1) ? (1 / min_value) : min_value;
			painter->drawRect(QRectF(m_center_cursor_point.x() - 5 * coef, m_center_cursor_point.y() - 5 * coef, 10 * coef, 10 * coef));
			painter->restore();
		}
#endif // 0

	}
}

void BQGraphicsScene::drawForeground(QPainter * painter, const QRectF & rect)
{
	if (m_cross_line_visible) {//十字中心线 跟随光标移动的十字实心线
		painter->save();
		QPen pen;
		pen.setWidth(2);
		pen.setColor(Qt::yellow);
		pen.setCosmetic(true);// 将保持线宽 不被缩放
		painter->setPen(pen);
		painter->drawLine(QPoint(rect.x(), m_cross_cursor_pt.y()), QPoint(rect.right() + 1, m_cross_cursor_pt.y()));
		painter->drawLine(QPoint(m_cross_cursor_pt.x(), rect.y()), QPoint(m_cross_cursor_pt.x(), rect.bottom() + 1));
		painter->restore();
	}
}
