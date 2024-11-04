#ifndef DEVICEALTERIP_H
#define DEVICEALTERIP_H

#include <QDialog>
#include <Device/device.h>

namespace Ui {
class DeviceAlterIp;
}

class DeviceAlterIp : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceAlterIp(QSharedPointer<Device> cur_dev, QWidget *parent = 0);
    ~DeviceAlterIp();

private slots:
    void on_pushButtonConfirm_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::DeviceAlterIp *ui;

	QSharedPointer<Device> cur_dev_;
};

#endif // DEVICEALTERIP_H
