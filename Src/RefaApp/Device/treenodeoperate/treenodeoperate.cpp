#include "treenodeoperate.h"
#include <QMessageBox>
#include <QRegExpValidator>

TreeNodeOperate::TreeNodeOperate(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	initUI();
	bind();
}

void TreeNodeOperate::setAllDeviceIpVessel(const QSet<QString>& ip_vessel)
{
	m_all_ip_set = ip_vessel;
}

void TreeNodeOperate::setManageDeviceIpVessel(const QSet<QString>& ip_vessel)
{
	m_manage_ip_set = ip_vessel;
}

void TreeNodeOperate::setGroupNameVessel(const QSet<QString>& group_vessel)
{
	m_group_set = group_vessel;
	setGroupLineEditText();
}

void TreeNodeOperate::setSynControllerVessel(const QSet<QString>& syn_vessel)
{
	m_syn_controller_set = syn_vessel;
}

TreeNodeOperate::OperatorData TreeNodeOperate::getOperatorData() const
{
	return m_operator_data;
}

void TreeNodeOperate::setTabIndex(int index)
{
	ui.tabWidget->setCurrentIndex(index);
}

void TreeNodeOperate::initUI()
{
	setWindowFlags(Qt::WindowCloseButtonHint);
	setTitleName(tr("Add"));

	{
		ui.type_name_label->setText(getStarText(tr("Type")));
		ui.ip_name_label->setText(getStarText(tr("IP")));
	}

	{
		m_ipLineEdit = new QIPLineEdit(1,this);
		m_ipLineEdit->setText("...");
		ui.ip_layout->addWidget(m_ipLineEdit);
	}
	{
		m_snLineEdit = new QLineEdit(this);
		QRegExp regx("[0-9]{1,16}");
		QRegExpValidator* validator = new QRegExpValidator(regx);
		m_snLineEdit->setValidator(validator);
		ui.ip_layout->addWidget(m_snLineEdit);
		m_snLineEdit->setText("0");
		m_snLineEdit->setVisible(false);
	}

	{
		m_device_type_btn_group = new QButtonGroup(this);
		m_device_type_btn_group->addButton(ui.mx_radio_btn, 0);
		m_device_type_btn_group->addButton(ui.gr_radio_btn, 1);
		m_device_type_btn_group->addButton(ui.syn_radio_btn, 2);
	}

	{
		ui.mx_radio_btn->setToolTip(m_mx_radio_tip);
		ui.gr_radio_btn->setToolTip(m_gr_radio_tip);
		ui.syn_radio_btn->setToolTip(m_syn_radio_tip);
	}

	{
		ui.mx_radio_btn->setChecked(true);
		ui.device_name_lineEdit->setText(m_device_name_default);
	}

	{
		ui.group_name_label->setText(getStarText(tr("Name")));
	}
	
}

void TreeNodeOperate::bind()
{
	bool ok = connect(m_device_type_btn_group, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled), \
		[=](int id, bool checked) {slotDeviceTypeBtnGroup(id, checked); });
	ok = connect(ui.device_name_lineEdit, &QLineEdit::textChanged, this, &TreeNodeOperate::slotTextChanged);
	ok = connect(ui.save_add_btn, &QPushButton::released, this, &TreeNodeOperate::slotSaveAddBtnClicked);
	ok = connect(ui.save_btn, &QPushButton::released, this, &TreeNodeOperate::slotSaveBtnClicked);
	ok = connect(ui.cancel_btn, &QPushButton::released, this, &TreeNodeOperate::slotCancelBtnClicked);
	ok = connect(ui.group_name_lineEdit, &QLineEdit::textChanged, this, &TreeNodeOperate::slotGroupTextChanged);
}

void TreeNodeOperate::setTitleName(const QString& title_name)
{
	this->setWindowTitle(title_name);
}

void TreeNodeOperate::setPartsEnabled(bool enable)
{
	ui.mx_radio_btn->setEnabled(enable);
	ui.gr_radio_btn->setEnabled(enable);
	ui.syn_radio_btn->setEnabled(enable);
	m_ipLineEdit->setEnabled(enable);
	m_snLineEdit->setEnabled(enable);
}

QString TreeNodeOperate::getStarText(const QString& text) const
{
	return QObject::tr("<font color = red>%1</font>").arg("*") + text;
}

bool TreeNodeOperate::checkLineEdit(QLineEdit* le, const QString& str)
{
	bool bRet = false;
	if (!le) {
		return bRet;
	}
	int nHeight = le->height();
	QPoint pos = le->mapToGlobal(QPoint(0, nHeight));
	int nWidth = le->width();
	QRect rect(pos.x(), pos.y(), nWidth, 25);
	
	if (str.isEmpty() && ui.device_name_lineEdit != le) {
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

bool TreeNodeOperate::checkIpEditBox(const QLineEdit* ip_le, const QString& str)
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
	if (getDeviceType() != GR_Series)
	{
		QStringList ip_list = str.split(".");
		if ((4 == ip_list.size())) {
			if (ip_list[0].isEmpty() || ip_list[1].isEmpty() || ip_list[2].isEmpty() || ip_list[3].isEmpty()) {
				ip_valid = false;
			}
		}
	}

	if (str.isEmpty() || !ip_valid) {
		bRet = true;
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_ip_empty);
		dlg.exec();
	}

	if (m_all_ip_set.contains(str)) {
		bRet = true;
		ToolTipsShowWidget dlg(this);
		dlg.setRectAndText(rect, m_ip_exit);
		dlg.exec();
	}
	return bRet;
}

