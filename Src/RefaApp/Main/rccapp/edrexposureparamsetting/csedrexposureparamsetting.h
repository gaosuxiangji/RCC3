/***************************************************************************************************
** @file: "二次曝光参数" 参数设置界面
** @author: mpp
** @date: 2022-02-24
*
*****************************************************************************************************/
#ifndef CSEDREXPOSUREPARAMSETTING_H
#define CSEDREXPOSUREPARAMSETTING_H

#include <QDialog>
#include "Device/device.h"
#include "Device/devicemanager.h"
#include "Device/deviceutils.h"
#include <QWeakPointer>
#include <QSharedPointer>

namespace Ui {
class CSEdrExposureParamSetting;
}

class CSEdrexposureParamSetting : public QDialog
{
    Q_OBJECT

public:
    explicit CSEdrexposureParamSetting(QSharedPointer<Device> device, const uint32_t value, QWidget *parent = 0);
    ~CSEdrexposureParamSetting();
public slots:
void SlotEdrForbid(bool used);
private:
    /************************
    * @brief: 初始化UI
    * @author: mpp
    *************************/
    void InitUI();

	/**************************
    * @brief: 连接信号槽
	* @author: mpp
	***************************/
	void ConnectSignalSlot();
signals:
	void Signal_EdrThresholdLineEdit_textChanged(int value);
	void Signal_EdrThresholdSlider_valueChanged(QString& text);
private slots:

    void on_EdrThresholdSlider_valueChanged(int value);
    void on_EdrExposureTimeLineEdit_editingFinished();

    void on_EdrThresholdLineEdit_editingFinished();

    void on_EdrComboBox_currentTextChanged(const QString &arg1);

private:
	QWeakPointer<Device> m_pCurrentDevice;    //当前设备指针
	QMap<QString, int>m_mapComBox;    //下拉框列表<单位字符串，单位索引>
	const uint32_t m_iThreadValue;
	agile_device::capability::Units m_current_unit;//当前曝光单位
	agile_device::capability::Units m_real_unit;//设备实际曝光单位
	bool m_b_init{ true };
	int m_nDevicePixelBit{ 8 }; //设备像素位深
	int m_nValueMax{ 255 }; //设备像素最大值
private:
    Ui::CSEdrExposureParamSetting *ui;
};

#endif // CSEDREXPOSUREPARAMSETTING_H
