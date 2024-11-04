#include "csdevicesearchlistwidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QWidget>
#include "System/DeviceList/devicesearchwidget.h"

CSDeviceSearchListWidget::CSDeviceSearchListWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	init();
	initUi();
	bind();
}

void CSDeviceSearchListWidget::startSearch()
{
	DeviceSearchWidget widget(getMainWindow());//设备搜索对话框
	int device_count = widget.startSearchDevices();
	searchFinished(device_count);//开始设备搜索,搜索完成自动通知界面
}

void CSDeviceSearchListWidget::refreshSearch()
{
	m_device_refresh_widget.startSearchDevices();
}

void CSDeviceSearchListWidget::slotItemClicked()
{
	m_current_device_ip = QString("");
	setAddBtnEnable(false);
	ui.device_tree_widget->setCurrentIndex(QModelIndex());
}

void CSDeviceSearchListWidget::init()
{
	qRegisterMetaType<DeviceState>();//QueuedConnection时需要注册
	qRegisterMetaType<Device::PropType>();
	qRegisterMetaType<QVariant>();

	m_device_magager_ptr = &DeviceManager::instance();
}

void CSDeviceSearchListWidget::initUi()
{
	setWindowFlags(Qt::FramelessWindowHint);
	updateRefreshLabel(m_const_count_down);
	ui.title_widget->setFixedHeight(30);
	ui.title_widget->setStyleSheet("background-color:#d8d8d8");
	QHBoxLayout* title_layout = new QHBoxLayout(ui.title_widget);
	title_layout->setContentsMargins(5, 0, 5, 0);
	m_title_name = new QLabel(ui.title_widget);
	m_title_name->setText(tr("Discovered Devices"));
	m_visible_icon = new QLabel(ui.title_widget);
	m_visible_icon->setFixedSize(12, 8);
	updateTitleIconStyle(false);
	m_visible_icon->setObjectName("TitleIcon");
	QSpacerItem* title_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
	title_layout->addWidget(m_title_name);
	title_layout->addItem(title_spacer);
	title_layout->addWidget(m_visible_icon);

	QStringList header_lables;
	header_lables << tr("") << tr("Model") << tr("IP/SN");
	ui.device_tree_widget->setHeaderLabels(header_lables);
	//ui.device_tree_widget->setColumnWidth(0, 30);
	ui.device_tree_widget->header()->setFrameShape(QFrame::StyledPanel);
	ui.device_tree_widget->header()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui.device_tree_widget->header()->setDefaultAlignment(Qt::AlignCenter);
	setAddBtnEnable(false);
}

void CSDeviceSearchListWidget::bind()
{
	bool ok = connect(ui.refresh_btn, &QPushButton::clicked, this, &CSDeviceSearchListWidget::slotRefreshBtnClicked);
	ok = connect(ui.add_btn, &QPushButton::clicked, this, &CSDeviceSearchListWidget::slotAddBtnClicked);

	m_countdown_timer = new QTimer();
	ok = connect(m_countdown_timer, &QTimer::timeout, this, &CSDeviceSearchListWidget::slotCountdownTimer);
	m_countdown_timer->start(1000);

	ok = connect(&m_device_refresh_widget, &DeviceRefreshWidget::signalRefreshSearchFinished, this, &CSDeviceSearchListWidget::slotRefreshSearchFinished, Qt::QueuedConnection);
	ok = connect(ui.device_tree_widget, &QTreeWidget::itemPressed, this, &CSDeviceSearchListWidget::slot_tree_item_clicked);
	ok = connect(m_device_magager_ptr, &DeviceManager::signalUpdateTree, this, &CSDeviceSearchListWidget::slotUpdateTree);
}

void CSDeviceSearchListWidget::slotRefreshBtnClicked()
{
	refresh();
}

void CSDeviceSearchListWidget::slotAddBtnClicked()
{
	TreeWidgetItemInfo  iteminfo{};
	
	iteminfo.p_device = m_device_magager_ptr->getDevice(m_current_device_ip);
	if (iteminfo.p_device) {
		iteminfo.device_model = iteminfo.p_device->getModel();
		if (1537 == iteminfo.device_model) {//DEVICE_TRIGGER_CF18:1537
			iteminfo.item_type = TreeWidgetItemType::SYN_CONTROLLER;
		}
		else {
			iteminfo.item_type = TreeWidgetItemType::DEVICE;
		}
	}
	m_device_magager_ptr->addManageItem(iteminfo);
	updateDeviceTree(false);
	setAddBtnEnable(false);
}

void CSDeviceSearchListWidget::slotCountdownTimer()
{
	if (0 == m_countdown) {
		setRefreshBtnEnable(false);
		refresh(true);
		return;
	}
	updateRefreshLabel(--m_countdown);
}

void CSDeviceSearchListWidget::slotRefreshSearchFinished(const int devicecount)
{
	emit signalSearchDeviceFinished();
	searchFinished(devicecount);//开始设备搜索,搜索完成自动通知界面
}

void CSDeviceSearchListWidget::slot_tree_item_clicked(QTreeWidgetItem *item, int column)
{
	emit signalItemClicked();
	if (item) {
		setAddBtnEnable(true);
		m_current_device_ip = item->text(2);
	}
	Q_UNUSED(column);
	if (m_device_magager_ptr) {
		m_device_magager_ptr->SetCurrentDevice(m_device_magager_ptr->getDevice(m_current_device_ip));
	}
}

