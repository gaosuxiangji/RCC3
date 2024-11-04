/***************************************************************************************************
** @file: �������������ɼ�����ϵͳ-����ӿ���
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
	//���ڷָ�����

public:
	enum SplitType
	{
		Splitter_Unknown = 0,
		Splitter1_1 = 1,    //1*1����
		Splitter1_2 = 2,    //1*2����
		Splitter2_2 = 4,    //2*2����
		Splitter3_3 = 9,    //3*3����
		Splitter4_4 = 16    //4*4����
	};
	Q_ENUM(SplitType);
	CSViewManager(const CSViewManager&) = delete;
	CSViewManager& operator=(const CSViewManager&) = delete;

	/************************
	* @brief:�����豸->��ͼ�Ĺ������Լ��豸����ʾ��
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
	*@brief	ע�������ȡ�������Ļص�
	**/
	void registerCbEx(AssociateCallback asso_cb, UnAssociateCallback un_asso_cb)
	{
		assocate_cb_ = asso_cb;
		un_associate_cb_ = un_asso_cb;
	}
	/************************
	* @brief: ���һ���豸�����Ŷ���,����Ԥ����߲��²��ܵ���
	* @author: mpp
	*************************/
	bool AddToDisplayList(const QString& devicename);

	/************************
	* @brief: �ж�һ���豸�Ƿ��ڲ��Ŷ���
	* @author: mpp
	*************************/
	bool IsInDisplayList(const QString& devicename) const;


	void RemoveFromDisplayList(const QString& device_name);
	/************************
	* @brief: ץ���Ƿ�ʹ��
	* @author: mpp
	*************************/
	bool IsSnapEnabled() const;

	/************************
	* @brief: �����пɼ���ͼץ�ģ�ץȡ��ǰ����֡
	* @param pParent ������
	* @author: mpp
	*************************/
	void Snap(QWidget* pParent = nullptr);

	/************************
	* @brief: �ر���ͼ
	*************************/
	void closeView(int,bool notify_associate_device_stop = false);

	/************************
	* @brief:  ���ſؼ���ʾ�б�
	* @return: ��ʾ�б�
	* @author: mpp
	*************************/
	QList<CPlayerViewBase*> GetDisplayList() const;

	/**
	*@brief ��ȡѡ���view
	**/
	CPlayerViewBase* getSelectView() const {
		if (getSelectViewId() == INVALID_SCREEN_ID)
			return nullptr;
		return m_playerList[getSelectViewId()];
	}

	/**
	*@brief ���ڲ��ֶ�Ӧ�����д�С
	**/
	static void getSplitRowCol(SplitType type, int& row, int & col);

	/**
	*@brief	��ȡ��ѡ���view id
	**/
	int getSelectViewId() const { return select_screen_id_; }
	/**
	*@brief	��ȡview���
	**/
	CPlayerViewBase* getView(int view_id) const
	{
		if (view_id < Splitter_Unknown || view_id>=Splitter4_4)
			return nullptr;
		return m_playerList[view_id];
	}

	/**
	*@brief	����ȴ�ڲ���ͼ�����ⲿѡ��״̬����Ӧ���豸��ѡ�е���Ӧ�������ǰ�����ڷ���ģʽ�£��ҵ�ǰѡ�еķǲ��Ŷ��е��豸�������ѡ��״̬
	*@param		
	*@return
	**/
	void on_device_clicked(const QString& device_name);

	/**
	*@brief	��ȡ�豸��Ӧ��view
	**/
	CPlayerViewBase* getDeviceView(const QString& device_name);

	/**
	*@brief	�豸�Ƿ����view
	**/
	bool isAssociated(const QString& device_name) const;

	/**
	*@brief	��ȡ��һ���޹�����view
	**/
	CPlayerViewBase* getFirstUnAssociateView(bool search_exclude_display_list) const;

	/**
	*@brief	��view�Ƿ�����豸
	**/
	bool containsDevice(CPlayerViewBase* view) const;

	/**
	*@brief	��view�Ƿ��ڲ��Ŷ���
	**/
	bool IsInDisplayList(CPlayerViewBase* view) const
	{
		return m_displayList.contains(view);
	}

	/**
	*@brief	���豸�Ƿ�Ϊ��ʱ����(������ͼ�İ󶨹�ϵ�����ǲ��ڲ��Ŷ���)
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
	*@brief	��Ҫ���贰�ڲ��ֵ��ź�
	*@param
	*@return
	**/
	void signalSplitWidget(int split_type);
	/************************
	* @brief:  ���ʹ��������ź�
	* @index: ��������
	* @author: mpp
	*************************/
	void SignalDisplayWidgetChanged(int index);

	/**
	*@brief	���ڹرպ󲥷Ŷ�������Ҫ�رյ��豸��������
	*@param 
	*@return
	**/
	void signalStopDevices(const QStringList& devices);
private slots:
	/************************
	* @brief: �����źŵĲۺ���
	* @param index: �ؼ�����
	* @author: mpp
	*************************/
	void on_view_clicked(int index);

	/************************
	* @brief: ˫���źŵĲۺ���
	* @param index: �ؼ�����
	* @author: mpp
	*************************/
	void on_view_double_clicked(int index);

	/**
	*@brief	view�Ҽ��˵��ر���Ӧ
	**/
	void on_close_view(int);

	/**
	*@brief	view�Ҽ��˵��ر�������Ӧ
	**/
	void on_close_all_view();
private:
	/**
	*@brief	��䲥�Ŷ��У������Ŷ��еĸ������㵱ǰ�Ĳ�����͸���ʱ��
	*���հ�view(3���豸Ԥ������Ҫ2*2��ʾ����Ҫ����һ���հ�view)
	**/
	void fillDisplayList(SplitType type);

	/**
	*@brief	���ݲ��������С����������
	**/
	SplitType reCalcSplitType() const;

	/**
	*@brief	��󻯻��߻�ԭ����
	**/
	void maximizeOrRestore(int id);

	/**
	*@brief	����ѡ��״̬
	**/
	void setSelected(int id);
	/**
	*@brief	�������
	**/
	void setMaximize(int id);
	/**
	*@brief	��������
	**/
	void hideAll();

	/**
	*@brief	�Ӳ��Ŷ���ɾ�����Ŀհ�view
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
	*@brief	���ò�ֲ���
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
		static const int INVALID_SCREEN_ID = -1;//��Ч��view ID
private:
	QMap<int,CPlayerViewBase*> m_playerList;    //���ſؼ��б�
	QList<CPlayerViewBase*> m_displayList;    //��ʾ�б�
	int full_screen_id_{ INVALID_SCREEN_ID };
	int select_screen_id_{ INVALID_SCREEN_ID };
	SplitType m_splitType{ Splitter_Unknown };

	AssociateCallback assocate_cb_;
	UnAssociateCallback un_associate_cb_;

};
#endif // !CSVIEWMANAGER_H_
