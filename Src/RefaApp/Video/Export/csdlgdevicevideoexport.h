#ifndef CSDLGDEVICEVIDEOEXPORT_H
#define CSDLGDEVICEVIDEOEXPORT_H

#include <QDialog>
#include <QTimer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QMap>
#include <QValidator>
#include <QStandardItem>
#include "Video/VideoItem/videoitem.h"
#include "HscExportHeader.h"
#include "videolisttabelmodel.h"
#include <QPointer>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include "../../videolistthumbnail/videolistthumbnail.h"

#include "ExportCacheFrame.h"
#include <QGraphicsSceneMouseEvent>
namespace Ui {
class CSDlgDeviceVideoExport;
}

class Device;

class QGraphicsScene;
/**
**@	Brife	设备视频导出对话框
*/

enum SkipUnit
{
	eFrame,
	eMillonSec,
	eSecond
};

typedef struct _TWaterMakerInfo
{
	_TWaterMakerInfo()
	{
		nTransparent = 100;
		nRotate = 0;
		nWaterSize = 100;
		pItem = nullptr;
		X = 0;
		Y = 0;
		nWidth = 0;
		nHeight = 0;
		bVisible = true;
	};
	QString strFileName;		// 文件路径 格式为绝对路径和名称
	QString strFileID;			// 文件索引 格式为名称
	int nTransparent;			// 透明度 0-100, 默认100 
	int nRotate;				// 旋转角度 0-360， 默认0
	int nWaterSize;				// 水印大小百分比，1-500， 默认100
	QGraphicsItem *pItem;
	qreal X;
	qreal Y;
	int nWidth;
	int nHeight;
	bool bVisible;
}TWaterMakerInfo;


typedef struct VideoSaveOpt {
	QVariant     ID;							// 视频ID
	bool         bIsCamera;						// 是否是相机
	int          nBpp;							// Bpp
	StreamType   stream_type;					// 协议类型
	HscIntRange  width_range{};					// ROI宽度范围
	HscIntRange  height_range{};				// ROI高度范围
    QList<VideoFormat> SupportVideoFormats;		// 支持的保存格式
    QList<HscDisplayMode> SupportVideoColors;	// 支持的显示色彩模式
    VideoFormat  videoformat;					// 保存格式
	HscDisplayMode videoCorlor;					// 显示模式
	qint64       fps;							// 帧率
	int          displayfps;					// 播放帧率
    double       skipFrame = 0.0;				// 跳帧
	int32_t		 skipUnit = eFrame;				// 单位
	QString      videoRuleName;					// 带规则的视频名称
	QString      ImageRuleName;					// 带规则的保存图像名称
	uint64_t     uiStart;						// 导出起始帧号
	uint64_t     uiEnd;							// 导出结束帧号
	int          nRoiWidth;						// 分辨率宽
	int          nRoiHeight;					// 分辨率高
    bool         enableVideoInfo;				// 叠加视频信息
    bool         enableAVIcompress = false;		// 是否压缩AVI视频
	QString		 export_path;
	int32_t		 video_index = -1;
    QList<TWaterMakerInfo> waterMarks;			// 自定义水印列表
}VideoSaveOpt;

enum ErrorFileNameValid
{
	EFNV_Success,
	EFNV_More_Dir,
	EFNV_Dir_Change,
	EFNV_SpecialCharacters,
	EFNV_More_length,
	EFNV_File_Name_Empty,
	EFNV_Dir_Invalid,
};

class CSImageGraphiceItem : public QGraphicsPixmapItem
{
public:
	explicit CSImageGraphiceItem(const QPixmap &pixmap, QGraphicsItem *parent = Q_NULLPTR);

protected:
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
private:

};

class CSDlgDeviceVideoExport : public QDialog
{
	Q_OBJECT

	enum WaterMarkEnum
	{
		e_default_watermark = 0x01,
		e_selfdef_watermark = 0x02,
	};

public:
    explicit CSDlgDeviceVideoExport(const std::pair<int32_t, VideoItem> &video_item,bool auto_export = false,QWidget *parent = 0);
	explicit CSDlgDeviceVideoExport(const std::vector<std::pair<int32_t, VideoItem>> &video_items, bool auto_export = false, QWidget *parent = 0);
    ~CSDlgDeviceVideoExport();
	

