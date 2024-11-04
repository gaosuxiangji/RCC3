#include "csviewmanager.h"
#include "Device/devicemanager.h"
#include "Device/device.h"
#include "render/PlayerViewBase.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
#include <QCoreApplication>
#include "Common/LogUtils/logutils.h"
CSViewManager::CSViewManager(QObject* parent)
	:QObject(parent)
{
}

void CSViewManager::AddView(CPlayerViewBase* view)
{
	if (!view)
		return;
	//默认不画框线
	view->EnableDrawBorderLine(false);
	m_playerList.insert(view->getId(),view);

	view->setBackgroundImage(QImage(":/image/image/videobackground.png"));
	view->FitView(true);
	view->SlotUpdateImage(makeOsdInfo());
	bool ok = connect(view, &CPlayerViewBase::SignalClicked, this, &CSViewManager::on_view_clicked);
	ok = connect(view, &CPlayerViewBase::SignalDoubleClicked, this, &CSViewManager::on_view_double_clicked);
	ok = connect(view, &CPlayerViewBase::sigAboutToClose, this, &CSViewManager::on_close_view);
	auto connection = connect(view, &CPlayerViewBase::sigAboutToCloseAll, this, &CSViewManager::on_close_all_view);
	view->hide();
	//初始化全屏view
	if (getFullScreenId() == INVALID_SCREEN_ID){
		setMaximize(view->getId());
	}
	//初始化拆分窗口布局为1*1
	if (m_splitType == Splitter_Unknown)
		m_splitType = Splitter1_1;
}

void CSViewManager::removeAllView()
{
	for (auto e : m_playerList)
	{
		disconnect(e, &CPlayerViewBase::SignalClicked, this, &CSViewManager::on_view_clicked);
		disconnect(e, &CPlayerViewBase::SignalDoubleClicked, this, &CSViewManager::on_view_double_clicked);
		disconnect(e, &CPlayerViewBase::sigAboutToClose, this, &CSViewManager::on_close_view);
		disconnect(e, &CPlayerViewBase::sigAboutToCloseAll, this, &CSViewManager::on_close_all_view);
	}
	m_playerList.clear();
}




bool CSViewManager::AddToDisplayList(const QString& devicename)
{
	if (devicename.isEmpty())
		return false;
	if (IsInDisplayList(devicename))//在播放队列
		return false;
	auto view = getDeviceView(devicename);
	if (!view){//该设备无绑定view
		view = getFirstUnAssociateView(false);//申请view，可直接从播放队列拿取空白view
		if (!view)
			return false;
		callAssociate(view->getId(), devicename);
	}
	if (!m_displayList.contains(view))
		m_displayList.push_back(view);//加入播放队列
	setSplitType(reCalcSplitType());//重设拆分布局
	if (getSelectViewId() != view->getId())//默认情况下是已选中的，下面两个if语句主要为了规避外界异常调用
	{
		setSelected(view->getId());
		emit SignalDisplayWidgetChanged(select_screen_id_);
	}
	if (getFullScreenId() != INVALID_SCREEN_ID)
		setFullScreenId(view->getId());
	return true;
}

bool CSViewManager::IsInDisplayList(const QString& devicename) const
{
	if (devicename.isEmpty())
		return false;
	for (auto item : m_displayList)
	{
		QString text = item->GetDeviceName();
		if (item->GetDeviceName() == devicename)
		{
			return true;
		}
	}
	return false;
}

bool CSViewManager::IsSnapEnabled() const
{

	//只要有一个视图正在预览高采就可以截图
	for (auto view_ptr : m_displayList)
	{
		if (view_ptr)
		{
			QSharedPointer<Device> device_ptr = DeviceManager::instance().getDevice(view_ptr->GetDeviceName());
			if (device_ptr && (device_ptr->getState() == Previewing|| device_ptr->getState() == Acquiring))
			{
				return true;
			}
		}
	}

	return false;
}

