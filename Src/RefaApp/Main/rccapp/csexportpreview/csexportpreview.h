#ifndef CSEXPORTPREVIEW_H
#define CSEXPORTPREVIEW_H

#include <QDialog>
#include <QButtonGroup>
#include "../cstargetscoring/cstargetscoring.h"
#include "../csdisplaysetting/csdisplaysetting.h"
#include "../render/PlayerViewBase.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoItem/videoitem.h"
#include "Video/VideoUtils/videoutils.h"
#include <QMenu>
#include <QHBoxLayout>
#include <QMap>
#include "../csthumbnailmanage/csthumbnailmanage.h"
#include "../csexportpreviewplayctrl/csexportpreviewplayctrl.h"
#include "../cssecondscreen/cssecondscreen.h"
#include "Playback/CSTargetScoring/cstargerscoring.h"
#include <QKeyEvent>
#include <QPushButton>
#include <QEvent>


#define TRUN_ON_THUMBNAIL_ANIMATION_MODE 1    //开启缩略图动画模式 1-开启 0-关闭

class CSPlaybackController;
class QCheckBox;

namespace Ui {
class CSExportPreview;
}

class FrameButton : public QPushButton
{
	Q_OBJECT
public:
	FrameButton(QWidget *parent = 0):QPushButton(parent) {};
	~FrameButton() {};
private:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
signals:
	void SignalEnterFrameBtn();
	void SignalLeaveFrameBtn();
};

class CSExportPreview : public QDialog
{
    Q_OBJECT 
#define POPUP_TITLE QObject::tr("RCC")
#define NOT_TIP tr("Do not remind again")
#define CAMERA_DISCONNECT tr("Device disconnected,\nplease check this device!")
public:
    explicit CSExportPreview(int32_t index, VideoItem video_item ,QWidget *parent = 0);
    ~CSExportPreview();
private:
	/**************************
	* @brief: 导出预览-视频信息-信息类型
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/30
	***************************/
	enum VideoInfoType
	{
		CAMERA_NAME,
		RECORD_TIME,
		TIME_LENGTH,
		RATE,
		RESOLUTION,
		DEPTH,
		TOTAL_FRAME,
		EXPOSURE_TIME
	};

	/**************************
	* @brief: 设置视频信息
	* @param:type-需设置的信息类型
	* @param:info-信息
	* @return:
	* @author:mpp
	* @date:2022/05/30
	***************************/
	void SetVideoInfo(VideoInfoType type, const QString& info);

	/**************************
	* @brief: 初始化
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/30
	***************************/
	void Init();

	/**************************
	* @brief: 初始化缩略图和播放状态
	* @param:
	* @return:
	* @author:chenyun
	* @date:2022/06/20
	***************************/
	void InitThumnailsAndPlayState();

	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/27
	***************************/
	void InitUI();

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/27
	***************************/
	void ConnectSignalAndSlot();

	/**************************
	* @brief: 设置视频列表
	* @param:strIp ip地址
	* @return:
	* @author:mpp
	* @date:2022/05/30
	***************************/
	void SetVideoList(const QString& strIp);

	/**************************
	* @brief: 更新视频信息
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void UapdateVideoInfo(const VideoItem& item);

	/**************************
	* @brief: 重载视频信息
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void ReloadVideoInfo(const VideoItem& item);

	/**************************
	* @brief: 将播放按钮切换到停止状态
	* @param:bPlay true-播放  false-停止
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SwitchDisplayBtnStatus(const bool bPlay);

	/**************************
	* @brief: 更新播放设置参数
	* @param:deviceIp 设备ip
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void UpadateDisplaySetting(const QString& deviceIp);

	/**************************
	* @brief: 更新报靶设置参数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void UpdateTargetSetting();

	/**************************
	* @brief: 设置key_frame_value_label值
	* @param:keyFrameNo 关键帧帧号
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SetKeyFrameLabelValue(const int64_t keyFrameNo);

	/**************************
	* @brief: 启用-禁用切换视频下拉框
	* @param:bForbid true-禁用 false-启用
	* @return:
	* @author:mpp
	* @date:2022/06/15
	***************************/
	void ForbidSwitchVideoCombobox(const bool bForbid);

	/**************************
	* @brief: 启用-禁用播放窗口控件
	* @param:bForbid true-禁用 false-启用
	* @return:
	* @author:mpp
	* @date:2022/06/15
	***************************/
	void ForbidDisplayWindowControls(const bool bForbid);

