#pragma once

#include <QtWidgets/QDialog>
#include "ui_editdeviceinfo.h"
#include "../../Video/Export/ToolTipsShowWidget.h"
#include "../../Main/rccapp/csnetconfig/qiplineedit.h"
#include <QButtonGroup>

class EditDeviceInfo : public QDialog
{
	Q_OBJECT

public:
	enum DeviceType {
		M_Series,
		GR_Series,
		Syn_Controller
	};

	typedef struct OperatorData {
		QString device_name{};
		QString device_ip{};
		DeviceType dt{ M_Series };

		QString old_device_name{};
		QString old_device_ip{};
		DeviceType old_dt{ M_Series };

		bool isDiff() { 
			return (device_ip != old_device_ip) || (dt != old_dt); 
		}
	}OperatorData;

	EditDeviceInfo(QWidget *parent = Q_NULLPTR);
	void setEditDeviceInfo(const OperatorData& od);
	void setAllDeviceIpVessel(const QSet<QString>& ip_vessel);
	void setManageDeviceIpVessel(const QSet<QString>& ip_vessel);
	void setSynControllerVessel(const QSet<QString>& syn_vessel);
	OperatorData getEditDeviceInfo()const;
	void setPartsEnabled(bool enable);
	void setDeviceStatus(bool connected);
private:
	void initUI();
	void bind();
	void setTitleName(const QString& title_name);
	QString getStarText(const QString& text) const;
	bool checkLineEdit(QLineEdit* le, const QString& str);
	bool checkIpEditBox(const QLineEdit* ip_le, const QString& str);
	DeviceType getDeviceType();
private slots:
	void slotDeviceTypeBtnGroup(int id, bool checked);
	void slotTextChanged(const QString &text);
	void slotSaveBtnClicked();
	void slotCancelBtnClicked();
private:
	QLineEdit* m_snLineEdit{};
	QIPLineEdit* m_ipLineEdit{};
	QButtonGroup* m_device_type_btn_group{};

	const QString m_device_name_default;//{ tr("High-speed camera") };
	const QString m_syn_controller_default;//{ tr("Synchronous controller") };
	const QString m_device_lineEdit_tip{ tr("Enter up to 35 characters!") };
	const QString m_mx_radio_tip{ tr("Series cameras starting with M/X/G") };
	const QString m_gr_radio_tip{ tr("Series cameras starting with GR") };
	const QString m_syn_radio_tip{ tr("CF18 synchronous controller") };
	const QString m_ip_empty{ (tr("Cannot be empty!")) };
	const QString m_ip_exit{ (tr("This user already exits!")) };
	const QString m_device_max_info{ (tr("Add up to 18 cameras!")) };
	const QString m_syn_max_info{ (tr("Add up to 18 simultaneous controllers!")) };
	const QString m_messageBox_Title{ QObject::tr("RCC") };
	const int m_lineEdit_length_max{ 35 };
	const int m_device_num_max{ 18 };
	const int m_syn_num_max{ 18 };

	QString m_device_name{};
	QSet<QString> m_manage_ip_set{};
	QSet<QString> m_all_ip_set{};
	QSet<QString> m_syn_controller_set{};
	OperatorData m_operator_data{};
	bool m_device_connected{ false };
private:
	Ui::EditDeviceInfoClass ui;
};
