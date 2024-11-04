#include "ToolTipsShowWidget.h"
#include <QPainter>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QStyle>
#include <QPalette>

ToolTipsShowWidget::ToolTipsShowWidget(QDialog *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	initUI();

	setMouseTracking(true);
	setStyleSheet("QDialog{border:1px solid red;background-color:rgb(255,204,153)}");
	//resize(windowWidth_, windowHeight_);
	//ui.pushButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
	ui.pushButton->setStyleSheet("background-color:transparent");
	QTimer::singleShot(1500, this, [=] {
		accept();
	});
}

ToolTipsShowWidget::~ToolTipsShowWidget()
{
}

void ToolTipsShowWidget::initUI()
{
	this->setWindowFlags(Qt::Popup);
}


void ToolTipsShowWidget::mousePressEvent(QMouseEvent *event)
{
	setAttribute(Qt::WA_NoMouseReplay);
	QDialog::mousePressEvent(event);
}

void ToolTipsShowWidget::setRectAndText(QRect rect, QString strText)
{
	move(rect.x(), rect.y());
	resize(rect.width(), rect.height());
	ui.label_text->setText(strText);
}