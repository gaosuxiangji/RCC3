#pragma once

#include <QtWidgets/QDialog>
#include "ui_editgroupinfo.h"
#include "../../Video/Export/ToolTipsShowWidget.h"

class EditGroupInfo : public QDialog
{
	Q_OBJECT

public:
	typedef struct OperatorData {
		QString group_name{};
		QString old_group_name{};
	}OperatorData;

	EditGroupInfo(QWidget *parent = Q_NULLPTR);
	void setEditGroupInfo(const OperatorData& od);
	void setGroupNameVessel(const QSet<QString>& group_vessel);
	OperatorData getEditGroupInfo()const;
private:
	void initUI();
	void bind();
	void setTitleName(const QString& title_name);
	QString getStarText(const QString& text) const;
	bool checkLineEdit(QLineEdit* le, const QString& str);
private slots:
	void slotTextChanged(const QString &text);
	void slotSaveBtnClicked();
	void slotCancelBtnClicked();
private:
	const QString m_ip_empty{ (tr("Cannot be empty!")) };
	const QString m_device_lineEdit_tip{ tr("Enter up to 35 characters!") };
	const QString m_group_lineEdit_exit{ tr("group name exit,please input again!") };
	const int m_lineEdit_length_max{ 35 };

	QString m_device_name{};
	QSet<QString> m_group_set{};
	OperatorData m_operator_data{};
private:
	Ui::EditGroupInfoClass ui;
};
