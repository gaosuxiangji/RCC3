/***************************************************************************************************
** @file: 菜单栏_采集_已导出视频_播放器界面
** @author: mpp
** @date: 2022/04/01
*
*****************************************************************************************************/

#ifndef CSLOCALVIDEOPLAYER_H_
#define CSLOCALVIDEOPLAYER_H_

#include <QtWidgets/QDialog>
#include "ui_cslocalvideoplayer.h"
#include <QButtonGroup>
#include "../render/PlayerViewBase.h"
#include <QTimer>

#define REFRESH_CYCLE 20	//设置的刷新周期

class IVideoFileReader;

class CSLocalVideoPlayer : public QDialog
{
	Q_OBJECT
	
	/**************************
	* @brief: 视频播放器QLabel的类型
	* @author:mpp
	* @date:2022/04/01
	***************************/
	enum VideoPlayerLabelType
	{
		VideoTimeValue,    //拍摄时间
		TimeLenValue,    //时长
		FrameRateValue,    //帧率
		TotalFramesValue,    //总帧数
		VideoLenValue,    //时长，显示在滑块编辑框后
	};

public:
	/**************************
	* @brief: 构造函数
	* @param: parent 父类指针
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	CSLocalVideoPlayer(QWidget *parent = Q_NULLPTR);
	~CSLocalVideoPlayer();

	/**************************
	* @brief: 载入文件对象，读取文件头信息
	* @param: filepath-视频文件路径
	* @return: bool 读取文件是否成功
	* @author:mpp
	* @date:2022/04/06
	***************************/
	bool LoadFile(const QString& filepath);
private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void InitUI();

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void ConnectSignalSlot();

	/**************************
	* @brief: 设置播放器中QLabel信息
	* @param: type-QLabel类型
	* @param: info-显示在QLabel上的信息
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void SetVideoPlayerLabelInfo(VideoPlayerLabelType type, const QString& info);

	/**************************
	* @brief: 初始化播放器的QLabel容器
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void InitPlayerLabelsMap();

	/**************************
	* @brief: 初始化播放器
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/02
	***************************/
	void InitVideoPlayer();

	/**************************
	* @brief: 初始化主界面信息
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/06
	***************************/
	void InitMainWindowInfo();

	/**************************
	* @brief: 更新帧信息
	* @param: iFrame 第iFrame帧
	* @return:
	* @author:mpp
	* @date:2022/04/06
	***************************/
	void UpdateFrameInfo(int iFrame);

	/**************************
	* @brief: 更新帧数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/16
	***************************/
	void UpdateFrameNum(int frameNum);

	

private slots:
	/**************************
	* @brief: 播放-暂停 单选操作组槽函数
	* @param: index 索引
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void SlotDisplayBtnClicked(int index);

	/**************************
	* @brief: 倍速 单选操作组槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/01
	***************************/
	void SlotSpeedBtnClicked(int index);
	
	/**************************
	* @brief: 响应定时器的槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/06
	***************************/
	void SlotTimeout();

	void OnPlayPauseBtnClicked();

    void on_ClrRevertBox_clicked(bool checked);

    void on_LumSlider_valueChanged(int value);

    void on_ContrastSlide_valueChanged(int value);

    void on_ProcessSlider_valueChanged(int value);

private:
	QButtonGroup* m_DisplayStatusGroup;    //单选操作组，播放-暂停
	QButtonGroup* m_SpeedGroup;   //单选操作组，倍速
	QMap<VideoPlayerLabelType, QLabel*> m_mapPlayerLabels;    //播放器的QLabel容器<label类型，label指针>
	CPlayerViewBase* m_VideoPlayer;    //播放器
	std::unique_ptr<IVideoFileReader> m_pVideoFileReader;     //视频读取
	VideoFormat m_videoFormat;    //视频格式
	HscVideoClipInfo m_infoVideo;	//视频片段信息
	bool m_bColorRevert;    //反色
	QTimer* m_Timer;    //定时器
	int m_iCurrentFrame;    //当前帧
	bool m_bPause;    //播放暂停标志 
	int m_iPlaySpeed;    //播放速度，按照每秒播放25帧，1倍速逐帧播放，2倍速隔1帧播放，4倍速隔3帧播放，8倍速隔7帧，16倍速隔15帧
	int m_iContrast;    //对比度
	int m_iLuminance;    //亮度

	bool m_bPlay = true;
private:
	Ui::CSLocalVideoPlayerClass ui;
};

#endif//!CSLOCALVIDEOPLAYER_H_
