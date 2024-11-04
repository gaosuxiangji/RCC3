#include "devicesearchwidget.h"
#include "ui_devicesearchwidget.h"

#include <QCloseEvent>

#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"

#include "Common/LogUtils/logutils.h"

DeviceSearchWidget::DeviceSearchWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceSearchWidget)
{
    ui->setupUi(this);

	initUI();
}

DeviceSearchWidget::~DeviceSearchWidget()
{
    delete ui;
}

int DeviceSearchWidget::startSearchDevices()
{
	setHidden(true);
	device_count_ = 0;
	//ui->progressBar->hide();
	ui->progressBar->setRange(0, 100);
	ui->progressBar->setValue(1);
	cur_progress_ = 1;

	float percent = 0;
	ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
	ui->labelTip->setText(tr("Searching devices..."));

	dev_search_thread_ptr_->start();
	timer->start(50);
	exec();
	return device_count_;
}

void DeviceSearchWidget::closeEvent(QCloseEvent *event)
{

	timer->stop();
	event->accept();
	CSLOG_INFO("search widget closed");
}

void DeviceSearchWidget::onProgressChanged(int progress)
{
// 	//进度刷新(TODO:由于目前搜索没有进度反馈,暂时改为手动模拟)
// 	ui->progressBar->setValue(progress);
// 
// 	float percent = float(progress);
// 	ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
}

void DeviceSearchWidget::onSearchFinished(int device_count)
{
	CSLOG_INFO("Search finished,creating devices.");
	dev_manager_ptr_->createDevices();
	CSLOG_INFO("Create devices finished");
	device_count_ = device_count;
	ui->progressBar->setValue(100);
	float percent = float(100);
	ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
	close();
}

void DeviceSearchWidget::onTimer()
{
	//手动刷新进度条
	if (cur_progress_ < ui->progressBar->maximum()-1)
	{
		ui->progressBar->setValue(++cur_progress_);
	 	float percent = float(cur_progress_);
		ui->labelProgress->setText(QString("%1%").arg(percent, 0, 'f', 2));
	}
}

void DeviceSearchWidget::initUI()
{
	// 设置标题
	setWindowTitle(UIExplorer::instance().getProductName());

	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	// 去除关闭按钮
	setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

	// 设备搜索关联
	dev_search_thread_ptr_ = new DeviceSearchThread();

	dev_manager_ptr_ = &DeviceManager::instance();
	connect(dev_manager_ptr_.data(), &DeviceManager::searchFinished, this, &DeviceSearchWidget::onSearchFinished);
	connect(dev_manager_ptr_.data(), &DeviceManager::searchProcessChanged, this, &DeviceSearchWidget::onProgressChanged);

	//进度条刷新
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &DeviceSearchWidget::onTimer);
}
