#include "editgroupinfo.h"

EditGroupInfo::EditGroupInfo(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	initUI();
	bind();
}

void EditGroupInfo::setEditGroupInfo(const OperatorData& od)
{
	m_operator_data.old_group_name = od.old_group_name;
	m_device_name = od.old_group_name;
	ui.group_name_lineEdit->setText(m_operator_data.old_group_name);
}

void EditGroupInfo::setGroupNameVessel(const QSet<QString>& group_vessel)
{
	m_group_set = group_vessel;
}

EditGroupInfo::OperatorData EditGroupInfo::getEditGroupInfo() const
{
	return m_operator_data;
}

void EditGroupInfo::initUI()
{
	setWindowFlags(Qt::WindowCloseButtonHint);
	setTitleName(tr("Edit"));
	{
		ui.group_name_label->setText(getStarText(tr("Name")));
	}
}

void EditGroupInfo::bind()
{
	bool ok = connect(ui.save_btn, &QPushButton::released, this, &EditGroupInfo::slotSaveBtnClicked);
	ok = connect(ui.cancel_btn, &QPushButton::released, this, &EditGroupInfo::slotCancelBtnClicked);
	ok = connect(ui.group_name_lineEdit, &QLineEdit::textEdited, this, &EditGroupInfo::slotTextChanged);
}

void EditGroupInfo::setTitleName(const QString& title_name)
{
	this->setWindowTitle(title_name);
}

QString EditGroupInfo::getStarText(const QString& text) const
{
	return QObject::tr("<font color = red>%1</font>").arg("*") + text;
}

bool EditGroupInfo::checkLineEdit(QLineEdit* le, const QString& str)
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
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_device_lineEdit_tip);
		dlg.exec();
	}

	if ((m_operator_data.old_group_name != str) && m_group_set.contains(str)) {
		bRet = true;
		le->setText(m_device_name);
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_group_lineEdit_exit);
		dlg.exec();
	}

	return bRet;
}

void EditGroupInfo::slotTextChanged(const QString &text)
{
	if (checkLineEdit(ui.group_name_lineEdit, text)) {
		return;
	}
	m_device_name = text;
}

void EditGroupInfo::slotSaveBtnClicked()
{
	if (checkLineEdit(ui.group_name_lineEdit, ui.group_name_lineEdit->text())) {
		return;
	}
	m_operator_data.group_name = m_device_name;
	accept();
}

void EditGroupInfo::slotCancelBtnClicked()
{
	reject();
}
