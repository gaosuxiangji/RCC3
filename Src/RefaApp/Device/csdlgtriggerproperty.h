#ifndef CSDLGTRIGGERPROPERTY_H
#define CSDLGTRIGGERPROPERTY_H

#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>

class Device;
namespace Ui {
class CSDlgTriggerProperty;
}
/**
* @brief 设备触发器信息展示界面
*/
class CSDlgTriggerProperty : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgTriggerProperty(QSharedPointer<Device> device_ptr, QWidget *parent = 0);
    ~CSDlgTriggerProperty();
	private slots:


    void on_pushButton_close_clicked();

private:
	void InitUI();

	QWeakPointer<Device> m_device_ptr;

    Ui::CSDlgTriggerProperty *ui;
};

#endif // CSDLGTRIGGERPROPERTY_H
