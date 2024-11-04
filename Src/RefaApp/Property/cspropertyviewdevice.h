#ifndef CSPROPERTYVIEWDEVICE_H
#define CSPROPERTYVIEWDEVICE_H

#include <QWidget>
#include <QMap>
#include <QPointer>
#include <QTimer>
#include <qtpropertymanager.h>
#include <qteditorfactory.h>
#include <qtvariantproperty.h>
#include "Property/csintcomboproperty.h"
class DeviceManager;
class Device;

namespace Ui {
class CSPropertyViewDevice;
}
class DeviceManager;
class Device;

enum PropertyType // 属性类型
{
	kPropUnknown,
	kRoot,
	kGroupBasicProps, // 基本信息
	kGroupAdvancedProps, // 高级参数(其他参数)
	kGroupDeviceProps, // 设备参数
	kGroupRecordingProps, // 采集参数
	kPropModel, // 型号
	kPropIP, // IP
	kPropSN, // SN
	kPropDeviceName, // 名称
	kPropDeviceIndex, // 设备编号
	kPropStatus, // 状态
	kGroupRoi, // 采集窗口
	kPropResolution, // 分辨率
	kPropRoiOffsetX, // 起点坐标X
	kPropRoiOffsetY, // 起点坐标Y
	kPropRoiWidth, // 宽度
	kPropRoiHeight, // 高度
	kGroupTrigger, // 触发
	kPropTriggerMode, // 触发方式
	kPropSpacebarTrigger, // 空格键触发
	kPropFrameRate, // 帧率
	kGroupExposure, // 曝光
	kPropExposureTime, // 曝光时间
	kPropExposureTimeUnit, // 曝光时间单位
	kPropExposureDelay, // PIV曝光延迟
	kPropOverExposureTip, // 过曝提示
	kGroupSync, // 同步
	kPropSyncSource, // 同步方式
	kGroupVideoSave, // 视频保存
	kPropRecordingOffsetMode, // 保存起点方式
	kPropRecordingOffset, // 保存起点
	kPropRecordingLength, // 保存长度
	kPropRecordingUnit, // 单位
	kPropVideoFormat, // 保存格式
	kPropWatermark, // 叠加水印
	kPropImageNamePrefix, // 图片名称前缀
	kPropImageNameSuffix, // 图片名称后缀
	kPropRealtimeExport, // 实时导出
	kPropAutoExport, // 自动导出
	kPropExternalTriggerMode, // 外触发模式
	kPropTriggerSyncEnable, // 触发同步
	kPropJitterEliminationLength, // 消抖长度
	kPropPulseWidth, // 脉冲宽度
	kPropPulseWidth100Ns, // 脉冲宽度百纳秒
	kPropTriggerPulseWidth, // 同步控制器脉冲宽度
	kPropHardwareStandby, // 硬件待机
	kPropStreamType, // 协议格式
	kPropGroupName, // 组名
	kPropGroupDeviceModel, // 组内相机类型
	kPropGroupDeviceCount, // 组内成员数量
	kPropProductName, // 产品名称
	kPropProductVersion, // 产品版本
	kPropPulseMode, // 脉冲模式
	kPropPulseNumber, // 脉冲数量
	kGroupEDR, // EDR
	kPropDoubleExposureTime, // 二次曝光时间
	kPropDoubleExposureTimeUnit, // 二次曝光时间单位
	kPropEDRLowerThreshold, // 下限阈值
	kPropEDRUpperThreshold, // 上限阈值
	kPropEDRThreshold, // 过曝阈值
	kPropEnableDoubleExposureTime, // 启用二次曝光
	kGroupPIV, // PIV
	kPropEnablePIV, // 启用PIV
	kPropPIVExposureTime, // PIV曝光时间
	kPropPIVAcquisitionFrequency, // PIV采集频率
	kPropPIVAcquisitionFrequencyUnit, // PIV采集频率单位
	kPropLaserInterval, // 激光间隔
	kGroupLaserA, // 激光器A参数
	kPropLaserAReadyOffset, // 准备信号偏移
	kPropLaserASendOffset, // 发射信号偏移
	kPropLaserAPulseWidth, // 脉冲宽度
	kGroupLaserB, // 激光器B参数
	kPropLaserBReadyOffset, // 准备信号偏移
	kPropLaserBSendOffset, // 发射信号偏移
	kPropLaserBPulseWidth, // 脉冲宽度
	kPropStationName, // 站点编号
	kPropStationAlias, // 站点名
	kPropDelayChannel, // 延时通道
	kPropDelay, // 延时
	kGroupChannelDelay, // 通道延时
	kPropChn1Delay, // 通道1延时
	kPropChn2Delay, // 通道2延时
	kPropChn3Delay, // 通道3延时
	kPropChn4Delay, // 通道4延时
	kPropChn5Delay, // 通道5延时
	kPropChn6Delay, // 通道6延时
	kPropChn7Delay, // 通道7延时
	kPropChn8Delay, // 通道8延时
	kPropVfpnCompensation, // VFPN补偿
	kPropHobCompensation, // HOB补偿
	kPropDisplayHighestBit, // 显示最高比特位
	kGroupSdiParam, // SDI参数
	kPropSdiFpsResol, // SDI分辨率&播放帧率
	kPropTimeAvgImage, // 导出时间平均图像
	kPropTimeStdevImage, // 导出时间标准差图像
	kPropPosition, //位置，无/左/右，供无人机标定使用
	kPropStripeTemperatureLevel, // 条纹校正温度级别
	kPropPixelBitDepth, // 像素位深
	kPropDisplayBitDepth, // 显示像素位深
	kPropGropName, //分组名称
	kPropGropDeviceNum, //分组设备数量

