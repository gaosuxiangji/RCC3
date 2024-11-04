#ifndef CSDLGCAMERAPROPERTY_H
#define CSDLGCAMERAPROPERTY_H

#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>
#include"cdgrcameradiskselect/csdlgdiskselect.h"
class Device;

namespace Ui {
class CSDlgCameraProperty;
}

/**
 * @brief 设备属性展示页
 */
class CSDlgCameraProperty : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgCameraProperty(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~CSDlgCameraProperty();

private slots:
    void on_pushButton_ok_clicked();

    void on_pushButton_cancel_clicked();

	void on_pushButton_diskSet_clicked();
private:
	void InitUI();

	QWeakPointer<Device> m_device_ptr;
    Ui::CSDlgCameraProperty *ui;
};

#endif // CSDLGCAMERAPROPERTY_H
