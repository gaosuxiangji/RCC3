/***************************************************************************************************
** @file: 国产化国产化采集控制系统界面
** @author: mpp
** @date: 2022-02-18
*
*****************************************************************************************************/
#ifndef CSRCCAPP_H
#define CSRCCAPP_H

#include <QMainWindow>
#include <QLabel>
#include <QPaintEvent>
#include <QActionGroup>
#include <QPainter>
#include <QPointer>
#include <QWidget>
#include <QHBoxLayout>
#include <QImage>
#include <QKeyEvent>
#include <QAbstractNativeEventFilter>
#include "edrexposureparamsetting/csedrexposureparamsetting.h"
#include "windowcrosslinesetting/cswindowcrosslinesetting.h"
#include "paramsetting/csparamsetting.h"
#include "Device/devicestate.h"
#include <array>
#include "csnetconfig/csnetconfig.h"
#include <QDockWidget>
#include "../csrccapptranslator.h"
#include "../../PointMeasure/CMeasureLineManage.h"
#include <QProgressDialog>
#include "Device/csdevicelistwidget.h"
#include "Device/csdevicesearchlistwidget.h"
#include "Property/cspropertyviewdevice.h"
#include "PointMeasure/CSMeasurePageDlg.h"
#include "Device/Temperature/csdevicetemperaturepanel.h"
#include "Device/cscf18controlpanel/cscf18controlpanel.h"
#include "Video/cslistviewvideo.h"
class CPlayerViewBase;
class CSViewManager;
class Device;
class DeviceManager;
class CSDlgImageTrainning;
namespace Ui {
class CSRccApp;
}

class CSDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	CSDockWidget(QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags()) {};
private:
	virtual void closeEvent(QCloseEvent *event) override;
signals:
	void SignalDocWidgetClose();
};

class CSRccApp : public QMainWindow ,public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit CSRccApp(QWidget *parent = 0);
    ~CSRccApp();

	/**
	* @brief 主界面显示后事件
	* @note 用于自动开启设备搜索
	*/
	void onMainWindowShown();

	/************************
	* @brief: 获取视图管理器
	* @return : 视图管理器
	* @author: chenyun
	*************************/
	CSViewManager* GetViewManagerPtr() const
	{
		return q_check_ptr(m_pViewManage);
	}

	/**
	* @brief 更新全部操作UI显示
	*/
	void updateMainWindowUI();

	/**
	* @brief 刷新设备属性列表
	* @note 用于设备属性变更后的主动刷新
	*/
	void UpdateDevicePropertyList();

	/**
	* @brief 主界面roi相关控件使能
	* @note 用于控制roi相关的界面使能
	*/
	void EnableRoiRelatedWidgets(bool enable );


	/**
	* @brief 添加设备到对应视图
	* @note 用于从设备获取状态时直接绑定视图
	*/
	void AddDeviceToView(const QString device_ip);
