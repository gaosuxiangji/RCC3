#ifndef FRAMELESSWINDOW_H_
#define FRAMELESSWINDOW_H_

#include <QWidget>

namespace Ui {
	class FramelessWindow;
}

/**
*@brief 无边框界面类
**/
class FramelessWindow : public QWidget {
	Q_OBJECT

public:
	explicit FramelessWindow(QWidget *parent = Q_NULLPTR);
	virtual ~FramelessWindow();

	/**
	*@brief 设置显示内容控件
	*@param [in] : w : QWidget*，控件指针
	**/
	void setContent(QWidget *w);

private:
	/**
	*@brief 是否选中左边框
	*@param [in] : &pos : const QPoint，相对屏幕的鼠标坐标
	*@return : bool : true-选中，false-未选中
	**/
	bool leftBorderHit(const QPoint &pos);

	/**
	*@brief 是否选中右边框
	*@param [in] : &pos : const QPoint，相对屏幕的鼠标坐标
	*@return : bool : true-选中，false-未选中
	**/
	bool rightBorderHit(const QPoint &pos);

	/**
	*@brief 是否选中上边框
	*@param [in] : &pos : const QPoint，相对屏幕的鼠标坐标
	*@return : bool : true-选中，false-未选中
	**/
	bool topBorderHit(const QPoint &pos);

	/**
	*@brief 是否选中下边框
	*@param [in] : &pos : const QPoint，相对屏幕的鼠标坐标
	*@return : bool : true-选中，false-未选中
	**/
	bool bottomBorderHit(const QPoint &pos);

public slots:
	/**
	*@brief 设置标题名称
	*@param [in] : text : const QString &，标题
	**/
	void setWindowTitle(const QString &text);

	/**
	*@brief 设置图标
	*@param [in] : ico : const QIcon &，图标
	**/
	void setWindowIcon(const QIcon &ico);

private slots:
	/**
	*@brief 最小化按钮响应槽函数
	**/
	void on_minimizeButton_clicked();

	/**
	*@brief 最大化按钮响应槽函数
	**/
	void on_maximizeButton_clicked();

	/**
	*@brief 关闭按钮响应槽函数
	**/
	void on_closeButton_clicked();

	/**
	*@brief 双击标题栏响应槽函数
	**/
	void on_windowTitlebar_doubleClicked();

protected:
	/**
	*@brief 改变事件
	*@param [in] : *event : QEvent，事件指针
	**/
	virtual void changeEvent(QEvent *event) override;

	/**
	*@brief 鼠标双击事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

	/**
	*@brief 边框拖动事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	virtual void checkBorderDragging(QMouseEvent *event);

	/**
	*@brief 鼠标按下事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	virtual void mousePressEvent(QMouseEvent *event) override;

	/**
	*@brief 鼠标释放事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	virtual void mouseReleaseEvent(QMouseEvent *event) override;

	/**
	*@brief 事件过滤器
	*@param [in] : *obj : QObject，对象指针
				   *event : QEvent，事件指针
	**/
	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	/**
	*@brief 关闭事件
	*@param [in] : *event : QCloseEvent，事件指针
	**/
	virtual void closeEvent(QCloseEvent *event) override;

private:
	Ui::FramelessWindow *ui;
	QRect m_StartGeometry;
	const quint8 CONST_DRAG_BORDER_SIZE = 5;
	bool m_bMousePressed;
	bool m_bDragTop;
	bool m_bDragLeft;
	bool m_bDragRight;
	bool m_bDragBottom;
};

#endif  // FRAMELESSWINDOW_H_
