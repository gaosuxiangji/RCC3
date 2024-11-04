#include "cslistviewvideo.h"
#include "ui_cslistviewvideo.h"
#include "Main/rccapp/csrccapp.h"
#include "Device/devicemanager.h"
#include "Device/device.h"
#include "Device/csdlgprogressformat.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoItem/videoitem.h"
#include "Video/VideoUtils/videoutils.h"
#include "Video/Export/csdlgdevicevideoexport.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/PathUtils/pathutils.h"
#include "Common/ErrorCodeUtils/errorcodeutils.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include <QList>
#include <QDateTime>
#include <QFileDialog>
#include "Video/preexportcolorcorrectrollback.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>




CSListViewVideo::CSListViewVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSListViewVideo)
{
    ui->setupUi(this);
	ui->tableViewVideo->setAttribute(Qt::WA_Hover, true);
	ui->tableViewVideo->setMouseTracking(true);
	InitUI();
	InitMenus();
}

CSListViewVideo::~CSListViewVideo()
{
    delete ui;
}

void CSListViewVideo::InitUI()
{
	//添加表头,设置表头样式,表格样式,设置右键菜单,设置视频列表刷新,绑定刷新信号.
	m_video_table_model_ptr = new CustomStandardItemModel();
	//ui->tableViewVideo->setModel(m_video_table_model_ptr);
	m_pFreezeHeader = new TableHeaderView(Qt::Horizontal, this);
	//ui->tableViewVideo->setHorizontalHeader(m_pFreezeHeader);
	TableDelegate *tableDelegate = new TableDelegate(this);
	ui->tableViewVideo->setItemDelegateForColumn(1, tableDelegate);
	CheckBoxDelegate *pDelegate = new CheckBoxDelegate(this);
	//ui->tableViewVideo->setItemDelegate(pDelegate);
	QStringList header_labels;
	header_labels << tr("") << tr("Video Name") << tr("Acquisition Time") << tr("Video Size(MB)");
	m_video_table_model_ptr->setHorizontalHeaderLabels(header_labels);
	ui->tableViewVideo->init(m_video_table_model_ptr, m_pFreezeHeader, pDelegate);
	//设置表头样式:视频名称,采集时间,视频大小	
	ui->tableViewVideo->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableViewVideo->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->tableViewVideo->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
	ui->tableViewVideo->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tableViewVideo->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
	//设置表格自定义右键菜单
	ui->tableViewVideo->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->tableViewVideo->setEditTriggers(QAbstractItemView::DoubleClicked);
	connect(m_pFreezeHeader, &TableHeaderView::SignalStateChanged, this, &CSListViewVideo::slotHeaderClick);
	connect(this, &CSListViewVideo::SignalStateChanged, m_pFreezeHeader, &TableHeaderView::SlotStateChanged);
	connect(pDelegate, &CheckBoxDelegate::signalTableCheck, this, &CSListViewVideo::slotTableItemCheck);

	//连接当前视频项切换信号
	connect(ui->tableViewVideo->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CSListViewVideo::slotCurrentSelectChanged);
	connect(m_video_table_model_ptr, &QAbstractItemModel::dataChanged, this, &CSListViewVideo::slotItemChanged);
	//连接设备切换信号
	connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSListViewVideo::slotCurrentDeviceChanged);
	//连接自动回放自动导出信号和自动导出并触发信号 
	connect(&DeviceManager::instance(), &DeviceManager::autoExportVideo, this, &CSListViewVideo::slotAutoExportVideoById);
	connect(&DeviceManager::instance(), &DeviceManager::autoPlaybackNeeded, this, &CSListViewVideo::slotAutoPlaybackVideoById);
	connect(&DeviceManager::instance(), &DeviceManager::autoExportVideoAndTrigger, this, &CSListViewVideo::slotAutoExportVideoByIdAndTrigger);
	connect(ui->tableViewVideo, &CSVideoTableView::tableviewMoveEvent, this, [=](const QModelIndex& index) {
		if (index.isValid()) {
			m_old_model_index = index;
			int current_row = m_video_table_model_ptr->itemFromIndex(index)->row();
			QStandardItem* currrent_item_name = m_video_table_model_ptr->item(current_row, (int)VideoTableColumn::kVideoName);
			QStandardItem* currrent_item_time = m_video_table_model_ptr->item(current_row, (int)VideoTableColumn::kVideoAcqTime);
			QStandardItem* currrent_item_size = m_video_table_model_ptr->item(current_row, (int)VideoTableColumn::kVideoSize);
			if (!currrent_item_name || !currrent_item_time || !currrent_item_size)
			{
				return;
			}

			VideoListThumbnail::ThumbnailInfo info{};
			QSharedPointer<Device> new_device_ptr = DeviceManager::instance().getCurrentDevice();
			if (new_device_ptr) {
				info.thumbnail_mat = new_device_ptr->getVideoClipScale(currrent_item_name->data(),&VideoItemManager::instance().getVideoItem(currrent_item_name->data()));
			}
			if (new_device_ptr->isGrabberDevice()) {
				ui->tableViewVideo->setEditTriggers(QAbstractItemView::NoEditTriggers);
			}
			info.video_name = currrent_item_name->text();
			info.record_time = currrent_item_time->text();
			info.video_size = currrent_item_size->text();
			m_thumbnail_dlg.setThumbnailInfo(QCursor::pos(), info);
			if (m_thumbnail_dlg.isHidden()) {
				m_thumbnail_dlg.show();
			}
		}
		else {
			m_thumbnail_dlg.hide();
			m_old_model_index = QModelIndex{};
		}
	});

	connect(ui->tableViewVideo, &CSVideoTableView::tableviewLeaveEvent, this, [=]() {
		m_thumbnail_dlg.hide();
	});

}

