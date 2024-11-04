#ifndef CSDLGGAINSETTINGV2_H
#define CSDLGGAINSETTINGV2_H

#include <QDialog>
#include <QSharedPointer>
namespace Ui {
class CSDlgGainSettingV2;
}
class Device;
/**
 * @brief 增益设置对话框V2
 */
class CSDlgGainSettingV2 : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgGainSettingV2(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~CSDlgGainSettingV2();

private slots:
    void on_comboBox_AnalogGain_currentIndexChanged(int index);

    void on_horizontalSlider_DigitalGain_sliderMoved(int position);

    void on_spinBox_DigitalGain_editingFinished();

    void on_pushButton_ok_clicked();

private:
	void InitUI();

private:
    Ui::CSDlgGainSettingV2 *ui;

	QSharedPointer<Device> m_device_ptr;
	int m_current_index;//当前模拟增益选项

};

#endif // CSDLGGAINSETTINGV2_H
