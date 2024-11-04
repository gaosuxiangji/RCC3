#ifndef BQGRAPHICSVIEW_H
#define BQGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QLabel>

enum ROTATE_TYPE
{
	ROTATE_0 = 0,	//旋转角度
	ROTATE_90 = 1,
	ROTATE_180 = 2,
	ROTATE_270 = 3
};

class BQGraphicsView : public QGraphicsView
{
	Q_OBJECT
public:
    BQGraphicsView(QWidget *parent = nullptr);
	~BQGraphicsView() {}

	enum ViewShowLabelType
	{
		ViewIpLabel,				//显示ip信息label
		ViewConnectStatusLabel,		//显示连接状态信息label
		ViewTimestampLabel,			//显示时间戳信息label
		ViewFrameLabel,				//显示已导出视频的帧数
		ViewLabel_Max_COUNT,		//数量
	};

	void setMousePressPos(const QPointF& point) { m_mouse_press_point = point; 
	m_mouse_press_point_scene = mapToScene(QPoint(m_mouse_press_point.x(),m_mouse_press_point.y()));
	}

	void SetViewShowLabelInfo(BQGraphicsView::ViewShowLabelType type, QVariant info);
	void SetViewShowLabelColor(BQGraphicsView::ViewShowLabelType type, QVariant info);

	void AdjustViewBaseLabel(const QSize& size);
	void UpdateQLabelStyle(const ViewShowLabelType& type, QLabel* label, int fontsize);
	int GetLabelFontSize(const QSize& size);
public:
	void zoomIn();
	void zoomOut();
	void rotateLeft(const int& angle) { //to do：暂时不支持
		Q_UNUSED(angle);
		/*rotate((-1)*(angle- m_rotate_angle));
		m_rotate_angle = angle;*/
	}
	void rotateRight(const int& angle) {
		rotate(angle- m_rotate_angle);
		m_rotate_angle = angle;
		if (isFitView) {
			this->fitView(true);
		}
	}
	int getRotate() {
		return m_rotate_angle;
	}
	void fitView(bool bg);
	void setWheelDisalble(bool disable);
#if 0
	void setWheelUpEnable(bool enable);
	void setWheelDownEnable(bool enable);
#endif // 0
	void fullView();
	void setTranslation(bool bEnable);
	void resetMousePress();


signals:
	void sigUpdateCoefficient();
private:
	void mouseMoveEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

	void paintEvent(QPaintEvent *event) override;

    void InitViewLabel();
	
	void resizeEvent(QResizeEvent *event) override;
private:
	QPointF m_mouse_press_point{0,0};
	QPointF m_mouse_press_point_scene{ 0,0 };
	int m_rotate_angle{ 0 };
	const qreal m_scale_coef_const{ 1.2 };
	QSizeF m_img_size{};
	const double zoomScale = 1.2;// 缩放比例按照1.2倍进行
	const double maxScale = 1600;//% 缩放上限 注意有%号 也就是16倍
	const double minScale = 10;//% 缩放下限 注意有%号 也就是0.1倍
	bool m_wheel_disabled{ false }; // 该标记 已经不是单纯指滚轮是否可用 基本是代替是否显示背景 此时是否可操作了
#if 0
	bool m_allow_wheel_up{ true };
	bool m_allow_wheel_down{ true };
#endif // 0

	bool m_is_mouse_pressed{ false };
	QPoint m_pos_anchor{};
	QPointF m_center_anchor{};
	bool m_bTranslation{ true };
	bool isFitView = false;

	QMap<ViewShowLabelType, QLabel*> m_mapViewLabel;    //播放控件上显示的label<label类型，QLabel*>
	QMap<ViewShowLabelType, QString> m_mapViewText;    //播放控件上显示的文字<label类型，QString>
	QMap<ViewShowLabelType, QColor> m_mapViewColor;    //播放控件上显示的颜色<label类型，QString>
	const int m_locationOffset{ 10 };    //位置偏移量
	const QSize m_viewBaseOriginalSize{ QSize(1176, 898) };    //播放控件原始尺寸大小，用于计算缩放因子
};

#endif // BQGRAPHICSVIEW_H