void CSListViewVideo::slotCurrentDeviceChanged(const QString current_ip)
{
	//判断是否跟之前的设备相同 ,不相同的话则断开原设备的信号连接并且跟新设备连接信号,判断是否需要刷新列表,相同的话则返回
	QSharedPointer<Device> new_device_ptr = DeviceManager::instance().getCurrentDevice();
	if (m_current_device_ptr == new_device_ptr)
	{
		return;
	}
	//不相同,断开原来的设备型号连接
	if (!m_current_device_ptr.isNull())
	{
		disconnect(m_current_device_ptr.data(), 0, this, 0);
	}
	m_current_device_ptr = new_device_ptr;
	//删除原有列表中的视频信息
	m_video_table_model_ptr->removeRows(0, m_video_table_model_ptr->rowCount());

	if (!m_current_device_ptr.isNull())
	{
		//连接视频列表刷新信号.
		connect(m_current_device_ptr.data(), &Device::videoSegmentListUpdated, this, &CSListViewVideo::slotDeviceVideoUpdated);
		connect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSListViewVideo::slotDeviceStateChanged);


		{
			//刷新视频列表
			updateCurrentDeviceVideoList();
		}
	}

}

void CSListViewVideo::slotDeviceStateChanged(const QString &ip, DeviceState state)
{
	if (m_current_device_ptr && ip != m_current_device_ptr->getIpOrSn()) return;
	if (state == Unconnected || state == Disconnected)
	{
		//删除原有列表中的视频信息
		m_video_table_model_ptr->removeRows(0, m_video_table_model_ptr->rowCount());
	}
}

void CSListViewVideo::slotDeviceVideoUpdated()
{
	updateCurrentDeviceVideoList();
}



QMainWindow * CSListViewVideo::getMainWindow()
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

void CSListViewVideo::updateCurrentDeviceVideoList()
{
	if (m_current_device_ptr.isNull())
	{
		return;
	}

	m_nShiftSelectedIndex = -1;
	//删除原有列表中的视频信息
	m_video_table_model_ptr->removeRows(0,m_video_table_model_ptr->rowCount());
	if (m_pFreezeHeader && m_pFreezeHeader->GetChecked())
	{
		m_pFreezeHeader->SlotStateChanged(Qt::Unchecked);
		slotHeaderClick(Qt::Unchecked);//切换设备时清空选中视频状态
	}

	if (m_current_device_ptr->getState() == DeviceState::Unconnected||
		m_current_device_ptr->getState() == DeviceState::Disconnected)
	{
		return;
	}
	//从视频管理器中获取与设备相关的信息放到视频列表数据model中
	QList<VideoItem> current_video_items = VideoItemManager::instance().findVideoItems(VideoItem::Remote, m_current_device_ptr->getIpOrSn());
	QList<QList<QStandardItem *>> video_info_list;
	int i = 0;
	for (auto video_item : current_video_items)
	{
		QList<QStandardItem*> video_item_info;//单个视频信息:视频名称,采集时间,视频大小

		//视频名称
		QStandardItem * name = new QStandardItem();
		name->setText(video_item.getName());
		name->setToolTip(video_item.getName());
		name->setData(video_item.getId());//写入视频项id,以便取用
		name->setTextAlignment(Qt::AlignCenter);

		//采集时间
		QStandardItem* time = new QStandardItem();
		uint64_t time_stamp =  video_item.getProperty(VideoItem::TimeStamp).toLongLong();
		QDateTime q_time = QDateTime::fromTime_t(time_stamp);
		time->setText(q_time.toString("yyyyMMdd-hhmmss"));
		time->setToolTip(q_time.toString("yyyyMMdd-hhmmss"));
		time->setTextAlignment(Qt::AlignCenter);

		//视频大小
		QStandardItem * size = new QStandardItem();
		double size_in_mb = video_item.getProperty(VideoItem::PropType::VideoSize).toLongLong()*1.0 / 1024 / 1024;
		size->setText( QString::number(size_in_mb,'f',3));
		size->setToolTip(QString::number(size_in_mb, 'f', 3));
		size->setTextAlignment(Qt::AlignCenter);

		// 选择
		QStandardItem *item = new QStandardItem();
	
		video_item_info << item << name << time << size;

		video_info_list << video_item_info;
	}

	//将表格写入数据model
	for (auto info :video_info_list)
	{
		m_video_table_model_ptr->appendRow(info);
	}

	ui->tableViewVideo->sortByColumn(2, Qt::AscendingOrder);
	

}