void CSViewManager::Snap(QWidget* pParent)
{
	QList<QSharedPointer<Device>> successDevices;
	QList<QSharedPointer<Device>> failureDevices;

	//界面上的全部图像抓拍,记录结果
	for (auto view_ptr:m_displayList)
	{
		if (view_ptr)
		{
			QSharedPointer<Device> device_ptr = DeviceManager::instance().getDevice(view_ptr->GetDeviceName());
			if (device_ptr && (device_ptr->getState() == Previewing || device_ptr->getState() == Acquiring))
			{
				if (view_ptr->Snapshot())
				{
					successDevices.push_back(device_ptr);
				}
				else
				{
					failureDevices.push_back(device_ptr);
				}
			}
		}
	}

	QString msg;
	if (!failureDevices.empty())
	{
		//有抓拍失败的设备
		msg = UIExplorer::instance().getStringById("STRID_SNAPSHOT_FAIL");
		for (auto pdevice : failureDevices)
		{
			msg += pdevice->getDescription();
			msg += "\r\n";
		}
		msg += UIExplorer::instance().getStringById("STRID_SNAPSHOT_FAIL_DISK_STATUS");
	}
	else
	{
		if (successDevices.empty())
		{
			msg = UIExplorer::instance().getStringById("STRID_SNAPSHOT_FAIL_NO_CAMERA");
		}
		else
		{
			msg = UIExplorer::instance().getStringById("STRID_SNAPSHOT_OK");
			for (auto pdevice : successDevices)
			{
				msg += pdevice->getDescription();
				msg += "\r\n";
			}
		}
	}
	UIUtils::showInfoMsgBox(pParent, msg);
}


void CSViewManager::closeView(int view_id, bool notify_associate_device_stop)
{
	auto view = getView(view_id);
	if (!view)
		return;
	auto removed_list = removeEmptyViewsFromDisplayList();//先移除空白视图
	if (removed_list.contains(getSelectViewId())) {
		setSelected(INVALID_SCREEN_ID);
		emit SignalDisplayWidgetChanged(INVALID_SCREEN_ID);
	}
	{ // 关闭时要清除所有的ROI操作
		view->setFeaturePointVisible(Device::kDeviceRoi,false);
		if (auto device = DeviceManager::instance().getDevice(view->GetDeviceName())) {
			if (device) {
				device->setDrawTypeStatusInfo(Device::DTSI_Noraml);
			}
		}
	}
	QString device_name = view->GetDeviceName();
	if (IsInDisplayList(device_name))
	{
		m_displayList.removeOne(view);
		if (notify_associate_device_stop) {
			QStringList vctDeviceNames;
			vctDeviceNames << device_name;
			emit signalStopDevices(vctDeviceNames);
		}
	}
	if (containsDevice(view)){
		callUnAssociate(view_id, device_name);
	}
	if (getSelectViewId() == view_id) {
		setSelected(INVALID_SCREEN_ID);
		emit SignalDisplayWidgetChanged(INVALID_SCREEN_ID);
	}
	setSplitType(reCalcSplitType());

	//取当前存在的第一个显示图像
	for (auto item : m_displayList)
	{
		setSelected(item->getId());
		if (m_splitType == Splitter1_1)//关闭时如果是全屏则切换当前全屏画面
		{
			setMaximize(select_screen_id_);
		}
		emit SignalDisplayWidgetChanged(select_screen_id_);
		break;
	}
}

void CSViewManager::RemoveFromDisplayList(const QString& device_name)
{
	for (auto e : m_displayList)
	{
		if (e->GetDeviceName() == device_name)
		{
			return closeView(e->getId());
		}
	}
}

QList<CPlayerViewBase*> CSViewManager::GetDisplayList() const
{
	return m_displayList;
}

void CSViewManager::getSplitRowCol(SplitType type, int& row, int & col)
{
	switch (type)
	{
	case CSViewManager::Splitter1_1:
		row = 1;
		col = 1;
		break;
	case CSViewManager::Splitter1_2:
		row = 1;
		col = 2;
		break;
	case CSViewManager::Splitter2_2:
		row = 2;
		col = 2;
		break;
	case CSViewManager::Splitter3_3:
		row = 3;
		col = 3;
		break;
	case CSViewManager::Splitter4_4:
		row = 4;
		col = 4;
		break;
	}
}