TreeNodeOperate::DeviceType TreeNodeOperate::getDeviceType()
{
	DeviceType dt = DeviceType::M_Series;
	if (ui.mx_radio_btn->isChecked()) {
		dt = M_Series;
	}
	else if (ui.gr_radio_btn->isChecked()) {
		dt = GR_Series;
	}
	else if (ui.syn_radio_btn->isChecked()) {
		dt = Syn_Controller;
	}
	return dt;
}

void TreeNodeOperate::renameGroup(QString& groupname)
{
	QString groupname_temp = groupname;
	int i = 1;
	while (m_group_set.contains(groupname)) {
		groupname = groupname_temp + QString::number(i++);
	}
	m_group_set.insert(groupname);
}

void TreeNodeOperate::setGroupLineEditText()
{
	ui.group_name_lineEdit->setText(tr("grouping") + QString(" %1").arg(m_group_set.size() + 1,2,10,QChar('0')));
}

bool TreeNodeOperate::saveInfo()
{
	bool bRet = false;
	switch (ui.tabWidget->currentIndex())
	{
	case Device: {
		if (getDeviceType() == GR_Series)
		{
			if (checkIpEditBox(m_snLineEdit, m_snLineEdit->text())) {
				return bRet;
			}
		}
		else
		{
			if (checkIpEditBox(m_ipLineEdit, m_ipLineEdit->text())) {
				return bRet;
			}
		}

		if (ui.syn_radio_btn->isChecked()) {
			if (m_syn_num_max <= m_syn_controller_set.size()) {
				QMessageBox::information(this, m_messageBox_Title, m_syn_max_info, QMessageBox::Yes);
				return bRet;
			}
		}
		else {
			if (m_device_num_max <= m_manage_ip_set.size()) {
				QMessageBox::information(this, m_messageBox_Title, m_device_max_info, QMessageBox::Yes);
				return bRet;
			}
		}
		bRet = true;
		m_operator_data.tb = Device;
		if (getDeviceType() == GR_Series)
		{
			m_operator_data.device_ip = m_snLineEdit->text();
		}
		else
		{
			m_operator_data.device_ip = m_ipLineEdit->text();
		}
		m_operator_data.device_name = ui.device_name_lineEdit->text();
		m_operator_data.dt = getDeviceType();
		break;
	}
	case Group: {
		if (checkLineEdit(ui.group_name_lineEdit, ui.group_name_lineEdit->text())) {
			return bRet;
		}
		if (m_device_num_max <= m_group_set.size()) {
			QMessageBox::information(this, m_messageBox_Title, m_group_max_info, QMessageBox::Yes);
			return bRet;
		}
		bRet = true;
		m_operator_data.tb = Group;
		m_operator_data.group_name = ui.group_name_lineEdit->text();
		break;
	}
	default:
		break;
	}

	return bRet;
}

void TreeNodeOperate::slotDeviceTypeBtnGroup(int id, bool checked)
{
	if (checked) {
		if (ui.device_name_lineEdit->text().isEmpty()) {
			switch (id)
			{
			case M_Series:
			case GR_Series: {
				ui.device_name_lineEdit->setText(m_device_name_default);
				break;
			}
			case Syn_Controller: {
				ui.device_name_lineEdit->setText(m_syn_controller_default);
				break;
			}
			default:
				break;
			}
		}
		else if ((Syn_Controller == id) && (m_device_name_default == ui.device_name_lineEdit->text())) {
			ui.device_name_lineEdit->setText(m_syn_controller_default);
		}
		else if ((Syn_Controller != id) && (m_syn_controller_default == ui.device_name_lineEdit->text())) {
			ui.device_name_lineEdit->setText(m_device_name_default);
		}

		switch (id)
		{
		case M_Series:
		case Syn_Controller: {
			ui.ip_name_label->setText(getStarText(tr("IP")));
			m_snLineEdit->setVisible(false);
			m_ipLineEdit->setVisible(true);
			break;
		}
		case GR_Series: {
			ui.ip_name_label->setText(getStarText(tr("SN")));
			m_snLineEdit->setVisible(true);
			m_ipLineEdit->setVisible(false);
			break;
		}
		default:
			break;
		}
	}
}

void TreeNodeOperate::slotTextChanged(const QString &text)
{
	if (checkLineEdit(ui.device_name_lineEdit, text)) {
		return;
	}
	m_device_name = text;
}

void TreeNodeOperate::slotSaveAddBtnClicked()
{
	if (!saveInfo()) {
		return;
	}
	done(2);
}

void TreeNodeOperate::slotSaveBtnClicked()
{
	if (!saveInfo()) {
		return;
	}
	done(1);
}

void TreeNodeOperate::slotCancelBtnClicked()
{
	reject();
}

void TreeNodeOperate::slotGroupTextChanged(const QString &text)
{
	//to do:mpp 暂未使用
}
