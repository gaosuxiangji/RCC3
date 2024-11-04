#ifndef DEVICEEDITNAME_H
#define DEVICEEDITNAME_H

#include <QDialog>
#include "Device/device.h"

namespace Ui {
class DeviceEditName;
}

class DeviceEditName : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceEditName(QSharedPointer<Device> cur_dev, QWidget *parent = 0);
    ~DeviceEditName();

private slots:
    void on_pushButtonConfirm_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::DeviceEditName *ui;

	QSharedPointer<Device> cur_dev_;
};

#endif // DEVICEEDITNAME_H
