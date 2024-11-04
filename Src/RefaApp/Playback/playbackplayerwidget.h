#ifndef PLAYBACKPLAYERWIDGET_H
#define PLAYBACKPLAYERWIDGET_H

#include "playbackplayerctrlthread.h"
#include "Video/VideoItemManager/videoitemmanager.h"
#include <RMAImage.h>
#include <QWidget>
#include <memory>
#include <QVariant>
#include <QPainter>

/***********前置声明区**********/
class PlayerControllerInterface;
class PlaybackPlayerController;
enum PLAYER_WORK_MODE;
class QEvent;
/*******************************/

namespace Ui {
class PlaybackPlayerWidget;
}

/**
 * @brief 回放播放器界面类
 */
class PlaybackPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlaybackPlayerWidget(QWidget *parent = 0);
    ~PlaybackPlayerWidget();

public:
	/**
	*@brief 添加视频
	*@param [in] : player_controller_ptr : std::shared_ptr<PlayerControllerInterface>，playercontroller指针
				   video_mark : const QVariant&，视频唯一标识符
	**/
	void addVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr, const QVariant& video_mark);
	
	/**
	*@brief 移除指定视频
	*@param [in] : player_controller_ptr : std::shared_ptr<PlayerControllerInterface>，playercontroller指针
	**/
	void removeVideo(std::shared_ptr<PlayerControllerInterface> player_controller_ptr);

	/**
	*@brief 移除播放器中的视频
	**/
	void removeVideo();

	/**
	*@brief 获取当前视频id
	*@return : QVariant : 当前播放视频id
	**/
	QVariant getCurrentVideoId() const;

	/**
	*@brief 设置十字线中心
	*@param [in] : pt : const QPoint &，中心点坐标
	**/
	void setFocusPoint(const QPoint &pt);//设置窗口中心线

	/**
	*@brief 设置roi
	*@param [in] : roi : const QRect &，roi
	**/
	void setRoi(const QRect &roi);//设置ROI

	/**
	*@brief 设置触发帧
	*@param [in] : frame_index : uint64_t，触发帧帧号
	**/
	void setTriggerFrame(uint64_t frame_index);

	/**
	*@brief 设置播放器工作模式
	*@param [in] : mode : PLAYER_WORK_MODE，工作模式
	*@note 使用时需注意在设置下一个模式前需先将模式设置为idle
	**/
	void setPlayerWorkStatus(PLAYER_WORK_MODE mode);

	/**
	*@brief 获取当前工作模式
	*@return PLAYER_WORK_MODE : 工作模式
	**/
	PLAYER_WORK_MODE getPlayerWorkStatus() const;

	/**
	*@brief 获取当前图像
	*@return RMAImage 图像信息
	*@note 标定时使用
	**/
	RMAImage getCurrentFrame();

	/**
	*@brief 播放器是否暂停播放
	*@return bool : true-暂停,false-播放中
	**/
	bool isPaused() const;

	/**
	*@brief 切换语言
	**/
	void switchLanguage();

	/**
	*@brief 获取缩略图是否显示
	*@return bool : true-显示，false-不显示
	**/
	bool isThumbnailVisible() const;

	/**
	 * @brief 获取播放控制器
	 * @return 播放控制器
	 */
	std::shared_ptr<PlayerControllerInterface> getPlayerController() const;