	//CF18-control-begin//////////////////////////////////////////////////
	kPropCF18ExternalTriggerSignal, // 外触发信号
	kPropCF18ETJitterEliminationTime, // 外触发信号-消抖时间/μs
	kPropCF18ETPolarityReversal, // 外触发信号-极性反转
	kPropCF18ETRisingCount, // 外触发信号-上升沿计数
	kPropCF18ETFallingCount, // 外触发信号-下降沿计数
	kPropCF18ExternalSynSignal, // 外同步信号
	kPropCF18ENJitterEliminationTime, // 外同步信号-消抖时间/μs
	kPropCF18ENPolarityReversal, // 外同步信号-极性反转
	kPropCF18ENRisingCount, // 外同步信号-上升沿计数
	kPropCF18ENFallingCount, // 外同步信号-下降沿计数
	kPropCF18BSignal, // B码信号
	kPropCF18BJitterEliminationTime, // B码信号-消抖时间/μs
	kPropCF18InternalSynSignal, // 内同步信号
	kPropCF18INChannel, // 内同步信号-通道
	kPropCF18Cycle, // 周期
	kPropCF18Unit, // 单位
	kPropCF18INHighLevelWidth, // 内同步信号-高电平宽度/μs
	kPropCF18INRisingDelay, // 内同步信号-上升沿延迟/μs
	kPropCF18InternalTriggerSignal, // 内触发信号
	kPropCF18ITChannel, // 内触发信号-通道
	kPropCF18ITHighLevelWidth, // 内触发信号-高电平宽度/μs
	kPropCF18ITRisingDelay, // 内触发信号-上升沿延迟/μs
	//CF18-control-end////////////////////////////////////////////////////
};

typedef  QString(*pf)(int);//函数指针

/**
 * @brief 设备属性列表界面
 */
class CSPropertyViewDevice : public QWidget
{
    Q_OBJECT

public:
    explicit CSPropertyViewDevice(QWidget *parent = 0);
    ~CSPropertyViewDevice();

	/**
	**@ Brife	根据类型更新列表属性
	**@ Note	有设备时显示设备属性,没有设备时显示系统属性
	*/
	void UpdateCurrentPropertyList();

	/**
	**@ Brife	根据手动ROI变更时重置保存长度范围
	**@ Note	
	*/
	void UpdateRecordingLengthEnable() { m_bRecordUnitChange = true; };
private:
	/**
	**@ Brife	初始化界面
	*/
	void InitUI();

	/**
	**@ Brife	属性列表类型 - 相机,触发器,系统(未选中设备时)
	*/
	enum PropertyListType
	{
		ListForCamera,
		ListForTrigger,
		ListForSystem,
		ListForGroup
	};

	/**
	**@ Brife	根据类型更新列表属性
	**@ Param	list_type 属性列表类型 
	**@ Note	有设备时显示设备属性,没有设备时显示系统属性
	*/
	void updatePropertyList(PropertyListType list_type);

	/**
	**@ Brife	根据设备获取需要显示的属性列表
	**@ Param	device_ptr 当前设备  
	**@ Param	[out] prop_types 设备对应的属性种类列表, 传入的属性列表不会被清空
	**@ Note	没有设备时返回软件系统属性列表
	*/
	void getPropertyTypes(QSharedPointer<Device> device_ptr, QList<PropertyType> & prop_types);


	/**
	**@ Brife	在界面属性列表中批量更新属性值
	**@ Param	device_ptr 当前设备
	**@ Param	prop_types 需要刷新的属性种类列表 
	**@ Note	如果界面列表中没有对应的属性自动创建属性,包括其父节点
	*/
	void updatePropertiesValue(QSharedPointer<Device> device_ptr,const QList<PropertyType>  prop_types);

	/**
	**@ Brife	更新列表中的枚举值
	**@ Param	device_ptr 当前设备
	**@ Param	prop_type 需要刷新的属性种类
	**@ Param	device_prop_type 设备对应的属性种类
	**@ Param	pfunction 获取属性文案的函数指针
	**@ Param	allow_edit 是否允许修改
	**@ Note	如果设备当前属性值设备不支持,则将设备的属性设置为支持的第一个属性值
	*/
	void updateEnumProperty(QSharedPointer<Device> device_ptr, const PropertyType prop_type, int device_prop_type, pf pfunction,bool allow_edit);

