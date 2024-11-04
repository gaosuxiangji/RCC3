#include "acqparamdiagram.h"
#include "ui_acqparamdiagram.h"

#include <QPainter>


AcqParamDiagram::AcqParamDiagram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AcqParamDiagram),
	trigger_lable_img(QPixmap(":/images/trigger_lable.png"))
{
    ui->setupUi(this);
	
}

AcqParamDiagram::~AcqParamDiagram()
{
    delete ui;
}

void AcqParamDiagram::drawDiagram(RecordOffsetMode offset_mode, int offset, int record_length)
{
	offset_mode_ = offset_mode;

	if (offset > -1)
	{
		offset_ = offset;
	}
	if (record_length > 0)
	{
		record_length_ = record_length;
	}
	


	MakePaintBuffer();

}

void AcqParamDiagram::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);

	if (isFirstPaint)
	{
		paintBuffer_backup = QPixmap(this->size());
		paintBuffer_backup.fill(this->palette().window().color());
		MakePaintBuffer();
		isFirstPaint = false;
	}

	painter.drawPixmap(0, 0, paintBuffer);
}

void AcqParamDiagram::MakePaintBuffer()
{
	paintBuffer = paintBuffer_backup;

	QPainter painter(&paintBuffer);
	painter.setRenderHint(QPainter::Antialiasing);

	QRect rc = this->rect();

	QPen pen(Qt::gray, 1);
	painter.setPen(pen);
	//painter.fillRect(rc, Qt::white);

	//绘制采集过程坐标轴(固定位置)
	float axisRectLeft = rc.width()*paddingWidthStretch;
	float axisRectRight = rc.width()*(1 - paddingWidthStretch);
	float axisRectTop = rc.height()*acqAreaHeightStretch;
	float axisRectBottom = rc.height()*(acqAreaHeightStretch + axisAreaHeightStretch);
	
	QPointF axisRectTopLeft(axisRectLeft, axisRectTop);
	QPointF axisRectBottomRight(axisRectRight, axisRectBottom);

	QRectF axisRect(axisRectTopLeft, axisRectBottomRight);
	painter.fillRect(axisRect, Qt::gray);
	painter.drawRect(axisRect);

	//计算采集区域和触发指针的位置
	float acqRectLeft = 0.f;
	float acqRectRight = 0.f;
	float triggerRectCenterX = 0.f;
	if (offset_mode_ == BEFORE_SHUTTER)//触发前:采集区域左侧靠左
	{
		acqRectLeft = axisRectLeft;
		if (offset_ < record_length_)//起点偏移小于录制长度,采集区域右侧靠右
		{
			acqRectRight = axisRectRight;
			//总长度为采集长度,等比例换算触发指针位置
			triggerRectCenterX = axisRectLeft + offset_ * (axisRect.width() / record_length_);

		}
		else//触发指针位置靠右
		{
			triggerRectCenterX = axisRectRight;
			//总长度为起点偏移,等比例换算采集区域右侧位置
			acqRectRight = axisRectLeft + record_length_ * (axisRect.width() / offset_);
		}
	}
	else//触发后:触发指针位置靠左,采集区域右侧靠右
	{
		triggerRectCenterX = axisRectLeft;
		acqRectRight = axisRectRight;
		//总长度为起点偏移加采集长度,等比例换算采集区域左侧位置
		acqRectLeft = axisRectLeft + offset_ * (axisRect.width() / (offset_ + record_length_));
	}

	//绘制采集区域
	float acqRectBottom = rc.height()*acqAreaHeightStretch;

	QPointF acqRectTopLeft(acqRectLeft, 1);
	QPointF acqRectBottomRight(acqRectRight, acqRectBottom);
	QRectF acqRect(acqRectTopLeft, acqRectBottomRight);

	painter.drawRect(acqRect);

	//绘制触发指针
	float triggerLableRectHeight = rc.height()*triggerLabelHeightStretch;
	float triggerLableRcetWidth = triggerLableRectHeight * 0.5f;

	QPointF triggerLableRcetTopLeft(triggerRectCenterX - 0.5f * triggerLableRcetWidth, axisRectBottom + 1);
	QSizeF triggerLableRcetSize(triggerLableRcetWidth, triggerLableRectHeight);

	QRectF triggerLableRcet(triggerLableRcetTopLeft, triggerLableRcetSize);

	trigger_lable_img = trigger_lable_img.scaledToWidth(triggerLableRcetWidth);

	painter.drawPixmap(triggerLableRcetTopLeft,trigger_lable_img);

	update();
}
