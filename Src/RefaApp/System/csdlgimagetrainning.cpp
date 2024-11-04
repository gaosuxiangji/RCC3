#include "csdlgimagetrainning.h"
#include "ui_csdlgimagetrainning.h"
#include "Device/device.h"
#include <thread>
CSDlgImageTrainning::CSDlgImageTrainning(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgImageTrainning)
{
    ui->setupUi(this);
	//setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	//去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//测试模式和图像模式按钮互斥
	m_button_group_test_mode = new QButtonGroup();
	m_button_group_test_mode->addButton(ui->pushButtonTestMode,1);
	m_button_group_test_mode->addButton(ui->pushButtonImageMode,0);
	m_button_group_test_mode->setExclusive(true);

	//连接按钮组操作
	connect(m_button_group_test_mode, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &CSDlgImageTrainning::slotTestModeButtonsClicked);

	//连接任务结束操作
	connect(this, &CSDlgImageTrainning::signalTaskFinished, this, &CSDlgImageTrainning::slotTaskFinished);
}

CSDlgImageTrainning::~CSDlgImageTrainning()
{
    delete ui;
}

void CSDlgImageTrainning::setDevice(QSharedPointer<Device> device)
{
	m_device = device;

}

void CSDlgImageTrainning::closeEvent(QCloseEvent *e)
{
	startTask(std::bind(&CSDlgImageTrainning::doEnableImageTraining, this, false, true));
}

void CSDlgImageTrainning::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QWidget::changeEvent(event);
}

void CSDlgImageTrainning::on_pushButtonMainBoard_clicked()
{
	startTask(std::bind(&CSDlgImageTrainning::doImageTraining, this, true));
}

void CSDlgImageTrainning::on_pushButtonSlaveBoard_clicked()
{
	startTask(std::bind(&CSDlgImageTrainning::doImageTraining, this, false));
}

void CSDlgImageTrainning::slotTestModeButtonsClicked(int id)
{
	//判断是否是测试模式
	m_button_group_test_mode->button(id)->setChecked(true);
	if (id == 1 )//开启测试模式
	{
		startTask(std::bind(&CSDlgImageTrainning::doEnableImageTraining, this, true, false));
	}
	else//关闭测试模式
	{
		startTask(std::bind(&CSDlgImageTrainning::doEnableImageTraining, this, false, false));
	}
}

void CSDlgImageTrainning::slotTaskFinished(bool hide_window)
{
	//恢复关闭按钮
//	setWindowFlags(windowFlags() | Qt::WindowCloseButtonHint);

	//恢复界面使能
	ui->pushButtonImageMode->setEnabled(true);
	ui->pushButtonImageMode->setChecked(!m_device->isImagingTrainingEnabled());
	ui->pushButtonTestMode->setEnabled(true);
	ui->pushButtonTestMode->setChecked(m_device->isImagingTrainingEnabled());
	ui->pushButtonMainBoard->setEnabled(true);
	ui->pushButtonSlaveBoard->setEnabled(true);

	if (hide_window)
	{
		hide();
	}
}

void CSDlgImageTrainning::startTask(Task task)
{
	//去除关闭按钮
//	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
	//全部按钮禁用
	ui->pushButtonImageMode->setEnabled(false);
	ui->pushButtonTestMode->setEnabled(false);
	ui->pushButtonMainBoard->setEnabled(false);
	ui->pushButtonSlaveBoard->setEnabled(false);

	//开始任务
	std::thread(task).detach();
}

void CSDlgImageTrainning::doEnableImageTraining(bool enable, bool hide)
{
	if (m_device)
	{
		m_device->enableImageTraining(enable);
	}
	emit signalTaskFinished(hide);
}

void CSDlgImageTrainning::doImageTraining(bool master)
{
	if (m_device)
	{
		if (master)
		{
			m_device->mainBoardPhaseAdjust();
		}
		else
		{
			m_device->slvPhaseAdjust();
		}
	}
	emit signalTaskFinished(false);

}