void CSViewManager::on_device_clicked(const QString& device_name)
{
	if (device_name.isEmpty()) {//空设备设置无选中状态，作用于所有相机
		auto current_full_view = getView(getFullScreenId());//获取当前全屏的view
		if(current_full_view && isTemporaryAssociate(current_full_view->GetDeviceName()))
			callUnAssociate(current_full_view->getId(), current_full_view->GetDeviceName());
		setSelected(INVALID_SCREEN_ID);
		return;
	}
	if (getSelectView() && getSelectView()->GetDeviceName() == device_name)//相同设备不响应
		return;
	if (INVALID_SCREEN_ID == getFullScreenId())
	{
		if (!IsInDisplayList(device_name)){//非全屏状态下点击非播放队列内的设备，清空选中状态
			setSelected(INVALID_SCREEN_ID);
			return;
		}
		auto view = getDeviceView(device_name);
		if (view){
			setSelected(view->getId());
		}
		return;
	}
	else {
		auto view = getDeviceView(device_name);
		if (!view){//无view关联
			auto current_full_view = getView(getFullScreenId());//获取当前全屏的view
			if (current_full_view && isTemporaryAssociate(current_full_view->GetDeviceName())) {//取消临时绑定
				callUnAssociate(current_full_view->getId(), current_full_view->GetDeviceName());
				view = current_full_view;
			}
			else
				view = getFirstUnAssociateView(false);//从链表取一个未绑定设备的view,可直接从播放队列获取
			if (view){
				callAssociate(view->getId(), device_name);
			}
		}
		else if (IsInDisplayList(view)) {//在播放队列内
			auto current_full_view = getView(getFullScreenId());//获取当前全屏的view
			if (current_full_view && isTemporaryAssociate(current_full_view->GetDeviceName())) {//取消临时绑定
				callUnAssociate(current_full_view->getId(), current_full_view->GetDeviceName());
			}	
		}
		else//有view关联,但是没有绑定在播放列表中(通常不会出现)
		{
			auto current_full_view = getView(getFullScreenId());//获取当前全屏的view
			if (current_full_view && isTemporaryAssociate(current_full_view->GetDeviceName())) {//取消临时绑定
				callUnAssociate(current_full_view->getId(), current_full_view->GetDeviceName());
			}
			if (view){
				callAssociate(view->getId(), device_name);
			}
		}
		if (view) {
			setSelected(view->getId());
			setMaximize(view->getId());
		}
	}
}

CPlayerViewBase* CSViewManager::getDeviceView(const QString& device_name)
{
	for (auto e : m_playerList)
	{
		if (e->GetDeviceName() == device_name)
			return e;
	}
	return nullptr;
}

bool CSViewManager::isAssociated(const QString& device_name) const
{
	if (device_name.isEmpty())
		return false;
	for (auto e : m_playerList)
	{
		if (e->GetDeviceName() == device_name)
			return true;
	}
	return false;
}

CPlayerViewBase* CSViewManager::getFirstUnAssociateView(bool search_exclude_display_list) const
{
	for (int i = 0 ; i < m_playerList.size() ;i++)
	{
		if (!containsDevice(m_playerList.value(i)))
		{
			if (search_exclude_display_list)
			{
				if (!IsInDisplayList(m_playerList.value(i)))
					return m_playerList.value(i);
			}
			else
				return m_playerList.value(i);
		}
			
	}
	return nullptr;
}

void CSViewManager::fillDisplayList(SplitType type)
{
	auto size = m_displayList.size();
	while (size<type)
	{
		auto player = getFirstUnAssociateView(true);
		if (player) {
			m_displayList.push_back(player);
			++size;
		}
	}
}

CSViewManager::SplitType CSViewManager::reCalcSplitType() const
{
	SplitType type = Splitter1_1;
	auto size = m_displayList.size();
	if (size < Splitter1_1)
	{
		type = Splitter_Unknown;
		//fillDisplayList(type);
	}
	else if (size == Splitter1_1)
		type = Splitter1_1;
	else if (size > Splitter1_1 && size <= Splitter1_2)
	{
		type = Splitter1_2;
		//fillDisplayList(type);
	}
		
	else if (size > Splitter1_2 && size <= Splitter2_2) {
		type = Splitter2_2;
		//fillDisplayList(type);
	}
	else if (size > Splitter2_2 && size <= Splitter3_3) {
		type = Splitter3_3;
		//fillDisplayList(type);
	}
	else if (size > Splitter3_3 && size <= Splitter4_4)
	{
		type = Splitter4_4;
		//fillDisplayList(type);
	}
	return type;
}
void CSViewManager::setSplitType(SplitType type)
{
	if (m_splitType != type)
	{
		if (type == Splitter_Unknown)//空白窗口
		{
			if(getFullScreenId()==INVALID_SCREEN_ID)
				setMaximize(0);
			return;
		}
		hideAll();
		fillDisplayList(type);
		m_splitType = type;
		if (m_splitType == Splitter1_1) {
			setFullScreenId(m_displayList.first()->getId());
		}
		else
			setFullScreenId(INVALID_SCREEN_ID);
		emit signalSplitWidget(type);
	}
}

