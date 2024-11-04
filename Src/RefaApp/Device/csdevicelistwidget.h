#ifndef CSDEVICELISTWIDGET_H
#define CSDEVICELISTWIDGET_H

#include <QWidget>
#include <QAction>
#include <QToolBar>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QPointer>
#include <QSharedPointer>
#include <QMap>
#include <QVariant>
#include <QMainWindow>
#include<QTimer>

#include "Device/devicestate.h"
#include "Device/device.h"
#include "Device/csdevicetreeview.h"
#include "Device/devicemanager.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <QSet>
#include "treenodeoperate/treenodeoperate.h"
#include "editdeviceinfo/editdeviceinfo.h"
#include "editgroupinfo/editgroupinfo.h"


class DeviceManager;
namespace Ui {
class CSDeviceListWidget;
}


/**
 * @brief 设备列表控件
 */
class CSDeviceListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CSDeviceListWidget(QWidget *parent = 0);
    ~CSDeviceListWidget();
private:
	void bind();
	//连接设备列表中的全部设备
	void autoConnectDevices();
signals:
	void signalUpdateDeviceRotate(const QString strIP);
	void signalRemoveCurrentDevice();
	void signalItemClicked();
public slots:
	void slotSearchDeviceFinished();
	void slotCurrentItemClicked();
private slots:
    void slotToolButtonCopyParam();//复制按钮单击
    void slotToolButtonPasteParam();//粘贴按钮单击

	/**
	**@ Brife	设备树被双击
	**@ Param	index 被双击的itemIndex
	**@ Note	需要执行连接操作
	*/
    void on_deviceTreeView_doubleClicked(const QModelIndex &index);


	/**
	**@ Brife	设备树选项切换
	**@ Param	current 当前选择的Index  previous 之前的index
	**@ Note	切换当前选中的设备,发射当前设备切换信号
	*/
	void slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous);

	/**
	**@ Brife	系统当前设备切换
	**@ Param	device_ip 当前设备ip
	**@ Note	切换当前选中的设备,注意不可再次通知设备管理器
	*/
	void slotCurrentDeviceChanged(const QString & device_ip);

	/**
	**@ Brife	连接当前设备
	**@ Return	是否连接成功
	**@ Note	当前设备从DeviceManager中获取,连接完成后刷新设备列表信息
	*/
	bool slotConnectCurrentDevice();

	/**
	**@ Brife	断开当前设备
	**@ Return	是否断连成功
	**@ Note	当前设备从DeviceManager中获取,断连完成后刷新设备列表信息
	*/
	bool slotDisconnectCurrentDevice();

	/**
	**@ Brife	设备状态变更
	**@ Param	state 变更后的设备状态
	**@ Note	变更设备列表中的设备状态
	*/
	void slotDeviceStateChanged(const QString &current_ip,DeviceState state);
	
	/**
	**@ Brife	设备属性变更
	* @param type 属性类型
	* @param value 属性值
	**@ Note	
	*/
	void slotDevicePropertyChanged(Device::PropType type, const QVariant & value);

	/**
	**@ Brife	设备报错处理
	**@ Param	err_code 错误码
	**@ Param	bShowTip 是否显示
	**@ Param	extTip 额外提示
	**@ Note	弹窗显示错误提示
	*/
	void slotDeviceErrorOccurred(quint64 err_code, bool bShowTip ,QString extTip);


	/**
	* @brief 原始状态正在采集图像
	* @param ip 设备ip
	*/
	void slotOrgStateIsAcquring(const QString & ip);

	/**
	**@ Brife	设备树情景菜单(右键或键盘)
	**@ Param	pos 鼠标激活菜单位置
	**@ Note	根据设备打开对应菜单选项
	*/
    void on_deviceTreeView_customContextMenuRequested(const QPoint &pos);


	/**
	**@ Brife	显示当前设备信息
	**@ Note	当前设备从DeviceManager中获取,连接完成后刷新设备列表信息
	*/
	void slotCurrentDeviceInfo();

	/**
	**@ Brife	显示当前触发脉冲控制
	**@ Note	当前设备从DeviceManager中获取
	*/
	void slotCurrentTriggerPulseControl();

	void slotDeleteCurrentCF18();

	/**
	**@ Brife	设备状态变化槽函数，主要是为了自动重连时设置旋转角度，
	**@ Note	在连接后只去设置一次旋转，然后解绑信号
	*/
	void slotDeviceStateChangedUpdateRotate(const QString &current_ip, DeviceState state);
	void slotDragFinished();
	void slotAddDevice(const TreeWidgetItemInfo&  iteminfo);
	void slotItemClicked(QTreeWidgetItem *item, int column);
	void slotAddBtnClicked();
	void slotEditBtnClicked();
	void slotDeleteCurrentNode();
	void slotCountdownTimer();
	void slotTreeItemClicked(QTreeWidgetItem *item, int column);
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;

	virtual void keyPressEvent(QKeyEvent *event);