void CSListViewVideo::InitMenus()
{
	//初始化视频右键菜单
	m_menu_video = new QMenu(ui->tableViewVideo);
	m_action_export_preview = new QAction(tr("Playback"));//导出预览更名:回放
	m_action_export = new QAction(tr("Export"));
	m_action_delete = new QAction(tr("Delete"));
	m_action_format = new QAction(tr("Formating"));
	m_action_batch_export = new QAction(tr("Batch Export"));

	m_menu_video->addAction(m_action_export_preview);
	m_menu_video->addAction(m_action_export);
	m_menu_video->addAction(m_action_delete);
	m_menu_video->addSeparator();
	m_menu_video->addAction(m_action_batch_export);
	m_menu_video->addSeparator();
	m_menu_video->addAction(m_action_format);

	//连接选项对应槽函数
	connect(m_action_export.data(), &QAction::triggered, this, &CSListViewVideo::slotExportCurrentVideo);
	connect(m_action_delete.data(), &QAction::triggered, this, &CSListViewVideo::slotDeleteCurrentVideo);
	connect(m_action_format.data(), &QAction::triggered, this, &CSListViewVideo::slotDeleteAllVideo);
	connect(m_action_batch_export.data(), &QAction::triggered, this, &CSListViewVideo::slotExportCurrentVideoAll);
	connect(m_action_export_preview.data(), &QAction::triggered, this, &CSListViewVideo::SlotPreviewCurrentVideo);
}

void CSListViewVideo::updateMenus(bool video_valid)
{
	if (m_current_device_ptr.isNull())
	{
		m_action_export->setEnabled(false);
		m_action_delete->setEnabled(false);
		m_action_format->setEnabled(false);
		m_action_batch_export->setEnabled(false);
		m_action_export_preview->setEnabled(false);

	}
	else
	{
		DeviceState state = m_current_device_ptr->getState();
		if (0 == m_video_table_model_ptr->rowCount())
		{//视频列表为空时，禁用全部导出选项
			m_action_batch_export->setEnabled(false);
		}
		else if (isListChecked())
		{
			bool bEnableBatch = true;
			if (!video_valid)
			{
				bEnableBatch = false;
				if (state == Connected || state == Previewing || state == Acquiring)
				{
					bEnableBatch = true;
				}
			}
			m_action_batch_export->setEnabled(bEnableBatch);
		}
		else
		{
			m_action_batch_export->setEnabled(false);
		}
		m_action_export_preview->setEnabled(video_valid);
		m_action_export->setEnabled(video_valid);

		bool enable = false;
		//只有在已连接和预览状态下可以进行删除和格式化操作
		if (state == Connected ||
			state == Previewing)
		{
			enable = true;
		}
		m_action_delete->setEnabled(enable && video_valid);
		m_action_format->setEnabled(enable);
	}
}

QString CSListViewVideo::getIdAndIpFromModelIndex(const QModelIndex & model_index, int &vid, QString & device_ip)
{
	//传入参数初始化
	vid = -1;
	device_ip.clear();
	
	if (!model_index.isValid())
	{
		return QString();
	}

	//获取当前行存放在首项data中的视频项ID
	QString current_video_item_id;
	int current_row = m_video_table_model_ptr->itemFromIndex(model_index)->row();
	QStandardItem* currrent_item = m_video_table_model_ptr->item(current_row, (int)VideoTableColumn::kVideoName);
	if (currrent_item && currrent_item->data().isValid())
	{
		//从中解析视频ID和对应的设备IP
		current_video_item_id = currrent_item->data().toString();
		vid = VideoUtils::parseVideoSegmentId(current_video_item_id);
		device_ip = VideoUtils::parseDeviceIp(current_video_item_id);
	}
	return current_video_item_id;
	
}

