#ifndef PLAYBACKPLAYERCTRLTHREAD_H
#define PLAYBACKPLAYERCTRLTHREAD_H

#include <RMAGlobalDefine.h>
#include <QThread>
#include <memory>
#include <QMutex>

/***********前置声明区**********/
class PlayerControllerInterface;
/*******************************/

//播放操作类型
enum PlayOperationType
{
    PLAY_OPT_PAUSE = 0,         //暂停
	PLAY_OPT_STOP,				//停止播放
    PLAY_OPT_FORWARD_PALY,      //正向播放
    PLAY_OPT_BACKWARD_PLAY,     //反向播放
    PLAY_OPT_FAST_FORWARD,      //正向倍速播放
    PLAY_OPT_FAST_BACKWARD,     //反向倍速播放
    PLAY_OPT_FORWARD,           //前进1帧
    PLAY_OPT_BACKWARD,          //后退1帧
    PLAY_OPT_SET_PLAYER_RANGE,  //设置播放范围
	PLAY_OPT_SET_PLAYER_BEGIN,  //设置播放起点
	PLAY_OPT_SET_PLAYER_END,	//设置播放终点
    PLAY_OPT_SET_CURRENT_FRAME, //设置当前帧
};

/**
*@brief 回放播放器控制线程
*@note 录制回放时播放器操作可能耗时，将耗时操作放在线程中处理，防止界面出现假死
       为统一播放器逻辑，录制回放和本地回放都将播放操作放在线程中
	   涉及的帧号都是绝对帧号
**/
class PlaybackPlayerCtrlThread : public QThread
{
    Q_OBJECT

public:
    PlaybackPlayerCtrlThread(QObject *parent = Q_NULLPTR);
    PlaybackPlayerCtrlThread(std::shared_ptr<PlayerControllerInterface> pctrl, QObject *parent = Q_NULLPTR);

	/**
	*@brief 重置播放控制器
	*@param [in] : pctrl : std::shared_ptr<PlayerControllerInterface>，播放控制器指针
	**/
    void resetPlayerController(std::shared_ptr<PlayerControllerInterface> pctrl);

	/**
	*@brief 设置播放操作类型
	*@param [in] : opt : PlayOperationType，播放操作类型
	**/
    void setPlayOperation(PlayOperationType opt);

	/**
	*@brief 获取播放操作类型
	*@return : PlayOperationType : 播放操作类型
	**/
	PlayOperationType getPlayOperation() const;

	/**
	*@brief 设置播放范围
	*@param [in] : begin : FRAME_INDEX，起始帧号
				   end : FRAME_INDEX，结束帧号
	**/
    void setPlayerRange(FRAME_INDEX begin, FRAME_INDEX end);

	/**
	*@brief 设置播放起始帧号
	*@param [in] : begin : FRAME_INDEX，起始帧号
	**/
	void setPlayerBegin(FRAME_INDEX begin);

	/**
	*@brief 设置播放结束帧号
	*@param [in] : end : FRAME_INDEX，结束帧号
	**/
	void setPlayerEnd(FRAME_INDEX end);

	/**
	*@brief 设置当前播放帧号
	*@param [in] : index : FRAME_INDEX，当前帧号
	**/
    void setCurrentFrameNo(FRAME_INDEX index);

protected:
	/**
	*@brief 线程入口
	**/
    void run() override;

private:
    std::shared_ptr<PlayerControllerInterface> pplayer_ctrl_;
	PlayOperationType current_opt_{ PLAY_OPT_PAUSE };
    FRAME_INDEX bgein_frameno_{0}, end_frameno_{0}, current_frameno_{0};
    mutable QMutex mutex_;

    static const int kPlaySpeed_ = 16;
};

#endif // PLAYBACKPLAYERCTRLTHREAD_H
