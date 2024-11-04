#include "CSMeasurePageDlg.h"
#include "../Device/devicemanager.h"

CSMeasurePageDlg::CSMeasurePageDlg(QWidget *parent)
	: QWidget(parent),
	m_pMeasureCalirationDlg(new CSMeasureCalibrationDlg(this)),
	m_pPointMeasureDlg(new CSPointMeasureDlg(this)),
	ui(new Ui::CSMeasurePageDlg)
{
	ui->setupUi(this);
	initUI();
	bindSignalSlot();
}

CSMeasurePageDlg::~CSMeasurePageDlg()
{
	delete ui;
}

void CSMeasurePageDlg::EscKeyPress()
{
	if (m_pPointMeasureDlg)
	{
		m_pPointMeasureDlg->EscKeyPress();
	}
	if (m_pMeasureCalirationDlg)
	{
		m_pMeasureCalirationDlg->EscKeyPress();
	}
// 	if (m_current_device_ptr)
// 	{
// 		CMeasureLineManage::instance().setMeasureModeType(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
// 	}
}

void CSMeasurePageDlg::initUI()
{
	//添加二级界面类
	ui->verticalLayout->addWidget(m_pPointMeasureDlg);
	ui->verticalLayout->addWidget(m_pMeasureCalirationDlg);
	m_pMeasureCalirationDlg->hide();
}

void CSMeasurePageDlg::bindSignalSlot()
{
	connect(m_pMeasureCalirationDlg, &CSMeasureCalibrationDlg::signalHideSelf, this, &CSMeasurePageDlg::slotShowMeasureDlg);
	connect(m_pPointMeasureDlg, &CSPointMeasureDlg::signalHideSelf, this, &CSMeasurePageDlg::slotShowCalirationDlg);
}


void CSMeasurePageDlg::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}
	QWidget::changeEvent(event);
}


void CSMeasurePageDlg::slotShowMeasureDlg()
{
	m_pMeasureCalirationDlg->hide();
	m_pPointMeasureDlg->show();
}

void CSMeasurePageDlg::slotShowCalirationDlg()
{
	m_pPointMeasureDlg->hide();
	m_pMeasureCalirationDlg->show();
	m_pMeasureCalirationDlg->InitCalibrationInfo();
}