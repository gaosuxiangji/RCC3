#ifndef REALTIMEWIDGET_H
#define REALTIMEWIDGET_H

#include "RMAImage.h"
#include <QWidget>
#include "Device/device.h"

namespace Ui {
class RealtimeMainWidget;
}

/**
 * @brief 实时图像主界面类
 */
class RealtimeMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealtimeMainWidget(QWidget *parent = 0);
    ~RealtimeMainWidget();

public slots:
	/**
	*@brief 设置是否全屏
	*@param [in] : benabled : bool，true-使能，false-不使能
	**/
    void setFullScreen(bool benable);

	/**
	*@brief 设置十字线中心
	*@param [in] : pt : const QPoint &，中心点坐标
	**/
    void setFocusPoint(const QPoint &pt);

	/**
	*@brief 设置roi
	*@param [in] : roi : const QRect &，roi区域
	**/
    void setRoi(const QRect &roi);

	/**
	*@brief 设备连接响应槽函数
	*@param [in] : device_ip : const QString &，设备ip
	**/
    void onDeviceConnected(const QString & device_ip);

	/**
	*@brief 设备断连响应槽函数
	*@param [in] : device_ip : const QString &，设备ip
	**/
    void onDeviceDisconnected(const QString & device_ip);

    /**
     * @brief 当前播放器切换
     * @param ip 设备IP
     */
    void onFocusPlayerChanged(const QString & ip);

	/**
	*@brief 截图快照
	*@param [in] : ip : const QString&，设备ip
				   img : const RMAImage&，图像
	*@return
	**/
	void slotSanpshot(const QString& ip, const RMAImage& img);

	/**
	*@brief 播放器中roi发生变化
	*@param [in] : ip : const QString&，设备ip
				    roi : const QRect&，roi区域
	*@return
	**/
	void onRoiChanged(const QString& ip, const QRect& roi);

	/**
	*@brief roi修改完成响应槽函数
	*@param [in] : b_applyed : bool，是否应用，true-应用，false-不应用
				   id : const QVariant&，设备id
				   roi : const QRect&，应用的roi
	**/
	void onRoiChangeFinished(bool b_applyed, const QString &ip, const QRect &roi);

	/**
	*@brief 播放器中十字线中心发生变化
	*@param [in] : ip : const QString&，设备ip
				   pt : const QPoint&，十字线焦点
	*@return
	**/
	void onFocusPointChanged(const QString& ip, const QPoint& pt);

	/**
	*@brief 播放器中十字线中心是否显示
	*@param [in] : ip : const QString&，设备ip
				   bvisible : bool，true-显示，false-不显示
	*@return
	**/
	void onFocusPointVisible(const QString &ip, bool bvisible);

	/**
	*@brief 设备属性发生变化
	*@param [in] : type : Device::PropType，发生变化的属性
				   value : const QVariant&，属性值
	*@return
	**/
	void onDevicePropertyChanged(Device::PropType type, const QVariant & value);

	/**
	*@brief 设备状态发生变化响应槽函数
	*@param [in] : state : DeviceState，状态
	**/
	void onDeviceStateChanged(DeviceState state);

signals:
	/**
	*@brief 全屏信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
    void fullScreen(bool benable);

	/**
	*@brief 选中窗口变化信号
	*@param [in] : ip : const QVariant& ，相机ip
	**/
	void focusPlayerChanged(const QString& ip);

private slots:
	/**
	*@brief 播放器更新属性
	*@param [in] : ip : const QString&，设备ip
	*@return
	**/
	void updateProperty(const QString& ip);

private:
    Ui::RealtimeMainWidget *ui;
};

#endif // REALTIMEWIDGET_H