private:
	/************************
	* @brief: 初始化信息
	* @author: mpp
	*************************/
	void Init();

    /************************
    * @brief: 初始化ui信息
    * @author: mpp
    *************************/
    void InitUI();

	/************************
	* @brief: 初始化ui信息
	* @param index: 播放控件索引
	* @author: mpp
	*************************/
	void InitVideoWidget(int index);

	/**************************
	* @brief: 连接信号槽
	* @author: mpp
	***************************/
	void ConnectSignalSlot();

	/**
	* @brief 更新文件操作UI显示
	*/
	void updateFileActionsUI();

	/**
	* @brief 根据状态更新采集相关操作UI
	*/
	void updateAcquisitionActionsUI();

	/**
	* @brief 更新设置操作UI显示
	*/
	void updateSettingActionsUI();

	/**
	 * @brief 根据状态更新图像操作UI
	 */
	void updateImageActionsUI();

	/**
	* @brief 根据状态更新图像中的白平衡操作UI
	*/
	void updateWhiteBalanceActionsUI();

	/**
	* @brief 根据状态更新图像中的色彩模式操作UI
	*/
	void updateColorModeActionsUI();

	/**
	* @brief 根据状态更新图像中的拍摄环境操作UI
	*/
	void updateWbEnvActionsUI();

	/**
	* @brief 更新工具操作UI显示
	*/
	void updatToolsActionsUI();

	/**
	* @brief 更新其他UI显示
	*/
	void updateOtherUI();

	//更新工具中的图像控制UI
	void updateImageCtrlUI();

	/**
	* @brief 更新视图操作UI显示
	*/
	void updatViewsActionsUI();

	/**
	* @brief 更新帮助操作UI显示
	*/
	void updatHelpsActionsUI();

	/**************************
	* @brief: 更新运行报警UI显示
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	void UpdatAlarmActionsUI();

	//刷新温控列表显示
	void UpdateTemperaturePanelDisplay(QSharedPointer<Device> device_ptr);

	//刷新CF18控制列表显示
	void UpdateCF18ControlPanelDisplay();

	/************************
	* @brief: 设置状态栏X信息
	* @param value: x的值
	* @author: mpp
	*************************/
	void SetXStatusValue(int value);

	/************************
	* @brief: 设置状态栏Y信息
	* @param value: y的值
	* @author: mpp
	*************************/
	void SetYStatusValue(int value);

	/************************
	* @brief: 刷新QLabel样式
	* @param label: 需要刷新样式的label
	* @param name: 样式名称
	* @author: mpp
	*************************/
	void RefreshQLabelStyle(QLabel* label, const QString& name);

	/************************
	* @brief: 设置缩放比例
	* @param coef:缩放比例
	* @author: mpp
	*************************/
	void SetZoomCoefficient(const float coef);

	/************************
	* @brief: 设置状态栏帧率信息
	* @param rate:帧率
	* @author: mpp
	*************************/
	void SetFrameRate(const int rate);

	/************************
	* @brief: 设置状态栏旋转信息
	* @param angle:旋转角度
	* @author: mpp
	*************************/
	void SetStatusBarRotateValue(int angle);


	/************************
	* @brief: 设置灰度值或RGB值
	* @param bshow:是否显示
	* @author: mpp
	*************************/
	void SetGrayOrRGB(bool bshow = true);

	/**
	* @brief 根据实验信息更新app标题 
	* @param exp_name 实验名称
	* @param exp_code 实验代号
	* @note
	*/
	void UpdateAppTitle(QString exp_name, QString exp_code);

	/************************
	* @brief: 更新采集窗口roi状态
	* @author: mpp
	*************************/
	void UpdateDeviceRoiStatus( QString device_ip);

	/************************
	* @brief: 更新自动曝光窗口roi状态
	* @author: mpp
	*************************/
	void UpdateAutoExposureRoiStatus(QString device_ip);

	/************************
	* @brief: 更新智能触发窗口roi状态
	* @author: mpp
	*************************/
	void UpdateIntelligentTriggerRoiStatus(QString device_ip);

	/************************
	* @brief: 更新二次曝光状态
	* @author: mpp
	*************************/
	void UpdateEdrExposureStatus();

	/************************
	* @brief: 更新焦点状态
	* @author: mpp
	*************************/
	void UpdatePropFocusPointStatus();

	/**************************
	* @brief: 更新过曝提示状态
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/19
	***************************/
	void UpdateOverExposureStatus();

	/**************************
	* @brief: 更新曝光时间数值
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/20
	***************************/
	void UpadateExploreTimeValue();

	/************************
	* @brief: 获取图像中心位置
	* @return : 中心点坐标
	* @author: mpp
	*************************/
	QPointF GetPictureCenterPoint() const;


	/**************************
	* @brief: QStringList转unsigned char数组
	* @param: strList-QStringList字符串  arr-unsigned char数组
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	void QString2UCharArray(const QStringList &strList, BYTE *arr);

	/**************************
	* @brief: 根据报警信息生成格式化好的报警文字内容
	* @param:pDevice-设备指针
	* @param:warning-告警信息
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	QString FormatWarningMessage(QSharedPointer<Device> pDevice, const Device::Warning warning);

	/**************************
	* @brief: 获取设备描述，格式为：类型[名称 IP或SN]，如：相机[test 192.168.10.1]
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	QString GetDesc() const;

	void SetBcode(const bool isOn);

	void SetAvgLum(const uint32_t avgLum);

	QRect GetRoiTypeRect(Device::RoiTypes type, QSharedPointer<Device> device_ptr);
	QPoint GetRoiOffsetPoint(QSharedPointer<Device> pDevice);
	void setDeviceRoiEditDisabled();

	void cf18ControlProcess(bool connected);
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;

	/**
	*@brief 显示事件
	*@param [in] : * event : QShowEvent，事件指针
	**/
	void showEvent(QShowEvent *event) override;

	/**
	*@brief 隐藏事件
	*@param [in] : * event : QShowEvent，事件指针
	**/
	void hideEvent(QHideEvent *event) override;
