#include "csdlgtriggerpulsecontrol.h"
#include "ui_csdlgtriggerpulsecontrol.h"
#include "Device/device.h"


CSDlgTriggerPulseControl::CSDlgTriggerPulseControl(QSharedPointer<Device> device_ptr, QWidget *parent) :
    QDialog(parent),
	m_device_ptr(device_ptr),
    ui(new Ui::CSDlgTriggerPulseControl)
{
    ui->setupUi(this);
	InitUI();
}

CSDlgTriggerPulseControl::~CSDlgTriggerPulseControl()
{
    delete ui;
}

void CSDlgTriggerPulseControl::on_pushButton_start_pulse_clicked()
{
	//获取设备指针
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->triggerStart();
	}

}

void CSDlgTriggerPulseControl::on_pushButton_end_pulse_clicked()
{
	//获取设备指针
	auto device_ptr = m_device_ptr.lock();

	if (device_ptr)
	{
		device_ptr->triggerStop();
	}

}

void CSDlgTriggerPulseControl::on_pushButton_close_clicked()
{
	close();
}

void CSDlgTriggerPulseControl::InitUI()
{
	// 去除问号按钮
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

}