private:

	/**
	**@ Brife 初始化界面,连接信号和槽	
	*/
	void InitUi();

	/**
	**@ Brife	初始化右键菜单
	*/
	void initMenus();

	//获取主窗口
	QMainWindow * getMainWindow();

	//菜单种类
	enum  DeviceMenuType
	{
		kMenuTypeCameraConnect,		//相机连接菜单
		kMenuTypeCameraDisconnect,		//相机断连菜单
		kMenuTypeTriggerConnect,		//触发器连接菜单
		kMenuTypeTriggerDisconnect,		//触发器断连菜单
		kMenuTypeCF18Connect,    //CF18连接菜单
		kMenuTypeCF18Disconnect,    //CF18断连菜单
		kMenuTypeGroup    //分组
	};

	enum DeviceType {
		M_Series,
		GR_Series,
		Syn_Controller
	};

	// 设备列表项枚举
	enum EDeviceListHeader
	{
		DLH_Name = 0,		// 名称
		DLH_Type,			// 型号
		DLH_IP,				// IP/SN
		DLH_Status,			// 状态
		DLH_Count			//
	};
	/**
	**@ Brife	更新右键菜单
	*/
	void updateMenus(DeviceMenuType menu_type);

	/**
	 * @brief 根据当前设备状态更新拷贝和粘贴操作的使能状态
	 */
	void updateCopyAndPasteCtrl();

	/**
	* @brief 根据当前设备状态更新设备操作的使能状态
	*/
	void updateDeviceActionsCtrl();

	QString getCurrentDeviceIP();

	//更新所有Device的icon；
	void UpdateDeviceIcon();

	void UpdateDeviceModeName();

	QTreeWidgetItem* FindTreeItem(const TreeWidgetItemInfo& iteminfo);
private:
	void addTreeWidgetItem(CSDeviceTreeView* parent);
	void renameGroup(QString& groupname);
	bool loadJson(const QString &filepath, bool init=false);
	void paraseJson(const QJsonDocument& json_doc, bool init = false);
	void recurseParaseJsonObj(const QJsonObject& json_obj, QTreeWidgetItem* item);
	void initRecurseParaseJsonObj(const QJsonObject& json_obj);
	void recurseTreeWidgetNode(QTreeWidgetItem* item, QJsonObject& obj);
	TreeWidgetItemInfo getTreeWidgetItemInfo(const QString& ip);
	QString getNodeKeyName();
	void updateTreeWidgetInfo();
	bool deleteJsonNode(QJsonObject& cur_obj, QJsonObject& par_obj, const QString& key, const QString& delete_name);
	bool changeJsonNode(QJsonObject& cur_obj, const QString& org_name, const QString& change_name);
	void calDeiveNumInSelectedGroup(QTreeWidgetItem* item);
	void changeGroupName(const QString& org_name, const QString& new_name);
	void updateTreeNodeVessel();
	void updateGroupInfo();
	void updateGroupVessel(QTreeWidgetItem* item);
	void updateDeviceVessel();
	EditDeviceInfo::DeviceType getCurrentDeviceType(QTreeWidgetItem* item);
	void delTreeNode(const QString& node_name);
	DeviceModel getDeviceModel(DeviceType dt);
	void updateRefreshLabel(const uint8_t count, bool normal=true);
	void setRefreshBtnEnable(bool enable);
	CSDeviceListWidget::DeviceType getDeviceType(const DeviceModel dm);
	void updateDeviceListValue(const QString& strIP, const EDeviceListHeader colum, QString strText);
