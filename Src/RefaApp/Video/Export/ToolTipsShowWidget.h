#pragma once

#include <QDialog>
#include "ui_ToolTipsShowWidget.h"
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QWheelEvent>


class ToolTipsShowWidget : public QDialog
{
	Q_OBJECT

public:
	ToolTipsShowWidget(QDialog *parent = Q_NULLPTR);
	~ToolTipsShowWidget();

	void setRectAndText(QRect rect, QString strText);
private:
	void initUI();

	/**
	*@brief 鼠标按下事件响应
	*@param [in] : event : QMouseEvent *，事件指针
	**/
	virtual void mousePressEvent(QMouseEvent *event) override;

private:
	Ui::ToolTipsShowWidget ui;
};
