#ifndef REALTIMEOTHERSETTINGSWIDGET_H
#define REALTIMEOTHERSETTINGSWIDGET_H

#include <QSharedPointer>
#include <QWidget>

class Device;

namespace Ui {
	class RealtimeOtherSettingsWidget;
}

/**
*@brief 实时预览-基本参数设置-其他设置界面类
**/
class RealtimeOtherSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RealtimeOtherSettingsWidget(QWidget *parent = 0);
	~RealtimeOtherSettingsWidget();

	/**
	* @brief 设置设备
	* @param device_ptr 设备指针
	*/
	void setDevice(QSharedPointer<Device> device_ptr);


signals:
	/**
	* @brief 显示信息信号
	* @param b_visible 是否显示
	*/
	void sigSetDisplayInfoVisible(bool b_visible);

	/**
	* @brief 返回
	*/
	void backButtonClicked();

private slots :
	/**
	* @brief 转到其他设置根界面
	*/
	void toOthersSettingRootWidget();

    /**
     * @brief 转到协议格式设置界面
     */
    void toStreamFormatWidget();

    /**
     * @brief 转到模拟增益设置界面
     */
    void toAnalogGainWidget();

    /**
     * @brief 转到触发方式设置界面
     */
    void toTriggerTypeWidget();

    /**
     * @brief 转到外触发具体方式设置
     */
    void toExTriggerTypeWidget();

	/**
	* @brief 视频信息显示设置
	* @param visible true-可见 ，false-不可见
	*/
	void setDisplayInfoVisible(bool visible);

	/**
	* @brief 设置触发方式
	* @param trigger_type 触发方式
	*/
	void setTriggerType(const QVariant & trigger_type);

	/**
	* @brief 设置外触发具体方式
	* @param ex_trigger_type 外触发具体方式
	*/
	void setExTriggerType(const QVariant & ex_trigger_type);

	/**
	* @brief 设置模拟增益
	* @param analog_gain 模拟增益
	*/
	void setAnalogGain(const QVariant & analog_gain);

	/**
	* @brief 设置协议格式
	* @param stream_format 协议格式
	*/
	void setStreamFormat(const QVariant & stream_format);

	/**
	* @brief 更新其他设置界面
	*/
	void updateOtherSettingsUI();

	/**
	* @brief 更新协议格式选项
	*/
	void updateStreamTypeUI();

	/**
	* @brief 更新模拟增益选项
	*/
	void updateAnalogGainUI();

	/**
	* @brief 更新触发方式选项
	*/
	void updateTriggerTypeUI();

	/**
	* @brief 更新外触发方式选项
	*/
	void updateExTriggerTypeUI();

private:
	/**
	* @brief 初始化界面
	*/
	void initUi();

	/**
	* @brief 获取设备
	* @return 设备指针
	*/
	QSharedPointer<Device> getDevice() const;

private:
	Ui::RealtimeOtherSettingsWidget *ui;
	QWeakPointer<Device> device_wptr_;
};

#endif // REALTIMEOTHERSETTINGSWIDGET_H
