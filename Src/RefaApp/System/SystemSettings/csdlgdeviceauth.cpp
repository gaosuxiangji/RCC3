#include "csdlgdeviceauth.h"
#include "ui_csdlgdeviceauth.h"
#include <QTimer>
#include <QFileDialog>
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Device/device.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/UIExplorer/uiexplorer.h"

CSDlgDeviceAuth::CSDlgDeviceAuth(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgDeviceAuth)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgDeviceAuth::~CSDlgDeviceAuth()
{
    delete ui;
}

QString CSDlgDeviceAuth::GetAuthFileName()
{
	return m_file_name;
}

void CSDlgDeviceAuth::on_pushButton_browse_clicked()
{
	//选择文件
	QString file_name = QFileDialog::getOpenFileName(this,UIExplorer::instance().getStringById("STRID_AUTH_WND_CAPTION"),
		SystemSettingsManager::instance().getWorkingDirectory(),
		tr("Files(*.dat)"));
	if (!file_name.isEmpty())
	{
		ui->lineEdit_path->setText(file_name);
	}
}

void CSDlgDeviceAuth::on_pushButton_comfirm_clicked()
{
	QString file_name = ui->lineEdit_path->text();
	if (file_name.isEmpty())
	{
		UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_AUTH_MSG_EMPTY_PATH"));
		return;
	}
	//检查文件是否存在
	QDir qdir;
	if (!qdir.exists(file_name))
	{
		UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_AUTH_MSG_FILE_NOT_EXIST"));
		return;
	}

	m_file_name = file_name;


	accept(); // 先关闭“设备授权”对话框，再做授权
	close();
}

void CSDlgDeviceAuth::on_pushButton_Cancel_clicked()
{
	reject();
}

void CSDlgDeviceAuth::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	auto device_ptr = m_device_ptr.lock();
	if (device_ptr)
	{
		HscAuthInfo authInfo;
		device_ptr->GetAuthInfo(&authInfo);
		
		QString  deviceDesc = device_ptr->getDescription();
		QString authInfoStr;
		switch (authInfo.authType)
		{
		case HSC_AUTH_FOREVER:
			authInfoStr = tr("%1 is authorized permanently.").arg(deviceDesc);
			break;
		case HSC_AUTH_TRIAL:
		{
			int days = authInfo.remainingAuthTime / (24 * 60 * 60);
			int hours = authInfo.remainingAuthTime / (60 * 60) - days * 24;
			int mins = authInfo.remainingAuthTime / 60 - (days * 24 + hours) * 60;

			authInfoStr = tr("The remaining auth time of %1 is %2 Days %3 Hours %4 Mins.").arg(deviceDesc).arg(days).arg(hours).arg(mins);
		}
		break;
		case HSC_AUTH_ERROR_NULL:
			authInfoStr = tr("%1 is not authorized.").arg(deviceDesc);
			break;
		case HSC_AUTH_ERROR_OVERDUE:
			authInfoStr = tr("%1 is overdue.").arg(deviceDesc);
			break;
		case HSC_AUTH_ERROR_SYSTEM_TIME:
			authInfoStr = tr("%1 auth error: Abnormal System Time.").arg(deviceDesc);
			break;
		case HSC_AUTH_ERROR_AUTH_TIME:
			authInfoStr = tr("%1 auth error: Auth time storage abnormally.").arg(deviceDesc);
			break;
		case HSC_AUTH_ERROR_USED_TIME:
			authInfoStr = tr("%1 auth error: Used Time storage abnormally.").arg(deviceDesc);
			break;
		case HSC_AUTH_ERROR_OTHER:
			authInfoStr = tr("%1 auth error: Other.").arg(deviceDesc);
			break;
		default:
			authInfoStr = tr("%1 auth error: Unknown.").arg(deviceDesc);
			break;
		
		}
		ui->label_auth_info->setText(authInfoStr);
	}
}