signals:
	/**************************
	* @brief: 本地视频列表-点击播放按钮后发送信号
	* @param: path 视频路径
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void SignalLocalVideoDispaly(const QString& path);

	/**************************
	* @brief: 智能触发亮度值信号
	* @param: value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SignalIntelligentAvgBright(const QString &ip, const int value);

	/**************************
	* @brief: 自动曝光灰度值信号
	* @param: value 灰度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SignalAutoExposureAvgGray(const QString &ip, const int value);

	void SignalForbid(bool used);
	void SignalEdrForbid(bool used);

	void SignalRecoveringDataProgress(int nProgress);
	void signalDisconnect2GainSet();

	/************************
	* @brief: 取消编辑
	* @author:
	*************************/
	void SignalCancelEdit();
private slots:
	/************************
	* @brief: 响应deviceConnected信号的槽函数
	* @param devicename: 设备名称
	* @author: mpp
	*************************/
	void SlotDeviceConnected(const QString& devicename);

	/************************
	* @brief: 响应deviceDisconnected信号的槽函数
	* @param devicename: 设备名称
	* @author: chenyun
	*************************/
	void SlotDeviceDisconnected(const QString& devicename);

	/************************
	* @brief: 槽函数 当前设备切换,响应DeviceManager
	* @param device_ip: 设备对应的IP
	* @author: chenyun
	*************************/
	void SlotCurrentDeviceChanged(const QString& device_ip);

	/************************
	* @brief: 槽函数 响应设备温度更新
	* @param temperature_info: 温度信息
	* @author: chenyun
	*************************/
	void SlotTemperatureInfoUpdated(QString temperature_info);

	/************************
	* @brief: 槽函数 当前设备的状态切换,响应Device
	* @param device_state: 新的设备状态
	* @author: chenyun
	*************************/
	void SlotCurrentDeviceStateChanged(const QString &current_ip,DeviceState device_state);

	/************************
	* @brief: 响应窗口点击信号槽函数
	* @param index: 窗口索引
	* @author: mpp
	*************************/
	void SlotDisplayWidgetClicked(int index);

	/**
	*@brief	响应播放窗口布局响应
	*@param 
	*@return
	**/
	void SlotDisplayWidgetAdjust(int);

	/************************
	* @brief: 鼠标位置变化信号的槽函数
	* @param pt: 鼠标所在位置图像坐标
	* @author: mpp
	*************************/
	void SlotCurrentCursorInVideo(const QPoint& pt);

	/************************
	* @brief: 缩放系数变化的槽函数
	* @param coef: 缩放系数
	* @author: mpp
	*************************/
	void SlotZoomCoefficientChanged(float coef);

	/************************
	* @brief: 更新帧率槽函数
	* @param fps: 帧率
	* @author: mpp
	*************************/
	void SlotUpdateFrameRate(qreal fps);

	void slotStopDevices(const QStringList& devices);


	/************************
	* @brief: 白平衡相关操作响应
	* @author:chenyun
	*************************/
	void SlotWhiteBalanceActionGroupTriggered();

	/************************
	* @brief: 更新灰度值和RGB值信号
	* @param pt: 鼠标点
	* @param matImage: 目标图像
	* @author: mpp
	*************************/
	void SlotUpdateGrayOrRGBValue(const QPoint& pt, const cv::Mat& matImage);

	/************************
	* @brief: 色彩模式相关操作响应
	* @author:chenyun
	*************************/
	void SlotColorModeActionGroupTriggered();

	/************************
	* @brief: 拍摄环境相关操作响应
	* @author:chenyun
	*************************/
	void SlotWbEnvActionGroupTriggered();

	/************************
	* @brief: 风扇控制操作响应
	* @author:mpp
	*************************/
	void SlotFanControlActionGroupTriggered();

	/************************
	* @brief: 自动曝光操作响应
	* @author:mpp
	*************************/
	void SlotAutoExposureActionGroupTriggered();

	/************************
	* @brief: 智能触发操作响应
	* @author:mpp
	*************************/
	void SlotIntelligentTriggerActionGroupTriggered();

	/************************
	* @brief: 二次曝光操作响应
	* @author:mpp
	*************************/
	void SlotEdrExposureTriggerActionGroupTriggered();

	/**************************
	* @brief: 翻译操作响应
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/08
	***************************/
	void SlotTranslateTriggerActionGroupTriggered();

	/************************
	* @brief: 获取中心点坐标槽函数
	* @author:mpp
	*************************/
	void SlotGetPictureCenterPoint();

	/************************
	* @brief: 响应属性变化
	* @param type 属性类型
	* @param value 属性值
	* @author:mpp
	*************************/
	void SlotPropertyChanged(Device::PropType type, const QVariant & value);

	/************************
	* @brief: 响应窗口中心线交点坐标信号的槽函数
	* @param centerpoint: 交点坐标
	* @author: mpp
	*************************/
	void SlotUpdateCustomCrossLineCenterPoint(const QPointF& centerpoint);

	/************************
	* @brief: 响应窗口中心线交点移动信号的槽函数
	* @param centerpoint: 交点坐标
	* @author: mpp
	*************************/
	void SlotCustomCrossLineCenterMovePoint(const QPoint& centerpoint);

	void updateRoiInfoSlot(const Device::RoiTypes& type, const QRect& rect);

	/**************************
	* @brief: 本地视频列表-接收点击播放按钮后发送信号的槽函数
	* @param: path 视频路径
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void SlotLocalVideoDispaly(const QString& path);

	/**************************
	* @brief: 接收智能触发亮度值
	* @param:value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SlotUpdateFrameIntelligentAvgBright(const QString &ip, const int value);

	/**************************
	* @brief: 接收自动曝光灰度值
	* @param:value 灰度值
	* @return:
	* @author:mpp
	* @date:2022/05/06
	***************************/
	void SlotUpdateFrameAutoExposureAvgGray(const QString &ip, const int value);

	/**************************
	* @brief: 接收平均亮度的信号的槽函数
	* @param: avgLum-平均亮度值
	* @return:
	* @author:mpp
	* @date:2022/05/12
	***************************/
	void SlotAvgLum(const uint32_t avgLum);

	/**************************
	* @brief: 接收运行报警信息信号的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/13
	***************************/
	void SlotAlarmMsg();

	void SlotBcode(bool bcode);
	void SlotErrMsg(int error_code);

	/**************************
	* @brief: 接收断电数据恢复信号的槽函数
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void SlotRecoveringData();

	/**************************
	* @brief: 接收断电数据恢复信号进度的槽函数
	* @param:
	* @return:
	* @author:
	* @date:
	***************************/
	void SlotRecoveringDataProgress(int nProgress);

	/************************
	* @brief: 更新界面信息
	* @param:
	* @return:
	* @author:
	* @date:
	*************************/
	void SlotUpdateUIInfo();

	/************************
	* @brief: 更新旋转
	* @param:
	* @return:
	* @author:
	* @date:
	*************************/
	void SlotUpdateDeviceRotate(const QString strIP);

	/**
	* @brief 更新鼠标绘制状态信息
	*/
	void slotUpdateDrawStatusInfo(Device::DrawTypeStatusInfo info);
	void slotETSnap();
	void slotRemoveCurrentDevice();

	void slotTemperatureAbnormal(QString strName);