public slots:
    //工具栏信号响应槽函数
	/**
	*@brief 设置十字线是否显示
	*@param [in] : bvisible : bool，true-显示，false-不显示
	**/
	void setFocusPointVisible(bool bvisible);

	/**
	*@brief 设置是否全屏
	*@param [in] : benabled : bool，true-使能，false-不使能
	**/
	void setFullScreen(bool benabled);

	/**
	*@brief 使能选择roi
	**/
	void setRoiSelectionEnabled(/*bool benabled*/);

	/**
	*@brief 快照截屏
	**/
	void snapshot();

	/**
	*@brief OSD信息是否显示
	*@param [in] : bvisible : bool，true-显示，false-不显示
	**/
	void setOSDVisible(bool bvisible);

	/**
	*@brief 设置亮度对比度
	*@param [in] : luminance : const int，亮度
				   : const int，对比度
	**/
	void setLuminanceAndContrast(const int luminance, const int contrast);

	/**
	*@brief 全屏时还原按钮响应
	**/
	void on_toolButtonRestoreFullScreen_clicked();

    //播放控制响应槽函数
	/**
	*@brief 设置播放起点
	*@param [in] : begin : uint64_t，起始帧
	*@note 这里的播放器只有绝对帧
	**/
	void setPlayerBegin(uint64_t begin);

	/**
	*@brief 设置播放终点
	*@param [in] : end : uint64_t，结束帧
	*@note 这里的播放器只有绝对帧
	**/
    void setPlayerEnd(uint64_t end);

	/**
	*@brief 设置播放终点
	*@param [in] : begin : uint64_t，起始帧
				   end : uint64_t，结束帧
	*@note 这里的播放器只有绝对帧
	**/
	void setPlayerRange(uint64_t begin, uint64_t end);

	/**
	*@brief 设置当前帧
	*@param [in] : frameIndex : uint64_t，当前帧
	*@note 拖动播放进度条时响应
	**/
    void setCurrentFrame(uint64_t frameIndex);

	/**
	*@brief 反向播放
	**/
    void backwardPlay();

	/**
	*@brief 正向播放
	**/
    void forwardPlay();

	/**
	*@brief 暂停
	*@note 播放控制模块发出信号通知播放器暂停
	**/
    void pause();

	/**
	*@brief 播放器通知暂停播放
	*@note 外部信号使播放器暂停
	**/
	void slotPause();

	/**
	*@brief 快进播放
	**/
    void fastForwardPlay();

	/**
	*@brief 快退播放
	**/
    void fastBackwardPlay();
	
	/**
	*@brief 后退一帧
	**/
    void backward();

	/**
	*@brief 前进一帧
	**/
    void forward();

	/**
	*@brief 停止播放
	**/
	void stop();

	/**
	*@brief 清空测量特征
	**/
	void clearMeasureFeatures();

	/**
	*@brief 缩略图是否显示
	*@param [in] : bvisible : bool，true-显示，false-不显示
	**/
	void setThumbnailVisible(bool bvisible);

	/**
	*@brief 开始载入缩略图
	**/
	void startLoadingThumbnails();

	/**
	*@brief 设置缩略图
	*@param [in] : thumbnail_index : int，索引
				   thumbnail : const RMAImage &，图像
	**/
	void setThumbnailImage(int thumbnail_index, const RMAImage & thumbnail);

	/**
	*@brief 缩略图载入完成
	*@param [in] : ok : bool，true-成功，false-失败
	**/
	void thumbnailLoadingFinished(bool ok);

	/**
	*@brief 选中缩略图时的响应
	*@param [in] : thumbnail_index : int，索引
				   thumbnail : const RMAImage &，图像
	**/
	void onSelectedThumbnail(uint thumbnail_index, const RMAImage& img);

	/**
	*@brief 更新缩略图
	**/
	void updateThumbnails();

private:
	/**
	*@brief 界面初始化
	**/
	void initUi();

	/**
	*@brief 初始信号绑定
	**/
	void initBinding();

	/**
	*@brief 添加测量特征
	*@param [in] : item : const VideoItem &，视频项
	**/
	void addMeasureFeature(const VideoItem &item);

	/**
	*@brief 添加播放器特征，包含测量特征、十字线、roi等
	**/
	void addPlayerLegends();

	/**
	*@brief 打开视频时刷新界面
	*@param [in] : frame_cnt : uint64_t，视频总帧数
	**/
	void updateAddVideoUi(uint64_t frame_cnt);

	/**
	*@brief 关闭视频时刷新界面
	**/
	void updateCloseVideoUi();

	/**
	*@brief 获取视频ip或路径
	*@return QString : ip或路径
	**/
	inline QString getPlayerIPOrPath() const;

	/**
	*@brief 设置播放操作
	*@param [in] : opt : PlayOperation，操作类型
	**/
	void setPlayOperation(PlayOperationType opt);

	/**
	*@brief 退出播放器控制线程
	**/
	void quitPlayCtrlThread();

	/**
	*@brief 获取视频项
	*@param 
	*@return VideoItem : 视频项
	**/
	inline VideoItem getVideoItem() const;

	/**
	*@brief 设置视频项
	*@param [in] : item : const VideoItem &，视频项
	**/
	inline void setVideoItem(const VideoItem &item);

	/**
	*@brief 获取录制回放播放控制器
	**/
	inline std::shared_ptr<PlaybackPlayerController> getPlaybackPlayerController() const;

	/**
	*@brief 更新缩略图输俩个
	*@param [in] : begin : int，播放起始帧
				   end : int，播放结束帧
	**/
	void updateThumbnailCount(int begin, int end);

	/**
	*@brief 绘制测量特征至图像
	*@param [in/out] : image :RMAImage &，图像
	**/
	void paintMeasureFeaturesToImage(RMAImage & image) const;

	/**
	*@brief 绘制测量特征
	*@param [in/out] : painter : QPainter &，绘制类对象
					   feature_data : const QVariant &，特征数据
					   user_data : const QVariant &，用户输出的特征数据
					   buffer_rc : const QRect &，绘制范围
	**/
	void paintMeasureFeature(QPainter & painter, const QVariant & feature_data, const QVariant & user_data, const QRect & buffer_rc) const;

	/**
	*@brief 获取文字绘制的矩形框
	*@param [in] : text_rc : QRect &，原始文字绘制矩形
				   buffer_rc : const QRect &，绘制范围
				   point : const QPoint &，文字对应的点
	*@return : QRect : 文字绘制矩形
	**/
	inline QRect getPaintTextRect(const QRect & text_rc, const QRect & buffer_rc, const QPoint & point) const;

	/**
	*@brief 获取文字绘制线段终点
	*@param [in] : begin_pt : const QPoint &，线段起点
				   text_rc : const QRect &，文字绘制矩形
	*@return : QPoint : 线段终点
	**/
	inline QPoint getPaintTextLineEndPoint(const QPoint & begin_pt, const QRect & text_rc) const;

	/**
	*@brief 获取两点间的距离
	*@param [in] : p1 : const QPoint &，点1
				   p2 : const QPoint &，点2
	*@return : float : 距离
	**/
	inline float getDistance(const QPoint & p1, const QPoint & p2) const;

	/**
	*@brief QImage转RMAImage
	*@param [in] : image :QImage，输入图像
	*@return : RMAImage : 输出图像
	**/
	inline RMAImage QImage2RMAImage(QImage image) const;

	/**
	*@brief 更新当前图像
	**/
	inline void updateCurrentFrame();

