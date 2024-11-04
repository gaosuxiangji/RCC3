#ifndef CSDLGMANUALWHITEBALANCE_H
#define CSDLGMANUALWHITEBALANCE_H

#include <QDialog>
#include <QSharedPointer>

namespace Ui {
class CSDlgManualWhiteBalance;
}
class Device;
class ImageProcessor;
/**
 * @brief 手动白平衡对话框
 */
class CSDlgManualWhiteBalance : public QDialog
{
    Q_OBJECT

public:
    explicit CSDlgManualWhiteBalance(QSharedPointer<Device> device_ptr ,QWidget *parent = 0);
    ~CSDlgManualWhiteBalance();

private slots:

    void on_horizontalSlider_R_valueChanged(int value);

    void on_horizontalSlider_G_valueChanged(int value);

    void on_horizontalSlider_B_valueChanged(int value);

private:
	void InitUI();

	/**
	 * @brief 位置转为参数
	 * @param pos 位置
	 * @return  参数值
	 */
	float posTofactor(int pos);

	/**
	 * @brief 参数转为位置
	 * @param factor 参数值
	 * @return 位置
	 */
	int factorToPos(float factor);
private:
	//RGB增益值
	float m_RGain = 0.0;
	float m_GGain = 0.0;
	float m_BGain = 0.0;


	QSharedPointer<Device> m_device_ptr;//设备指针
	QSharedPointer<ImageProcessor> m_processor_ptr;//图像处理对象指针

    Ui::CSDlgManualWhiteBalance *ui;
};

#endif // CSDLGMANUALWHITEBALANCE_H
