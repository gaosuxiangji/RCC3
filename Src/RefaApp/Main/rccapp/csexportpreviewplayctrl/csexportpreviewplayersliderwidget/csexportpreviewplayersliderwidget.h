#ifndef CSEXPORTPREVIEWPLAYERSLIDERWIDGET_H
#define CSEXPORTPREVIEWPLAYERSLIDERWIDGET_H

#include <QWidget>
#include <QScopedPointer>

/*
播放器进度条窗口类。使用示例如下：
CSExportPreviewPlayerSliderWidget sliderWidget;
uint64_t totalFrameCnt=1000;
sliderWidget.SetTotalFrameCnt(totalFrameCnt);
uint64_t trigger=50;
sliderWidget.SetTriggerFrame(trigger);
......
*/

#define PLAY_RANGE_TOP_LINE 5

namespace Ui {
class CSExportPreviewPlayerSliderWidget;
}

class CSExportPreviewPlayerSliderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CSExportPreviewPlayerSliderWidget(QWidget *parent = 0);
    ~CSExportPreviewPlayerSliderWidget();

	/** @brief 设置总帧数
	@param [in] : uint64_t total : 总帧数
	@return
	@note : 该函数会重置与帧有关的计数值，如帧数范围等
	*/
	void SetTotalFrameCnt(uint64_t total, bool bForceFlush = false);

	/**************************
	* @brief: 设置帧率
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SetFrameRate(const qint64& fps);

	/** @brief 获取播放器范围
	@param [out] : uint64_t& begin : 起始帧
	       [out] : uint64_t& end : 结束帧
	@return
	@note
	*/
	void GetPlayerRange(uint64_t& begin, uint64_t& end);

	/** @brief 设置播放器范围
	@param [in] : uint64_t begin : 起始帧
	       [in] : uint64_t end : 结束帧
	@return : bool : 设置是否成功
	@note : 当当前帧位置不在设定的起始帧和结束帧范围内时，会强制修改当前帧位置到起始帧
	*/
	bool SetPlayerRange(uint64_t begin, uint64_t end);

	/** @brief 设置关键帧
	@param [in] : uint64_t trigger : 关键帧
	@return
	@note 
	*/
	void SetTriggerFrame(uint64_t trigger);

	/** @brief 清除关键帧
	@param
	@return
	@note
	*/
	void ClearTriggerFrame();

	/** @brief 是否使能该界面
	@param [in] : bool enable : 是否使能
	@return
	@note
	*/
	void SetEnabled(bool enable);

	/** @brief 设置播放器“忙”状态标志
	@param [in] : bool busy : 是否处于“忙”状态
	@return
	@note : 当播放器处于“忙”状态时，点击播放控制界面的控件无反应
	*/
	void SetPlayerBusyFlag(bool busy);

	/** @brief 设置当前播放器是否处于暂停状态
	@param [in] : bool pause : 是否处于暂停状态
	@return
	@note
	*/
	void SetPlayerPausedFlag(bool pause);

	/** @brief 设置当前帧号
	@param [in] : uint64_t frameIndex : 当前帧号
	@return
	@note
	*/
	void SetCurFrameIndex(uint64_t frameIndex);

	/**************************
	* @brief: 切换帧和ms
	* @param:bFrame true-帧 false-ms
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SwitchFrameAndMs(const bool bFrame);

	/**************************
	* @brief: 设置缩略图区间帧号范围
	* @param:iStart 起始帧号
	* @param:iEnd 结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SetThumbnailRange(const uint64_t& iStart, const uint64_t& iEnd);
protected:
    /** @brief 重载绘制事件
        @param [in] : QPaintEvent *event : 事件对象
        @return
        @note
        */
    virtual void paintEvent(QPaintEvent *event) override;

	/** @brief 尺寸变化事件
	@param [in] : QResizeEvent *event : 事件对象
	@return
	@note
	*/
	virtual void resizeEvent(QResizeEvent *event) override;

	/** @brief 鼠标按下事件
	@param [in] : QMouseEvent *event : 事件对象
	@return
	@note
	*/
	virtual void mousePressEvent(QMouseEvent *event) override;

	/** @brief 鼠标释放事件
	@param [in] : QMouseEvent *event : 事件对象
	@return
	@note
	*/
	virtual void mouseReleaseEvent(QMouseEvent *event) override;

	/** @brief 鼠标移动事件
	@param [in] : QMouseEvent *event : 事件对象
	@return
	@note
	*/
	virtual void mouseMoveEvent(QMouseEvent *event) override;

