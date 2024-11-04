#ifndef CSDLGARMMANUALWHITEBALANCE_H
#define CSDLGARMMANUALWHITEBALANCE_H

#include <QDialog>
#include <QSharedPointer>
#include "HscSelectors.h"

class Device;
namespace Ui {
class CSDlgArmManualWhiteBalance;
}

/**
 * @brief Arm端 手动白平衡设置对话框
 */
class CSDlgArmManualWhiteBalance : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgArmManualWhiteBalance(QSharedPointer<Device> device_ptr,QWidget *parent = 0);
    ~CSDlgArmManualWhiteBalance();
private slots:
    void on_horizontalSlider_R_sliderMoved(int position);

    void on_horizontalSlider_GR_sliderMoved(int position);

    void on_horizontalSlider_GB_sliderMoved(int position);

    void on_horizontalSlider_B_sliderMoved(int position);

    void on_spinBox_R_editingFinished();

    void on_spinBox_GR_editingFinished();

    void on_spinBox_GB_editingFinished();

    void on_spinBox_B_editingFinished();

    void on_pushButton_close_clicked();

private:

	void InitUI();

	/**
	 * @brief 应用来自滑动条的数据
	 */
	void applyDataFromSlider();

	/**
	* @brief 应用来自编辑框的数据
	*/
	void applyDataFromSpinBox();

private:
	//设备指针
	QSharedPointer<Device> m_device_ptr;

	HscIntRange m_gain_range{ 0 }; // 增益范围

    Ui::CSDlgArmManualWhiteBalance *ui;
};

#endif // CSDLGARMMANUALWHITEBALANCE_H