void CSViewManager::setViewDeviceName(int view_id, const QString& device_name)
{
	if (view_id < Splitter_Unknown || view_id >= Splitter4_4)
		return;
	if (device_name.isEmpty())
	{
		getView(view_id)->setBackgroundImage(QImage(":/image/image/videobackground.png"));
		getView(view_id)->FitView(true);
	}	
	getView(view_id)->SetDeviceName(device_name);
}

void CSViewManager::maximizeOrRestore(int id)
{
	if (getFullScreenId() == id)//restore
	{
		if (m_displayList.size()<=1)
			return;
		setSplitType(reCalcSplitType());
	}
	else {//maximize
		setMaximize(id);
	}
}


void CSViewManager::setSelected(int id)
{
	if (select_screen_id_ == id)
		return;
	if (INVALID_SCREEN_ID != select_screen_id_) { // 切换时时要清除所有的ROI操作
		if (m_playerList.size() > select_screen_id_) {		
			if (auto view = m_playerList[select_screen_id_]) {	
				if (view){
					view->setFeaturePointVisible(Device::kDeviceRoi, false);
					if (auto device = DeviceManager::instance().getDevice(view->GetDeviceName())) {
						if (device){
							device->setDrawTypeStatusInfo(Device::DTSI_Noraml);
						}
					}
				}
			}
		}
	}
	if(INVALID_SCREEN_ID!=select_screen_id_)
		m_playerList[select_screen_id_]->EnableDrawBorderLine(false);
	if(INVALID_SCREEN_ID!=id)
		m_playerList[id]->EnableDrawBorderLine(true);
	CSLOG_INFO("select view changed,pre:{},new:{}", select_screen_id_, id);
	select_screen_id_ = id;
}

void CSViewManager::setMaximize(int id)
{
	if (getFullScreenId() == id)//restore
		return;
	for (auto e : m_playerList)
	{
		if (e->getId() != id)
			e->hide();
		else
			e->show();
	}
	setFullScreenId(id);
	if(m_splitType!=Splitter1_1)
		m_splitType = Splitter1_1;
}


void CSViewManager::hideAll()
{
	for (auto e : m_playerList)
		e->hide();
}

void CSViewManager::showDisplayList()
{
	for (auto e : m_displayList)
		e->show();
}

QList<int> CSViewManager::removeEmptyViewsFromDisplayList()
{

	QList<int> removed_items;
	auto iter = m_displayList.begin();
	while (iter!=m_displayList.end())
	{
		if ((*iter)->GetDeviceName().isEmpty()) {
			removed_items.push_back((*iter)->getId());
			iter = m_displayList.erase(iter);		
		}
			
		else
			++iter;
	}
	return removed_items;
}

bool CSViewManager::containsDevice(CPlayerViewBase* view) const
{
	if (view && !view->GetDeviceName().isEmpty())
		return true;
	return false;
}
/****************************//* 槽函数-begin  *//*****************************************/
void CSViewManager::on_view_clicked(int index)
{
	if (getSelectViewId() != index)
	{
		setSelected(index);
		emit SignalDisplayWidgetChanged(index);
	}
}
//clicked event will preReceived
void CSViewManager::on_view_double_clicked(int index)
{
	maximizeOrRestore(index);
}


void CSViewManager::on_close_view(int view_id)
{
	closeView(view_id,true);
}

void CSViewManager::on_close_all_view()
{
	QStringList stopDevices;
	removeEmptyViewsFromDisplayList();//先移除空白视图
	for (auto e : m_playerList)
	{
		QString device_name = e->GetDeviceName();
		if (IsInDisplayList(device_name))
		{
			m_displayList.removeOne(e);
			stopDevices << device_name;
		}
		if (!device_name.isEmpty() && un_associate_cb_)//取消设备绑定
			un_associate_cb_(e->getId(), device_name);
		setSelected(INVALID_SCREEN_ID);
		emit SignalDisplayWidgetChanged(INVALID_SCREEN_ID);
	}
	setSplitType(reCalcSplitType());
	if (!stopDevices.isEmpty())
		emit signalStopDevices(stopDevices);
}
/****************************//* 槽函数-end  *//*****************************************/
