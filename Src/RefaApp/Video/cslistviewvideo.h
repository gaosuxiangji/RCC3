#ifndef CSLISTVIEWVIDEO_H
#define CSLISTVIEWVIDEO_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QAction>
#include <QPointer>
#include <QMainWindow>
#include <atomic>
#include "Device/devicestate.h"
#include "tabelheaderview.h"
#include "checkboxdelegate.h"
#include "Video/VideoItem/videoitem.h"
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QLineEdit>
#include <atomic>
#include "../videolistthumbnail/videolistthumbnail.h"
#include "../Main/rccapp/csexportpreview/csexportpreview.h"

namespace Ui {
class CSListViewVideo;
}

class TableHeaderView;
class Device;

class TableDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	TableDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) { }

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const
	{
		QLineEdit *editor = new QLineEdit(parent);
		QRegExp regx("[a-zA-Z0-9\-]{1,32}");
		editor->setValidator(new QRegExpValidator(regx, parent));
		return editor;
	}

	void setEditorData(QWidget *editor, const QModelIndex &index) const
	{
		QString text = index.model()->data(index, Qt::EditRole).toString();
		QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
		lineEdit->setText(text);
	}

	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const
	{
		QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
		QString text = lineEdit->text();
		model->setData(index, text, Qt::EditRole);
	}

	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		editor->setGeometry(option.rect);
	}
};

class CustomStandardItemModel :public QStandardItemModel {
	Qt::ItemFlags flags(const QModelIndex& index) const {
 		if (1 == index.column())
 			return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
 		else
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	}
};

/**
 * @brief 视频列表界面
 */
class CSListViewVideo : public QWidget
{
    Q_OBJECT

public:
    explicit CSListViewVideo(QWidget *parent = 0);
    ~CSListViewVideo();

	void InitUI();

signals:
	void SignalStateChanged(int state);

	void signalExportpreviewShowMain(bool bshow);
private slots:
	
/**
**@ Brife	槽函数 当前设备切换
**@ Param	current_ip 当前设备ip
**@ Note	设备切换后根据情况刷新视频列表
*/
void slotCurrentDeviceChanged(const QString current_ip);

/**
**@ Brife	槽函数 设备状态切换
**@ Note	状态切换后刷新属性列表
*/
void slotDeviceStateChanged(const QString &current_ip, DeviceState state);

/**
**@ Brife	槽函数 设备状态切换
**@ Note	状态切换后刷新属性列表
*/
void slotDeviceVideoUpdated();

/**
**@	Brife	右键菜单信号响应
**@	Param	pos 当前鼠标位置
**@	Note	根据鼠标位置判断是否弹出菜单
*/
void on_tableViewVideo_customContextMenuRequested(const QPoint &pos);

/**
**@	Brife	槽函数 导出当前选中的视频
*/
void slotExportCurrentVideo();

/**
**@	Brife	槽函数 删除当前选中的视频
*/
void slotDeleteCurrentVideo();

/**
**@	Brife	槽函数 格式化全部视频
*/
void slotDeleteAllVideo();

/**************************
* @brief: 预览当前选中的视频
* @param:
* @return:
* @author:mpp
* @date:2022/05/27
***************************/
void SlotPreviewCurrentVideo();

/**
* @brief 自动导出id对应的视频
* @param video_id 视频
*/
void slotAutoExportVideoById(const QVariant video_id);

/**
* @brief 自动回放id对应的视频
* @param video_id 视频
*/
void slotAutoPlaybackVideoById(const QVariant video_id);

/**
* @brief 自动导出id对应的视频并且自动触发
* @param video_id 视频
*/
void slotAutoExportVideoByIdAndTrigger(const QVariant video_id);


/**
**@ Brife	视频表格选项切换
**@ Param	current 当前选择的Index  previous 之前的index
**@ Note	切换当前选中的视频项
*/
void slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous);

void slotExportCurrentVideoAll();

	/**
	**@	Brife 列表头check选择
	*/
	void slotHeaderClick(Qt::CheckState state);

	/**
	**@	Brife 列表子项check选择
	*/
	void slotTableItemCheck(const QModelIndex &index,bool bChecked);
	void slotItemChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
private:

	//获取主窗口
	QMainWindow * getMainWindow();

	/**
	**@	Brife	刷新当前设备的视频列表
	**@	Note	视频列表信息从视频项管理器(VideoItemManager)中提取
	*/
	void updateCurrentDeviceVideoList();

	/**
	**@	Brife	右键菜单初始化
	*/
	void InitMenus();

	/**
	**@	Brife	更新右键菜单
	*/
	void updateMenus(bool video_valid);

	/**
	**@ Brife	使用表格中的编号获取视频ID
	**@ Param	[in]model_index 表格数据model中的index
	**@ Param	[out]vid 从视频项id中解析出来的视频片段id
	**@ Param	[out]device_ip 从视频项id中解析出来的设备IP
	**@ Return	QString 视频项id
	**@ Note	视频项ID在每行第一个Item的data中,如果传入参数没有对应视频ID则返回-1和空字符串
	*/
	QString getIdAndIpFromModelIndex(const QModelIndex & model_index, int &vid, QString & device_ip);

	/**
	**@	Brife	获取视频是否支持导出
	*/
	bool GetEnableExport();

	/**
	**@	Brife	判断视频是否支持导出
	*/
	bool isAllowExport(std::vector<std::pair<int32_t, VideoItem>> video_items, QString& strVideos);

	/**
	**@	Brife	判断是否有选择的视频
	*/
	bool isListChecked();

	/**
	**@	Brife	绑定与回放界面显示与否的消息
	*/
	void bindReplayMessage(CSExportPreview *dlg);

	/**
	**@	Brife	解绑与回放界面显示与否的消息
	*/
	void unbindReplayMessage(CSExportPreview *dlg);
	protected:
		/**
		*@brief 变化事件
		*@param [in] : * event : QEvent，事件指针
		**/
		void changeEvent(QEvent *event) override;

		void keyPressEvent(QKeyEvent *event) override;
		void keyReleaseEvent(QKeyEvent *event) override;
private:
	//当前的设备
	QSharedPointer<Device> m_current_device_ptr;

	//视频列表中选中的视频信息
	QString m_selected_video_item_id;//视频管理中对应的视频项ID
	int m_selected_vid = -1;//选中的视频对应的视频片段id
	QString m_selected_device_ip;//选中的视频对应的设备IP
	int m_current_idx = -1;

	QPointer< CustomStandardItemModel> m_video_table_model_ptr;//视频列表数据

	QPointer<QMenu> m_menu_video;//视频选项右键菜单
	QPointer<QAction> m_action_export_preview;//导出预览选项
	QPointer<QAction> m_action_export;//导出选项
	QPointer<QAction> m_action_delete;//删除选项
	QPointer<QAction> m_action_format;//格式化选项
	QPointer<QAction> m_action_batch_export;//批量导出
		 
    std::atomic_bool m_after_record_processing{false}; //采集后事件执行中
	QModelIndex m_old_model_index{};
	VideoListThumbnail m_thumbnail_dlg{};

	bool m_bPressShift{ false };
	int m_nShiftSelectedIndex{ -1 };

	//视频列表列号
	enum VideoTableColumn
	{
		kChoose,
		kVideoName,			//视频名称
		kVideoAcqTime,		//视频采集时间
		kVideoSize			//视频大小
	};

	TableHeaderView* m_pFreezeHeader{nullptr};    //冻结列TableHeaderView对象指针
    Ui::CSListViewVideo *ui;
};

#endif // CSLISTVIEWVIDEO_H
