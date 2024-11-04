#include "csdlgluminanceandcontrastsetting.h"
#include "ui_csdlgluminanceandcontrastsetting.h"
#include "Device/device.h"

CSDlgLuminanceAndContrastSetting::CSDlgLuminanceAndContrastSetting(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgLuminanceAndContrastSetting)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgLuminanceAndContrastSetting::~CSDlgLuminanceAndContrastSetting()
{
    delete ui;
}


void CSDlgLuminanceAndContrastSetting::InitUI()
{	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//判断使能,读取设备亮度对比度,刷新界面
	bool enable = false;
	int luminance = (MAX_LUMINANCE + MIN_LUMINANCE) / 2;
	int contrast = (MAX_CONTRAST + MIN_CONTRACT) / 2;
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{
		if (device_ptr->getState() == Previewing)
		{
			enable = true;
		}
		luminance = device_ptr->getProperty(Device::PropLuminance).toInt();
		contrast = device_ptr->getProperty(Device::PropContrast).toInt();
	}

	if (luminance < MIN_LUMINANCE)
	{
		luminance = MIN_LUMINANCE;
	}
	if (luminance > MAX_LUMINANCE)
	{
		luminance = MAX_LUMINANCE;
	}

	if (contrast < MIN_CONTRACT)
	{
		contrast = MIN_CONTRACT;
	}
	if (contrast > MAX_CONTRAST)
	{
		contrast = MAX_CONTRAST;
	}

	ui->horizontalSlider_Luminance->setRange(MIN_LUMINANCE, MAX_LUMINANCE);
	ui->horizontalSlider_Contrast->setRange(MIN_CONTRACT, MAX_CONTRAST);
	ui->horizontalSlider_Luminance->setSliderPosition(luminance);
	ui->horizontalSlider_Contrast->setSliderPosition(contrast);

	ui->spinBox_Luminance->setRange(MIN_LUMINANCE, MAX_LUMINANCE);
	ui->spinBox_Contrast->setRange(MIN_CONTRACT, MAX_CONTRAST);
	ui->spinBox_Luminance->setValue(luminance);
	ui->spinBox_Contrast->setValue(contrast);

	ui->horizontalSlider_Contrast->setEnabled(enable);
	ui->horizontalSlider_Luminance->setEnabled(enable);
	ui->spinBox_Contrast->setEnabled(enable);
	ui->spinBox_Luminance->setEnabled(enable);

}

void CSDlgLuminanceAndContrastSetting::on_pushButton_clicked()
{
	close();
}


void CSDlgLuminanceAndContrastSetting::on_horizontalSlider_Luminance_valueChanged(int value)
{
	//设置到设备中,同步spinbox
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropLuminance, value);
	}
	ui->spinBox_Luminance->setValue(value);
}

void CSDlgLuminanceAndContrastSetting::on_horizontalSlider_Contrast_valueChanged(int value)
{
	//设置到设备中,同步spinbox
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropContrast, value);
	}
	ui->spinBox_Contrast->setValue(value);
}

void CSDlgLuminanceAndContrastSetting::on_spinBox_Luminance_valueChanged(int arg1)
{
	//设置到设备中,同步slider
	int value = ui->spinBox_Luminance->value();
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropLuminance, value);
	}
	ui->horizontalSlider_Luminance->setValue(value);
}

void CSDlgLuminanceAndContrastSetting::on_spinBox_Contrast_valueChanged(int arg1)
{
	//设置到设备中,同步slider
	int value = ui->spinBox_Contrast->value();
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropContrast, value);
	}
	ui->horizontalSlider_Contrast->setValue(value);
}