	/**
	**@ Brife	在界面属性列表中更新属性值
	**@ Param	device_ptr 当前设备
	**@ Param	prop_type 需要刷新的属性种类
	**@ Note	如果界面列表中没有对应的属性自动创建属性,包括其父节点
	*/
	void updatePropertyValue(QSharedPointer<Device> device_ptr,const PropertyType  prop_type);


	/**
	**@ Brife	添加或更新属性组(属性组)
	**@ Param	prop_item  需要添加到组内的属性
	**@ Param	prop_group 属性组
	**@ Note
	*/
	void addProperty(QtProperty* prop_item, QtProperty* prop_group);

	/**
	**@ Brife	添加或更新属性和属性值(字符串)
	**@ Param	prop_type 需要添加或更新的属性
	**@ Param	value_str 属性值
	**@ Param	allowsEdit 是否允许编辑
	**@ Note	
	*/
	void addProperty(const PropertyType  prop_type ,const QString value_str , bool allowsEdit = false );

	void addUserDefineProperty(const PropertyType  prop_type, const QString value_str, bool allowsEdit = false);

	/**
	**@ Brife	添加或更新属性和属性值(枚举值)
	**@ Param	prop_type 需要添加或更新的属性
	**@ Param	option_str_list 属性选项列表
	**@ Param	option_index 当前选中属性序号
	**@ Param	allowsEdit 是否允许编辑
	**@ Note
	*/
	void addProperty(const PropertyType  prop_type, const QStringList option_str_list,int option_index, bool allowsEdit = false);

	/**
	**@ Brife	添加或更新属性和属性值(数值)
	**@ Param	prop_type 需要添加或更新的属性
	**@ Param	value 属性值 
	**@ Param	min 最小值
	**@ Param	max 最大值
	**@ Param	bEnable 是否允许变更
	**@ Note 最大值和最小值均为0时,代表参数无限制
	*/
	void addProperty(const PropertyType  prop_type, long value, long min = 0, long max = 0, long inc = 0, bool bEnable =true);

	/**
	**@ Brife	获取属性类型的父节点
	**@ Param	property_type	属性类型
	**@ Return	父节点
	**@ Note	如果属性父节点不存在的话会被递归创建到根节点,并且添加到列表中.
	*/
	QtProperty * getPropertyParent(PropertyType property_type);

private slots:

/**
**@ Brife	槽函数 当前设备切换
**@ Param	current_ip 当前设备ip
**@ Note	设备切换后刷新属性列表
*/
void slotCurrentDeviceChanged(const QString current_ip);
void slotCurrentGroupChanged(const QString group_name, const QString device_num);

/**
**@ Brife	槽函数 设备状态切换
**@ Note	状态切换后刷新属性列表
*/
void slotDeviceStateChanged();

/**
**@ Brife	槽函数 设备属性切换
**@ Note	属性切换后刷新属性列表
*/
void slotDevicePropertyChanged();

void slotPropertyEditingFinished(QtProperty *property);
/**
**@ Brife	槽函数 属性值变更
**@ Param	property	属性控件
**@ Param	val	属性值
**@ Note	属性值变更后下发给设备,与相关属性产生联动,刷新属性列表
*/
void slotPropertyValueChanged(QtProperty *property, const QVariant &val);

/**
**@ Brife	槽函数 选中项变更
**@ Param	browser_item	属性控件
**@ Note	选中项变更后切换描述窗口
*/
void slotPropertyItemChanged(QtBrowserItem* browser_item);

void slotUpdateCF18Info();

protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;
private:

	//当前的设备
	QSharedPointer<Device> m_currrent_device_ptr;
	//之前选择的属性类型列表
	QList<PropertyType> m_old_property_types;
	//属性关系表, <属性类型, <属性名称,属性描述,属性父节点,是否有子节点> >
	QMap<PropertyType, std::tuple<QString, QString, PropertyType, bool>> m_map_props;

	//属性管理工厂类
	QtVariantPropertyManager * m_property_manager;
	//属性编辑工厂类
	QtVariantEditorFactory * m_property_editor;

	QPointer<DeviceManager> m_device_magager_ptr;//设备管理器

	QMap<PropertyType, QMap<int, QVariant > > m_map_enums;//枚举类型对应的选项顺序: <属性种类,<选项序号,属性值>>

	QMap<QtProperty*, PropertyType> m_map_edit_type;//属性控件对应的属性类型:<属性控件指针,属性类型>
	QMap<PropertyType,QtProperty* > m_map_type_edit;//属性类型对应的属性控件:<属性类型,属性控件指针>
	
	bool m_enable_value_change{ false };//允许属性变更(由于属性列表在刷新内容时也发出属性变更信号,需要限制信号触发动作)

	bool m_bRecordUnitChange { false };	// 记录手动变动单位，此时需要更新保存长度范围
	QString m_group_name{};
	QString m_group_device_num{};
	bool m_is_group{ false };
	//CF18相关
	const int kUpdateCF18InfoInterval{ 2000 }; // 更新CF18信息间隔
	QPointer<QTimer> m_CF18_info_timer_ptr; //CF18更新定时器
    Ui::CSPropertyViewDevice *ui;
};

#endif // CSPROPERTYVIEWDEVICE_H
