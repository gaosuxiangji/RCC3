#include "csdlgarmmanualwhitebalance.h"
#include "ui_csdlgarmmanualwhitebalance.h"
#include "Device/device.h"


CSDlgArmManualWhiteBalance::CSDlgArmManualWhiteBalance(QSharedPointer<Device> device_ptr,QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgArmManualWhiteBalance)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgArmManualWhiteBalance::~CSDlgArmManualWhiteBalance()
{
    delete ui;
}

void CSDlgArmManualWhiteBalance::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	bool bEnable = false;
	do {
		if (m_device_ptr.isNull())
		{
			break;
		}

		//获取设置范围
		HscResult res = m_device_ptr->GetArmMwbGainRange(m_gain_range);
		if (res != HSC_OK)
		{
			break;
		}
		//限定设置范围
		ui->horizontalSlider_R->setRange(m_gain_range.min, m_gain_range.max);
		ui->horizontalSlider_GR->setRange(m_gain_range.min, m_gain_range.max);
		ui->horizontalSlider_GB->setRange(m_gain_range.min, m_gain_range.max);
		ui->horizontalSlider_B->setRange(m_gain_range.min, m_gain_range.max);

		ui->spinBox_R->setRange(m_gain_range.min, m_gain_range.max);
		ui->spinBox_GR->setRange(m_gain_range.min, m_gain_range.max);
		ui->spinBox_GB->setRange(m_gain_range.min, m_gain_range.max);
		ui->spinBox_B->setRange(m_gain_range.min, m_gain_range.max);

		//获取全部增益值
		uint16_t r_gain = 0, gr_gain = 0, gb_gain = 0, b_gain = 0;
		res = m_device_ptr->GetArmMwbGain(r_gain, gr_gain, gb_gain, b_gain);
		if (res != HSC_OK)
		{
			break;
		}

		ui->horizontalSlider_R->setSliderPosition(r_gain);
		ui->horizontalSlider_GR->setSliderPosition(gr_gain); 
		ui->horizontalSlider_GB->setSliderPosition(gb_gain);
		ui->horizontalSlider_B->setSliderPosition(b_gain);

		ui->spinBox_R->setValue(r_gain);
		ui->spinBox_GR->setValue(gr_gain);
		ui->spinBox_GB->setValue(gb_gain);
		ui->spinBox_B->setValue(b_gain);


		bEnable = true;


	} while (0);

	//控件使能
	ui->horizontalSlider_R->setEnabled(bEnable);
	ui->horizontalSlider_GR->setEnabled(bEnable);
	ui->horizontalSlider_GB->setEnabled(bEnable);
	ui->horizontalSlider_B->setEnabled(bEnable);

	ui->spinBox_R->setEnabled(bEnable);
	ui->spinBox_GR->setEnabled(bEnable);
	ui->spinBox_GB->setEnabled(bEnable);
	ui->spinBox_B->setEnabled(bEnable);

}

void CSDlgArmManualWhiteBalance::applyDataFromSlider()
{
	if (m_device_ptr.isNull())
	{
		return;
	}

	//获取设备中的设定值
	uint16_t r_gain = 0, gr_gain = 0, gb_gain = 0, b_gain = 0;
	HscResult res = m_device_ptr->GetArmMwbGain(r_gain, gr_gain, gb_gain, b_gain);
	if (res != HSC_OK)
	{
		return;
	}

	//R增益应用
	uint16_t cur_r_gain = ui->horizontalSlider_R->value();
	if (cur_r_gain != r_gain)
	{
		res = m_device_ptr->ApplyArmRGain(cur_r_gain);
		if (res != HSC_OK)
		{
			cur_r_gain = r_gain;
			ui->horizontalSlider_R->setSliderPosition(r_gain);
		}

		ui->spinBox_R->setValue(cur_r_gain);
	}

	//GR增益应用
	uint16_t cur_gr_gain = ui->horizontalSlider_GR->value();
	if (cur_gr_gain != gr_gain)
	{
		res = m_device_ptr->ApplyArmGrGain(cur_gr_gain);
		if (res != HSC_OK)
		{
			cur_gr_gain = gr_gain;
			ui->horizontalSlider_GR->setSliderPosition(gr_gain);
		}

		ui->spinBox_GR->setValue(cur_gr_gain);
	}

	//GB增益应用
	uint16_t cur_gb_gain = ui->horizontalSlider_GB->value();
	if (cur_gb_gain != gb_gain)
	{
		res = m_device_ptr->ApplyArmGbGain(cur_gb_gain);
		if (res != HSC_OK)
		{
			cur_gb_gain = gb_gain;
			ui->horizontalSlider_GB->setSliderPosition(gb_gain);
		}

		ui->spinBox_GB->setValue(cur_gb_gain);
	}

	//B增益应用
	uint16_t cur_b_gain = ui->horizontalSlider_B->value();
	if (cur_b_gain != b_gain)
	{
		res = m_device_ptr->ApplyArmBGain(cur_b_gain);
		if (res != HSC_OK)
		{
			cur_b_gain = b_gain;
			ui->horizontalSlider_B->setSliderPosition(b_gain);
		}

		ui->spinBox_B->setValue(cur_b_gain);
	}
}