	/**
	*@brief 导出进度回调
	*@param handle 设备句柄
	*@param value 导出进度
	*@param state 导出状态
	*@note 用来发出进度信号,跨线程更新界面
	**/
	void progressChanged(DeviceHandle handle, uint32_t value, uint32_t state);

	bool getExportResult() { return m_export_result; }

protected:
	void closeEvent(QCloseEvent *event) override;
	void reject() override { close(); }

private slots:
	//保存为默认配置
	void on_pushButton_default_clicked();
	
	//导出
    void on_pushButton_start_clicked();

	//取消导出
    void on_pushButton_cancel_clicked();

	//选择导出路径
    void on_pushButton_browse_clicked();

	//打开导出路径
    void on_pushButton_open_clicked();

	//添加水印
	void on_pushButton_addWatermark_clicked();

	//选择保存格式
    void on_comboBox_save_format_activated(int index);

	//选择色彩模式
    void on_comboBox_color_mode_activated(int index);

	//添加文件名模板
	void on_comboBox_add_file_name_currentIndexChanged(int index);

	//文件名改变
	void on_lineEdit_video_name_textChanged(const QString &arg1);

	//文件名编辑完成
	void on_lineEdit_video_name_editingFinished();

	//透明度编辑框
    void on_spinBox_transparent_editingFinished();
	
	//旋转角编辑框
    void on_spinBox_rotate_editingFinished();

	//水印大小编辑框
    void on_spinBox_watermark_editingFinished();

	void on_doubleSpinBox_jump_editingFinished();

	//
	void on_comboBox_save_unit_activated(int index);

	//设置avi压缩开启
	void on_checkBox_enableAVIcompress_clicked(bool checked);

	//设置水印开启
	void on_checkBox_watermark_clicked(bool checked);

	/**
	**@	Brife	当前视频选择改变
	*/
	void slotTabViewCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous);

	/**
	**@	Brife 导出范围变化
	*/
	void slotExportRangeDataChanged(VideoInfoRecordList info);

	/**
	**@	Brife	槽函数,导出进度变化
	**@ Param	value 导出进度
	**@ Param	state 导出状态
	*/
	void slotProgressChanged(uint32_t value, uint32_t state);

	/**
	**@	Brife	水印透明度编辑
	*/
	void slotSetTransparentLineEditValue(int);

	/**
	**@	Brife	水印旋转角编辑
	*/
	void slotSetRotateLineEditValue(int);

	/**
	**@	Brife	水印大小编辑
	*/
	void slotSetWatermarkLineEditValue(int);

	/**
	**@	Brife	水印列表点击
	*/
	void slotTabViewWatermarkClicked(QModelIndex index);

	/**
	**@	Brife	选择水印变化
	*/
	void slotfocusItemChanged(QGraphicsItem *newFocus, QGraphicsItem *oldFocus, Qt::FocusReason reason);

	/**
	**@	Brife	水印列表复选框选择
	*/
	void slotWatermarkItemCheck(const QModelIndex &index,bool bChecked);

signals:
	/**
	*@brief 导出进度变化信号
	*@param value 导出进度
	*@param state 导出状态
	**/
	void signalProgressChanged(uint32_t value, uint32_t state);

	void SignalChangeCheckBoxState(QAbstractItemModel *model, const QModelIndex &index, bool state);
private:
    QList<VideoSaveOpt> m_videoOpts;