	/**************************
	* @brief: 更新播放控制状态
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/16
	***************************/
	void UpadateDisplayCtrlStatus(bool bUsed);

	/**************************
	* @brief: 更新sdi播放范围
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/21
	***************************/
	void UpadateSdiRange(const int64_t& iStartFrameNo, const int64_t& iEndFrameNo);

	void UpadateRange(const int64_t & iStartFrameNo, const int64_t & iEndFrameNo);

	virtual void closeEvent(QCloseEvent *e);
	virtual void keyPressEvent(QKeyEvent *ev);

	bool event(QEvent *);
	/**************************
	* @brief: 解析时间信息
	* @param:item 视频信息
	* @return:  格式化后的时间信息
	* @author:mpp
	* @date:2022/06/27
	***************************/
	QString paraseShootTimeInfo(const VideoItem& item);

	/** @brief 尺寸变化事件
	@param [in] : QResizeEvent *event : 事件对象
	@return
	@note
	*/
	virtual void resizeEvent(QResizeEvent *event) override;

	/**
	**@	Brife	获取视频是否支持导出
	*/
	bool GetEnableExport(const QString &str);

	/**
	**@	Brife	使能速度切换按钮
	*/
	void enableSpeedCtrlBtns(bool benable);

	/**
	**@	Brife	设置播放区间
	*/
	void SetEditRange();

	/**************************
	* @brief: 缩略图加载结束
	***************************/
	void thumbnailLoadingFinished(bool ok);

	/**************************
	* @brief: 设置播放速度
	***************************/
	void setPalySpeed(int nSpeed);

	private slots:

	void SlotSpeedAllClicked();

	/**************************
	* @brief: 跳帧编辑槽函数
	* @param:
	* @return:
	***************************/
	void on_spinBox_skip_frame_value_editingFinished();

	/**************************
	* @brief: 倍速 单选操作组槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/27
	***************************/
	void SlotSpeedBtnClicked(int index);

	/**************************
	* @brief: 鼠标右键设置关键帧槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SlotKeyFrame(const int64_t frameNo);

	/**************************
	* @brief: 鼠标右键设置起始帧槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SlotBeginFrame(const int64_t frameNo);

	/**************************
	* @brief: 鼠标右键设置结束帧槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SlotEndFrame(const int64_t frameNo);

	/**************************
	* @brief: 更新图像槽函数
	* @param:image 图像信息
	* @return:
	* @author:mpp
	* @date:2022/05/31
	***************************/
	void SlotImageUpdate(const RccFrameInfo& image);

	/**************************
	* @brief: 图像显示到大屏槽函数
	* @param:index 帧索引
	* @return:
	* @author:mpp
	* @date:2022/06/01
	***************************/
	void SlotShowImgInBigScreen(const int index);

	/**************************
	* @brief: 更新“起始-结束”帧范围槽函数
	* @param:iStart-起始帧号 iEnd-结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/02
	***************************/
	void SlotUapdateFrameRange(const qint64 iStart, const qint64 iEnd);

	/**************************
	* @brief: 关键帧显示到大屏的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SlotKeyFrameToBigScreen();

	/**************************
	* @brief: 播放器寻帧槽函数
	* @param:frameIndex : 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotSeekFrame(uint64_t curFrameIndex);

	/**************************
	* @brief: 滑动条设置起始帧槽函数
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotSliderBeginFrame(const int64_t frameNo);

	/**************************
	* @brief: 滑动条设置结束帧槽函数
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotSliderEndFrame(const  int64_t frameNo);

	/**************************
	* @brief: 鼠标在滑动条上移动，发送鼠标当前位置对应的帧号的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotMouseMoveOnSlider(uint64_t frameNo);

	/**************************
	* @brief: 鼠标移出滑动条槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotMouseMoveOutSlider();

	/**************************
	* @brief: 播放控制槽函数
	* @param:bPlay true-播放  false-暂停
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotDisplayControl(bool bPlay);

	/**************************
	* @brief: 反色复选框状态变化槽函数
	* @param:bCheced true-被勾选 false-未被勾选
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotRevertCheckBoxStatusChanged(bool bChecked);

	/**************************
	* @brief: 亮度值变化槽函数
	* @param: value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotLuminanceChanged(const int value);

	/**************************
	* @brief: 对比度值变化槽函数
	* @param: value 对比度值
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotContrastChanged(const int value);

	/**************************
	* @brief: sdi向前一帧
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotSdiForwardStep();

	/**************************
	* @brief: sdi向后一帧
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotSdiBackwardStep();

	/**************************
	* @brief: sdi播放暂停按钮信号
	* @param:bPlay true-播放  false-暂停
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotSdiDisplayControl(bool bPlay);

	/**************************
	* @brief: sdi开启关闭控制
	* @param:bOpened-true 开启 false-关闭
	* @param:dSpeed 倍速
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotSdiSwitchControl(bool bOpened, const double dSpeed);

	/**************************
	* @brief: 分辨率&播放帧率列表选中索引槽函数
	* @param:index-索引
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SlotSdiFpsResolsListIndex(const int index);

	/**************************
	* @brief: 播放倍速值槽函数
	* @param: value 倍速值
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SlotSdiSpeedValue(const double value);

	/**************************
	* @brief: 开启投屏槽函数
	* @param: bOpen true-开启 false-关闭
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SlotSdiSecondScreen(bool bOpen);

	// Add by Juwc 2022/9/27
	/************************************************************
	* @brief:	响应投屏窗口关闭信号
	* @param:	无
	* @return:	无
	************************************************************/
	void OnSecondScreenClose();