public slots:

	void on_actionPreview_P_triggered();

	void on_actionHigh_speed_Acquisition_triggered();

private slots:	
	void SlotImageCtrlActionGroupTriggered(QAction*);
    void on_actionStatus_Bar_S_triggered(bool checked);
    void on_actionDevice_List_triggered(bool checked);
    void on_actionProperty_List_triggered(bool checked);
    void on_actionVideo_List_triggered(bool checked);
	void on_actionMeasure_triggered(bool checked);
    void on_actionWindow_Tool_triggered(bool checked);
    void on_actionCamera_Acquisition_triggered(bool checked);
    void on_actionAcquisition_Window_triggered(bool checked);
    void on_actionRun_Alarm_triggered(bool checked);
    void on_actionUser_Manual_triggered(bool checked);
    void on_actionEnlarge_triggered();
    void on_actionZoom_Out_Z_triggered();
    void on_action1_1Show_triggered();
    void on_actionFit_Window_triggered();
    void on_actionCrossHair_triggered(bool checked);
    void on_actionWindow_Centerline_triggered(bool checked);
	void on_actionWindow_Centerline_Settings_triggered();
    void on_actionTake_photo_triggered();
    void on_actionTrigger_T_triggered();
    void on_actionStop_Camera_S_triggered();

    void on_actionImage_Training_E_triggered();

    void on_actionDisplay_The_Acquisition_Window_triggered(bool checked);

    void on_actionMaximize_The_Acquisition_Window_triggered();

    void on_actionCenter_The_Acquisition_Window_triggered();

    void on_actionAcquisition_Window_Settings_triggered(bool checked);

    void on_actionSet_Up_Darkfield_triggered();

    void on_action_DarkFieldCorrection_triggered(bool checked);

    void on_actionGain_Setting_triggered();

    void on_actionBrightness_Contrast_triggered();
    void on_action_Intelligent_Param_Setting_triggered();

    void on_action_AutoExposure_Param_Setting_triggered();

    void on_action_EdrExposure_Param_Setting_triggered();

    void on_actionRotate_The_Image_R_triggered();

    void on_actionExit_X_triggered();

    void on_action_Menu_LocalVideo_triggered();

    void on_action_O_triggered();

	void on_actionNew_File_N_triggered();

	void on_actionOpen_File_O_triggered();

	void on_actionSave_File_S_triggered();

    void on_actionDevice_Authorization_triggered();

    void on_actionMachine_Self_check_triggered();

    void on_actionHealth_Management_Settings_triggered();
    void on_actionAnti_aliasing_triggered(bool checked);

	void on_actionBinning_Mode_triggered(bool checked);

    void on_actionCompany_Website_W_triggered();

    void on_actionSoftware_Version_S_triggered();

    void on_actionUser_Manual_D_triggered();

    void on_actionNetwork_Configuration_triggered();

    void on_actionReset_triggered();




    void on_actionDrop_Measurement_Settings_triggered();

    void on_actionOnline_Upgrade_triggered();
    void on_actionRun_Alarm_W_triggered();

    void on_actionTemperature_Panel_triggered(bool checked);
	void on_actionControl_Panel_triggered(bool checked);

	void slotLowLightModeTrigger();

	void slotExportpreviewShowMain(bool bShow);

	void on_actionFilter_triggered(bool checked);
	void on_actionFiler1_triggered(bool checked);

