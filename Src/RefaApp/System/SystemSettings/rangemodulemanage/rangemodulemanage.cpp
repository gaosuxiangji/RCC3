#include "rangemodulemanage.h"
#include <QHBoxLayout>
#include <QRegExpValidator>
#include <QDebug>
#include <math.h>

RangeModuleManage::RangeModuleManage(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initUI();
	bind();
}

void RangeModuleManage::setRangeLowValue(int lowValue)
{
	if (lowValue != m_range_slider->GetLowerValue()) {
		m_range_slider->SetLowerValue(lowValue);
	}
}

void RangeModuleManage::setRangeHighValue(int highValue)
{
	if (highValue != m_range_slider->GetUpperValue()) {
		m_range_slider->SetUpperValue(highValue);
	}
}

int RangeModuleManage::getLowerValue()
{
	return m_range_slider->GetLowerValue();
}

int RangeModuleManage::getHighValue()
{
	return m_range_slider->GetUpperValue();
}

void RangeModuleManage::initUI()
{
	setWindowFlags(Qt::FramelessWindowHint);
	m_range_slider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
	m_range_slider->setMaximum(m_const_max_value);
	m_range_slider->setMinimum(m_const_min_value);
	m_range_slider->SetLowerValue(m_const_min_value);
	m_range_slider->SetUpperValue(m_const_max_value);
	m_range_slider->SetTracking(false);
	QHBoxLayout* hl = new QHBoxLayout(ui.slider_widget);
	hl->setContentsMargins(0, 0, 0, 0);
	hl->addWidget(m_range_slider);

	QRegExp regx("[0-9]{1,5}");
	QRegExpValidator* validator = new QRegExpValidator(regx);
	ui.low_value_lineEdit->setValidator(validator);
	ui.high_value_lineEdit->setValidator(validator);

	ui.low_value_lineEdit->setText(QString::number(m_const_min_value));
	int nValue = std::round(m_const_max_value*m_dbSwitchLineScale);
	ui.high_value_lineEdit->setText(QString::number(nValue));
}

void RangeModuleManage::bind()
{
	bool ok = connect(ui.low_value_lineEdit, &QLineEdit::editingFinished, this, &RangeModuleManage::slotLowLineEditFinished);
	ok = connect(ui.high_value_lineEdit, &QLineEdit::editingFinished, this, &RangeModuleManage::slotHighLineEditFinished);
	ok = connect(m_range_slider, &RangeSlider::lowerChandedFinished, this, &RangeModuleManage::slotLowerChanged);
	ok = connect(m_range_slider, &RangeSlider::upperChandedFinished, this, &RangeModuleManage::slotUpperChanged);
	ok = connect(m_range_slider, &RangeSlider::lowerValueChanged, this, &RangeModuleManage::slotLowerChanged);
	ok = connect(m_range_slider, &RangeSlider::upperValueChanged, this, &RangeModuleManage::slotUpperChanged);
}

void RangeModuleManage::slotLowLineEditFinished()
{
	int low_value = ui.low_value_lineEdit->text().toInt();
	int high_value = ui.high_value_lineEdit->text().toInt();

	if (low_value > high_value) {
		low_value = high_value;
	}

	if (low_value > m_nLineEditRangeMax) {
		low_value = m_nLineEditRangeMax;
	}

	qDebug() << "slotLowLineEditFinished:" << low_value;
	low_value = std::round(low_value / m_dbSwitchLineScale);
	qDebug() << "slotLowLineEditFinished: switch = " << low_value;
	if (low_value != m_range_slider->GetLowerValue()) {
		m_range_slider->SetLowerValue(low_value);
	}
	
}

void RangeModuleManage::slotHighLineEditFinished()
{
	int low_value = ui.low_value_lineEdit->text().toInt();
	int high_value = ui.high_value_lineEdit->text().toInt();

	if (low_value > high_value) {
		high_value = low_value ;
	}

	if (high_value > m_nLineEditRangeMax) {
		high_value = m_nLineEditRangeMax;
	}
	qDebug() << "slotHighLineEditFinished:" << high_value;
	high_value = std::round(high_value / m_dbSwitchLineScale);
	qDebug() << "slotHighLineEditFinished:" << high_value;
	if (high_value != m_range_slider->GetUpperValue()) {
		m_range_slider->SetUpperValue(high_value);
	}
}

void RangeModuleManage::slotLowerChanged(int low_value)
{
	int nEditValue = std::round(low_value*m_dbSwitchLineScale);
	ui.low_value_lineEdit->setText(QString::number(nEditValue));
	emit signalRangeLowValue(low_value);
}

void RangeModuleManage::slotUpperChanged(int up_value)
{
	int nEditValue = std::round(up_value*m_dbSwitchLineScale);
	ui.high_value_lineEdit->setText(QString::number(nEditValue));
	emit signalRangeHighValue(up_value);
}

void RangeModuleManage::setLineEditRangeMax(int nMax)
{
	m_nLineEditRangeMax = nMax;
	m_dbSwitchLineScale = m_nLineEditRangeMax * 1.0 / m_const_max_value;
	int nValue = std::round(m_const_max_value*m_dbSwitchLineScale);
	ui.high_value_lineEdit->setText(QString::number(nValue));
}

void RangeModuleManage::updateDisplayValue(int lowValue, int highValue)
{
	if (m_range_slider)
	{
		m_range_slider->SetLowerValue(lowValue, false);
		m_range_slider->SetUpperValue(highValue, false);
	}
	int nEditValue = std::round(lowValue*m_dbSwitchLineScale);
	ui.low_value_lineEdit->setText(QString::number(nEditValue));
	nEditValue = std::round(highValue*m_dbSwitchLineScale);
	ui.high_value_lineEdit->setText(QString::number(nEditValue));
}
