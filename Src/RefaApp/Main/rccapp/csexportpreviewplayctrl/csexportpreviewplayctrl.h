#ifndef CSEXPORTPREVIEWPLAYCTRL_H
#define CSEXPORTPREVIEWPLAYCTRL_H

#include <QWidget>
#include <QHBoxLayout>
#include "csexportpreviewplayersliderwidget/csexportpreviewplayersliderwidget.h"
#include "Video/VideoUtils/videoutils.h"
namespace Ui {
class CSExportPreviewPlayCtrl;
}

class CSExportPreviewPlayCtrl : public QWidget
{
    Q_OBJECT

public:
    explicit CSExportPreviewPlayCtrl(QWidget *parent = 0);
    ~CSExportPreviewPlayCtrl();
public:
	/**************************
	* @brief: 设置总帧数
	* @param:uint64_t total : 总帧数
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SetTotalFrameCnt(uint64_t total, bool bForceFlush = false);

	/**************************
	* @brief: 设置帧率
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SetFrameRate(const qint64& fps);

	/**************************
	* @brief: 设置播放器范围
 	* @param:begin : 起始帧
	* @param:end : 结束帧
	* @return:设置是否成功
	* @note : 当当前帧位置不在设定的起始帧和结束帧范围内时，会强制修改当前帧位置到起始帧
	* @author:mpp
	* @date:2022/06/07
	***************************/
	bool SetPlayerRange(uint64_t begin, uint64_t end);

	/**************************
	* @brief: 设置播放器范围
	* @param:uint64_t begin : 起始帧
	* @param:uint64_t end : 结束帧
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void GetPlayerRange(uint64_t& begin, uint64_t& end);

	/**************************
	* @brief: 设置关键帧
	* @param:uint64_t trigger : 关键帧
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SetTriggerFrame(uint64_t trigger);

	/**************************
	* @brief: 设置当前播放器是否处于暂停状态
	* @param:bool pause : 是否处于暂停状态
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SetPlayerPausedFlag(bool pause);

	/**************************
	* @brief: 设置当前帧号
	* @param:uint64_t frameIndex : 当前帧号
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SetCurFrameIndex(uint64_t frameIndex);

	/**************************
	* @brief: 是否使能该界面
	* @param:enable : 是否使能
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SetEnabled(bool enable);

	/**************************
	* @brief: 设置播放暂停按钮状态
	* @param:bPlay true-播放 false-暂停
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetPlayPauseStatus(bool bPlay);

	/**************************
	* @brief: 设置缩略图区间帧号范围
	* @param:iStart 起始帧号
	* @param:iEnd 结束帧号
	* @return:
	* @author:mpp
	* @date:2022/06/30
	***************************/
	void SetThumbnailRange(const uint64_t& iStart, const uint64_t& iEnd);

	void resetSwitchFrameAndMs();
signals:
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

	/**************************
	* @brief: 关键帧信号点击信号
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void SignalKeyFrameToBigScreen();

	/**************************
	* @brief: 播放器寻帧
	* @param:frameIndex : 帧号
	* @return:
	* @note:用户拖动播放位置图标时触发
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void sigSeekFrame(uint64_t frameIndex);

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
	* @brief: 播放控制信号
	* @param:bPlay true-播放  false-暂停
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalDisplayControl(bool bPlay);

	/**************************
	* @brief: 切换帧和ms信号
	* @param:bFrame true-帧 false-ms
	* @return:
	* @author:mpp
	* @date:2022/06/14
	***************************/
	void SignalSwitchFrameAndMs(const bool bFrame);

	/**************************
	* @brief: 点击进度条后跳转的信号
	* @param:frameNo 帧号
	* @return:
	* @author:mpp
	* @date:2022/07/05
	***************************/
	void SignalSliderMouseClicked(const uint64_t& frameNo);
private slots:
	/**************************
	* @brief: 播放器寻帧槽函数
	* @param:frameIndex : 帧号
	* @return:
	* @author:mpp
	* @date:2022/06/07
	***************************/
	void SlotSeekFrame(uint64_t curFrameIndex);
private slots:
    void on_play_pause_btn_clicked(bool checked);

    void on_current_frame_lineEdit_editingFinished();

    void on_frame_switch_comboBox_currentIndexChanged(int index);

private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void InitUI();

	/**************************
	* @brief: 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/06
	***************************/
	void ConnectSignalAndSlot();
private:
	CSExportPreviewPlayerSliderWidget* m_pExportPreviewPlayerSliderWidget;
	qint64 m_iFrameRate{ 1 };    //帧率
	bool m_bFrame{ true };    //帧-ms true-帧 false-ms
	uint64_t m_total{ 0 };
	uint64_t m_currentIndex{ 0 };
private:
    Ui::CSExportPreviewPlayCtrl *ui;
};

#endif // CSEXPORTPREVIEWPLAYCTRL_H
