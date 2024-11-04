#include "csdlgfallpointmeasuresetup.h"
#include "ui_csdlgfallpointmeasuresetup.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "FallPointMeasure/FallPointMeasure.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
#include "opencv2/opencv.hpp"
#include <vector>
#include <QFileDialog>
#include <QTextCodec>

CSDlgFallPointMeasureSetup::CSDlgFallPointMeasureSetup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgFallPointMeasureSetup)
{
    ui->setupUi(this);
	InitUI();

}

CSDlgFallPointMeasureSetup::~CSDlgFallPointMeasureSetup()
{
    delete ui;
}

void CSDlgFallPointMeasureSetup::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	m_table_model_ptr = new QStandardItemModel();
	ui->tableView_cali_params->setModel(m_table_model_ptr);

	//设置表头样式:序号,文件名,参数类型,相机型号,相机IP/SN
	QStringList header_labels;
	header_labels << tr("Index") << tr("File Name") << tr("Param Type") << tr("Device Model") << tr("Device IP/SN");
	m_table_model_ptr->setHorizontalHeaderLabels(header_labels);
	ui->tableView_cali_params->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
	ui->tableView_cali_params->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->tableView_cali_params->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tableView_cali_params->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->tableView_cali_params->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
	ui->tableView_cali_params->verticalHeader()->setVisible(false);

	//加载已有的标定数据
	UpdateParamsInfo();

	//全选按钮三态
	ui->checkBox_selete_all->setTristate(true);

	//按钮使能
	

	//绑定信号
}

bool CSDlgFallPointMeasureSetup::UpdateParamsInfo()
{
	//从测量库中获取全部已加载的信息
	std::vector<RetrievalCalibrationInfo> map_calibration_params;
	FallPointMeasure::GetInstance().getCalibrationParamsInfo(map_calibration_params);

	//清除列表中全部数据
	m_table_model_ptr->removeRows(0, m_table_model_ptr->rowCount());


	QList<QList<QStandardItem *>> info_list;//表格全部数据
	int index = 0;
	for (auto current_param : map_calibration_params)
	{
		QList<QStandardItem*> item_info;//单个信息:序号,文件名,参数类型,相机型号,相机IP/SN

		//序号和复选框
		index++;
		QStandardItem * item_index = new QStandardItem();
		item_index->setText(QString::number(index));
		item_index->setCheckable(true);
		item_index->setTextAlignment(Qt::AlignCenter);
		

		//文件名称
		QStandardItem * file_name = new QStandardItem();
		// Modify by Juwc  2022/7/6
		//file_name->setText(current_param.fileName.c_str());
		//file_name->setToolTip(current_param.fileName.c_str());
		file_name->setText(QString::fromLocal8Bit(current_param.fileName.c_str()));
		file_name->setToolTip(QString::fromLocal8Bit(current_param.fileName.c_str()));
		file_name->setTextAlignment(Qt::AlignCenter);

		//参数类型
		QStandardItem * param_type = new QStandardItem();
		param_type->setText(UIExplorer::instance().getStringById("STRID_PT_MONOCULAR"));
		param_type->setToolTip(UIExplorer::instance().getStringById("STRID_PT_MONOCULAR"));
		param_type->setTextAlignment(Qt::AlignCenter);

		//相机型号
		QStandardItem * camera_type = new QStandardItem();
		camera_type->setText(current_param.cameraType.c_str());
		camera_type->setToolTip(current_param.cameraType.c_str());
		camera_type->setTextAlignment(Qt::AlignCenter);

		//相机IP/SN
		QStandardItem * camera_ip = new QStandardItem();
		camera_ip->setText(current_param.cameraIp.c_str());
		camera_ip->setToolTip(current_param.cameraIp.c_str());
		camera_ip->setTextAlignment(Qt::AlignCenter);

		item_info << item_index << file_name << param_type << camera_type << camera_ip;
		info_list << item_info;
	}


	//将表格写入数据model
	for (auto info : info_list)
	{
		m_table_model_ptr->appendRow(info);
	}

	return true;
}

Qt::CheckState CSDlgFallPointMeasureSetup::GetCurrentCheckState()
{
	//获取全部复选框的状态
	bool has_check = false;//有选中的
	bool has_uncheck = false;//有没选中的
	for (int i = 0; i < m_table_model_ptr->rowCount(); i++)
	{
		if (m_table_model_ptr->item(i)->checkState())
		{
			has_check = true;
		}
		else
		{
			has_uncheck = true;
		}

	}

	if (has_check)
	{
		if (has_uncheck)
		{
			return Qt::CheckState::PartiallyChecked;//部分选中
		}
		else
		{
			return Qt::CheckState::Checked;//全选
		}
	}
	else
	{
		return Qt::CheckState::Unchecked;//无选中
	}

	return Qt::CheckState::Unchecked;
}