void CSListViewVideo::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		//设置表头样式:视频名称,采集时间,视频大小
		QStringList header_labels;
		header_labels << tr("") << tr("Video Name") << tr("Acquisition Time") << tr("Video Size(MB)");
		m_video_table_model_ptr->setHorizontalHeaderLabels(header_labels);
		ui->tableViewVideo->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
		ui->tableViewVideo->horizontalHeader()->setStyleSheet("QTableView{border: 0px}\
		QHeaderView{border: 0px solid ; border-bottom: 1px solid #d8d8d8;} \
		QTableView::item:selected {color:rgb(255,255,255); background-color:rgb(0,120,215);}");
		ui->tableViewVideo->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
		ui->tableViewVideo->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
		
		updateCurrentDeviceVideoList();

		m_action_export_preview->setText(tr("Playback"));//导出预览更名:回放
		m_action_export->setText(tr("Export"));
		m_action_delete->setText(tr("Delete"));
		m_action_format->setText(tr("Formating"));
		m_action_batch_export->setText(tr("Batch Export"));
	}

	QWidget::changeEvent(event);
}

void CSListViewVideo::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Shift:
	{
		m_bPressShift = true;
		if (ui->tableViewVideo)
		{
			m_nShiftSelectedIndex = ui->tableViewVideo->currentIndex().row();
		}
		break;
	}
	default:
		break;
	}
	QWidget::keyPressEvent(event);
}

void CSListViewVideo::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Shift:
	{
		m_bPressShift = false;
		break;
	}
	default:
		break;
	}
	QWidget::keyReleaseEvent(event);
}

void CSListViewVideo::on_tableViewVideo_customContextMenuRequested(const QPoint &pos)
{
	//判断设备是不是有效设备
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	if (!m_current_device_ptr->IsCamera())
	{
		return;
	}

	//判断当前设备此时是否支持导出
	DeviceState state =	m_current_device_ptr->getState();
	if (state == Unconnected ||
		state == Disconnected ||
		state == StandBy ||
		state == DeviceState::ToWakeup)
	{
		return;
	}
	bool bEnable = false;
	if (state == Connected || state == Acquiring || state == Previewing)
	{
		bEnable = true;
	}

	//鼠标位置是否是有效位置
	QModelIndex index = ui->tableViewVideo->indexAt(pos);

	//刷新菜单选项
	updateMenus(index.isValid() && bEnable);

	if (!m_thumbnail_dlg.isHidden()) {
		m_thumbnail_dlg.hide();
	}
	//开启菜单
	m_menu_video->exec(ui->tableViewVideo->viewport()->mapToGlobal(pos));
	
}

void CSListViewVideo::slotExportCurrentVideo()
{
	// [2023/2/21 rgq]: 导出功能取代批量导出功能，原有的导出不再使用
// 	slotExportCurrentVideoAll();
// 	return;
	//判断当前设备是不是已连接状态,不是则提示弹窗,停止设备
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	if (m_current_device_ptr->getState() != Connected)
	{
		DeviceState curState = m_current_device_ptr->getState();
		if (curState == Reconnecting || curState == Disconnected || curState == Unconnected || curState == StandBy)
		{
			UIUtils::showInfoMsgBox(this, tr("The current state of the device is not supported."));
			return;
		}
		if (UIUtils::showQuestionMsgBox(this, tr("The device will quit preview and acquisition mode before export, continue?")))
		{
			if (m_current_device_ptr.isNull())
			{
				return;
			}
			m_current_device_ptr->stop(Device::eNomal);
		}
		else
		{
			return;
		}
	}

	//弹出路径选择对话框,选择路径,检查权限,
	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}
	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	if (m_selected_video_item_id.isEmpty())
	{
		return;
	}

	VideoItem video_item = VideoItemManager::instance().getVideoItem(m_selected_video_item_id);
	video_item.setExportPath(export_path);
	video_item.setProperty(VideoItem::PropType::StreamType,m_current_device_ptr->getProperty(Device::PropStreamType) );
	video_item.setProperty(VideoItem::PropType::VideoFormat, m_current_device_ptr->getProperty(Device::PropVideoFormat));
	video_item.setProperty(VideoItem::PropType::OsdVisible, m_current_device_ptr->getProperty(Device::PropWatermarkEnable));

	std::vector<std::pair<int32_t, VideoItem>> video_items;
	QString strVideos;
	video_items.push_back(std::make_pair(m_current_idx, video_item));
	if (!isAllowExport(video_items, strVideos) && !strVideos.isEmpty())
	{
		QString szText;
		szText = tr("Please change the protocol format or reselect.The current protocol format does not support the following video export:");
		szText += "\r\n";
		szText += strVideos;
		UIUtils::showErrorMsgBox(this, szText);
		return;
	}
	//参数修正
	PreExportColorCorrectRollback preExport(m_current_device_ptr->getProcessor(), video_item);
	//开启导出对话框,传入设备指针和需要导出的视频项,准备导出视频
	CSDlgDeviceVideoExport dlg(std::make_pair(m_current_idx, video_item), false, this);
	dlg.exec();
}