private slots:
	/// \brief 处理鼠标移动事件
	void slotHandleMouseMoveEvent();

private:
	//命中的图标类型
	enum HitIconType
	{
		HIT_NONE,                        //未命中图标
		HIT_ICON_PLAYER_RANGE_LEFT,      //左播放范围图标
		HIT_ICON_PLAYER_RANGE_RIGHT,     //右播放范围图标
		HIT_ICON_PLAYER_POS,             //播放位置图标
		HIT_ICON_TRIGGER_FRAME,          //关键帧图标
	};

	/** @brief 判断鼠标命中的图标类型
	@param [in] : const QPoint& mousePos : 鼠标位置
	@return : HitIconType : 命中图标的类型
	@note
	*/
	HitIconType HitIconDetect(const QPoint& mousePos) const;

    /**
     * @brief 刷新各个按钮热区
     */
    void refreshTouchRect();

	/** @brief 设置图像的背景色
	@param [in] : const QPixmap& src : 输入图像
	       [out] : QImage& dst : 输出图像
	       [in] : const QColor& oldBackground : 原先的背景色
		   [in] : const QColor& newBackgroundColor : 新的背景色
	@return
	@note
	*/
	void SetImageBackgroundColor(const QPixmap& src, QImage& dst, const QColor& oldBackground, const QColor& newBackgroundColor) const;

	/// \brief 制作绘图缓冲区
	/// \param [in] needs_repaint 是否重绘
	void MakePaintBuffer(bool needs_repaint = true);

	/// \brief 处理鼠标移动事件
	/// \param [in] mouse_event 鼠标事件
	void handleMouseMoveEvent(QMouseEvent* mouse_event);

Q_SIGNALS:
	/** @brief 播放起始范围发生变化
	@param [in] : uint64_t begin : 播放范围起始帧
	@return 
	@note
	*/
	void sigPlayerBeginRangeChanged(uint64_t begin);

	/** @brief 播放结束范围发生变化
	@param [in] : uint64_t end : 播放范围结束帧
	@return
	@note
	*/
	void sigPlayerEndRangeChanged(uint64_t end);

	/** @brief 播放器寻帧
	@param [in] : uint64_t frameIndex : 帧号
	@return
	@note : 用户拖动播放位置图标时触发
	*/
	void sigSeekFrame(uint64_t frameIndex);

	/** @brief 到达最小允许播放帧
	@param [in] : bool reach : 是否到达
	@return 
	@note
	*/
	void sigReachMinValidFrameIndex(bool reach);

	/** @brief 到达最大允许播放帧
	@param  [in] : bool reach : 是否到达
	@return 
	@note
	*/
	void sigReachMaxValidFrameIndex(bool reach);

	/**************************
	* @brief: 关键帧信号点击信号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SignalKeyFrameToBigScreen();

	/**************************
	* @brief: 鼠标在滑动条上移动，发送鼠标当前位置对应的帧号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SignalMouseMoveOnSlider(uint64_t frameNo);

	/**************************
	* @brief: 鼠标移出滑动条
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SignalMouseMoveOutSlider();

	/**************************
	* @brief: 点击进度条后跳转的信号
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/07/05
	***************************/
	void SignalSliderMouseClicked(const uint64_t& frameNo);
