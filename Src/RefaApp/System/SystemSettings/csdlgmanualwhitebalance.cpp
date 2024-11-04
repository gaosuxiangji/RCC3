#include "csdlgmanualwhitebalance.h"
#include "ui_csdlgmanualwhitebalance.h"
#include "Device/device.h"
#include "Device/imageprocessor.h"
#include "Common/UIUtils/uiutils.h"
static const float g_EPSINON = 0.01f;


CSDlgManualWhiteBalance::CSDlgManualWhiteBalance(QSharedPointer<Device> device_ptr ,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgManualWhiteBalance)
{
    ui->setupUi(this);
	if (!device_ptr.isNull())
	{
		m_device_ptr = device_ptr;
		m_device_ptr->getProcessor();
	}

	InitUI();
}

CSDlgManualWhiteBalance::~CSDlgManualWhiteBalance()
{
    delete ui;
}



void CSDlgManualWhiteBalance::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	if (m_device_ptr.isNull())
	{
		ui->horizontalSlider_R->setEnabled(false);
		ui->horizontalSlider_G->setEnabled(false);
		ui->horizontalSlider_B->setEnabled(false);
		return;
	}
	//获取RGB增益值
	if (m_device_ptr->isGetParamsFromDevice())
	{
		HscColorCorrectInfo info;
		if (m_device_ptr->getColorCorrectInfo(info) == HSC_OK)
		{
			m_RGain = info.r_gain_;
			m_GGain = info.g_gain_;
			m_BGain = info.b_gain_;
		}
		else
		{
			UIUtils::showErrorMsgBox(this, tr("Get color correct info failed."));
			m_RGain = 1;
			m_GGain = 1;
			m_BGain = 1;
		}
	}
	else
	{
		if (!m_processor_ptr.isNull())
		{
			m_processor_ptr->getManualGain(m_RGain, m_GGain, m_BGain);
		}
	}

	//参数同步到界面中
	ui->horizontalSlider_R->setRange(0, 400);
	ui->horizontalSlider_G->setRange(0, 400);
	ui->horizontalSlider_B->setRange(0, 400);

	ui->horizontalSlider_R->setSliderPosition(factorToPos(m_RGain));
	ui->horizontalSlider_G->setSliderPosition(factorToPos(m_GGain));
	ui->horizontalSlider_B->setSliderPosition(factorToPos(m_BGain));

	ui->lineEdit_R->setText(QString::number((double)m_RGain, 'g', 3));
	ui->lineEdit_G->setText(QString::number((double)m_GGain, 'g', 3));
	ui->lineEdit_B->setText(QString::number((double)m_BGain, 'g', 3));

}

float CSDlgManualWhiteBalance::posTofactor(int pos)
{
	float factor = 0.0;
	factor = (float)pos / 100.0f;
	if (factor >= -g_EPSINON && factor <= g_EPSINON)
	{ 
		factor = g_EPSINON;
	}
	return factor;
}


int CSDlgManualWhiteBalance::factorToPos(float factor)
{
	return factor * 100.0f;
}

void CSDlgManualWhiteBalance::on_horizontalSlider_R_valueChanged(int value)
{
	//获取位置对应的参数
	m_RGain = posTofactor(value);
	if (m_device_ptr.isNull())
	{
		return;
	}

	//应用参数
	if (m_device_ptr->isGetParamsFromDevice())
	{
		HscColorCorrectInfo info;
		if (m_device_ptr->getColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
		info.r_gain_ = m_RGain;
		if (m_device_ptr->setColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
	}
	else
	{
		if (!m_processor_ptr.isNull())
		{
			m_processor_ptr->setManualGain(m_RGain, m_GGain, m_BGain);
		}
	}

	//应用到界面
	ui->lineEdit_R->setText(QString::number((double)m_RGain, 'g', 3));
}

void CSDlgManualWhiteBalance::on_horizontalSlider_G_valueChanged(int value)
{
	//获取位置对应的参数
	m_GGain = posTofactor(value);
	if (m_device_ptr.isNull())
	{
		return;
	}

	//应用参数
	if (m_device_ptr->isGetParamsFromDevice())
	{
		HscColorCorrectInfo info;
		if (m_device_ptr->getColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
		info.g_gain_ = m_GGain;
		if (m_device_ptr->setColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
	}
	else
	{
		if (!m_processor_ptr.isNull())
		{
			m_processor_ptr->setManualGain(m_RGain, m_GGain, m_BGain);
		}
	}

	//应用到界面
	ui->lineEdit_G->setText(QString::number((double)m_GGain, 'g', 3));
}

void CSDlgManualWhiteBalance::on_horizontalSlider_B_valueChanged(int value)
{
	//获取位置对应的参数
	m_BGain = posTofactor(value);
	if (m_device_ptr.isNull())
	{
		return;
	}

	//应用参数
	if (m_device_ptr->isGetParamsFromDevice())
	{
		HscColorCorrectInfo info;
		if (m_device_ptr->getColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
		info.b_gain_ = m_BGain;
		if (m_device_ptr->setColorCorrectInfo(info) != HSC_OK)
		{
			return;
		}
	}
	else
	{
		if (!m_processor_ptr.isNull())
		{
			m_processor_ptr->setManualGain(m_RGain, m_GGain, m_BGain);
		}
	}

	//应用到界面
	ui->lineEdit_B->setText(QString::number((double)m_BGain, 'g', 3));
}