bool CSListViewVideo::GetEnableExport()
{
	if (m_current_device_ptr.isNull())
	{
		return false;
	}

	HscIntRange width_range{};
	if (!m_current_device_ptr->getRoiWidthRange(0, width_range))
	{
		return false;
	}

	HscIntRange height_range{};
	if (!m_current_device_ptr->getRoiHeightRange(0, height_range))
	{
		return false;
	}

	VideoItem video_item = VideoItemManager::instance().getVideoItem(m_selected_video_item_id);
	QRect rect = video_item.getRoi();
	if (height_range.min > rect.height() || width_range.min > rect.width())
	{
		return false;
	}
	return true;
}

void CSListViewVideo::slotDeleteCurrentVideo()
{
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	//删除当前视频项
	if (m_selected_vid >-1 )
	{
		//判断删除模式,判断是不是最后一个视频
		HscVideoClipDeleteMode deleteMode = m_current_device_ptr->GetVideoClipDeleteMode();
		if (deleteMode == HSC_VCDM_REVERSED)
		{
			//获取当前行存放在末项data中的视频项ID
			QString end_video_item_id;
			int end_vid = -1;
			int end_row = m_video_table_model_ptr->rowCount() - 1;
			QStandardItem* end_item = m_video_table_model_ptr->item(end_row, (int)VideoTableColumn::kVideoName);
			if (end_item && end_item->data().isValid())
			{
				//从中解析视频ID和对应的设备IP
				end_video_item_id = end_item->data().toString();
				end_vid = VideoUtils::parseVideoSegmentId(end_video_item_id);
			}

			if (end_vid != m_selected_vid)
			{
				UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_ONLY_REVERSE_ORDER_DEL"));
				return;
			}
		}

		//询问是否确认
		if (UIUtils::showQuestionMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_DEL_TIP")))
		{
			if (m_current_device_ptr.isNull())
			{
				return;
			}
			m_current_device_ptr->removeVideoSegment(m_selected_vid);

			//刷新设备属性列表
			CSRccApp* main_window = dynamic_cast<CSRccApp*>(getMainWindow());
			if (main_window)
			{
				main_window->UpdateDevicePropertyList();
			}
			//刷新当前视频列表
			updateCurrentDeviceVideoList();
		}
	}
}

void CSListViewVideo::slotDeleteAllVideo()
{
	//询问是否确定
	if (UIUtils::showQuestionMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_FORMAT_TIP")))
	{
		if (m_current_device_ptr.isNull())
		{
			return;
		}

		HscResult res;
		//采集中的设备先停机
		if (m_current_device_ptr->getState() == DeviceState::Acquiring || 
			m_current_device_ptr->getState() == DeviceState::Recording)
		{
			m_current_device_ptr->stop();
		}

		//通知相机开始格式化
		res = m_current_device_ptr->DeleteAllVideoClips();
		if (res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_FORMAT_FAIL"));
			return;
		}
		if (m_current_device_ptr.isNull())
		{
			return;
		}
		//显示格式化进度对话框
		CSDlgProgressFormat dlg(m_current_device_ptr);
		dlg.exec();

		//刷新设备属性列表
		CSRccApp* main_window = dynamic_cast<CSRccApp*>(getMainWindow());
		if (main_window)
		{
			main_window->UpdateDevicePropertyList();
		}

		//刷新当前视频列表
		updateCurrentDeviceVideoList();
	}
}

void CSListViewVideo::SlotPreviewCurrentVideo()
{
	//判断当前设备是不是已连接状态,不是则提示弹窗,停止设备
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	if (m_current_device_ptr->getState() != Connected)
	{
		if (UIUtils::showQuestionMsgBox(this, tr("The device will quit preview and acquisition mode before export, continue?")))
		{
			if (m_current_device_ptr.isNull())
			{
				return;
			}
			DeviceState curStatus = m_current_device_ptr->getState();
			if (curStatus == Disconnected || curStatus == Reconnecting || curStatus == Disconnecting || curStatus == StandBy)
			{
				return;
			}
			m_current_device_ptr->stop(Device::StopMode::eNoStop);
		}
		else
		{
			return;
		}
	}
	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}
	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	if (m_selected_video_item_id.isEmpty())
	{
		return;
	}
	
	// [2022/8/19 rgq]: 添加是否支持预览的判断，不支持的不进入预览界面
	if (!GetEnableExport())
	{
		UIUtils::showErrorMsgBox(this, QObject::tr("The current protocol format does not support playback. Please change the protocol format and try again."));
		return;
	}

	VideoItem video_item = VideoItemManager::instance().getVideoItem(m_selected_video_item_id);
	video_item.setExportPath(export_path);
	video_item.setProperty(VideoItem::PropType::StreamType, m_current_device_ptr->getProperty(Device::PropStreamType));
	video_item.setProperty(VideoItem::PropType::VideoFormat, m_current_device_ptr->getProperty(Device::PropVideoFormat));
	video_item.setProperty(VideoItem::PropType::OsdVisible, m_current_device_ptr->getProperty(Device::PropWatermarkEnable));

	{
		//参数修正
		PreExportColorCorrectRollback preExport(m_current_device_ptr->getProcessor(), video_item);

		CSExportPreview dlg(m_current_idx, video_item, this);
		dlg.setWindowState(Qt::WindowMaximized);
		bindReplayMessage(&dlg);
		dlg.exec();
		unbindReplayMessage(&dlg);
	}
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	DeviceState status = m_current_device_ptr->getState();
	if (status == Exporting || status == Recording || status == ToExport || status == Replaying || status == ToReplay)
	{
		//预览结束,停机
		m_current_device_ptr->stop();
	}
}

void CSListViewVideo::slotAutoExportVideoById(const QVariant video_id)
{
	QString ip = VideoUtils::parseDeviceIp(video_id);
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
	{
		return;
	}

	if (device_ptr->getState() != Connected)
	{
		device_ptr->stop();
	}

	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}
	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	if (video_id.isNull())
	{
		return;
	}
	VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
	video_item.setExportPath(export_path);
	video_item.setProperty(VideoItem::PropType::StreamType, device_ptr->getProperty(Device::PropStreamType));
	video_item.setProperty(VideoItem::PropType::VideoFormat, device_ptr->getProperty(Device::PropVideoFormat));
	video_item.setProperty(VideoItem::PropType::OsdVisible, device_ptr->getProperty(Device::PropWatermarkEnable));

	{
		//参数修正
		PreExportColorCorrectRollback preExport(device_ptr->getProcessor(), video_item);
		int index = m_video_table_model_ptr->rowCount() - 1;
		//开启导出对话框,传入设备指针和需要导出的视频项,准备导出视频
		CSDlgDeviceVideoExport dlg(std::make_pair(index, video_item), true, this);
		dlg.exec();
	}

	//导出结束,进入高采
	device_ptr->setShowTip(true);
	device_ptr->acquire();
}

