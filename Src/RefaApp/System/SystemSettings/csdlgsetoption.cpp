#include "csdlgsetoption.h"
#include "ui_csdlgsetoption.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QStorageInfo>
#include "System/SystemSettings/systemsettingsmanager.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "Common/PathUtils/pathutils.h"
#include "Common/UIUtils/uiutils.h"
#include "Main/rccapp/csrccapp.h"
#include <QMainWindow>
#include <thread>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

CSDlgSetOption::CSDlgSetOption(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgSetOption)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgSetOption::~CSDlgSetOption()
{
	if (m_groupBoxAcquisitionSetting)
	{
		delete m_groupBoxAcquisitionSetting;
		m_groupBoxAcquisitionSetting = nullptr;
	}
    delete ui;
}

void CSDlgSetOption::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->labelLocalIp->setVisible(false);
	ui->comboBox_IP->setVisible(false);

	auto & manager = SystemSettingsManager::instance();

	//初始化本地IP选项
	ui->comboBox_IP->addItems(manager.getAllLocalIp());
	ui->comboBox_IP->setCurrentText(manager.getLocalIp());

	//初始化本地工作目录
	QString work_dir = manager.getWorkingDirectory();
	ui->lineEditWorkdingDir->setText(work_dir);

	//提前创建文件夹
	QDir dir;
	dir.mkpath(work_dir);

	// 剩余磁盘空间
	QStorageInfo storage_info = QStorageInfo(work_dir);
	for (int i = 0; i < 10; i++)//等待磁盘信息
	{
		if (storage_info.isValid()&& storage_info.isReady())
		{
			break;
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}
	double free_space_GB = double(storage_info.bytesAvailable()) / 1024 / 1024 / 1024;
	ui->labelFreeDiskSpaceValue->setText(QString::number(free_space_GB, 'f', 1) + QString("GB"));

	// 设备自动连接
	ui->checkBoxAutoConnect->setChecked(manager.isAutoConnectEnabled());

	//avi压缩使能
	//ui->checkBoxEnableAVICompress->setChecked(manager.isAviCompressEnabled());
	//即录即停使能
	ui->checkBoxRecordingStop->setChecked(manager.isRecordingStopEnabled());

	//录制结束后操作类型使能,按钮id对应选项枚举值
	m_groupBoxAcquisitionSetting = new QButtonGroup(this);
	m_groupBoxAcquisitionSetting->addButton(ui->radioButtonAcqusitionMode_autoAquire,SystemSettingsManager::ActionTypesAfterRecording::kAcquire);
	m_groupBoxAcquisitionSetting->addButton(ui->radioButtonAcqusitionMode_autoExport, SystemSettingsManager::ActionTypesAfterRecording::kAutoExport);
	m_groupBoxAcquisitionSetting->addButton(ui->radioButtonAcqusitionMode_autoPlayback, SystemSettingsManager::ActionTypesAfterRecording::kPlayback);
	m_groupBoxAcquisitionSetting->addButton(ui->radioButtonAcqusitionMode_autoExportAndTrigger, SystemSettingsManager::ActionTypesAfterRecording::kAutoExportAndTrigger);
	m_groupBoxAcquisitionSetting->button(manager.getActionTypesAfterRecording())->setChecked(true);


}


void CSDlgSetOption::on_pushButtonBrowseWorkingDir_clicked()
{
	//开启路径选择对话框,结果保存在编辑框中
	QString default_dir_path = ui->lineEditWorkdingDir->text();
	do
	{
		// 选择工作路径
		QString dir_path = QFileDialog::getExistingDirectory(this, tr("Select the file save directory"), default_dir_path, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
		if (dir_path.isEmpty())
		{
			break;
		}

		// 读写权限检查
		if (!PathUtils::isReadable(dir_path) || !PathUtils::isWritable(dir_path))
		{
			UIUtils::showInfoMsgBox(this, tr("No access right!"));
			default_dir_path = dir_path;
			continue;
		}

		// 磁盘空间检查
		uint64_t min_free_space = 100 * 1024 * 1024;
		QStorageInfo storage_info = QStorageInfo(dir_path);
		if (storage_info.bytesAvailable() < min_free_space)
		{
			UIUtils::showInfoMsgBox(this, tr("Out Of Disk Space!"));
			default_dir_path = dir_path;
			continue;
		}

		// 设置工作路径
		ui->lineEditWorkdingDir->setText(dir_path);
		//刷新磁盘空间
		double free_space_GB = double(storage_info.bytesAvailable()) / 1024 / 1024 / 1024;
		ui->labelFreeDiskSpaceValue->setText(QString::number(free_space_GB, 'f', 1) + QString("GB"));

		break;
	} while (1);
}

void CSDlgSetOption::on_buttonBox_accepted()
{
	//点击确认,应用全部设置
	//设置本机ip
	if (!ui->comboBox_IP->currentText().isEmpty())
	{
		SystemSettingsManager::instance().setLocalIp(ui->comboBox_IP->currentText());
	}

	// 设置工作路径
	SystemSettingsManager::instance().setWorkingDirectory(ui->lineEditWorkdingDir->text());

	// 设备自动连接
	SystemSettingsManager::instance().setAutoConnectEnabled(ui->checkBoxAutoConnect->isChecked());

	//即录即停设置
	SystemSettingsManager::instance().setRecordingStopEnabled(ui->checkBoxRecordingStop->isChecked());

	//视频avi压缩
	//SystemSettingsManager::instance().setAviCompressEnabled(ui->checkBoxEnableAVICompress->isChecked());

	//录制后动作设置
	bool bModify = false;
	CSRccApp* main_window = nullptr;
	if (SystemSettingsManager::instance().getActionTypesAfterRecording() != m_groupBoxAcquisitionSetting->checkedId())
	{
		//刷新设备属性列表
		for (QWidget* widget : qApp->topLevelWidgets())
		{
			if (QMainWindow * mainWin = qobject_cast<QMainWindow*>(widget))
			{
				main_window = qobject_cast<CSRccApp*>(widget);
				bModify = true;
				break;
			}
		}
	}
	SystemSettingsManager::instance().setActionTypesAfterRecording(m_groupBoxAcquisitionSetting->checkedId());
	if (bModify && main_window)
	{
		main_window->UpdateDevicePropertyList();
	}
	accept();
}


void CSDlgSetOption::on_buttonBox_rejected()
{
	reject();
}
