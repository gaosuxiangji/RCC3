#include "csdlggainsettingv3.h"
#include "ui_csdlggainsettingv3.h"
#include "Device/device.h"
#include "Device/deviceutils.h"

CSDlgGainSettingV3::CSDlgGainSettingV3(QSharedPointer<Device> device_ptr, QWidget *parent) :
	QDialog(parent),
	m_device_ptr(device_ptr),
	ui(new Ui::CSDlgGainSettingV3)
{
	ui->setupUi(this);

	InitUI();
}

CSDlgGainSettingV3::~CSDlgGainSettingV3()
{
    delete ui;
}

void CSDlgGainSettingV3::on_comboBox_analogGain_IndexChanged(int index)
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

void CSDlgGainSettingV3::on_pushButton_default_clicked()
{
	if (ui->widget_lut->IsEnabled())
	{
		ui->widget_lut->Default();
	}
}

void CSDlgGainSettingV3::on_pushButton_ok_clicked()
{
	close();
}

void CSDlgGainSettingV3::InitUI()
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
	// 增益曲线
	ui->widget_lut->setDevice(m_device_ptr);
	ui->pushButton_default->setEnabled(ui->widget_lut->IsEnabled());

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

	//连接combox信号
	connect(ui->comboBox_analogGain, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this, &CSDlgGainSettingV3::on_comboBox_analogGain_IndexChanged);
	connect(ui->slider_digital_gain, &QSlider::valueChanged, this, &CSDlgGainSettingV3::slot_digital_gain_valueChanged);
}

void CSDlgGainSettingV3::slot_digital_gain_valueChanged(int value)
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