	/**************************
	* @brief: 测量网格显隐控制槽函数
	* @param: bShow true-显示 false-隐藏
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotMeasuringGridVisible(const bool bShow);

	/**************************
	* @brief: 目标位置手动选择槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotPositionSelect();

	/**************************
	* @brief: 报靶手动选择槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotTargetSelect();

	/**************************
	* @brief: 鼠标按下点的坐标信号
	* @param:point 鼠标按下后的点的坐标
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotMousePressPointF(const QPointF& point);

	/**************************
	* @brief: 叠加图像显示槽函数
	* @param:bShow true-显示叠加图像  false-不显示叠加图像
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotOverlaImageVisible(const bool bShow);

	/**************************
	* @brief: 关闭手动选择
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotCloseManualSelect();

	/**************************
	* @brief: 网格间距变化槽函数
	* @param:interval -变化后的间距值
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotGridIntervalChanged(const int interval);

	/**************************
	* @brief: 报靶结果有变动槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/10
	***************************/
	void SlotTargetScoringInfoChanged();

	/**************************
	* @brief: 移出缩略图区域槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/20
	***************************/
	void SlotMoveOutThumbnailArea();

	/**************************
	* @brief: 单张缩略图点击槽函数
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/28
	***************************/
	void SlotSingleThumbnailClicked(const uint64_t& frameNo);

	/**************************
	* @brief: 切换帧和ms槽函数
	* @param:bFrame true-帧 false-ms
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SlotSwitchFrameAndMs(const bool bFrame);

	/**************************
	* @brief: 切换视频弹框，不再提示复选框槽函数
	* @param:bChecked 是否选中
	* @return:
	* @author:mpp
	* @date:2022/06/15
	***************************/
	void SlotPopupSwitchCheckBox(bool bChecked);

	/**************************
	* @brief: sdi倍速判断弹框，不再提示复选框槽函数
	* @param:bChecked 是否选中
	* @return:
	* @author:mpp
	* @date:2022/06/15
	***************************/
	void SlotPopupSdiCheckBox(bool bChecked);

	/**************************
	* @brief: 设备错误发生槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/16
	***************************/
	void SlotErrorOccurred(quint64 error);

	/**************************
	* @brief: 左右按钮是否启用槽函数
	* @param:type 1-左按钮  2-右按钮
	* @param:enable true-启用 false-禁用
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SlotFrameStepStatus(int type, bool enable);

	/**************************
	* @brief: 缩略图区间，帧号范围
	* @param:iStart 起始帧号
	* @param:iEnd 结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SlotThumbnailRange(const uint64_t& iStart, const uint64_t& iEnd);

	/**************************
	* @brief: 缩略图加载开始
	* @param:
	* @return:
	***************************/
	void SlotThumbnailLoadingStarted();

	/**************************
	* @brief: 缩略图加载结束
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SlotThumbnailLoadingFinished(bool ok);
	
	/**************************
	* @brief: 编辑播放范围
	* @param:
	* @return:
	* @author:
	* @date:2022/10/17
	***************************/
	void on_doubleSpinBox_begin_editingFinished();

	/**************************
	* @brief: 编辑播放范围
	* @param:
	* @return:
	* @author:
	* @date:2022/10/17
	***************************/
	void on_doubleSpinBox_end_editingFinished();

    void on_reset_pushButton_clicked();

    void on_lastTime_pushButton_clicked();

