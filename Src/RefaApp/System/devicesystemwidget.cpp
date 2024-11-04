#include "devicesystemwidget.h"
#include "ui_devicesystemwidget.h"
#include "LogRunner.h"

using namespace FHJD_LOG;

DeviceSystemWidget::DeviceSystemWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceSystemWidget)
{
	LOG_INFO("Contruction.")
    ui->setupUi(this);

    initUi();
}

DeviceSystemWidget::~DeviceSystemWidget()
{
    delete ui;
}

void DeviceSystemWidget::onMainWindowShown()
{
	ui->buttonGroupNavigator->button(1)->setChecked(true);
	ui->stackedWidget->setCurrentIndex(1);
	ui->pageDeviceList->SearchDevices();
}

void DeviceSystemWidget::onHostIpChanged()
{
	ui->pageDeviceList->DisconnectAllDevice();
	ui->pageDeviceList->SearchDevices();
}

void DeviceSystemWidget::initUi()
{
    ui->buttonGroupNavigator->setId(ui->toolButtonSystemSettings, 0);
    ui->buttonGroupNavigator->setId(ui->toolButtonDeviceList, 1);
    ui->buttonGroupNavigator->button(0)->setChecked(true);
    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->buttonGroupNavigator, QOverload<int>::of(&QButtonGroup::buttonClicked), ui->stackedWidget, &QStackedWidget::setCurrentIndex);
	connect(ui->pageDeviceList, &DeviceListWidget::sigAutoConnectDeviceSuccess, this, &DeviceSystemWidget::sigAutoConnectDeviceSuccess);
}
