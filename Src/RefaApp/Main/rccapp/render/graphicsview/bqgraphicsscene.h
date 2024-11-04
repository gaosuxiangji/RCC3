#ifndef BQGRAPHICSSCENE_H
#define BQGRAPHICSSCENE_H

#include <QGraphicsScene>
#include "bgmeasuremanage.h"
#include <QImage>
#include <mutex>
#include <QMutex>
#include "Device/device.h"
#include "PointMeasure/CMeasureLineManage.h"

class BQGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
	typedef struct CommonParam {
		bool is_acquring{ false };
		bool is_draw_center_rect{ false };
		bool is_draw_h_line{ false };
		bool is_draw_v_line{ false };
		QPointF center_pt{};
		QRectF device_roi{};
	}CommonParam;
    BQGraphicsScene(QObject *parent = nullptr);
	~BQGraphicsScene(){}

	void clearGraphicsItem();
	void refreshScene(const Device::RoiTypes& type);
	void setCenterLinePt(const QPoint& pt);
	void setCenterLineVisible(bool bvisible);
	void setRotateValue(const int rt);
	void setWheelDisalble(bool enable);
	void setCrossLineRect(const QRectF& rect);
	void setCrossLineVisible(bool bvisible);
	void setCommonParam(const CommonParam& param);
	void setWhiteBalanceBg(bool is_white_balance_bg);
	void setBgCoef(const qreal& coef);

	void setMeasureModeType(CMeasureLineManage::TMeasureModeType mode);
	void setCalibration(CMeasureLineManage::TMeasureModeType mode);
public slots:
	void updateImage(QImage img);
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)override;
	virtual void drawBackground(QPainter *painter, const QRectF &rect)override;
	virtual void drawForeground(QPainter* painter, const QRectF& rect)override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)override;
signals:
    void updatePoint(QPointF p, QList<QPointF> list, bool isCenter);
    void createFinished();
	void updateTwoPoint(PointInfo& item);
	void updateRoiInfo(const Device::RoiTypes& type);
	void imageRectSignal(const QRect& rect);
	void sendMousePtInSceneSignal(const QPoint& point);
	void SignalCustomCrossLineCenterMovePoint(const QPoint& centerpoint);
	void SignalUpdateCustomCrossLineCenterPoint(const QPointF& centerpoint);
	void signalMousePress();
	void measureLineItemSignal(const PointInfo& item);
	void pushPointSignal(const QPoint& pt, bool cancel);
	void pushMultiPtSignal(const QPoint& pt, bool feature, bool cancel);
	void multiMeasureLineItemSignal(const MultiPointInfo& item);

	void signalPressOnePt(const QPoint& pt);
	void signalMoveOnePt(const QPoint& pt);
	void signalReleassMouse();
private:
#if 0
	void drawCrossLine(QPainter *painter, const QPen& pen, const qreal& min_value);
#endif // 0
	void drawCenterLine(QPainter *painter, const qreal& min_value);

	const qreal qCenterOffset{ 10 };
	QTransform m_transform;
	QRectF m_rcCenterArea{};
protected:
    QList<QPointF> m_list;
	bool m_is_creating_two_point{ false };
	bool m_is_begin_pressed{ false };
	bool m_is_end_pressed{ false };
	uint32_t m_measure_count{ 0 };
	QList<PointInfo>m_two_point_list{};

	std::mutex m_background_image_lock{};
	QImage m_background_image{};

	QPoint m_cursor_pt{ 0,0 };    //为了显示状态栏的坐标信息
	QRect m_image_original_size{};

	bool m_center_rect_press{ false };
	QPoint m_center_cursor_point{ 0,0 };
	bool m_center_line_visible{ false };

	int m_rotate_angle{ 0 };
	bool m_wheel_disabled{ false };
	
	QRect m_cross_line_rect{ 1,1,1,1 };
	bool m_cross_line_visible{ false };
	QPointF m_cross_cursor_pt{};

	CommonParam m_common_param{};

	QRectF m_current_rect{};

	bool m_is_white_balance_bg{ false };
	qreal m_bg_coef{ 1.0 };
	PointInfo item{};
	MultiPointInfo m_multi_item{};

	bool m_is_creating_multi_point{ false };

	CMeasureLineManage::TMeasureModeType m_MeasureMode{ CMeasureLineManage::MMT_Show };
	CMeasureLineManage::TMeasureModeType m_modeCalibration{ CMeasureLineManage::MMT_Normal };
};

#endif // BQGRAPHICSSCENE_H