void CSListViewVideo::slotAutoPlaybackVideoById(const QVariant video_id)
{
	QString ip = VideoUtils::parseDeviceIp(video_id);
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
	{
		return;
	}

	if (device_ptr->getState() != Connected)
	{
		device_ptr->stop();
	}


	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}
	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	if (video_id.isNull())
	{
		return;
	}

	//采集后事件执行中,则退出当前流程
	if (m_after_record_processing)
	{
		return;
	}
	m_after_record_processing = true;

	VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
	video_item.setExportPath(export_path);
	video_item.setProperty(VideoItem::PropType::StreamType, device_ptr->getProperty(Device::PropStreamType));
	video_item.setProperty(VideoItem::PropType::VideoFormat, device_ptr->getProperty(Device::PropVideoFormat));
	video_item.setProperty(VideoItem::PropType::OsdVisible, device_ptr->getProperty(Device::PropWatermarkEnable));

	{
		//参数修正
		PreExportColorCorrectRollback preExport(device_ptr->getProcessor(), video_item);
		//开启预览对话框,传入设备指针和需要导出的视频项,准备导出视频
		int index = m_video_table_model_ptr->rowCount() - 1;
		CSExportPreview dlg(index, video_item, this);
		dlg.setWindowState(Qt::WindowMaximized);
		bindReplayMessage(&dlg);
		dlg.exec();
		unbindReplayMessage(&dlg);
	}

	if (!device_ptr)
	{
		return;
	}
	DeviceState status = device_ptr->getState();
	if (status == Exporting || status == Recording || status == ToExport || status == Replaying || status == ToReplay)
	{
		//预览结束,停机
		device_ptr->stop();
	}


	//结束采集后事件
	m_after_record_processing = false;
}