void CSDeviceSearchListWidget::slotUpdateTree(const QString& ip)
{
	updateDeviceTree(false);
}

void CSDeviceSearchListWidget::titleBarClicked(bool checked)
{
	ui.refresh_btn->setVisible(!checked);
	ui.add_btn->setVisible(!checked);
	ui.device_tree_widget->setVisible(!checked);

	m_countdown = m_const_count_down;
	checked ? m_countdown_timer->stop() : m_countdown_timer->start(1000);
	checked ? m_visible_icon->setObjectName("TitleIconChecked") : m_visible_icon->setObjectName("TitleIcon");
}

void CSDeviceSearchListWidget::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	m_title_bar_checked = !m_title_bar_checked;
	titleBarClicked(m_title_bar_checked);
	updateTitleIconStyle(m_title_bar_checked);
}

void CSDeviceSearchListWidget::updateTitleIconStyle(bool checked)
{
	if (checked) {
		m_visible_icon->setStyleSheet("QLabel{background-color:transparent;border-width:0px;border-image:url(:/image/image/title_hide.png);}");
	}
	else {
		m_visible_icon->setStyleSheet("QLabel{background-color:transparent;border-width:0px;border-image:url(:/image/image/title_show.png);}");
	}
}

void CSDeviceSearchListWidget::updateRefreshLabel(const uint8_t count)
{
	ui.refresh_btn->setText(tr("Refresh(Automatic refresh after %1s)").arg(count));
}

void CSDeviceSearchListWidget::refresh(bool brefresh/*=false*/)
{
	m_countdown_timer->stop();
	m_countdown = m_const_count_down;
	brefresh ? refreshSearch() : startSearch();
}

void CSDeviceSearchListWidget::updateDeviceTree(bool reget /*= true*/)
{
	ui.device_tree_widget->clear();
	if (reget) {
		m_search_device_list.clear();
		m_device_magager_ptr->getDevices(m_search_device_list);
	}
	auto it = m_search_device_list.begin();
	while (it != m_search_device_list.end())
	{
		auto dev = *it;
// 	}
// 	for (auto dev : m_search_device_list)
// 	{
		if (m_device_magager_ptr->findManageDevice(dev)) {
			it = m_search_device_list.erase(it);
			continue;
		}
		QTreeWidgetItem *child = new QTreeWidgetItem(ui.device_tree_widget);
		child->setFlags(child->flags() & ~Qt::ItemIsDropEnabled & ~Qt::ItemIsEditable);
		switch (dev->getModel())
		{
		case DeviceModel::DEVICE_TRIGGER_CF18:
			child->setIcon(0, QIcon(":/images/syn_normal.png"));
			break;
		default:
			child->setIcon(0, QIcon(":/images/MX_normal.png"));
			break;
		}
		if(dev->getModel())
		
		child->setText(1, dev->getModelName());
		child->setTextAlignment(1, Qt::AlignCenter);
		child->setText(2, dev->getIpOrSn());
		child->setTextAlignment(2, Qt::AlignCenter);
		it++;
	}
}

QMainWindow * CSDeviceSearchListWidget::getMainWindow()
{
	for (QWidget* widget : qApp->topLevelWidgets())
	{
		if (QMainWindow * mainWin = qobject_cast<QMainWindow*>(widget))
		{
			return mainWin;
		}
	}
	return nullptr;
}

void CSDeviceSearchListWidget::searchFinished(int device_count)
{
	Q_UNUSED(device_count);
	//通知列表更新设备树
	updateDeviceTree();
	m_countdown_timer->start();
	updateRefreshLabel(m_const_count_down);
	setRefreshBtnEnable(true);
}

void CSDeviceSearchListWidget::setRefreshBtnEnable(bool enable)
{
	ui.refresh_btn->setEnabled(enable);
}

void CSDeviceSearchListWidget::setAddBtnEnable(bool enable)
{
	ui.add_btn->setEnabled(enable);
}

void CSDeviceSearchListWidget::changeEvent(QEvent *event)
{
	QStringList header_lables;
	header_lables << tr("") << tr("Model") << tr("IP/SN");
	ui.device_tree_widget->setHeaderLabels(header_lables);

	if(m_title_name)
		m_title_name->setText(tr("Discovered Devices"));

	if (event->type() == QEvent::LanguageChange)
	{
		ui.retranslateUi(this);
	}

	QWidget::changeEvent(event);
}

void CSDeviceSearchListWidget::on_device_tree_widget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);
	emit signalItemClicked();
	if (item) {
		//setAddBtnEnable(true);
		m_current_device_ip = item->text(2);
		if (m_device_magager_ptr) {
			m_device_magager_ptr->SetCurrentDevice(m_device_magager_ptr->getDevice(m_current_device_ip));
		}
		TreeWidgetItemInfo  iteminfo{};

		iteminfo.p_device = m_device_magager_ptr->getDevice(m_current_device_ip);
		if (iteminfo.p_device) {
			iteminfo.device_model = iteminfo.p_device->getModel();
			if (1537 == iteminfo.device_model) {//DEVICE_TRIGGER_CF18:1537
				iteminfo.item_type = TreeWidgetItemType::SYN_CONTROLLER;
			}
			else {
				iteminfo.item_type = TreeWidgetItemType::DEVICE;
			}
		}
		m_device_magager_ptr->addManageItem(iteminfo);
		updateDeviceTree(false);
		setAddBtnEnable(false);
	}
}