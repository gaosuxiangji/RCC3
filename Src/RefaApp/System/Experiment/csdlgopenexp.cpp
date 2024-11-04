#include "csdlgopenexp.h"
#include "ui_csdlgopenexp.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include <QPushButton>
CSDlgOpenExp::CSDlgOpenExp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CSDlgOpenExp)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgOpenExp::~CSDlgOpenExp()
{
    delete ui;
}

void CSDlgOpenExp::InitUI()
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	//初始化列表数据模型
	m_list_model = new QStandardItemModel();
	ui->tableViewProjects->setModel(m_list_model);
	ui->tableViewProjects->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	//表格样式
	ui->tableViewProjects->horizontalHeader()->setFrameShape(QFrame::StyledPanel); 
	ui->tableViewProjects->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->tableViewProjects->setShowGrid(true);
	ui->tableViewProjects->setGridStyle(Qt::SolidLine);
	//表格初始化 表头:代号,名称,描述
	QStringList headers;
	headers << tr("Code") << tr("Name") << tr("Description");
	m_list_model->setHorizontalHeaderLabels(headers);
	
	//获取实验项目数据
	FillList();

}

void CSDlgOpenExp::FillList()
{
	//遍历工作目录下的全部文件夹
	QString working_dir_str = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir working_dir(working_dir_str);
	working_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

	QStringList folder_list = working_dir.entryList();
	bool has_item = false;
	//在文件夹中找到REFA.ini文件
	for (auto folder : folder_list)
	{
		QString file_path = working_dir_str+'/'+ folder + "/Revealer_Camera_Control.ini";
		if (QFile::exists(file_path))
		{
			//读取文件,找到实验项的对应名称,代号,描述文字,如果代号为空则跳过,数据写入表格
			QSettings setting(file_path,QSettings::IniFormat);
			QString exp_code =  setting.value(kExperimentCodeKey).toString();
			if (exp_code.isEmpty())
			{
				continue;
			}
			QString exp_name = setting.value(kExperimentNameKey).toString();
			QString exp_desc = setting.value(kExperimentDescKey).toString();

			QList<QStandardItem*> item_row;
			QStandardItem* item_exp_code = new QStandardItem(exp_code);
			item_exp_code->setToolTip(exp_code);
			QStandardItem* item_exp_name = new QStandardItem(exp_name);
			item_exp_name->setToolTip(exp_name);
			QStandardItem* item_exp_desc = new QStandardItem(exp_desc);
			item_exp_desc->setToolTip(exp_desc);
			item_row << item_exp_code;
			item_row << item_exp_name;
			item_row << item_exp_desc;

			has_item = true;
			m_list_model->appendRow(item_row);
		}
	}

	ui->buttonBox->button(QDialogButtonBox::StandardButton::Open)->setEnabled(has_item);

	ui->tableViewProjects->sortByColumn(0, Qt::SortOrder::DescendingOrder);
}

void CSDlgOpenExp::on_buttonBox_accepted()
{
	//拷贝当前ini文件到默认目录
	QString working_dir_str = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QModelIndex current_index = ui->tableViewProjects->selectionModel()->currentIndex();
	if (current_index.isValid())
	{
		QString exp_code = m_list_model->item(current_index.row(), 0)->text();
		QString file_path = working_dir_str + '/' + exp_code + "/Revealer_Camera_Control.ini";
		if (QFile::exists(file_path))
		{
			QString old_setting_file_path = SystemSettingsManager::instance().getCurrentExperimentDir();
			if (QFile::exists( old_setting_file_path))
			{
				QFile::remove(old_setting_file_path);
			}
			QFile::copy(file_path, old_setting_file_path);
		}
	}
	else
	{
		reject();
	}

	accept();
}

void CSDlgOpenExp::on_buttonBox_rejected()
{
	reject();
}