private:
    /**
    **@	Brife	初始化界面
    */
    void InitUI();

	/**
	* @brief 初始化视频列表表头
	*/
	void InitVideoListTableHeader();

	/**
	* @brief 初始化视频列表
	*/
	void InitVideoListTableInfo();

	/**
	* @brief 初始化视频列表
	*/
	void InitWidgetInfo();

	/**
	* @brief 初始化水印列表
	*/
	void InitWatermarkInfo();

	/**
	**@	Brife	读取默认导出设置
	*/
	void ReadSettings();

	/**
	**@	Brife	初始化视频导出信息
	*/
    QList<VideoSaveOpt> loadVideoInfo(QList<QPair<int32_t, VideoItem>> &items);

	/**
	**@	Brife	更新界面视频信息
	*/
    void UpdateVideoUI(VideoSaveOpt opt);

	/**
	**@	Brife更新视频保存参数信息
	*/
    void UpdateVideoSaveInfo(int idx, VideoSaveOpt &opt);

	/**
	* @brief	刷新图像
	* @note
	*/
	void UpdateVideoImage(int idx, VideoSaveOpt opt);

	/**
	* @brief	刷新叠加水印列表
	* @note
	*/
	void UpdateWatermarkerListInfo(VideoSaveOpt &opt);

	/**
	* @brief	刷新默认水印勾选状态
	* @note
	*/
	void UpdateWaterMarkOption(VideoSaveOpt opt);

	/**
	* @brief	刷新avi压缩勾选状态
	* @note
	*/
	void updateAviCompressOption(VideoSaveOpt opt);

	/**
	* @brief	刷新当前位深提示
	* @note
	*/
	void updateDepthHint(VideoSaveOpt opt);

	/**
	* @brief	刷新控件状态
	* @note
	*/
	void updateWidgetsState(VideoSaveOpt opt);

	/**
	* @brief	刷新导出按钮状态，防止文件路径及名称输入过长进入导出会出现异常报错
	* @note
	*/
	void UpdateExportButtonStatus();

	/**
	**@	Brife	开始导出
	*/
	void startExport();

	/**
	* @brief 停止导出
	*/
	void stopExport();

	double getDiskFreeSizeGB(QString path);
	/**
	*@brief 绘制水印
	*@param image_data 图像数据指针
	*@param timestamp 时间戳指针
	*@param frame_no 帧号
	*@param width 宽度
	*@param height 高度
    *@param channel 通道数/home/kylin/new_work/RefaApp/cross_platform/Src/RefaApp/Video/Export/csdlgdevicevideoexport.cpp
	*@note 导出由HscExport模块做，这里作为回调来叠加水印
	**/
	void paintWatermark(uint8_t * image_data, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, int type);
	void paintWatermark1(cv::Mat &mat, uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel);

	/**
	* @brief 获取水印使能
	*/
	bool GetWaterMarkOptionEnable(VideoSaveOpt opt);

	/**
	* @brief 是否支持导出
	*/
	bool GetEnableExport();

	QString FormatStringReplace(QString strText, bool bImage = false);
	
	void saveOptToItm(VideoSaveOpt opt, VideoItem &item);

	bool doExport(int index);

	bool disableAVICompress(VideoSaveOpt opt);
	int32_t getVideoNameType();
	bool getImage(int index, QImage &image);

	/**
	* @brief 设置水印设置项使能
	*/
	void SetWatermarkerCtrEnable(bool bEnable);

	/**
	* @brief 获取水印信息内容
	*/
	int8_t getWaterMarkValue(int index);

	/**
	* @brief 重置水印参数控件初始值
	*/
	void InitWatermarkerCtr();

	/**
	* @brief 更新显示视频名称规则显示的字段
	*/
	void UpdateShowFormatString(VideoSaveOpt opt);

	/**
	* @brief 更新水印显示
	*/
	void UpdateEnableWatermarker(VideoSaveOpt opt);

	/**
	* @brief 更新显示视频名称规则显示的字段
	*/
	void UpdateFileName(VideoSaveOpt opt);

	/**
	* @brief 判断名称是否合法
	* 返回值 0 正常， 1：文件目录存在多层，2：存在特殊字符， 3：文件字符长度超出
	*/
	ErrorFileNameValid isPathFileValid(QString strFileName, QString strPath, VideoSaveOpt opt);

	/**
	* @brief 判断是否保存为图片格式
	*/
	bool isSaveImage(VideoFormat videoformat);

	/**
	* @brief 获取翻译后的文件全称
	*/
	bool FormatStringToFilePath(QString strFileName, QString strPath, VideoSaveOpt opt, QString & strFilePath);
	QString getFileNameErrorString(ErrorFileNameValid nError);
	void CreateVideoDir(QString strPath, QString strFileContainDir);

	double getRange(VideoSaveOpt &opt);

	virtual bool eventFilter(QObject *, QEvent *) override;
private:
	int mCrtIdx = 0;

	//需要导出的视频项
	//VideoItem m_video_item;
	//VideoItem *m_cur_edit_video_item;
	std::vector<std::pair<int32_t,VideoItem>> m_video_items;

// 	int m_device_stream_type = -1;//进入导出状态之前的设备协议格式
// 	int m_video_stream_type = -1;//需要导出的视频的协议格式

	//是否自动导出
	bool m_enable_auto_export = false;

	//导出结果
	bool m_export_result = false;

	//设备指针
	QWeakPointer<Device> m_device_ptr;

