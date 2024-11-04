#pragma once
#include <QGraphicsItem>
#include <QWidget>

class BLineItem : public QGraphicsItem {
public:
	BLineItem(){}
	~BLineItem(){}
public:
	void updateInfo(const QPoint& cspt, const QRectF& rect);
	void drawCenterLineEnalbe(bool beable) { 
		m_draw_center_line_enable = beable; 
		update();
	}
	void setDisplayStatus(bool acquiring);
	void setDeviceRoi(const QRectF device_roi);
	QPoint getCenterCursorPt();
	void setDrawCenterRectEnalbe(bool enable);
	void setDrawCenterHLineEnalbe(bool enable);
	void setDrawCenterVLineEnalbe(bool enable);
	void setMinCoefficient(qreal coef);
private:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = Q_NULLPTR */) override;
	virtual QRectF boundingRect() const;
private:
	bool m_draw_center_line_enable{ false };
	QRectF m_center_line_rect{ 0,0,0,0 };
	QPoint m_center_cursor_point{ 0,0 };
	bool m_is_acquiring{ false };
	QRectF m_device_roi{};
	bool m_is_draw_center_rect{ false };
	bool m_is_draw_center_h_line{ false };
	bool m_is_draw_center_v_line{ false };
	qreal m_min_coefficient{ 1 };
};
