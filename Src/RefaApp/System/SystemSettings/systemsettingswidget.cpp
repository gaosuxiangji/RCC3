#include "systemsettingswidget.h"
#include "ui_systemsettingswidget.h"

#include <QFileInfo>
#include <QFileDialog>
#include "ghc/filesystem.hpp"

#include "systemsettingsmanager.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/PathUtils/pathutils.h"
#include "device/devicemanager.h"
#include "Common/UIUtils/uiutils.h"
#include "System/devicesystemwidget.h"


SystemSettingsWidget::SystemSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemSettingsWidget)
{
    ui->setupUi(this);

    initUi();
}

SystemSettingsWidget::~SystemSettingsWidget()
{
    delete ui;
}

void SystemSettingsWidget::initUi()
{
    auto & manager = SystemSettingsManager::instance();

    // 本地IP
    ui->comboBoxLocalIp->addItems(manager.getAllLocalIp());
    ui->comboBoxLocalIp->setCurrentText(manager.getLocalIp());

    // 工作路径
    QString work_dir = manager.getWorkingDirectory();
    ui->lineEdit->setText(work_dir);

	// 剩余磁盘空间
    ghc::filesystem::path path(work_dir.toStdString());
    std::error_code error_code;
    ghc::filesystem::space_info space_info = ghc::filesystem::space(path, error_code);
    double free_space_GB = double(space_info.free) / 1024 / 1024/ 1024;
    ui->labelFreeSpaceValue->setText(QString::number(free_space_GB, 'f', 1));

    // 自动回放
    ui->checkBoxAutoPlayback->setChecked(manager.instance().isAutoPlaybackEnabled());
}

void SystemSettingsWidget::on_pushButtonBrowseWorkingDirectory_clicked()
{
    QString default_dir_path = ui->lineEdit->text();
    do
    {
        // 选择工作路径
        QString dir_path = QFileDialog::getExistingDirectory(this, tr("Select Working Directory"), default_dir_path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir_path.isEmpty())
        {
            break;
        }

        // 读写权限检查
        if (!PathUtils::isReadable(dir_path) || !PathUtils::isWritable(dir_path))
		{
			UIUtils::showInfoMsgBox(this, tr("No Access!"));
			default_dir_path = dir_path;
			continue;
		}

        // 磁盘空间检查
        uint64_t min_free_space = 100 * 1024 * 1024;
        ghc::filesystem::path path(dir_path.toStdString());
        std::error_code error_code;
        ghc::filesystem::space_info space_info = ghc::filesystem::space(path, error_code);
        if (space_info.free < min_free_space)
        {
            UIUtils::showInfoMsgBox(this, tr("Out Of Disk Space!"));
            default_dir_path = dir_path;
            continue;
        }

		// 设置工作路径
		SystemSettingsManager::instance().setWorkingDirectory(dir_path);
		ui->lineEdit->setText(dir_path);

		break;
    } while(1);
}

void SystemSettingsWidget::on_comboBoxLocalIp_activated(const QString &arg1)
{
	//获取之前的ip,判断是否相同,相同则忽略
	QString old_local_ip = SystemSettingsManager::instance().getLocalIp();
	if (old_local_ip == arg1)
	{
		SystemSettingsManager::instance().setLocalIp(arg1);
	}
	else
	{
		//判断当前是否已经连接了相机
		if (DeviceManager::instance().getConnectedDeviceCount())
		{
			//如果是已连接状态则弹窗提示,
			QString msg = QString("%1\r\n%2")
				.arg(tr("Tip"))
				.arg(tr("After switch, the system will disconnect all devices and research devices, continue?"));
			if (UIUtils::showQuestionMsgBox(this, msg))
			{
				//选择是则移除所有相机连接,并且重新触发搜索
				dynamic_cast<DeviceSystemWidget*>(this->parentWidget())->onHostIpChanged();
			}
			else
			{
				//选择否则跳过,重新设置ip为以前的ip
				SystemSettingsManager::instance().setLocalIp(old_local_ip);
				ui->comboBoxLocalIp->setCurrentText(old_local_ip);
			}
		}
		else
		{
			//未连接状态,设置本地IP
			SystemSettingsManager::instance().setLocalIp(arg1);
		}
	}
}

void SystemSettingsWidget::on_checkBoxAutoPlayback_clicked(bool checked)
{
    // 设置自动回放
    SystemSettingsManager::instance().setAutoPlaybackEnabled(checked);
}
