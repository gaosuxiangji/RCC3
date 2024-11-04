#include "devicealterip.h"
#include "ui_devicealterip.h"

#include <QRegExp>
#include <QRegExpValidator>

DeviceAlterIp::DeviceAlterIp(QSharedPointer<Device> cur_dev , QWidget *parent) :
    QDialog(parent),
	cur_dev_(cur_dev),
    ui(new Ui::DeviceAlterIp)
{
    ui->setupUi(this);

    //输入框正则表达式控制
    QRegExp rx_IP("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator * v_IP = new QRegExpValidator(rx_IP,this);
    ui->lineEditCameraIp->setValidator(v_IP);

	if (!cur_dev_.isNull())
	{
		ui->lineEditCameraIp->setText(cur_dev_->getIp());
	}
}

DeviceAlterIp::~DeviceAlterIp()
{
    delete ui;
}

void DeviceAlterIp::on_pushButtonConfirm_clicked()
{
	QString new_ip = ui->lineEditCameraIp->text();
	if (!new_ip.isEmpty())
	{
		cur_dev_->setIp(new_ip);
	}
	accept();
}

void DeviceAlterIp::on_pushButtonCancel_clicked()
{
    reject();
}
