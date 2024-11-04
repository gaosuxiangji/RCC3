#include "csdlgselfcheck.h"
#include "ui_csdlgselfcheck.h"
#include "Device/device.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/AgErrorCode/agerrorcode.h"
CSDlgSelfCheck::CSDlgSelfCheck(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgSelfCheck)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgSelfCheck::~CSDlgSelfCheck()
{
    delete ui;
}

void CSDlgSelfCheck::processSelfCheck()
{
	//执行单设备的自检操作,定时刷新界面进度条和显示内容
	int n = m_device_list.size();
	if (m_current_device_index >= n)
	{
		m_timer_ptr->stop();
		ui->textEditSelfCheckResults->append(tr("Self-check End."));
		return;
	}

	QString str;
	ErrorInfo errors[MAX_SYSTEM_SELF_CHECK_COUNT];
	auto device_ptr = m_device_list.at(m_current_device_index);
	bool isOK = true;
	QString ipOrSn = device_ptr->getIpOrSn();
	DeviceState state = device_ptr->getState();
	if (state == Connected || state == Previewing || state == Acquiring || state == Recording)
	{
		int nError = device_ptr->SystemSelfCheck(errors);
		for (int i = 0; i < nError; i++)
		{
			AgErrorCodeInfo errorCodeInfo;
			AgErrorCode::instance().get(errors[i].error_code_, errorCodeInfo);
			str = tr("The Device(%1) failed: errorcode(0x%2) %3.")
				.arg(ipOrSn)
				.arg(QString::number(errors[i].error_code_,16))
				.arg(errorCodeInfo.desc);
			ui->textEditSelfCheckResults->append(str);
			isOK = false;
		}
	}
	else
	{
		str = tr("The Device(%1) Disconnected.").arg(ipOrSn);
		ui->textEditSelfCheckResults->append(str);
		isOK = false;
	}

	if (isOK)
	{
		str = tr("The Device(%1) Normal.").arg(ipOrSn);
		ui->textEditSelfCheckResults->append(str);
	}

	m_current_device_index++;
	ui->progressBar->setValue(m_current_device_index);

}

void CSDlgSelfCheck::InitUI()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//初始化界面显示,开启刷新设备状态计时器
	//DeviceManager::instance().getDevices(m_device_list);
	DeviceManager::instance().getManageDevices(m_device_list);
	if (m_device_list.size() == 0)
	{
		ui->progressBar->setRange(-1, 0);
	}
	else {
		ui->progressBar->setRange(0, m_device_list.size());
	}
	ui->progressBar->setValue(0);

	ui->textEditSelfCheckResults->setPlainText(tr("Self-check Start..."));
	m_timer_ptr = new QTimer();
	connect(m_timer_ptr, &QTimer::timeout, this, &CSDlgSelfCheck::processSelfCheck);
	m_timer_ptr->start(200);
}
