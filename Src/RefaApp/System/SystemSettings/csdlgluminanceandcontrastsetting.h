#ifndef CSDLGLUMINANCEANDCONTRASTSETTING_H
#define CSDLGLUMINANCEANDCONTRASTSETTING_H

#include <QDialog>
#include <QSharedPointer>
#include <QWeakPointer>
class Device;

namespace Ui {
class CSDlgLuminanceAndContrastSetting;
}

/**
 * @brief 亮度对比度设置对话框
 */
class CSDlgLuminanceAndContrastSetting : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgLuminanceAndContrastSetting(QSharedPointer<Device> device_ptr, QWidget *parent = 0);
    ~CSDlgLuminanceAndContrastSetting();

private slots:

    void on_pushButton_clicked();

    void on_horizontalSlider_Luminance_valueChanged(int value);

    void on_horizontalSlider_Contrast_valueChanged(int value);

    void on_spinBox_Luminance_valueChanged(int arg1);

    void on_spinBox_Contrast_valueChanged(int arg1);

private:
	void InitUI();

private:

	QWeakPointer<Device> m_device_ptr;

    Ui::CSDlgLuminanceAndContrastSetting *ui;
};

#endif // CSDLGLUMINANCEANDCONTRASTSETTING_H