void CSListViewVideo::slotAutoExportVideoByIdAndTrigger(const QVariant video_id)
{
	QString ip = VideoUtils::parseDeviceIp(video_id);
	auto device_ptr = DeviceManager::instance().getDevice(ip);
	if (!device_ptr)
	{
		return;
	}

	if (device_ptr->getState() != Connected)
	{
		device_ptr->stop();
	}

	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}
	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	if (video_id.isNull())
	{
		return;
	}
	VideoItem video_item = VideoItemManager::instance().getVideoItem(video_id);
	video_item.setExportPath(export_path);
	video_item.setProperty(VideoItem::PropType::StreamType, device_ptr->getProperty(Device::PropStreamType));
	video_item.setProperty(VideoItem::PropType::VideoFormat, device_ptr->getProperty(Device::PropVideoFormat));
	video_item.setProperty(VideoItem::PropType::OsdVisible, device_ptr->getProperty(Device::PropWatermarkEnable));

	{
		//参数修正
		PreExportColorCorrectRollback preExport(device_ptr->getProcessor(), video_item);
		int index = m_video_table_model_ptr->rowCount() - 1;
		//开启导出对话框,传入设备指针和需要导出的视频项,准备导出视频
		CSDlgDeviceVideoExport dlg(std::make_pair(index, video_item), true, this);
		dlg.exec();
		if (!dlg.getExportResult())
		{
			return;
		}
	}

	HscResult res;
	//导出结束,自动删除视频
	if (device_ptr->IsAutoDeleteVideoSegmentSupported())
	{
		res = (HscResult)device_ptr->removeVideoSegment(VideoUtils::parseVideoSegmentId(video_id));
		//刷新当前视频列表
		updateCurrentDeviceVideoList();
		if (res != HSC_OK)
		{
			ErrorCodeUtils::handle(this, res,video_id);
			return ;
		}
	}
	
	//导出结束,进入高采
	device_ptr->setShowTip(true);
	device_ptr->acquire();
	auto start_time = std::chrono::high_resolution_clock::now();
	bool is_allow_trigger = device_ptr->allowsTrigger();
	while (!is_allow_trigger) {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() >= 1000) {
			CSLOG_WARN("wait for acquire timeout");
			return;
		}
		boost::this_thread::sleep_for(boost::chrono::microseconds(100));
		is_allow_trigger = device_ptr->allowsTrigger();
	}

	//等待高采结束自动触发
	//boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
	device_ptr->setShowTip(true);
	device_ptr->trigger();

}

void CSListViewVideo::slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (previous == current || !current.isValid())
	{
		return;
	}
	//根据当前选中的index获取当前对应的视频id和设备ip
	m_selected_video_item_id = getIdAndIpFromModelIndex(current, m_selected_vid, m_selected_device_ip);
	m_current_idx = m_video_table_model_ptr->itemFromIndex(current)->row();

}

void CSListViewVideo::slotExportCurrentVideoAll()
{
	//判断当前设备是不是已连接状态,不是则提示弹窗,停止设备
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	if (m_current_device_ptr->getState() != Connected)
	{
		DeviceState curState = m_current_device_ptr->getState();
		if (curState == Reconnecting || curState == Disconnected || curState == Unconnected || curState == StandBy)
		{
			UIUtils::showInfoMsgBox(this, tr("The current state of the device is not supported."));
			return;
		}
		if (UIUtils::showQuestionMsgBox(this, tr("The device will quit preview and acquisition mode before export, continue?")))
		{
			if (m_current_device_ptr.isNull())
			{
				return;
			}
			m_current_device_ptr->stop();
		}
		else
		{
			return;
		}
	}

	//弹出路径选择对话框,选择路径,检查权限,
	QString export_path = SystemSettingsManager::instance().getWorkingDirectory();

	if (export_path.isEmpty())
	{
		return;
	}

	//保存参数，完成导出后回写（析构中回写）
	PreExportColorCorrectRollback preExport(m_current_device_ptr->getProcessor());

	//准备videoItem视频项,写入导出路径,协议格式,视频保存格式,
	std::vector<std::pair<int32_t,VideoItem>> video_items;
	for (int i = 0; i < m_video_table_model_ptr->rowCount(); i++)
	{
		QStandardItem *checkBox_item = m_video_table_model_ptr->item(i, 0);
		bool data = m_video_table_model_ptr->data(m_video_table_model_ptr->indexFromItem(checkBox_item), Qt::UserRole).toBool();
		if (!data) continue;

		QStandardItem* currrent_item = m_video_table_model_ptr->item(i, (int)VideoTableColumn::kVideoName);
		if (currrent_item && currrent_item->data().isValid())
		{
			//从中解析视频ID和对应的设备IP
			auto current_video_item_id = currrent_item->data().toString();
			VideoItem video_item = VideoItemManager::instance().getVideoItem(current_video_item_id);
			video_item.setExportPath(export_path);
			video_item.setProperty(VideoItem::PropType::StreamType, m_current_device_ptr->getProperty(Device::PropStreamType));
			video_item.setProperty(VideoItem::PropType::VideoFormat, m_current_device_ptr->getProperty(Device::PropVideoFormat));
			video_item.setProperty(VideoItem::PropType::OsdVisible, m_current_device_ptr->getProperty(Device::PropWatermarkEnable));

			video_items.push_back(std::make_pair(i,video_item));
		}

	}
	if (video_items.size() == 0)
	{
		UIUtils::showErrorMsgBox(this, tr("No video selected. Please select a video first."));
		return;
	}
	QString strVideos;
	if (!isAllowExport(video_items, strVideos) && !strVideos.isEmpty())
	{
		QString szText;
		szText = tr("Please change the protocol format or reselect.The current protocol format does not support the following video export:");
		szText += "\r\n";
		szText += strVideos;
		UIUtils::showErrorMsgBox(this, szText);
		return;
	}
	//开启导出对话框,传入设备指针和需要导出的视频项,准备导出视频
	CSDlgDeviceVideoExport dlg(video_items, false, this);
	dlg.exec();
}