private slots:
	/**
	*@brief 播放器内拖动roi
	*@param [in] : roi : const QRect &，roi
	**/
	void slotRoiChanged(const QRect &roi);

	/**
	*@brief 播放器内拖动十字线
	*@param [in] : pt : const QPoint &，roi
	**/
	void slotFocusPointChanged(const QPoint &pt);

	/**
	*@brief 播放控制组件是否使能
	*@param [in] : benabled : bool，true-使能，false-不使能
	**/
	void slotPlayerCtrlWidgetEnabled(bool benabled);

	/**
	*@brief 添加特征特征id
	*@param [in] : legend_id : LEGENDS_ID，true-使能，false-不使能
				   feature : const QVariant&，特征内容
	**/
	void slotFeatureAdded(LEGENDS_ID legend_id, const QVariant& feature);

	/**
	*@brief 播放器控制线程开始工作响应槽函数
	**/
	void slotPlayCtrlThreadStarted();

	/**
	*@brief 播放器控制线程操作结束响应槽函数
	**/
	void slotPlayCtrlThreadFinished();

	/**
	*@brief 图像更新响应槽函数
	*@param [in] : idx : int64_t，当前帧号
	**/
	void slotUpdatedFrame(int64_t idx);

	/**
	*@brief 清除选中缩略图
	**/
	void clearThumbnailSelect();

	/**
	*@brief 开启工具栏显示定时器
	**/
	void startToolbarVisibleTimer();

protected:
	/**
	*@brief 改变事件
	*@param [in] : *event : QEvent，事件指针
	**/
	virtual void changeEvent(QEvent *event);

	/**
	*@brief 鼠标双击事件
	*@param [in] : *event : QMouseEvent，事件指针
	**/
	void mouseDoubleClickEvent(QMouseEvent *event);

	/**
	*@brief 事件过滤器
	*@param [in] : *watched : QObject，对象指针
				   *event : QEvent，事件指针
	**/
	bool eventFilter(QObject *watched, QEvent *event) override;

signals:
	/**
	*@brief 全屏信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void fullScreenTriggered(bool benabled);//全屏触发

	/**
	*@brief roi变化信号
	*@param [in] : id : const QVariant& ，相机ip/视频id
				   roi : const QRect& ，roi区域
	**/
	void roiChanged(const QVariant& id, const QRect &roi);//ROI变化

	/**
	*@brief 十字线变化信号
	*@param [in] : id : const QVariant& ，相机ip/视频id
				   pt : const QPoint& ，十字线交点
	**/
	void focusPointChanged(const QVariant& id, const QPoint &pt);//

	/**
	*@brief 保存截图信号
	*@param [in] : id : const QVariant& ，相机ip/视频id
				   img : const RMAImage& ，图片
	**/
    void saveSnapshot(const QVariant& id, const RMAImage &img);//保存截图

private:
    Ui::PlaybackPlayerWidget *ui;
	PlaybackPlayerCtrlThread playback_ctrl_thread_;
	QVariant cur_video_mark_;//当前视频的唯一标识符
	bool bupdated_add_video_ui_{ false };//添加视频后是否已经刷新过界面
	QTimer *toolbar_visible_timer_;
	static const int kToolbarVisibleTimer_{ 5000 };//工具栏显示时间
};

#endif // PLAYBACKPLAYERWIDGET_H
