#include "deviceeditname.h"
#include "ui_deviceeditname.h"

DeviceEditName::DeviceEditName(QSharedPointer<Device> cur_dev, QWidget *parent) :
    QDialog(parent),
	cur_dev_(cur_dev),
    ui(new Ui::DeviceEditName)
{
    ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	if (parent)
	{
		setWindowTitle(parent->windowTitle());
	}

	//输入框正则表达式控制
	QRegExp rx_Name("\\S+");
	QRegExpValidator * v_Name = new QRegExpValidator(rx_Name, this);
	ui->lineEditCameraName->setValidator(v_Name);

	//最大长度和提示文字
	ui->lineEditCameraName->setMaxLength(10);
	ui->lineEditCameraName->setPlaceholderText(QString(tr("Input 10 Characters max")));

	if (!cur_dev_.isNull())
	{
		ui->lineEditCameraName->setText(cur_dev_->getProperty(Device::PropName).toString());
	}
}

DeviceEditName::~DeviceEditName()
{
    delete ui;
}

void DeviceEditName::on_pushButtonConfirm_clicked()
{
    QString new_name = ui->lineEditCameraName->text();
    if (!new_name.isEmpty())
    {
        cur_dev_->setProperty(Device::PropName , new_name);
    }

    accept();
}

void DeviceEditName::on_pushButtonCancel_clicked()
{
    reject();
}
