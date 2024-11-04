#ifndef CSDLGTRIGGERPULSECONTROL_H
#define CSDLGTRIGGERPULSECONTROL_H

#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>
class Device;

namespace Ui {
class CSDlgTriggerPulseControl;
}

/**
* @brief 设备触发器脉冲控制界面
*/
class CSDlgTriggerPulseControl : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgTriggerPulseControl(QSharedPointer<Device> device_ptr, QWidget *parent = 0);
    ~CSDlgTriggerPulseControl();

private slots:
    void on_pushButton_start_pulse_clicked();

    void on_pushButton_end_pulse_clicked();

    void on_pushButton_close_clicked();


private:
	void InitUI();

	QWeakPointer<Device> m_device_ptr;
    Ui::CSDlgTriggerPulseControl *ui;
};

#endif // CSDLGTRIGGERPULSECONTROL_H