#ifndef Q_OS_WIN32 //麒麟系统
    const int m_widget_height_normal{ 820 };//不带导出进度条时的窗口高度
#else//win10
	const int m_widget_height_normal{ 740 };//不带导出进度条时的窗口高度
#endif
// 	const int m_widget_height_exporting{ 480 + 100 };//带进度条时的窗口高度(正在导出中)

	const int m_max_path_name{ 260 };//最大名称长度
	QValidator * m_video_name_validator;//输入名称限制
	bool m_bPathIsValid{ true }; // 输入名称及路径是否有效
	QString m_strVideName;// 输入框上一次输入值

	const QString conststrIP{ tr(VIDEONAME_IP) };							// IP
	const QString conststrRecordtime{ tr(VIDEONAME_RECORD_TIME) };			// 录制时间
	const QString conststrFramerate{ tr(VIDEONAME_FRAME_RATE) };			// 帧率
	const QString conststrResolutionratio{ tr(VIDEONAME_RESOLUTION_RATIO) };// 分辨率
	const QString conststrTimestamp{ tr(VIDEONAME_TIMESTAMP) };				// 时间戳
	const QString conststrFramenumber{ tr(VIDEONAME_FRAME_NO) };			// 帧编号
	const QString conststrFramesubnumber{ tr(VIDEONAME_FRAME_SUB_NO) };		// 帧子编号
	const QString conststrFormat{ tr(VIDEONAME_FORMAT) };					// 格式
	const QString conststrRecordDate{ tr(VIDEONAME_RECORD_DATE) };			// 录制日期
	const QString conststrDefaultName{ tr("DefaultName") };					// 默认名称
	const QString conststrVideoName{ tr(VIDEONAME_VIDEO_NAME) };			// 视频名称

	const int constPercentValueMin{ 0 };								// 百分比范围下限
	const int constPercentValueMax{ 100 };								// 百分比范围上限
	const int constAngleValueMin{ 0 };									// 角度下限
	const int constAngleValueMax{ 360 };								// 角度上限
	const int constMaxImageSize{ 25 };									// 水印数量最大值
	const int constZoomValueMin{ 1 };									// 百分比范围下限
	const int constZoomValueMax{ 500 };									// 百分比范围上限
	const int constPlayRateMin{ 1 };									// 导出的播放帧率最小值
	const int constPlayRateMax{ 100 };									// 导出的播放帧率最大值

	//自定义水印列表
	QList<TWaterMakerInfo> m_watermark_list;

	//协议格式选项
	QVariantList m_stream_type_list;

	//保存格式选项
	QList<VideoFormat> m_video_format_list;

	//色彩模式选项
	QList<HscDisplayMode> m_color_mode_list;

	//角度数据矫正选项
	QList<int> m_angle_data_correct_list;

	VideoListTableModel *m_pModeVideoList{ nullptr };
	QList<VideoInfoRecordList> m_recordList;    //文件信息容器

	QPointer<QStandardItemModel> m_watermark_table_model_ptr;
	QGraphicsScene *scene{ nullptr };
	QGraphicsItem *m_RectItem{ nullptr };
	QList<TWaterMakerInfo> m_vctWatermarkerInfo;
	QGraphicsItem *m_pSelectedWaterItem{ nullptr };

	QMap<int, QList<TWaterMakerInfo>> m_mapVideoWatermarker;

	HscIntRange m_width_range{};
	HscIntRange m_height_range{};
	bool m_bInitShow{ false };

    Ui::CSDlgDeviceVideoExport *ui;
	std::shared_ptr<ExportCacheFrame>	m_export_cache_frame;

	int m_nVideoNameEditPos{ -1 };	// 记录视频名称的光标位置，方便添加文件见命令插入位置
	//是否正在导出
	bool b_exporting = false;
	int m_nExortIdx = 0; // 导出的索引
	uint8_t m_header_type_ = 1;
	QImage m_thumbnail{};
	QModelIndex m_old_model_index{};
	VideoListThumbnail m_thumbnail_dlg{};

	QList<uint32_t> m_listPlaySpeed;			// 播放速度列表
};

#endif // CSDLGDEVICEVIDEOEXPORT_H

// 全局导出进度回调
void progressCallback(DeviceHandle handle, uint32_t value, uint32_t state, void *user_ptr);
