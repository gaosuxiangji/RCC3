#include "csdlggainsettingv2.h"
#include "ui_csdlggainsettingv2.h"
#include "Device/device.h"
#include "Device/deviceutils.h"

CSDlgGainSettingV2::CSDlgGainSettingV2(QSharedPointer<Device> device_ptr,QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgGainSettingV2)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgGainSettingV2::~CSDlgGainSettingV2()
{
    delete ui;
}

void CSDlgGainSettingV2::on_comboBox_AnalogGain_currentIndexChanged(int index)
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
	m_device_ptr->setProperty(Device::PropAnalogGain, ui->comboBox_AnalogGain->itemData(index));
	m_current_index = index;
}

void CSDlgGainSettingV2::on_horizontalSlider_DigitalGain_sliderMoved(int position)
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	//同步控件,参数下发
	ui->doubleSpinBox_DigitalGain->setValue((double)position / 10);
	m_device_ptr->setProperty(Device::PropDigitalGain,position);
}

void CSDlgGainSettingV2::on_spinBox_DigitalGain_editingFinished()
{
	if (m_device_ptr.isNull())
	{
		return;
	}
	//同步控件,参数下发
	int value = ui->doubleSpinBox_DigitalGain->value();
	ui->horizontalSlider_DigitalGain->setSliderPosition(value);
	m_device_ptr->setProperty(Device::PropDigitalGain,value);
}

void CSDlgGainSettingV2::on_pushButton_ok_clicked()
{
	close();
}

void CSDlgGainSettingV2::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	//初始化选项控件并使能
	bool analog_gain_enable = false;
	bool digital_gain_enable = false;
	if (m_device_ptr)
	{
		//模拟增益初始化
		if (m_device_ptr->IsAnalogGainSupported())
		{
			//获取设备支持的模拟增益值
			QVariantList support_analogGain;
			m_device_ptr->getSupportedProperties(Device::PropAnalogGain, support_analogGain);
			//将支持的选项放入下拉框
			for (auto analog_gain_item : support_analogGain)
			{
				ui->comboBox_AnalogGain->addItem(DeviceUtils::getAnalogGainText(analog_gain_item.toInt()), analog_gain_item.toInt());
			}
			//选中当前选项
			QVariant current_analog_gain;
			current_analog_gain = m_device_ptr->getProperty(Device::PropAnalogGain);
			m_current_index = ui->comboBox_AnalogGain->findData(current_analog_gain.toInt());
			if (m_current_index > -1)
			{
				ui->comboBox_AnalogGain->setCurrentIndex(m_current_index);
			}
			if (ui->comboBox_AnalogGain->count() > 1)
			{
				analog_gain_enable = true;
			}
		}

		//数字增益初始化
		if (m_device_ptr->IsDigitalGainSupported())
		{
			qint64 min = 0, max = 0, inc = 0;
			m_device_ptr->getPropertyRange(Device::PropDigitalGain, min, max, inc);
			int value = m_device_ptr->getProperty(Device::PropDigitalGain).toInt();
			//控件参数配置
			ui->horizontalSlider_DigitalGain->setRange(min, max);
			ui->horizontalSlider_DigitalGain->setSliderPosition(value);
			ui->doubleSpinBox_DigitalGain->setRange((double)min / 10, (double)max / 10);
			ui->doubleSpinBox_DigitalGain->setValue((double)value / 10);

			digital_gain_enable = true;
		}

	}
	ui->comboBox_AnalogGain->setEnabled(analog_gain_enable);
	ui->horizontalSlider_DigitalGain->setEnabled(digital_gain_enable);
	ui->doubleSpinBox_DigitalGain->setEnabled(digital_gain_enable);
}