protected:
	/**
	*@brief 关闭事件
	*@param [in] : *event : QCloseEvent，事件指针
	**/
	virtual void closeEvent(QCloseEvent *event) override;

	/**
	*@brief 键盘按下事件
	*@param [in] : *event : QKeyEvent，事件指针
	**/
	virtual void keyPressEvent(QKeyEvent *event)override;
	virtual void keyReleaseEvent(QKeyEvent *event)override;

	virtual bool eventFilter(QObject *obj, QEvent *event) override;

	//系统事件响应
	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
private:
	void UpdateStatusBarTranslate();
	void UpdateCapLabelStatus(bool bLight);
	void UpdateNumLabelStatus(bool bLight);
	void UpdateScrlLabelStatus(bool bLight);
	void RecoveringDataFail();
	void handleFrameRateChange();
	void updateProjectItem(bool bdiable);
	void updateLowLightActionStatus(bool enable);
private:
    QLabel* m_pXLabel;    //显示状态栏X信息
    QLabel* m_pYLabel;    //显示状态栏Y信息
	QLabel* m_pFrameRateLabel;    //帧率
	QLabel* m_pGrayOrRGBLabel;    //显示灰度值或RGB值
	QLabel* m_pAvgLumLabel;    //平均亮度
	QLabel* m_pZoomCoefficientLabel;    //缩放比例
	QLabel* m_pRotateLabel;    //旋转
	QLabel* m_pBcode;    //B码

	//当前设备指针
	QSharedPointer<Device> m_current_device_ptr;

	QToolBar * m_trigger_toolbar{nullptr};//西光所版本触发按钮工具栏

	QActionGroup * m_actionGroup_WhiteBalance;//单选操作组,白平衡
	QActionGroup * m_actionGroup_ColorMode;//单选操作组,色彩模式
	QActionGroup * m_actionGroup_WbEnv;//单选操作组,拍摄环境
	QActionGroup * m_actionGroup_FanControl;//单选操作组,风扇控制
	QActionGroup * m_actionGroup_AutoExposure;//单选操作组,自动曝光
	QActionGroup * m_actionGroup_ImageCtrl;//单选操作组,自动曝光	
	QActionGroup* m_actionGroup_IntelligentTrigger;//单选操作组,智能触发
	QActionGroup* m_actionGroup_EdrExposure;//单选操作组,二次曝光
	QActionGroup* m_actionGroup_Translate;//单选操作组，翻译
	QPointer<CSDlgImageTrainning> m_dlg_imageTraining;

    CSWindowCrossLineSetting* m_winCrossLineSet;    //窗口十字线弹框
	CSViewManager* m_pViewManage;    //播放控件管理类

	QGridLayout* m_mainLayout;    //播放控件布局器

	const int m_iViewNum;    //播放控件数量

	const int m_ui_max_size{ 16777215 };//界面尺寸最大值

	int m_iWindowIndex;    //鼠标点击窗口索引

	uint16_t m_nGrayOrRGBAtQueryPoint;	//查询点的灰度值
	std::array<uint16_t, 3> bgr_values;	//查询点的RGB值

	bool m_bColor = true;    //是否彩色模式，默认是
	uint32_t m_iEdrExploreTimeValue{ 0 };    //二次曝光时间数值

	bool m_bIntelligentOpened{ false };    //智能触发开启
	bool m_bAutoExposureOpened{ false };    //自动曝光开启

	float m_fZoomCoefficient{ 0 };
	int m_iFrameRate{ 0 };
	int m_iBarRotate{ 0 };
	bool m_bGrayOrRGB{ false };
	uint32_t m_uAvgLum{ 0 };
	bool m_bBarRotateVisible{ false };
	bool m_BcodeOn{ false };

	QLabel* m_pCapLabel;
	QLabel* m_pNumLabel;
	QLabel* m_pScrlLabel;
	Device::RoiTypes m_ExType{ Device::RoiTypes::kDeviceRoi };

	QProgressDialog* m_pProgressDlg{ nullptr };
	int m_nProgressValue{ -1 };
	bool m_bDataRecovery{ false };
	bool m_mid_btn_clicked{ false };
	QString m_project_info{};

	bool m_is_gateway_device{ false };
	CSDeviceListWidget* m_device_list_widget{};
	CSDeviceSearchListWidget* m_device_search_list_widget{};
	bool m_allow_auto_connect{ false };
	const int m_const_cf18_channel_num{ 8 };    //cf18共8个通道
	CSParamSetting *paramSettingPtr = nullptr;

	CSPropertyViewDevice* m_property_view_device_widget{};
	CSListViewVideo* m_list_view_video_widget{};
	CSDeviceTemperaturePanel* m_devicetemperaturepanel_widget{};
	CSMeasurePageDlg* m_measure_page_dlg_widget{};
	CSCF18ControlPanel* m_cf18_control_panel_widget{};
private:
    Ui::CSRccApp *ui;
};
#endif // CSRCCAPP_H
