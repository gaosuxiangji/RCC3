#include "csdlgprogressformat.h"
#include "ui_csdlgprogressformat.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
CSDlgProgressFormat::CSDlgProgressFormat(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgProgressFormat)
{
    ui->setupUi(this);
	InitUI();
	if (m_device_ptr.lock())
	{
		connect(m_device_ptr.lock().data(), &Device::SignalFormatProgress, this, &CSDlgProgressFormat::SlotUpdateProgress);
	}
}

CSDlgProgressFormat::~CSDlgProgressFormat()
{
	if (m_device_ptr.lock())
	{
		disconnect(m_device_ptr.lock().data(), &Device::SignalFormatProgress, this, &CSDlgProgressFormat::SlotUpdateProgress);
	}
	if (m_timer)
	{
		m_timer->stop();
	}
    delete ui;
}

void CSDlgProgressFormat::SlotUpdateProgress(int progress)
{
	m_elapsed_timer.start();
	ui->progressBar_format->setValue(progress);
	if (progress== ui->progressBar_format->maximum())
	{
		close();
	}
}

void CSDlgProgressFormat::SlotCheckTimeOut()
{
	if (m_elapsed_timer.hasExpired(kTimeout))
	{
		close();
	}
}

void CSDlgProgressFormat::InitUI()
{
	// 设置标题
	setWindowTitle(UIExplorer::instance().getProductName());

	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	// 去除关闭按钮
	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

	//初始化进度条
	ui->progressBar_format->setRange(0, 100);
	ui->progressBar_format->setValue(1);

	//检查进度刷新超时
	m_timer = new QTimer;
	m_timer->setInterval(50);
	QObject::connect(m_timer.data(), &QTimer::timeout, this, &CSDlgProgressFormat::SlotCheckTimeOut);
	m_timer->start();
	m_elapsed_timer.start();
}
