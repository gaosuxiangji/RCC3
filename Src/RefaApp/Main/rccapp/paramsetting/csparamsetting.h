/***************************************************************************************************
** @file: “智能触发参数”及“自动曝光参数” 设置界面
** @author: mpp
** @date: 2022-02-23
*
*****************************************************************************************************/

#ifndef CSPARAMSETTING_H
#define CSPARAMSETTING_H

#include <QDialog>
#include <QColorDialog>
#include <QMouseEvent>
#include <QPushButton>
#include "../render/PlayerViewBase.h"
#include "../csviewmanager.h"
#include "Device/devicestate.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
#include "Device/deviceutils.h"
#include <QWeakPointer>
#include <QSharedPointer>
#include <QRect>
class ColorQPushButton;
class QIntValidator;
namespace Ui {
class CSParamSetting;
}

class CSParamSetting : public QDialog
{
    Q_OBJECT

    //弹框类型
public:
    /************************
    * @brief: 构造函数
    * @param type: 弹框类型 0-智能触发参数弹框 1-自动曝光参数弹框
    * @author: mpp
    *************************/
    explicit CSParamSetting(int type, QSharedPointer<Device> device,QWidget *parent = 0);
    ~CSParamSetting();
public slots:
	/**************************
	* @brief: 接收视频刷新信号的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SlotIntelligentAvgBright(const QString &ip, const int value);

	/**************************
	* @brief: 接收视频刷新信号的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SlotAutoExposureAvgGray(const QString &ip, const int value);

	void SlotForbid(bool used);
private:
	/************************
	* @brief:设置编辑框数值
	* @author: mpp
	*************************/
	void SetEditValue(Device::RoiTypes type);
signals:
	/************************
	* @brief: 设置roi颜色信号
	* @param color: roi颜色
	* @author:mpp
	*************************/
	void SignalSetColorRoi(QColor color);
	/************************
	* @brief: 通知需要进入绘制roi
	* @author:whf
	*************************/
	void SignalManualDrawRoi();

private slots:
	/************************
	* @brief: 接收选择的颜色信号的槽函数
	* @param color: 颜色
	* @author: mpp
	*************************/
	void SlotSelectColor(QColor color);

    void on_AreaDisplayBtn_clicked(bool checked);

    void on_AreaCenterBtn_clicked();

    void on_MaxTriAreaBtn_clicked();

	void SlotSlider_lineEditChanged(const QString &text);
	void SlotSliderReleased();

	/**************************
	* @brief: 选择灰度模式槽函数
	* @param index: 选择索引
	* @author: mpp
	***************************/
	void SlotComboBox(int index);

	void SlotComboNumber_condition(int index);
	void on_lineEdit_number_editingFinished();
	void SlotCoordinate_X_lineEditFinished();
	void SlotCoordinate_Y_lineEditFinished();
	void SlotWidth_lineEditFinished();
	void SlotHeight_lineEditFinished();
	//void SlotCurrentAreaPixels(const uint64_t value);
protected:
	void showEvent(QShowEvent *) override;
private:
	/************************
    * @brief: 初始化UI
	* @author: mpp
	*************************/
	void InitUI();

	void ShowCtrl(bool bNoraml);

	/**************************
	* @brief: 连接信号槽
	* @author: mpp
	***************************/
	void ConnectSignalSlot();

	/************************
	* @brief: 设置区域灰度值/亮度值
	* @param value: 数值
	* @author: mpp
	*************************/
	void SetAreaValue(const int value);

	/************************
	* @brief: 获取更改编辑框后的rect
	* @author: mpp
	*************************/
	QRect GetRect() const;

	void UpdateUI2Grabber(bool enable);
private:
	int m_iType{};    //弹框类型
	ColorQPushButton* m_colorBtn;  //显示颜色按钮
	QWeakPointer<Device> current_device_;
	QIntValidator* m_coordinate_X_Validator;
	QIntValidator* m_coordinate_Y_Validator;
	QIntValidator* m_width_Validator;
	QIntValidator* m_height_Validator;	
	int m_bDevicePixelBit{ 8 }; //设备像素位深
	QRect m_current_rect; // 当前ROI区域
	int m_current_area_mode{0}; //当前区域选择
private:
    Ui::CSParamSetting *ui;
};

class ColorQPushButton : public QPushButton
{
	Q_OBJECT
public:
	ColorQPushButton(QWidget* parent);
	~ColorQPushButton();

	/************************
	* @brief: 设置特定颜色
	* @param color: 颜色
	* @author: mpp
	*************************/
	void SetColor(QColor color);
signals:
	/************************
	* @brief: 发送选择的颜色信号
	* @param color: 颜色
	* @author: mpp
	*************************/
	void SignalSelectColor(QColor color);
private:
	/************************
	* @brief: 获取颜色
	* @author: mpp
	*************************/
	QColor GetColor() const;

	/************************
	* @brief: 鼠标按下事件响应
	* @param event: 事件指针
	* @author: mpp
	*************************/
	void mousePressEvent(QMouseEvent* event);
};

#endif // CSPARAMSETTING_H
