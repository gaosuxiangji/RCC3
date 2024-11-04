#include "csdlggainsettingv4.h"
#include "ui_csdlggainsettingv4.h"
#include "Device/device.h"
#include "Device/deviceutils.h"

CSDlgGainSettingV4::CSDlgGainSettingV4(QSharedPointer<Device> device_ptr, QWidget *parent) :
	QDialog(parent),
	m_device_ptr(device_ptr),
	ui(new Ui::CSDlgGainSettingV4)
{
	ui->setupUi(this);

	InitUI();
}

CSDlgGainSettingV4::~CSDlgGainSettingV4()
{
    delete ui;
}

void CSDlgGainSettingV4::on_comboBox_analogGain_IndexChanged(int index)
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	if (m_current_index == index)
	{
		return;
	}
	//设置当前选项
	m_device_ptr->setProperty(Device::PropAnalogGain, ui->comboBox_analogGain->itemData(index));
	m_current_index = index;
}

void CSDlgGainSettingV4::on_pushButton_default_clicked()
{
	if (m_range_module_manage)
	{
		m_range_module_manage->setRangeLowValue(0);
		m_range_module_manage->setRangeHighValue(1023);
	}
	if (ui->widget_lut->IsEnabled())
	{
		ui->widget_lut->Default();
	}
}

void CSDlgGainSettingV4::on_pushButton_ok_clicked()
{
	close();
}

void CSDlgGainSettingV4::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	if (m_device_ptr.isNull())
	{
		return;
	}

	//判断是否支持模拟增益
	if (m_device_ptr->IsAnalogGainSupported())
	{

		//获取设备支持的模拟增益值
		QVariantList support_analogGain;
		m_device_ptr->getSupportedProperties(Device::PropAnalogGain, support_analogGain);
		auto values = m_device_ptr->getAnalogain();
		//将支持的选项放入下拉框
		for (auto analog_gain_item : support_analogGain)
		{
			int nValue = analog_gain_item.toInt();
			QString strText;
			if (nValue >= 0 && nValue < 4)
			{
				strText = values[nValue];
			}
			else
			{
				strText = QString::number(nValue);
			}
			ui->comboBox_analogGain->addItem(strText, nValue);
		}
		//选中当前选项
		QVariant current_analog_gain;
		current_analog_gain = m_device_ptr->getProperty(Device::PropAnalogGain);
		m_current_index = ui->comboBox_analogGain->findData(current_analog_gain.toInt());
		if (m_current_index > -1)
		{
			ui->comboBox_analogGain->setCurrentIndex(m_current_index);
		}
		if (ui->comboBox_analogGain->count() <= 1)
		{
			ui->comboBox_analogGain->setEnabled(false);
		}
	}
	else
	{
		ui->comboBox_analogGain->setEnabled(false);
	}

	m_range_module_manage = new RangeModuleManage(ui->range_widget);
	QHBoxLayout* range_hl = new QHBoxLayout(ui->range_widget);
	range_hl->setContentsMargins(0, 0, 0, 0);
	range_hl->addWidget(m_range_module_manage);

	// 增益曲线
	ui->widget_lut->setDevice(m_device_ptr);
	ui->pushButton_default->setEnabled(false);
	ui->pushButton_default->setAutoDefault(false);
	ui->pushButton_ok->setAutoDefault(false);
	// 数字增益
	ui->lineEdit_digitalGain->setEnabled(false);
	ui->slider_digital_gain->setRange(0, constDigitalMax - 1);
	ui->slider_digital_gain->setSingleStep(1);
	int nValue = m_device_ptr->getProperty(Device::PropCmosDigitalVal).toInt();
	if (nValue >= 0 && nValue < constDigitalMax)
	{
		ui->lineEdit_digitalGain->setText(QString::number(constDIGITALGAIN[nValue], 'f', 4));
		ui->slider_digital_gain->setValue(nValue);
	}
	else
	{
		ui->lineEdit_digitalGain->setText(QString::number(constDIGITALGAIN[0], 'f', 4));
		ui->slider_digital_gain->setValue(0);
	}
	int nBitsPerPixel = m_device_ptr->getProperty(Device::PropPixelBitDepth).toInt();
	m_range_module_manage->setLineEditRangeMax(std::pow(2, nBitsPerPixel) - 1);
	updateDisplayValue();
	//连接combox信号
	connect(ui->comboBox_analogGain, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this, &CSDlgGainSettingV4::on_comboBox_analogGain_IndexChanged);

	connect(ui->slider_digital_gain, &QSlider::valueChanged, this, &CSDlgGainSettingV4::slot_digital_gain_valueChanged);

	connect(m_range_module_manage, &RangeModuleManage::signalRangeLowValue, this, [=](int lowValue) {
		ui->widget_lut->updatePtInfo(QPoint(lowValue, 0), QPoint(m_range_module_manage->getHighValue(), 1023)); });

	connect(m_range_module_manage, &RangeModuleManage::signalRangeHighValue, this, [=](int highValue) {
		ui->widget_lut->updatePtInfo(QPoint(m_range_module_manage->getLowerValue(), 0), QPoint(highValue, 1023)); });

	connect(ui->widget_lut, &CSLutCurveEditWidget::valueChanged, this, [=]() {
		updateDisplayValue(); });
	ui->pushButton_default->setEnabled(ui->widget_lut->IsEnabled());
}

void CSDlgGainSettingV4::slot_digital_gain_valueChanged(int value)
{
	if (value >= 0 && value < constDigitalMax)
	{
		ui->lineEdit_digitalGain->setText(QString::number(constDIGITALGAIN[value], 'f', 4));

		if (m_device_ptr)
		{
			DeviceState state = m_device_ptr->getState();
			if (state == DeviceState::Connected || state == DeviceState::Previewing || state == DeviceState::Acquiring)
			{
				m_device_ptr->setProperty(Device::PropCmosDigitalVal, value);
			}
		}
	}
}
void CSDlgGainSettingV4::updateDisplayValue()
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	QPoint lower_pt, upper_pt;
	ui->widget_lut->getPtInfo(lower_pt, upper_pt);

	if (m_range_module_manage)
	{
		m_range_module_manage->updateDisplayValue(lower_pt.x(), upper_pt.x());
	}
}