void CSDlgArmManualWhiteBalance::applyDataFromSpinBox()
{
	if (m_device_ptr.isNull())
	{
		return;
	}

	//获取设备中的设定值
	uint16_t r_gain = 0, gr_gain = 0, gb_gain = 0, b_gain = 0;
	HscResult res = m_device_ptr->GetArmMwbGain(r_gain, gr_gain, gb_gain, b_gain);
	if (res != HSC_OK)
	{
		return;
	}

	//R增益应用
	uint16_t cur_r_gain = ui->spinBox_R->value();
	if (cur_r_gain != r_gain)
	{
		res = m_device_ptr->ApplyArmRGain(cur_r_gain);
		if (res != HSC_OK)
		{
			cur_r_gain = r_gain;
		}
		ui->horizontalSlider_R->setSliderPosition(cur_r_gain);
		ui->spinBox_R->setValue(cur_r_gain);
	}

	//GR增益应用
	uint16_t cur_gr_gain = ui->spinBox_GR->value();
	if (cur_gr_gain != gr_gain)
	{
		res = m_device_ptr->ApplyArmGrGain(cur_gr_gain);
		if (res != HSC_OK)
		{
			cur_gr_gain = gr_gain;
		}
		ui->horizontalSlider_GR->setSliderPosition(gr_gain);
		ui->spinBox_GR->setValue(cur_gr_gain);
	}

	//GB增益应用
	uint16_t cur_gb_gain = ui->spinBox_GB->value();
	if (cur_gb_gain != gb_gain)
	{
		res = m_device_ptr->ApplyArmGbGain(cur_gb_gain);
		if (res != HSC_OK)
		{
			cur_gb_gain = gb_gain;
		}
		ui->horizontalSlider_GB->setSliderPosition(gb_gain);
		ui->spinBox_GB->setValue(cur_gb_gain);
	}

	//B增益应用
	uint16_t cur_b_gain = ui->spinBox_B->value();
	if (cur_b_gain != b_gain)
	{
		res = m_device_ptr->ApplyArmBGain(cur_b_gain);
		if (res != HSC_OK)
		{
			cur_b_gain = b_gain;
		}
		ui->horizontalSlider_B->setSliderPosition(b_gain);
		ui->spinBox_B->setValue(cur_b_gain);
	}
}

void CSDlgArmManualWhiteBalance::on_horizontalSlider_R_sliderMoved(int position)
{
	Q_UNUSED(position);
	applyDataFromSlider();
}

void CSDlgArmManualWhiteBalance::on_horizontalSlider_GR_sliderMoved(int position)
{
	Q_UNUSED(position);
	applyDataFromSlider();
}

void CSDlgArmManualWhiteBalance::on_horizontalSlider_GB_sliderMoved(int position)
{
	Q_UNUSED(position);
	applyDataFromSlider();
}

void CSDlgArmManualWhiteBalance::on_horizontalSlider_B_sliderMoved(int position)
{
	Q_UNUSED(position);
	applyDataFromSlider();
}

void CSDlgArmManualWhiteBalance::on_spinBox_R_editingFinished()
{
	applyDataFromSpinBox();
}

void CSDlgArmManualWhiteBalance::on_spinBox_GR_editingFinished()
{
	applyDataFromSpinBox();
}

void CSDlgArmManualWhiteBalance::on_spinBox_GB_editingFinished()
{
	applyDataFromSpinBox();
}

void CSDlgArmManualWhiteBalance::on_spinBox_B_editingFinished()
{
	applyDataFromSpinBox();
}

void CSDlgArmManualWhiteBalance::on_pushButton_close_clicked()
{
	close();
}