void CSListViewVideo::slotHeaderClick(Qt::CheckState state)
{
	int nSize = m_video_table_model_ptr->rowCount();
	bool bChecked = false;
	if (state == Qt::Checked)
	{
		bChecked = true;
	}
	for (int i = 0; i < nSize; i++)
	{
		QStandardItem *item = m_video_table_model_ptr->item(i);
		if (item)
		{
			m_video_table_model_ptr->setData(m_video_table_model_ptr->indexFromItem(item), bChecked, Qt::UserRole);
		}
	}
}

void CSListViewVideo::slotTableItemCheck(const QModelIndex &index,bool bChecked1)
{
	bool bChecked = true;
	int nSize = m_video_table_model_ptr->rowCount();
	int nCurrentIndex = index.row();
	if (m_bPressShift)
	{
		if (m_nShiftSelectedIndex == -1)
		{
		}
		else
		{
			if (m_nShiftSelectedIndex != nCurrentIndex)
			{
				int nMin = m_nShiftSelectedIndex;
				int nMax = nCurrentIndex - 1;
				if (m_nShiftSelectedIndex > nCurrentIndex)
				{
					nMin = nCurrentIndex + 1;
					nMax = m_nShiftSelectedIndex;
				}
				for (int i = nMin; i <= nMax; i++)
				{
					QStandardItem *item = m_video_table_model_ptr->item(i);
					if (item)
					{
						m_video_table_model_ptr->setData(m_video_table_model_ptr->indexFromItem(item), bChecked1, Qt::UserRole);
					}
				}
			}
		}
	}
	for (int i = 0; i < nSize; i++)
	{
		QStandardItem *item = m_video_table_model_ptr->item(i);
		if (item)
		{
			bool data = m_video_table_model_ptr->data(m_video_table_model_ptr->indexFromItem(item), Qt::UserRole).toBool();
			if (!data)
			{
				bChecked = false;
				break;
			}
		}
	}
	if (nSize == 0)
	{
		bChecked = false;
	}
	Qt::CheckState state = Qt::Unchecked;
	if (bChecked)
	{
		state = Qt::Checked;
	}
	SignalStateChanged(state);
}

void CSListViewVideo::slotItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	if ((1 != topLeft.column()) || (1 != bottomRight.column())) {
		return;
	}

	VideoItem video_item = VideoItemManager::instance().getVideoItem(m_selected_video_item_id);
	QString text_ = m_video_table_model_ptr->item(topLeft.row(), bottomRight.column())->text();
	HscVideoListItemInfo info{};
	info.method = VIDEP_CLIP_METHOD::SET_VIDEO_CLIP_RENAME;
	memcpy(info.name, text_.toStdString().c_str(), sizeof(info.name));
	info.vid = VideoUtils::parseVideoSegmentId(video_item.getId());;
	auto res = m_current_device_ptr->SetVideoListItemInfo(info);
	if (res == HSC_OK)
	{
		video_item.setName(text_);
		VideoItemManager::instance().setVideoItem(m_selected_video_item_id, video_item);
	}
}

bool CSListViewVideo::isAllowExport(std::vector<std::pair<int32_t, VideoItem>> video_items, QString& strVideos)
{
	if (m_current_device_ptr.isNull())
	{
		return false;
	}
	bool bRet = true;
	StreamType stream_type = m_current_device_ptr->getProperty(Device::PropStreamType).value<StreamType>();
	if (stream_type == TYPE_YUV420 || stream_type == TYPE_H264)
	{
		for (auto temp : video_items)
		{
			auto item = temp.second;
			if (item.getRoi().height() < 128)
			{
				if (!strVideos.isEmpty())
				{
					strVideos += "\r\n";
				}
				strVideos += item.getName();
				bRet = false;
			}
		}
	}
	return bRet;
}

bool CSListViewVideo::isListChecked()
{
	bool bChecked = false;
	if (m_video_table_model_ptr)
	{
		int nSize = m_video_table_model_ptr->rowCount();
		for (int i = 0; i < nSize; i++)
		{
			QStandardItem *item = m_video_table_model_ptr->item(i);
			if (item)
			{
				bool data = m_video_table_model_ptr->data(m_video_table_model_ptr->indexFromItem(item), Qt::UserRole).toBool();
				if (data)
				{
					bChecked = true;
					break;
				}
			}
		}
	}
	return bChecked;
}

void CSListViewVideo::bindReplayMessage(CSExportPreview *dlg)
{
	connect(dlg, &CSExportPreview::signalExportpreviewShowMain, this, &CSListViewVideo::signalExportpreviewShowMain);
}

void CSListViewVideo::unbindReplayMessage(CSExportPreview *dlg)
{
	disconnect(dlg, &CSExportPreview::signalExportpreviewShowMain, this, &CSListViewVideo::signalExportpreviewShowMain);
}