#include "csdlghealthmanagersetting.h"
#include "ui_csdlghealthmanagersetting.h"
#include "System/cshealthmanager.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QValidator>
#include "Common/UIUtils/uiutils.h"
CSDlgHealthManagerSetting::CSDlgHealthManagerSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgHealthManagerSetting)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgHealthManagerSetting::~CSDlgHealthManagerSetting()
{
    delete ui;
}

void CSDlgHealthManagerSetting::on_buttonBox_accepted()
{

	QString host_ip = ui->lineEdit_IP->text();
	int pos = 0;
	if (m_validator_ip->validate(host_ip,pos) != QValidator::Acceptable)
	{
		UIUtils::showWarnMsgBox(this, tr("Please input valid ip address."));
		return;
	}
	QString port = ui->lineEdit_port->text();
	if (m_validator_port->validate(port, pos) != QValidator::Acceptable)
	{
		UIUtils::showWarnMsgBox(this, tr("Please input valid port."));
		return;
	}
	QString period = ui->lineEdit_period->text();
	if (m_validator_period->validate(period, pos) != QValidator::Acceptable)
	{
		UIUtils::showWarnMsgBox(this, tr("Please input valid period."));
		return;
	}

	CSHealthManager::instance().apply(host_ip, port.toInt(), period.toInt());

	accept();

}

void CSDlgHealthManagerSetting::on_buttonBox_rejected()
{
	reject();
}

void CSDlgHealthManagerSetting::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//编辑框输入控制
	QRegExp rx_IP("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");//ip地址正则表达式
	m_validator_ip = new QRegExpValidator(rx_IP, this);
	m_validator_period = new QIntValidator(1, UINT16_MAX, this);
	m_validator_port = new QIntValidator(1, UINT16_MAX, this);
	ui->lineEdit_period->setValidator(m_validator_period);
	ui->lineEdit_port->setValidator(m_validator_port);
	
	ui->lineEdit_IP->setValidator(m_validator_ip);

	//获取健康管理信息
	ui->lineEdit_IP->setText(CSHealthManager::instance().getHostIp());
	ui->lineEdit_port->setText(QString::number(CSHealthManager::instance().getPort()));
	ui->lineEdit_period->setText(QString::number(CSHealthManager::instance().getPeriod()));

}
