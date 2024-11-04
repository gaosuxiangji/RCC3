#include "csnetconfig.h"
#include <QMessageBox>
#include "Common/UIUtils/uiutils.h"
#include "Common/LogUtils/logutils.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"

CSNetConfig::CSNetConfig(const NetParam& param, QWidget *parent)
	: QDialog(parent)
	, m_netParam(param)
{
	ui.setupUi(this);
	InitUI();
	ConnectSignalAndSlot();
}

void CSNetConfig::InitUI()
{
	//设置弹框的尺寸
	this->setFixedSize(290, 280);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
	setWindowTitle(tr("Network Configuration"));

	m_ipLineEdit = new QIPLineEdit(0,this);
	m_ipLineEdit->setText(m_netParam.strIp);
	ui.IpLayout->addWidget(m_ipLineEdit);

	m_maskLineEdit = new QIPLineEdit(0,this);
	m_maskLineEdit->setText(m_netParam.strMask);
	//m_maskLineEdit->setReadOnly(true);
	ui.MaskLayout->addWidget(m_maskLineEdit);

	m_gateLineEdit = new QIPLineEdit(0,this);
	m_gateLineEdit->setText(m_netParam.strGate);
	ui.GateLayout->addWidget(m_gateLineEdit);

	ui.macEdit->setText(m_netParam.trMac);
}

void CSNetConfig::ConnectSignalAndSlot()
{
	connect(ui.CertainBtn, &QPushButton::clicked, [this]() {CertainBtnClicked(); });
	connect(ui.CancelBtn, &QPushButton::clicked, [this]() {close(); });
	connect(m_ipLineEdit, &QLineEdit::textChanged, this, &CSNetConfig::slotTextChanged);
}

void CSNetConfig::CertainBtnClicked()
{
	if (!checkIpEditBox(m_ipLineEdit))
	{
		return;
	}
	if ((m_ipLineEdit->text() != m_netParam.strIp) || (m_maskLineEdit->text() != m_netParam.strMask) \
		|| (m_gateLineEdit->text() != m_netParam.strGate))
	{
		//TODO:网络配置校验和修正
		QString err_msg;
		if (!isNetParamValid(m_ipLineEdit->text(),m_gateLineEdit->text(),err_msg))
		{
			UIUtils::showErrorMsgBox(this, err_msg);
			return;
		}

		QString message{};
		if (m_is_gateway_device) {
			message = tr("Modifying the device network configuration requires refreshing \
the device list.Are you sure you want to continue?");
		}
		else {
			message = tr("Modifying the device network configuration requires restarting the device and refreshing \
the device list.Are you sure you want to continue?");
		}
		QMessageBox::StandardButton rb = QMessageBox::information(this, tr("Network Configuration"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (rb == QMessageBox::No)
		{
			reject();
		}
		else
		{
			m_netParam.strIp = m_ipLineEdit->text();
			m_netParam.strMask = m_maskLineEdit->text();
			m_netParam.strGate = m_gateLineEdit->text();
			CSLOG_INFO("set net_param new,ip:{} ,gate:{} ,mask:{}",
				m_netParam.strIp.toStdString(),
				m_netParam.strGate.toStdString(),
				m_netParam.strMask.toStdString());
			accept();
		}
	}
	else
	{
		reject();
	}
}

bool CSNetConfig::isNetParamValid(QString ip, QString gateway, QString & err_msg)
{
	QStringList ip_text = ip.split(".");
	QStringList gateway_text = gateway.split(".");
	QStringList ip_correct_text;
	QStringList gateway_correct_text;
	err_msg = tr("Network params error.");

	for (auto ip_str : ip_text)
	{
		int ip_int = ip_str.toInt();
		if (ip_int == 0 || ip_int == 255)
		{
			err_msg = tr("Error IP segments!");
			return false;
		}
		ip_correct_text.push_back(QString::number(ip_int));
	}
	for (auto gateway_str : gateway_text)
	{
		int gateway_int = gateway_str.toInt();
		if (gateway_int == 0 || gateway_int == 255)
		{
			err_msg = tr("Error gateway segments!");
			return false;
		}
		gateway_correct_text.push_back(QString::number(gateway_int));
	}

	//调试模式下去除对192.168网段的限制
	if (!FunctionCustomizer::GetInstance().isConsoleEnabled())
	{

		if (ip_text.at(0) != "192" || ip_text.at(1) != "168" ||
			gateway_text.at(0) != "192" || gateway_text.at(1) != "168")
		{
			err_msg = tr("Only support 192.168 network segments.");
			return false;
		}
	}
	if (ip_correct_text.at(2) != gateway_correct_text.at(2)||
		ip_correct_text.at(1) != gateway_correct_text.at(1) || 
		ip_correct_text.at(0) != gateway_correct_text.at(0))
	{
		err_msg = tr("IP and gateway are not in the same network segment.");
		return false;
	}

	if (ip_correct_text == gateway_correct_text)
	{
		err_msg = tr("IP is conflict with gateway.");
		return false;
	}

	err_msg = QString("No error");
	return true;
}

void CSNetConfig::slotTextChanged(const QString &text)
{
	if (checkLineEdit(m_ipLineEdit, text)) {
		return;
	}
}


bool CSNetConfig::checkLineEdit(QLineEdit* le, const QString& str)
{
	bool bRet = false;
	if (!le) {
		return bRet;
	}
	int nHeight = le->height();
	QPoint pos = le->mapToGlobal(QPoint(0, nHeight));
	int nWidth = le->width();
	QRect rect(pos.x(), pos.y(), nWidth, 25);

	if (str.isEmpty()) {
		bRet = true;
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_ip_empty);
		dlg.exec();
	}

	if (m_lineEdit_length_max < str.length()) {
		bRet = true;
		le->setText(m_device_name);
		int nHeight = le->height();
		QPoint pos = le->mapToGlobal(QPoint(0, nHeight));
		int nWidth = le->width();
		QRect rect(pos.x(), pos.y(), nWidth, 25);
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_device_lineEdit_tip);
		dlg.exec();
	}

	return bRet;
}

bool CSNetConfig::checkIpEditBox(const QLineEdit* ip_le)
{
	bool bRet = false;
	if (!ip_le) {
		return bRet;
	}

	int nHeight = ip_le->height();
	QPoint pos = ip_le->mapToGlobal(QPoint(0, nHeight));
	int nWidth = ip_le->width();
	QRect rect(pos.x(), pos.y(), nWidth, 25);

	bool ip_valid = true;
	QString strText = m_ipLineEdit->text();

	if (strText.isEmpty() || strText.compare("...") == 0) {
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_ip_empty);
		dlg.exec();
		return bRet;
	}

	if (strText.compare(m_netParam.strIp) == 0)
	{
		bRet = true;
	}
	else
	{
		if (m_all_ip_set.contains(strText)) {
			ToolTipsShowWidget dlg(this);
			dlg.setRectAndText(rect, m_ip_exit);
			dlg.exec();
		}
		else
		{
			bRet = true;
		}
	}
	return bRet;
}

void CSNetConfig::setAllDeviceIp(const QSet<QString> &all_Ip)
{
	m_all_ip_set = all_Ip;
}