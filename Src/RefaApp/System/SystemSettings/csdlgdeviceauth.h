#ifndef CSDLGDEVICEAUTH_H
#define CSDLGDEVICEAUTH_H

#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>
class Device;
namespace Ui {
class CSDlgDeviceAuth;
}

//设备授权对话框
class CSDlgDeviceAuth : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgDeviceAuth(QSharedPointer<Device> device_ptr, QWidget *parent = 0);
    ~CSDlgDeviceAuth();

	//返回选择的授权文件路径
	QString GetAuthFileName();

private slots:
    void on_pushButton_browse_clicked();

    void on_pushButton_comfirm_clicked();

    void on_pushButton_Cancel_clicked();
private:
	void InitUI();
private:
	QString m_file_name;
	QWeakPointer<Device> m_device_ptr;
    Ui::CSDlgDeviceAuth *ui;
};

#endif // CSDLGDEVICEAUTH_H
