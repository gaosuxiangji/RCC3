/***************************************************************************************************
** @file: 菜单栏-帮助-网络配置
** @author: mpp
** @date: 2022/05/11
*
*****************************************************************************************************/
#pragma once
#include <QtWidgets/QDialog>
#include "ui_csnetconfig.h"
#include "qiplineedit.h"
#include "Video/Export/ToolTipsShowWidget.h"

class CSNetConfig : public QDialog
{
	Q_OBJECT

public:
	struct NetParam
	{
		QString strIp{};
		QString strMask{};
		QString strGate{};
		QString trMac{};
	};
	CSNetConfig(const NetParam& param, QWidget *parent = Q_NULLPTR);

	/**************************
	* @brief: 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	NetParam GetNetConfigParam() const
	{
		return m_netParam;
	}
	void setGatewayDeviceTip(bool is_gateway_device) {
		m_is_gateway_device = is_gateway_device;
	}

	void setAllDeviceIp(const QSet<QString> &all_Ip);
private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/11
	***************************/
	void InitUI();

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/11
	***************************/
	void ConnectSignalAndSlot();

	/**************************
	* @brief: 点击确定按钮的响应函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/11
	***************************/
	void CertainBtnClicked();

	bool isNetParamValid(QString ip, QString gateway, QString & err_msg);

	bool checkLineEdit(QLineEdit* le, const QString& str);
	bool checkIpEditBox(const QLineEdit* ip_le);

	public slots:
	void slotTextChanged(const QString &text);

private:
	NetParam m_netParam;

	QIPLineEdit* m_ipLineEdit;
	QIPLineEdit* m_maskLineEdit;
	QIPLineEdit* m_gateLineEdit;
	bool m_is_gateway_device{ false };

	const QString m_device_lineEdit_tip{ tr("Enter up to 35 characters!") };
	const QString m_ip_empty{ (tr("Cannot be empty!")) };
	const QString m_ip_exit{ (tr("This user already exits!")) };
	const int m_lineEdit_length_max{ 35 };
	QString m_device_name{};
	QSet<QString> m_all_ip_set{};

	Ui::CSNetConfigClass ui;
};