void CSDlgFallPointMeasureSetup::on_checkBox_selete_all_clicked(bool checked)
{
	Q_UNUSED(checked);

	//获取当前状态
	Qt::CheckState state = GetCurrentCheckState();
	bool select_all = false;
	switch (state)
	{
	case Qt::Unchecked:
	case Qt::PartiallyChecked:
	{
		//选中全部
		select_all = true;
	}
		break;
	case Qt::Checked:
	{
		//全部取消选中
		select_all = false;
	}
		break;
	default:
		break;
	}

	//设置按钮
	for (int i = 0; i < m_table_model_ptr->rowCount(); i++)
	{
		m_table_model_ptr->item(i)->setCheckState(select_all?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
	}
	//设置状态
	ui->checkBox_selete_all->setCheckState(select_all ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

void CSDlgFallPointMeasureSetup::on_pushButton_delete_clicked()
{
	//拿到全部选中的相机IP
	for (int i = 0; i < m_table_model_ptr->rowCount(); i++)
	{
		if (m_table_model_ptr->item(i)->checkState())
		{
			//删除对应的相机属性
			FallPointMeasure::GetInstance().RemoveCalibrationParam(m_table_model_ptr->item(i, 4)->text().toStdString());
			SystemSettingsManager::instance().removeCalibrationFile(m_table_model_ptr->item(i, 4)->text());
		}
	}
	//刷新列表
	UpdateParamsInfo();
	//刷新全选按钮
	ui->checkBox_selete_all->setCheckState(GetCurrentCheckState());

}

void CSDlgFallPointMeasureSetup::on_pushButton_import_clicked()
{
	//选择文件
	QStringList file_path_names = QFileDialog::getOpenFileNames(this, tr("Selete Calibration Files"),
		SystemSettingsManager::instance().getCalibrationDirectory(),
		tr("All Calibration files(*.yml)"));

	if (file_path_names.isEmpty())
	{
		return;
	}
	//记录路径
	QDir cali_dir(file_path_names.first());
	SystemSettingsManager::instance().setCalibrationDirectory(cali_dir.absolutePath());

	//载入全部文件
	for (auto file_path_name : file_path_names)
	{
		QDir file_dir(file_path_name);
		//提取文件名
		QString file_name = file_dir.dirName();

		//打开文件
		cv::FileStorage fs(file_path_name.toLocal8Bit().data(), cv::FileStorage::READ);
		if (!fs.isOpened())
		{
			QString messageStr;
			messageStr = tr("%1 import fail.").arg(file_name);
			UIUtils::showErrorMsgBox(this, messageStr);
			return;
		}

		int projMode = -1;
		fs["projMode"] >> projMode;
		if (0 != projMode)//非单目
		{
			fs.release();
			QString messageStr;
			messageStr = tr("%1 is a binocular parameter file, which does not meet the requirements and cannot be imported.").arg(file_name);
			UIUtils::showErrorMsgBox(this, messageStr);
			return;
		}


		std::string cameraType, cameraIp;
		//导入标定文件
		int res = FallPointMeasure::GetInstance().ImportCalibrationParams(file_path_name.toLocal8Bit().data(), cameraIp, cameraType);
		switch (res)
		{
		case -3:   //导入标定数据失败
			break;
		case -2:   //IP地址为空
		{
			QString messageStr;
			messageStr = tr("There is no device IP in the parameter file %1, so it cannot be imported.").arg(file_name);
			UIUtils::showErrorMsgBox(this, messageStr);
			return;
		}
		case -1:   //打开标定文件失败
		{
			QString messageStr;
			messageStr = tr("The parameter file %1 is corrupt and cannot be imported.").arg(file_name);
			UIUtils::showErrorMsgBox(this, messageStr);
			return;
		}
		default:
			break;
		}

		//记录标定文件路径
		SystemSettingsManager::instance().addCalibrationFile(file_path_name,QString(cameraIp.c_str()));
	}

	//刷新列表
	UpdateParamsInfo();

}

void CSDlgFallPointMeasureSetup::on_pushButton_close_clicked()
{
	close();
}

void CSDlgFallPointMeasureSetup::on_tableView_cali_params_clicked(const QModelIndex &index)
{
	if (index.column() == 0)
	{
		//获取全部复选框的状态
		ui->checkBox_selete_all->setCheckState(GetCurrentCheckState());
	}
}