private:
    Ui::CSExportPreviewPlayerSliderWidget *ui;//界面对象

    QPixmap slider_left_img;//播放范围左边界图标
    QPixmap slider_right_img;//播放范围右边界图标
    QPixmap slider_pos_img;//播放位置图标
	QPixmap trigger_frame_img;//关键帧图标

	QImage slider_left_img_disabled;//置灰的播放范围左边界图标
	QImage slider_right_img_disabled;//置灰的播放范围右边界图标
	QImage slider_pos_img_disabled;//置灰的播放位置图标
	QImage trigger_frame_img_disabled;//置灰的关键帧图标

	QImage slider_left_img_pressed;//按下的播放范围左边界图标
	QImage slider_right_img_pressed;//按下的播放范围右边界图标
	QImage slider_pos_img_pressed;//按下的播放位置图标
	QImage trigger_frame_img_pressed;//按下的关键帧图标

	int playerRangeIconWidth = 18;//播放范围图标的宽度

    const float playerRangeTextStretch=0.2f;//播放范围信息所占的比例
    const float playerRangeIconStretch=0.3f;//播放范围图标所占的比例
    const float playerSliderStretch=0.1f;//播放进度条所占的比例
    const float playerPosIconStretch=0.2f;//播放位置图标所占的比例
    const float playerTriggerFrameIconStretch=0.2f;//关键帧图标所占的比例

	uint64_t totalFrameCnt = 0;//总帧数
	uint64_t validPlayerRangeBeginFrameIndex = 0;//播放范围的起点
	uint64_t validPlayerRangeEndFrameIndex = 0;//播放范围的终点
	uint64_t validTotalFrameCnt = 0;//有效的总帧数
	uint64_t curFrameIndex = 0;//当前播放进度

	QRectF slider_left_img_rect;//左播放范围图标所在区域
	QRectF slider_right_img_rect;//右播放范围图标所在区域
	QRectF playerPosIconRect;//播放位置图标所在区域
	QRectF triggerFrameRect;//关键帧图标所在的区域

	QRectF touch_slider_left_img_rect;//左播放范围图标按钮热区
	QRectF touch_slider_right_img_rect;//右播放范围图标热区
	QRectF touchPlayerPosIconRect;//播放位置图标按钮热区
	QRectF touchTriggerFrameRect;//关键帧图标按钮热区

	HitIconType curHitIconType = HIT_NONE;//当前命中的图标类型

	bool enabled = false;//界面是否使能

	const QColor iconBackgroundColor= QColor(255, 255, 255);//图标的背景色
	const QColor iconPressedBackgroundColor = QColor(15, 130, 230);//图标按下的背景色

	uint64_t triggerFrameIndex = 0;//关键帧
	bool isTriggerFrameExists = false;//关键帧是否存在

	bool isPlayerPaused = true;//播放器是否处于暂停状态

	//以下变量为较少界面闪烁所增加
	bool isFirstPaint = true;//是否是初次绘制播放控制控件
	QPixmap paintBuffer;//绘图缓冲区
	QPixmap paintBuffer_backup;//绘图缓冲区的备份

	bool isBusy = false;//界面是否处于“忙”状态，处于“忙”状态时，点击界面上的控件无反应
	QRectF m_sliderRect{};    //进度条区域
	bool m_bFrame{ true };    //帧-ms true-帧 false-ms
	qint64 m_iFrameRate{ 1 };    //帧率

	bool m_bPressed{ false };    //鼠标是否按下 true-按下 false-释放

	uint64_t m_thumbnailRangeBeginFrameIndex = 0;//播放范围的起点
	uint64_t m_thumbnailRangeEndFrameIndex = 0;//播放范围的终点

	const int kMouseMoveHandlePeriod = 50; // 鼠标移动事件处理周期，单位：ms
	QScopedPointer<QMouseEvent> mouse_move_event_; // 鼠标移动事件

	bool m_bSwtiching{ false };//是否正在修改范围
	bool m_key_tip_visible{ false };
};

#endif // CSEXPORTPREVIEWPLAYERSLIDERWIDGET_H