private:
	QPointer<DeviceManager> m_device_magager_ptr;//设备管理器

	QSharedPointer<Device> m_device_to_copy;//需要被拷贝参数的设备指针

	QPointer<QMenu> m_menu_device;//设备右键菜单
	QPointer<QAction> m_action_connect;//设备连接操作
	QPointer<QAction> m_action_disconnect;//设备断连操作
	QPointer<QAction> m_action_copy;//复制设备信息操作
	QPointer<QAction> m_action_paste;//粘贴设备信息操作
	QPointer<QAction> m_action_device_info;//相机信息操作

	QPointer<QAction> m_action_connect_all;//触发器全部连接
	QPointer<QAction> m_action_disconnect_all;//触发器全部断开
	QPointer<QAction> m_action_preview_all;//触发器全部预览
	QPointer<QAction> m_action_acquire_all;//触发器全部高采
	QPointer<QAction> m_action_stop_all;//触发器全部停机

	QPointer<QAction> m_action_pulse_control;//触发器脉冲控制
	QPointer<QAction> m_action_video_mosaic_export;//触发器视频拼接导出

	QPointer<QAction> m_action_delete_current_cf18{};//删除当前选中的cf18
	QPointer<QAction> m_action_cf18_info;//CF18信息
	QPointer<QAction> m_action_delete_current_node{};


	bool m_allow_selection_changed = true;//是否允许设备列表通知设备管理器切换当前设备
	
    //外同步连接
	QPointer<QTimer>m_external_sync_timer;
	//相机列表列号
	enum class DeviceTreeColumn
	{
		kDeviceName,			//相机名
		kDeviceModelName,		//型号名
		kDeviceIP,				//ip
		kDeviceState			//状态
	};

	//节点类型
	enum NodeType {
		nGroup,    //组
		nDevice,    //设备
		nSynController    //同步控制器
	};

    Ui::CSDeviceListWidget *ui;
	const uint8_t m_const_device_count{ 18 };
	QList<QString> m_group_list{};
	QJsonDocument m_json_doc{};
	const QString m_const_json_file{ "./new.json" };
	const QString m_const_node_type{ "node_type" };
	const QString m_const_node_name{ "node_name" };
	const QString m_const_child_node{ "child_node" };
	const QString m_const_model_info{ "model_name" };
	const QString m_const_model_type_info{ "model_type_name" };
	const QString m_const_cf18{ "CF18" };
	const QString m_const_unknown{ "unknown" };
	bool m_is_first_search{ true };
	uint32_t m_key_index{ 0 };
	int m_deive_num_in_selected_group{ 0 };

	QSet<QString> m_manage_ip_set{};
	QSet<QString> m_all_ip_set{};
	QSet<QString> m_group_set{};
	QSet<QString> m_syn_controller_set{};

	QTimer* m_countdown_timer{};
	const uint8_t m_const_count_down{ 5 };
	uint8_t m_countdown{ m_const_count_down };

	QList<QTreeWidgetItem*>m_device_tree_item_list{};
	int m_tab_index{ 0 }; //操作界面 tab页索引
};
Q_DECLARE_METATYPE(QVariant)

#endif // CSDEVICELISTWIDGET_H
