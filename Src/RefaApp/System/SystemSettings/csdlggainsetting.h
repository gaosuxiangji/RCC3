#ifndef CSDLGGAINSETTING_H
#define CSDLGGAINSETTING_H

#include <QDialog>
#include <QSharedPointer>
#include "rangemodulemanage/rangemodulemanage.h"
namespace Ui {
class CSDlgGainSetting;
}
class Device;
/**
 * @brief 增益设置对话框
 */
class CSDlgGainSetting : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgGainSetting(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~CSDlgGainSetting();
public slots:
void slotDisconnect2GainSet();
private slots:
    void on_comboBox_analogGain_IndexChanged(int index);

    void on_pushButton_default_clicked();

    void on_pushButton_ok_clicked();
private:
	void InitUI();
	void bind();
	void updateDisplayValue();
private:
    Ui::CSDlgGainSetting *ui;

	QSharedPointer<Device> m_device_ptr;
	RangeModuleManage* m_range_module_manage{ nullptr };
	int m_current_index;//当前模拟增益选项
};

#endif // CSDLGGAINSETTING_H
