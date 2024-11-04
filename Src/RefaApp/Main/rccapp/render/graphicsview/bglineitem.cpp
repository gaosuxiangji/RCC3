#include "bglineitem.h"
#include <QPen>
#include <QPainter>

void BLineItem::updateInfo(const QPoint& cspt, const QRectF& rect)
{
	m_center_cursor_point = cspt;
	if (m_is_acquiring) {
		m_center_cursor_point = QPoint(abs(m_device_roi.x() - cspt.x()), abs(m_device_roi.y() - cspt.y()));
	}
	m_center_line_rect = rect;
	update();
}

void BLineItem::setDisplayStatus(bool acquiring)
{
	m_is_acquiring = acquiring;
}

void BLineItem::setDeviceRoi(const QRectF device_roi)
{
	m_device_roi = device_roi;
}

QPoint BLineItem::getCenterCursorPt()
{
	return m_center_cursor_point;
}

void BLineItem::setDrawCenterRectEnalbe(bool enable)
{
	m_is_draw_center_rect = enable;
}

void BLineItem::setDrawCenterHLineEnalbe(bool enable)
{
	m_is_draw_center_h_line = enable;
}

void BLineItem::setDrawCenterVLineEnalbe(bool enable)
{
	m_is_draw_center_v_line = enable;
}

void BLineItem::setMinCoefficient(qreal coef)
{
	m_min_coefficient = coef;
}

void BLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = Q_NULLPTR */)
{
	if (m_draw_center_line_enable) {
		if (0 == m_center_line_rect.width() || 0 == m_center_line_rect.height()) {
			return;
		}
		painter->save();
		QPen pen;
		pen.setWidthF(3 * m_min_coefficient);
		pen.setColor(Qt::yellow);

		//ªÊ÷∆–Èœﬂ
		QVector<qreal>dashes;
		qreal space = 4;
		dashes << 3 << space;
		pen.setDashPattern(dashes);
		painter->setPen(pen);

		if ((m_is_acquiring && m_is_draw_center_h_line) || !m_is_acquiring) {
			painter->drawLine(m_center_line_rect.x(), m_center_cursor_point.y(), m_center_line_rect.x() + m_center_line_rect.width(), m_center_cursor_point.y());
		}
		if ((m_is_acquiring && m_is_draw_center_v_line) || !m_is_acquiring) {
			painter->drawLine(m_center_cursor_point.x(), m_center_line_rect.y(), m_center_cursor_point.x(), m_center_line_rect.y() + m_center_line_rect.height());
		}	
		painter->restore();

		if ((m_is_acquiring && m_is_draw_center_rect) || !m_is_acquiring) {
			painter->save();
			painter->setPen(pen);
			painter->drawRect(m_center_cursor_point.x() - 5, m_center_cursor_point.y() - 5, 10, 10);
			painter->restore();
		}
	}
}

QRectF BLineItem::boundingRect() const
{
	return	QRectF{};
}

