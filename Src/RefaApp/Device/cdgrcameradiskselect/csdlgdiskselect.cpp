#include "csdlgdiskselect.h"
#include "ui_csdlgdiskselect.h"
#include <qtreewidget.h>
#include "Common/UIUtils/uiutils.h"
csdlgdiskselect::csdlgdiskselect(QSharedPointer<Device> device_ptr,QWidget *parent) :
    QDialog(parent), m_device_ptr(device_ptr),
    ui(new Ui::csdlgdiskselect)
{
    ui->setupUi(this);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	InitUI();
}

csdlgdiskselect::~csdlgdiskselect()
{
    delete ui;
}
void csdlgdiskselect::InitUI()
{
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr.isNull())
	{
		return;
	}
	std::vector<HscDiskMessage> diskmessages;
	std::string disk_names;
	device_ptr->GetCurrentDisk(disk_names);
	mCurrentDisk = QString::fromStdString(disk_names);
	//根据当前硬盘初始化UI
	device_ptr->GetAllDiskVolumes(diskmessages);
	InitMap(diskmessages);
	//根据m_WidegtButton初始化窗口
	QGridLayout *gridLayout = new QGridLayout(ui->groupBox_disk);
	for (int i = 0; i < m_MaxRadioNum + 1; i++) {
		gridLayout->setRowStretch(i,i);
	}
	gridLayout->setColumnStretch(0, 3);
	gridLayout->setColumnStretch(1, 4);
	int i = 0;
	for (auto disktype = m_disktype.begin(); disktype != m_disktype.end(); disktype++) {
		QCheckBox* checkBox = new QCheckBox(ui->groupBox_disk);  // 创建复选框控件
		QComboBox* comboBox = new QComboBox(ui->groupBox_disk);  // 创建下拉框控件
		checkBox->setText(tr("Disk ") + disktype.key());
		m_WidegtButton.insert(checkBox, comboBox);
		i++;
		gridLayout->addWidget(checkBox, i, 0);
		gridLayout->addWidget(comboBox, i, 1);
		connect(checkBox, &QCheckBox::stateChanged, this, &csdlgdiskselect::CheckStatusChange);
		comboBox->setEnabled(false);
		for (auto diskname : disktype.value()) {
			comboBox->addItem(diskname, diskname);
			if (mCurrentDisk.contains(diskname)) {
				checkBox->setChecked(true);
				comboBox->setCurrentText(diskname);
			}
		}

	}
	
	//for (auto widegtButton = m_WidegtButton.begin(); widegtButton != m_WidegtButton.end(); widegtButton++) {	
	//	i++;
	//	gridLayout->addWidget(widegtButton.key(), i, 0);
	//	gridLayout->addWidget(widegtButton.value(), i, 1);
	//}	
	connect(ui->pushButton_save, &QPushButton::clicked, this, &csdlgdiskselect::SaveDisk);
	connect(ui->pushButton_cancel, &QPushButton::clicked, this, &csdlgdiskselect::close);
}
void  csdlgdiskselect::SaveDisk() {
	QString diskString = "";
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr.isNull())
	{
		return;
	}
	for (auto widegtButton = m_WidegtButton.begin(); widegtButton != m_WidegtButton.end(); widegtButton++) {
		if (widegtButton.key()->isChecked()) {
			diskString += widegtButton.value()->currentText();
		}
	}
	device_ptr->SetCurrentDisk(diskString.toStdString());
	device_ptr->refreshCurrentDeviceSettings();
	close();
}
void csdlgdiskselect::InitMap(const std::vector<HscDiskMessage> &diskmessages)
{
	for (auto& diskits : diskmessages) {
		QList<QString> disk_name;
		disk_name.push_back(QString::fromLatin1(&diskits.name));
		auto disktype = m_disktype.find(QString::number(diskits.phyDeviceNumber));
		if (disktype != m_disktype.end()) {
			disktype.value().push_back(QString::fromLatin1(&diskits.name));
		}
		else {
			m_disktype.insert(QString::number(diskits.phyDeviceNumber), disk_name);
		}
	}

}
void csdlgdiskselect::CheckStatusChange(int status) {
	QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender());
	if (checkBox) {
		if (checkBox->isChecked()) {
			if (m_BoxCheckNum==4) {
				checkBox->setChecked(false);
			}
			auto comboBox = m_WidegtButton.value(checkBox);
			comboBox->setEnabled(true);
			m_BoxCheckNum++;
		}
		else {
			auto comboBox = m_WidegtButton.value(checkBox);
			comboBox->setEnabled(false);
			m_BoxCheckNum--;
		}
	}
}