    void on_export_pushButton_clicked();

    void on_video_swtch_comboBox_currentIndexChanged(const QString &arg1);
    void on_forward_frame_btn_clicked();
	void on_comboBox_display_bpp_currentIndexChanged(int index);


    void on_backward_frame_btn_clicked();
	void slotSnapBtnPressed();

	/**
	* @brief 处理记录范围变化后的堆栈数量
	* @param nSize 数量
	*/
	void slotRangeStackChange(int nSize);

signals:
	void signalExportpreviewShowMain(bool bshow);

private:
	static bool compareList(const VideoItem &v1, const VideoItem &v2);
	QString getCameraName(QString strIp);

	void UpdateSkipFrameInfo();
private:
//	QButtonGroup* m_SpeedGroup;   //单选操作组，倍速
	int m_iPlaySpeed;    //播放速度
	CSTargetScoring* m_targetScoring{ nullptr };
	CSDisplaySetting* m_displaySetting{ nullptr };
	CPlayerViewBase* m_player{ nullptr };    //播放器
	QStringList m_listVideoList{};    //视频列表
	VideoItem m_videoItem;
	QString m_strIp;
	QMenu * m_rightMenu{ nullptr };
	CSPlaybackController* m_playerController{ nullptr };
	QHBoxLayout* m_hThumbnailManageLayout{ nullptr };
	CSThumbnailManage *m_pThumbnailManage{ nullptr };
    CSExportPreviewPlayCtrl*  m_pExportPreviewPlayCtr{ nullptr };
	QSharedPointer<Device> m_devicePtr{nullptr};
	CSSecondScreen* m_pSecondScreen{nullptr};
	CSCtrl::CSTargerScoring* m_targetScoringControl{ nullptr };
	QImage m_currentImg{};   //当前图片
	RccFrameInfo m_currentImgInfo{};
	bool m_bPosManualSelectClicked{ false };    //点击-目标位置-手动选择按钮
	bool m_bTargetManualSelectClicked{ false };    //点击-报靶-手动选择按钮
	bool m_bFrame{ true };    //帧-ms true-帧 false-ms
	int64_t m_fps{ 1 };    //帧率
	bool m_bInit{false};     //国产化软件未退出，是否第一次进入回放界面
	QMap<QString, int>m_mapVideoList{};
	QCheckBox* m_popupSwitchCheckBox{ nullptr };    //切换视频弹框
	QCheckBox* m_popupSdiCheckBox{ nullptr };    //sdi倍速判断弹框
	bool m_bSwictchBoxChecked{ false };    //切换视频弹框 复选框是否被选中
	bool m_bSwitchBoxForbid{ false };    //切换视频弹框是否再显示
	bool m_bSdiBoxChecked{ false };    //开启sdi 倍速判断弹框 复选框是否被选中
	bool m_bSdiboxForbid{ false };    //开启sdi 倍速判断弹框是否再显示
	bool m_bOpenClosePopup{ false };    //关闭界面时，是否弹框
	bool m_bGridVisible{ false };    //是否显示测量网格
	QString m_strSwitchBoxDefaultText{};    //切换视频框默认显示文本
	int m_nSwitchBoxOldVid{ -1 };    //切换视频框的VID
	bool m_bKeyboardUsed{ false };    //键盘事件是否启用
	bool m_bOldPlayState{ false };    //鼠标移入缩略图时的播放状态 true-播放 false-暂停
	QString m_strSwitchBoxOldText{};    //下拉框切换之前的值
	bool m_bForbidSwitchBox{ false };    //是否禁用切换视频下拉框

	FrameButton* m_leftFrameBtn{ nullptr };    //向左移动图像
	FrameButton* m_rightFrameBtn{ nullptr };    //向右移动图像
	bool m_bVideoStated{ true };    //视频状态，缩略图更新结束后，是否播放标识

	uint64_t m_uiOldStart{ 0 };		// 记录设置的起始帧
	uint64_t m_uiOldEnd{ 0 };		// 记录设置的终止帧
	uint64_t m_uiFrameTotal{ 0 };				// 记录总帧

	int32_t	 m_current_idx = -1;
	std::thread  m_InitThumnailsAndPlayStateThrd;
	double m_begin_old_value{ 0 };
	double m_end_old_value{ 0 };
	bool m_bEnableLastButton{ false };		// 记录回到上次的使能状态
private:
    Ui::CSExportPreview *ui;
};

#endif // CSEXPORTPREVIEW_H
