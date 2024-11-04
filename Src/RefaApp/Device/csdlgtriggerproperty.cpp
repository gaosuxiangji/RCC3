#include "csdlgtriggerproperty.h"
#include "ui_csdlgtriggerproperty.h"
#include "Device/device.h"
#include "Common/UIUtils/uiutils.h"

CSDlgTriggerProperty::CSDlgTriggerProperty(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgTriggerProperty)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgTriggerProperty::~CSDlgTriggerProperty()
{
    delete ui;
}

void CSDlgTriggerProperty::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	//获取设备指针
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr.isNull())
	{
		return;
	}

	//设备名
	ui->lineEdit_name->setText(device_ptr->getProperty(Device::PropName).toString());

	//型号
	ui->lineEdit_model->setText(device_ptr->getModelName());

	//IP
	ui->lineEdit_IP->setText(device_ptr->getIp());


	//初始化表格
	ui->tableWidget_device_version->clear();
	ui->tableWidget_device_version->setColumnCount(3);
	ui->tableWidget_device_version->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
	ui->tableWidget_device_version->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->tableWidget_device_version->setHorizontalHeaderLabels({ "",tr("Type"),tr("Version") });
	ui->tableWidget_device_version->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget_device_version->verticalHeader()->setVisible(false);


	//硬件版本

	std::vector<VersionInfo> hardwareVersions;
	HscResult res = device_ptr->GetDeviceVersion(hardwareVersions);
	if (res != HSC_OK)
	{
		UIUtils::showErrorMsgBox(this, tr("Fail to get Device version."));
	}
	else
	{

		ui->tableWidget_device_version->setRowCount(hardwareVersions.size());
		for (int i = 0; i < hardwareVersions.size(); i++)
		{
			VersionInfo info = hardwareVersions.at(i);
			QTableWidgetItem * item_index = new QTableWidgetItem(QString::number(i + 1));
			QTableWidgetItem * item_type = new QTableWidgetItem(info.type);
			QTableWidgetItem * item_version = new QTableWidgetItem(info.version);

			ui->tableWidget_device_version->setItem(i, 0, item_index);
			ui->tableWidget_device_version->setItem(i, 1, item_type);
			ui->tableWidget_device_version->setItem(i, 2, item_version);
		}
	}

	if (!device_ptr->IsCF18()) 
	{
		//更新保存的设备信息
		HscResult res;
		HscDeviceInfo deviceInfo;
		res = device_ptr->ReloadDeviceInfo();
		if (res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, tr("Failed to get device information."));
			return;
		}
		//SDK软件版本号
		VersionInfo sdk_version_info;
		res = device_ptr->GetSdkVersion(sdk_version_info);
		if (res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, tr("Fail to get SDK version."));
		}
		else
		{
			ui->lineEdit_SDK->setText(sdk_version_info.version);;
		}

		//插件版本
		VersionInfo plugin_version_info;
		res = device_ptr->GetPluginVersion(plugin_version_info);
		if (res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, tr("Fail to get Plugin version."));
		}
		else
		{
			ui->lineEdit_plugin->setText(plugin_version_info.version);
		}
	}
	else
	{
		//SDK软件版本号
		HscResult res = HSC_OK;
		ui->lineEdit_SDK->setEnabled(false);
		ui->lineEdit_plugin->setEnabled(false);
	}
}

void CSDlgTriggerProperty::on_pushButton_close_clicked()
{
	close();
}
