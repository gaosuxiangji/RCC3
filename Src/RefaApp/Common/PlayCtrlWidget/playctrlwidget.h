#ifndef PLAYCTRLWIDGET_H
#define PLAYCTRLWIDGET_H

#include <QWidget>

class QToolButton;

namespace Ui {
class PlayCtrlWidget;
}

/**
 * @brief 播放控制类
 */
class PlayCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayCtrlWidget(QWidget *parent = 0);
    ~PlayCtrlWidget();

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

	/** @brief 设置播放器范围
	@param [in] : uint64_t begin : 起始帧
	[in] : uint64_t end : 结束帧
	@return : bool : 设置是否成功
	@note : 当当前帧位置不在设定的起始帧和结束帧范围内时，会强制修改当前帧位置到起始帧
	*/
	bool SetPlayerRange(uint64_t begin, uint64_t end);

	/** @brief 设置总帧数
	@param [in] : uint64_t total : 总帧数
	@return
	@note : 该函数会重置与帧有关的计数值，如帧数范围等
	*/
	void SetTotalFrameCnt(uint64_t total);

	/** @brief 设置触发帧
	@param [in] : uint64_t trigger : 触发帧
	@return
	@note
	*/
	void SetTriggerFrame(uint64_t trigger);

	/** @brief 清除触发帧
	@param
	@return
	@note
	*/
	void ClearTriggerFrame();

	/**
	*@brief 暂停播放,由外部控制
	*@param 
	*@return
	**/
	void setPause();

public Q_SLOTS:
	/** @brief 设置当前帧号
	@param [in] : uint64_t frameIndex : 当前帧号
	@return
	@note
	*/
	void SetCurFrameIndex(uint64_t frameIndex);

private:
	/** @brief 绑定消息
	@param 
	@return
	@note
	*/
	void InitBinding();

	/** @brief 获取播放器范围
	@param [out] : uint64_t& begin : 起始帧
	[out] : uint64_t& end : 结束帧
	@return
	@note
	*/
	void GetPlayerRange(uint64_t& begin, uint64_t& end);

private Q_SLOTS:
	/** @brief 点击反向播放按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonBackwardPlay_clicked(bool checked = false);

	/** @brief 点击暂停播放按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonPause_clicked(bool checked = false);

	/** @brief 点击正向播放按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonForwardPlay_clicked(bool checked = false);

	/** @brief 点击快退播放按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonFastBackwardPlay_clicked(bool checked = false);

	/** @brief 点击后退一帧按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonBackward_clicked(bool checked = false);

	/** @brief 点击前进一帧按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonForward_clicked(bool checked = false);

	/** @brief 点击快进播放按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonFastForwardPlay_clicked(bool checked = false);

	/** @brief 播放起始范围发生变化
	@param [in] : uint64_t begin : 播放范围起始帧
	@return
	@note
	*/
	void slotPlayerBeginRangeChanged(uint64_t begin);

	/** @brief 播放结束范围发生变化
	@param [in] : uint64_t end : 播放范围结束帧
	@return
	@note
	*/
	void slotPlayerEndRangeChanged(uint64_t end);

	/** @brief 播放器寻帧
	@param [in] : uint64_t frameIndex : 帧号
	@return
	@note : 用户拖动播放位置图标时触发
	*/
	void slotSeekFrame(uint64_t frameIndex);


	/** @brief 到达最小允许播放帧
	@param [in] : bool reach : 是否到达
	@return
	@note
	*/
	void slotReachMinValidFrameIndex(bool reach);

	/** @brief 到达最大允许播放帧
	@param [in] : bool reach : 是否到达
	@return
	@note
	*/
	void slotReachMaxValidFrameIndex(bool reach);

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

	/** @brief 反向播放
	@param 
	@return 
	@note
	*/
	void sigBackwardPlay();

	/** @brief 暂停
	@param 
	@return 
	@note
	*/
	void sigPause();

	/** @brief 正向播放
	@param 
	@return 
	@note
	*/
	void sigForwardPlay();

	/** @brief 快退播放
	@param 
	@return 
	@note
	*/
	void sigFastBackwardPlay();

	/** @brief 后退一帧
	@param 
	@return 
	@note
	*/
	void sigBackward();

	/** @brief 前进一帧
	@param 
	@return 
	@note
	*/
	void sigForward();

	/** @brief 快进播放
	@param 
	@return 
	@note
	*/
	void sigFastForwardPlay();

private:
    Ui::PlayCtrlWidget *ui;

	bool isPlayerPaused = true;//播放器是否处于暂停状态

	bool isBusy = false;//界面是否处于“忙”状态，处于“忙”状态时，点击界面上的控件无反应
};

#endif // PLAYCTRLWIDGET_H
