#ifndef REALTIMECTRLWIDGET_H
#define REALTIMECTRLWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include <QVariant>




class Device;
class QButtonGroup;
class QSpinBox;

namespace Ui {
class RealtimeCtrlWidget;
}

/**
 * @brief 实时图像控制界面类
 */
class RealtimeCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RealtimeCtrlWidget(QWidget *parent = 0);
    ~RealtimeCtrlWidget();

	enum DeviceTopoState //当前设备拓扑状态 (级联状态)
	{
		NoTopo = 0,//无级联状态
		ParentDevice,//是级联状态,父设备
		ChildDevice//是级联状态,子设备
	};

       /**
        * @brief 设置设备
        * @param device_ptr 设备指针
        */
       void setDevice(QSharedPointer<Device> device_ptr);

       /**
        * @brief 获取设备
        * @return 设备指针
        */
       QSharedPointer<Device> getDevice() const;



signals:

       /**
        * @brief 显示信息信号
        * @param b_visible 是否显示
        */
       void sigSetDisplayInfoVisible(bool b_visible);

public slots:

       void slotUpdateDynamicRoi(const QString& ip, const QRect& roi);

private slots:

    /**
     * @brief 转到参数设置根界面
     */
    void toParamSettingRootWidget();

    /**
     * @brief 转到分辨率设置界面
     */
    void toResolutionWidget();

    /**
     * @brief 转到帧率设置界面
     */
    void toFPSWidget();

    /**
     * @brief 转到曝光时间设置界面
     */
    void toExposureTimeWidget();

	/**
	* @brief 转到其他设置界面
	*/
	void toOtherSettingsWidget();

    /**
     * @brief 转到保存起点方式设置
     */
    void toRecordingOffsetModeWidget();

    /**
     * @brief 转到保存单位设置
     */
    void toRecordingUnitWidget();

    /**
     * @brief 转到保存格式设置
     */
    void toRecordingFormatWidget();

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
     * @brief 设置曝光时间
     * @param exposure_time 曝光时间选项
     */
    void setExposureTime(const QVariant & exposure_time);

    /**
     * @brief 设置帧率
     * @param fps 帧率
     */
    void setFPS(const QVariant & fps);

    /**
     * @brief 设置分辨率
     * @param resolution 分辨率
     */
    void setResolution(const QVariant & resolution);

    /**
     * @brief 设置保存起点模式
     * @param recording_offset_mode 触发前、触发后
     */
    void setRecordingOffsetMode(const QVariant & recording_offset_mode);

    /**
     * @brief 设置保存起点
     * @param recording_offset
     */
    void setRecordingOffset();

    /**
     * @brief 设置保存长度
     * @param recording_length
     */
    void setRecordingLength();

    /**
     * @brief 设置保存单位
     * @param recording_unit
     */
    void setRecordingUnit(const QVariant & recording_unit);

    /**
     * @brief 设置保存视频格式
     * @param recording_format
     */
    void setRecordingFormat(const QVariant & recording_format);

private slots:
    void onDeviceStateChanged();

    void onDevicePropertyChanged();

    void onDeviceErrorOccurred();

    void on_pushButtonPreview_clicked();

    void on_pushButtonAcquireTrigger_clicked();

    void on_pushButtonStop_clicked();


private:
    /**
     * @brief 初始化界面
     */
    void initUi();

	void updateDeviceTopoState();

    /**
     * @brief 更新设备控制界面
     */
    void updateDeviceCtrlUi();

    /**
     * @brief 更新参数设置界面
     */
    void updateParamSettingUI();

    /**
     * @brief 更新分辨率选项
     */
    void updateResolutionUI();

    /**
     * @brief 更新帧率选项
     */
    void updateFpsUI();

    /**
     * @brief 更新曝光时间选项
     */
    void updateExposureTimeUI();

    /**
     * @brief 更新保存起点方式选项
     */
    void updateRecordingOffsetTypeUI();

    /**
     * @brief 更新保存起点编辑框
     */
    void updateRecordingOffsetUI();

    /**
     * @brief 更新保存长度编辑框
     */
    void updateRecordingLengthUI();

	/**
	* @brief 更新其他设置控件显示
	*/
	void updateOtherSettingsWidget();

    /**
     * @brief 更新保存单位选项
     */
    void updateRecordingUnitUI();

    /**
     * @brief 更新保存格式选项
     */
    void updateRecordingFormatUI();

	//更新采集参数示意图
	void updateAcqParamDiagram();

    /**
     * @brief 高采模式相关参数按钮使能
     * @param b_enable true-启用 false-置灰
     */
    void enableAcqModeRelatedWidgets(bool b_enable);

	/**
	* @brief 拓扑相关参数按钮使能
	* @param b_enable true-启用 false-置灰
	*/
	void enableTopoRelatedWidgets(bool b_enable);

	/**
	* @brief 设置控件列表使能
	* @param widgets_to_enable 控件列表
	* @param b_enbable 使能状态
	*/
	void SetWidgetsEnable(QList<QWidget*> widgets_to_enable, bool b_enbable);

	/**
	* @brief 设置父设备
	* @param device_ptr 父设备指针
	*/
	void setParentDevice(QSharedPointer<Device> device_ptr);

	/**
	* @brief 获取父设备
	* @return 父设备指针
	*/
	QSharedPointer<Device> getParentDevice() const;

	/**
	* @brief 设置子设备
	* @param device_ptr 子设备指针
	*/
	void setChildDevice(QSharedPointer<Device> device_ptr);

	/**
	* @brief 获取子设备
	* @return 子设备指针
	*/
	QSharedPointer<Device> getChildDevice() const;

    /**
     * @brief 将父设备的相关参数设置到子设备
     */
    void setParentProperties2ChildDevice();

private:
    Ui::RealtimeCtrlWidget *ui;

    QWeakPointer<Device> device_wptr_;

	QWeakPointer<Device> parent_device_wptr_;

	QWeakPointer<Device> child_device_wptr_;



	 
	//保留的自定义选项
	QVariant resolution_dynamic_;


	QList<QWidget*> widgets_to_enable_;//需要控制使能的控件列表

	DeviceTopoState cur_device_topo_state_;//当前设备拓扑状态

	QSpinBox* spin_box_exposure_time_{ nullptr };
};

#endif // REALTIMECTRLWIDGET_H
