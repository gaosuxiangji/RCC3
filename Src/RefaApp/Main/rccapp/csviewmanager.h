/***************************************************************************************************
** @file: 国产化国产化采集控制系统-对外接口类
** @author: mpp
** @date: 2022-03-03
*
*****************************************************************************************************/
#ifndef CSVIEWMANAGER_H_
#define CSVIEWMANAGER_H_

#include <QObject>
#include <QMap>
#include <QList>
#include <functional>
#include "Device/csframeinfo.h"
class CPlayerViewBase;
class Device;
class DeviceManager;


using BeginAddCallback = std::function<void(int, const QString&)>;

using EndAddCallback = std::function<void(int, const QString&)>;

using BeginCloseCallback = std::function<void(int, const QString&)>;

using EndCloseCallback = std::function<void(int, const QString&)>;

using BeginAddDeviceCallback = std::function<void(int, const QString&)>;

using EndAddDeviceCallback = std::function<void(int, const QString&)>;




using AssociateCallback = std::function<void(int, const QString&)>;

using UnAssociateCallback = std::function<void(int, const QString&)>;
class CSViewManager :public QObject
{
	Q_OBJECT
	//窗口分割类型

public:
	enum SplitType
	{
		Splitter_Unknown = 0,
		Splitter1_1 = 1,    //1*1分屏
		Splitter1_2 = 2,    //1*2分屏
		Splitter2_2 = 4,    //2*2分屏
		Splitter3_3 = 9,    //3*3分屏
		Splitter4_4 = 16    //4*4分屏
	};
	Q_ENUM(SplitType);
	CSViewManager(const CSViewManager&) = delete;
	CSViewManager& operator=(const CSViewManager&) = delete;

	/************************
	* @brief:管理设备->视图的关联，以及设备的显示灯
	* @author: mpp
	*************************/
	CSViewManager(QObject* parent = Q_NULLPTR);

	/************************
	* @brief: 
	* @author: mpp
	*************************/
	void AddView(CPlayerViewBase* view);

	void removeAllView();



	/**
	*@brief	注册关联与取消关联的回调
	**/
	void registerCbEx(AssociateCallback asso_cb, UnAssociateCallback un_asso_cb)
	{
		assocate_cb_ = asso_cb;
		un_associate_cb_ = un_asso_cb;
	}
	/************************
	* @brief: 添加一个设备到播放队列,仅在预览或高采下才能调用
	* @author: mpp
	*************************/
	bool AddToDisplayList(const QString& devicename);

	/************************
	* @brief: 判断一个设备是否在播放队列
	* @author: mpp
	*************************/
	bool IsInDisplayList(const QString& devicename) const;


	void RemoveFromDisplayList(const QString& device_name);
	/************************
	* @brief: 抓拍是否使能
	* @author: mpp
	*************************/
	bool IsSnapEnabled() const;

	/************************
	* @brief: 对所有可见视图抓拍，抓取当前播放帧
	* @param pParent 父窗口
	* @author: mpp
	*************************/
	void Snap(QWidget* pParent = nullptr);

	/************************
	* @brief: 关闭视图
	*************************/
	void closeView(int,bool notify_associate_device_stop = false);

	/************************
	* @brief:  播放控件显示列表
	* @return: 显示列表
	* @author: mpp
	*************************/
	QList<CPlayerViewBase*> GetDisplayList() const;

	/**
	*@brief 获取选择的view
	**/
	CPlayerViewBase* getSelectView() const {
		if (getSelectViewId() == INVALID_SCREEN_ID)
			return nullptr;
		return m_playerList[getSelectViewId()];
	}

	/**
	*@brief 窗口布局对应的行列大小
	**/
	static void getSplitRowCol(SplitType type, int& row, int & col);

	/**
	*@brief	获取已选择的view id
	**/
	int getSelectViewId() const { return select_screen_id_; }
	/**
	*@brief	获取view句柄
	**/
	CPlayerViewBase* getView(int view_id) const
	{
		if (view_id < Splitter_Unknown || view_id>=Splitter4_4)
			return nullptr;
		return m_playerList[view_id];
	}

	/**
	*@brief	（除却内部视图所有外部选中状态的响应）设备被选中的响应，如果当前正处于分屏模式下，且当前选中的非播放队列的设备，仅清空选中状态
	*@param		
	*@return
	**/
	void on_device_clicked(const QString& device_name);

