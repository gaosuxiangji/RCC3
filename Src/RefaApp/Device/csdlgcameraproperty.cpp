#include "csdlgcameraproperty.h"
#include "ui_csdlgcameraproperty.h"
#include "Device/device.h"
#include "Common/UIUtils/uiutils.h"


CSDlgCameraProperty::CSDlgCameraProperty(QSharedPointer<Device> device_ptr,QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgCameraProperty)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgCameraProperty::~CSDlgCameraProperty()
{
    delete ui;
}

void CSDlgCameraProperty::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	//获取设备指针
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr.isNull())
	{
		return;
	}

	//更新保存的设备信息
	HscResult res;
	HscDeviceInfo deviceInfo;
	res = device_ptr->ReloadDeviceInfo();
	if (res != HSC_OK)
	{
		UIUtils::showErrorMsgBox(this,tr("Failed to get device information."));
		return ;
	}

	//设备名
	ui->lineEdit_name->setText(device_ptr->getProperty(Device::PropName).toString());

	//型号
	ui->lineEdit_model->setText(device_ptr->getModelName());

	//IP
	ui->lineEdit_IP->setText(device_ptr->getIp());

	//SN
	ui->lineEdit_SN->setText(device_ptr->getSn());

	//视频存储器
	int nTotalMem = device_ptr->GetDeviceTotalMemorySize();	// 视频存储器总共大小 MB
	int nFreeMem = 0;
	if (nTotalMem < 0)
	{
		UIUtils::showErrorMsgBox(this, tr("Failed to get video buffer size."));
	}
	else
	{
		int used = device_ptr->GetDeviceUsedMemorySize();	// 视频存储器已用大小 MB
		if (used < 0)
		{
			UIUtils::showErrorMsgBox(this, tr("There is not enough free space in the video buffer."));
		}
		else
		{
			nFreeMem = nTotalMem - used;	// 视频存储器可用大小 MB
		}
	}
	ui->lineEdit_avaliable->setText(QString::number((double)nFreeMem / 1024));//GB
	ui->lineEdit_total->setText(QString::number((double)nTotalMem / 1024));//GB

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

	//硬件版本
	//初始化表格
	ui->tableWidget_device_version->clear();
	ui->tableWidget_device_version->setColumnCount(3);
	ui->tableWidget_device_version->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
	ui->tableWidget_device_version->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->tableWidget_device_version->setHorizontalHeaderLabels({"",tr("Type"),tr("Version")});
    ui->tableWidget_device_version->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	ui->tableWidget_device_version->verticalHeader()->setVisible(false);
	std::vector<VersionInfo> hardwareVersions;
	res = device_ptr->GetDeviceVersion(hardwareVersions);
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
			QTableWidgetItem * item_index = new QTableWidgetItem(QString::number(i+1));
			QTableWidgetItem * item_type = new QTableWidgetItem(info.type);
			QTableWidgetItem * item_version = new QTableWidgetItem(info.version);

			ui->tableWidget_device_version->setItem(i, 0, item_index);
			ui->tableWidget_device_version->setItem(i, 1, item_type);
			ui->tableWidget_device_version->setItem(i, 2, item_version);
		}
	}

	//线路延迟
	DWORD delay = 0;
	device_ptr->GetRouteDelay(delay);
	ui->lineEdit_line_delay->setText(QString::number(delay));

	//授权信息
	HscAuthInfo authInfo;
	res = device_ptr->GetAuthInfo(&authInfo);
	if (res != HSC_OK)
	{
		UIUtils::showErrorMsgBox(this,tr("Failed to get authorization information."));
	}
	else
	{
		QString auth_info_str;
		switch (authInfo.authType)
		{
		case HSC_AUTH_FOREVER:
			auth_info_str = tr("Permanent");
			break;
		case HSC_AUTH_TRIAL:
		{
			float days = (float)authInfo.remainingAuthTime / (24 * 60 * 60);
			auth_info_str = QString::number((double)days, 'f', 2);
		}
		break;
		case HSC_AUTH_ERROR_NULL:
			auth_info_str = tr("Unauthorized");
			break;
		case HSC_AUTH_ERROR_OVERDUE:
			auth_info_str = tr("Overdue");
			break;
		case HSC_AUTH_ERROR_SYSTEM_TIME:
			auth_info_str = tr(" System Time Error");
			break;
		case HSC_AUTH_ERROR_AUTH_TIME:
			auth_info_str = tr("Auth Time Error");
			break;
		case HSC_AUTH_ERROR_USED_TIME:
			auth_info_str = tr("Usage Time Error");
			break;
		case HSC_AUTH_ERROR_OTHER:
			auth_info_str = tr("Unknown Error");
			break;
		default:
			break;
		}
		ui->lineEdit_remaining->setText(auth_info_str);
	}
	//盘符控制按钮,GR专用，阿里嘎多
	if (device_ptr->getState() > Previewing) {
		ui->pushButton_diskSet->setEnabled(false);
	}
	if (!device_ptr->isGrabberDevice()) {
		ui->pushButton_diskSet->setVisible(false);
	}
}

void CSDlgCameraProperty::on_pushButton_ok_clicked()
{
	accept();
}

void CSDlgCameraProperty::on_pushButton_cancel_clicked()
{
	reject();
}

void CSDlgCameraProperty::on_pushButton_diskSet_clicked()
{
	auto device_ptr = m_device_ptr.lock();
	csdlgdiskselect m_diskselect(device_ptr,this);
	m_diskselect.exec();
}
