#ifndef WINDOWDRAGGER_H_
#define WINDOWDRAGGER_H_

#include <QMouseEvent>
#include <QWidget>

/**
*@brief 可拖动标题栏界面类
**/
class WindowDragger : public QWidget {
	Q_OBJECT

public:
	explicit WindowDragger(QWidget *parent = Q_NULLPTR);
	virtual ~WindowDragger() {}

signals:
	/**
	*@brief 双击标题栏信号
	**/
	void doubleClicked();

protected:
	/**
	*@brief 鼠标按下事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mousePressEvent(QMouseEvent *event);

	/**
	*@brief 鼠标移动事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mouseMoveEvent(QMouseEvent *event);

	/**
	*@brief 鼠标释放事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mouseReleaseEvent(QMouseEvent *event);

	/**
	*@brief 鼠标双击事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mouseDoubleClickEvent(QMouseEvent *event);

	/**
	*@brief 绘制事件
	*@param [in] : *event : QPaintEvent，事件指针
	**/
	void paintEvent(QPaintEvent *event);

protected:
	QPoint mousePos;
	QPoint wndPos;
	bool mousePressed;
};

#endif  // WINDOWDRAGGER_H_