	/**
	*@brief	获取设备对应的view
	**/
	CPlayerViewBase* getDeviceView(const QString& device_name);

	/**
	*@brief	设备是否关联view
	**/
	bool isAssociated(const QString& device_name) const;

	/**
	*@brief	获取第一个无关联的view
	**/
	CPlayerViewBase* getFirstUnAssociateView(bool search_exclude_display_list) const;

	/**
	*@brief	该view是否关联设备
	**/
	bool containsDevice(CPlayerViewBase* view) const;

	/**
	*@brief	该view是否在播放队列
	**/
	bool IsInDisplayList(CPlayerViewBase* view) const
	{
		return m_displayList.contains(view);
	}

	/**
	*@brief	该设备是否为临时关联(存在视图的绑定关系，但是不在播放队列)
	**/
	bool isTemporaryAssociate(const QString& device_name) const
	{
		if (isAssociated(device_name) && !IsInDisplayList(device_name))
			return true;
		return false;
	}

	CPlayerViewBase* GetFullViewBase() const
	{
		return getView(getFullScreenId());
	}

signals:
	/**
	*@brief	需要重设窗口布局的信号
	*@param
	*@return
	**/
	void signalSplitWidget(int split_type);
	/************************
	* @brief:  发送窗口索引信号
	* @index: 窗口索引
	* @author: mpp
	*************************/
	void SignalDisplayWidgetChanged(int index);

	/**
	*@brief	窗口关闭后播放队列内需要关闭的设备名称链表
	*@param 
	*@return
	**/
	void signalStopDevices(const QStringList& devices);
private slots:
	/************************
	* @brief: 单击信号的槽函数
	* @param index: 控件索引
	* @author: mpp
	*************************/
	void on_view_clicked(int index);

	/************************
	* @brief: 双击信号的槽函数
	* @param index: 控件索引
	* @author: mpp
	*************************/
	void on_view_double_clicked(int index);

	/**
	*@brief	view右键菜单关闭响应
	**/
	void on_close_view(int);

	/**
	*@brief	view右键菜单关闭所有响应
	**/
	void on_close_all_view();
private:
	/**
	*@brief	填充播放队列，当播放队列的个数不足当前的拆分类型个数时，
	*填充空白view(3个设备预览，需要2*2显示，需要增加一个空白view)
	**/
	void fillDisplayList(SplitType type);

	/**
	*@brief	根据播放链表大小计算拆分类型
	**/
	SplitType reCalcSplitType() const;

	/**
	*@brief	最大化或者还原布局
	**/
	void maximizeOrRestore(int id);

	/**
	*@brief	设置选中状态
	**/
	void setSelected(int id);
	/**
	*@brief	设置最大化
	**/
	void setMaximize(int id);
	/**
	*@brief	隐藏所有
	**/
	void hideAll();

	/**
	*@brief	从播放队列删除填充的空白view
	**/
	QList<int> removeEmptyViewsFromDisplayList();

	void callAssociate(int view_id, const QString& device_name)
	{
		if (assocate_cb_)
			assocate_cb_(view_id, device_name);
	}

	void callUnAssociate(int view_id, const QString& device_name)
	{
		if (un_associate_cb_)
			un_associate_cb_(view_id, device_name);
	}

	/**
	*@brief	设置拆分布局
	**/
	void setSplitType(SplitType type);
	void setFullScreenId(int id) {
		full_screen_id_ = id;
	}

	int getFullScreenId() const
	{
		return full_screen_id_;
	}
public:
	void setViewDeviceName(int view_id, const QString& device_name);

	void showDisplayList();
public:
		static const int INVALID_SCREEN_ID = -1;//无效的view ID
private:
	QMap<int,CPlayerViewBase*> m_playerList;    //播放控件列表
	QList<CPlayerViewBase*> m_displayList;    //显示列表
	int full_screen_id_{ INVALID_SCREEN_ID };
	int select_screen_id_{ INVALID_SCREEN_ID };
	SplitType m_splitType{ Splitter_Unknown };

	AssociateCallback assocate_cb_;
	UnAssociateCallback un_associate_cb_;

};
#endif // !CSVIEWMANAGER_H_
