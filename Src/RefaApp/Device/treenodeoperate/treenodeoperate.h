#pragma once

#include <QtWidgets/QDialog>
#include "ui_treenodeoperate.h"
#include "../../Main/rccapp/csnetconfig/qiplineedit.h"
#include <QButtonGroup>
#include "../../Video/Export/ToolTipsShowWidget.h"
#include <QSet>
#include <QLineEdit>

class TreeNodeOperate : public QDialog
{
	Q_OBJECT

public:
	enum TabType {
		Device,
		Group
	};

	enum DeviceType {
		M_Series,
		GR_Series,
		Syn_Controller
	};

	typedef struct OperatorData {
		TabType tb{ Device };
		QString device_name{};
		QString device_ip{};
		DeviceType dt{ M_Series };
		QString group_name{};
	}OperatorData;

	TreeNodeOperate(QWidget *parent = Q_NULLPTR);
	void setAllDeviceIpVessel(const QSet<QString>& ip_vessel);
	void setManageDeviceIpVessel(const QSet<QString>& ip_vessel);
	void setGroupNameVessel(const QSet<QString>& group_vessel);
	void setSynControllerVessel(const QSet<QString>& syn_vessel);
	OperatorData getOperatorData() const;
	void setTabIndex(int index);
private:
	void initUI();
	void bind();
	void setTitleName(const QString& title_name);
	void setPartsEnabled(bool enable);
	QString getStarText(const QString& text) const;
	bool checkLineEdit(QLineEdit* le, const QString& str);
	bool checkIpEditBox(const QLineEdit* ip_le, const QString& str);
	DeviceType getDeviceType();
	void renameGroup(QString& groupname);
	void setGroupLineEditText();
	bool saveInfo();
private slots:
	void slotDeviceTypeBtnGroup(int id, bool checked);
	void slotTextChanged(const QString &text);
	void slotSaveAddBtnClicked();
	void slotSaveBtnClicked();
	void slotCancelBtnClicked();
	void slotGroupTextChanged(const QString &text);
private:
	QLineEdit* m_snLineEdit{};
	QIPLineEdit* m_ipLineEdit{};

	QButtonGroup* m_device_type_btn_group{};

	const QString m_device_name_default{ tr("High-speed camera") };
	const QString m_syn_controller_default{ tr("Synchronous controller") };
	const QString m_device_lineEdit_tip{ tr("Enter up to 35 characters!") };
	const QString m_mx_radio_tip{ tr("Series cameras starting with M/X/G") };
	const QString m_g_radio_tip{ tr("Series cameras starting with G") };
	const QString m_gr_radio_tip{ tr("Series cameras starting with GR") };
	const QString m_syn_radio_tip{ tr("CF18 synchronous controller") };
	const QString m_ip_empty{ (tr("Cannot be empty!")) };
	const QString m_ip_exit{ (tr("This user already exits!")) };
	const QString m_device_max_info{ (tr("Add up to 18 cameras!")) };
	const QString m_syn_max_info{ (tr("Add up to 18 simultaneous controllers!")) };
	const QString m_group_max_info{ (tr("Add up to 18 groups!")) };
	const QString m_messageBox_Title{ QObject::tr("RCC") };
	const int m_lineEdit_length_max{ 35 };
	const int m_device_num_max{ 18 };
	const int m_syn_num_max{ 18 };
	const int m_group_num{ 18 };

	QString m_device_name{};
	QSet<QString> m_all_ip_set{};
	QSet<QString> m_manage_ip_set{};
	QSet<QString> m_group_set{};
	QSet<QString> m_syn_controller_set{};
	OperatorData m_operator_data{};
private:
	Ui::TreeNodeOperateClass ui;
};
