#pragma once

#include <QtWidgets/QWidget>
#include "ui_rangemodulemanage.h"
#include "rangeslider.h"

class RangeModuleManage : public QWidget
{
	Q_OBJECT

public:
	RangeModuleManage(QWidget *parent = Q_NULLPTR);
public:
	void setRangeLowValue(int lowValue);
	void setRangeHighValue(int highValue);
	int getLowerValue();
	int getHighValue();
	void setLineEditRangeMax(int nMax);
	void updateDisplayValue(int lowValue, int highValue);
signals:
	void signalRangeLowValue(int lowValue);
	void signalRangeHighValue(int highValue);
private:
	void initUI();
	void bind();
private slots:
	void slotLowLineEditFinished();
	void slotHighLineEditFinished();
	void slotLowerChanged(int low_value);
	void slotUpperChanged(int up_value);
private:
	RangeSlider* m_range_slider{};
	const int m_const_min_value{ 0 };
	const int m_const_max_value{ 1023 };
	int m_nLineEditRangeMax{ 255 };//编辑框编辑范围，应该依据界面显示的像素位深来设置大小，eg. 像素8，最大值应该为255，而不是1023.
	double m_dbSwitchLineScale{ 255.0 / 1023 };
private:
	Ui::RangeModuleManageClass ui;
};
