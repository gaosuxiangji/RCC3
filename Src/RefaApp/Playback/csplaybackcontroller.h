#ifndef CSPLAYBACKCONTROLLER_H
#define CSPLAYBACKCONTROLLER_H
#include <memory>
#include <mutex>
#include <QObject>
#include <stack>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QWeakPointer>
#include <QTimer>
#include <QPointer>
#include <thread>
#include <atomic>
#include "Playback/CSPlaybackCacher/csplaybackcacher.h"
#include "Video/VideoItem/videoitem.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

enum PlayState//播放模式
{
	PS_STOP,//停止
	PS_PLAY,//播放
	PS_PAUSE//暂停
};

#define TRUN_ON_THUMBNAIL_ANIMATION_MODE 1    //开启缩略图动画模式 1-开启 0-关闭

/**
* @brief 回放模块控制类
* @note 功能:控制播放器显示图像数据,控制播放逻辑(播放暂停倍速范围等),
**		控制缩略图数据显示更新
*/
using CrtlCallback = std::function<void(bool)>;
class CSPlaybackController : public QObject
{
	Q_OBJECT
public:
	/**
	* @brief
	* @param video_item 需要控制的视频项
	* @param parent 父对象
	* @note 一个视频项对应一个controller
	*/
	explicit CSPlaybackController(VideoItem video_item, QObject *parent = 0);
	~CSPlaybackController();

	/**
	* @brief	获取编辑过后的VideoItem
	* @return	VideoItem
	* @note		导出范围,亮度对比度,反色参数同步到该VideoItem中
	*/
	VideoItem GetVideoItem();


	/**
	* @brief 切换视频播放状态
	* @param play_state 需要切换的状态: 播放,暂停,停止
	* @return 是否成功
	* @note
	*/
	bool SwitchState(PlayState play_state);

	/**
	* @brief 获取视频播放状态
	* @return PlayState 状态: 播放,暂停,停止
	* @note
	*/
	PlayState GetState() const;


	/**
	* @brief 禁止回放模块获取图像
	* @note 避免与视频导出相冲突
	*/
	void DisableGetFrame(bool b_disable);

	/**
	* @brief 切换到下一帧
	* @note 暂停播放,单帧切换
	*/
	void NextFrame();

	/**
	* @brief 切换到上一帧
	* @note 暂停播放,单帧切换
	*/
	void PreviousFrame();

	//刷新一次图像
	void RefreshImage();

	/**
	* @brief 切换播放帧
	* @param play_frame 需要切换的帧号
	* @note 只会定位到播放帧的位置,不会改变视频播放状态
	*/
	void SeekFrame(int64_t play_frame);
	void SeekFrameMs(double play_frame);

	/**
	* @brief 获取当前帧的帧号
	* @return 当前帧的帧号
	* @note
	*/
	int64_t GetCurrentFrameNo() const;
	double GetCurrentFrameMS() const;


	/**
	* @brief 获取上一帧的帧号
	* @return 上一帧的帧号
	* @note 考虑计算倍速
	*/ 
	int64_t GetPreviousFrameNo() const;
	double GetPreviousFrameMs() const;


	/**
	* @brief 获取下一帧的帧号
	* @return 下一帧的帧号
	* @note 考虑计算倍速
	*/
	int64_t GetNextFrameNo() const;
	double GetNextFrameMs() const;

	/**
	* @brief 切换视频区间,并且保存切换记录
	* @param start_frame_no 需要切换的开始帧号
	* @param end_frame_no 需要切换的结束帧号
	* @param save_change 是否保存切换记录
	* @return 是否成功
	* @note 如果结束区间小于开始区间则直接返回false,切换区间会修正当前帧并且触发缩略图加载
	*/
	bool SwitchRange(int64_t start_frame_no, int64_t end_frame_no, bool save_change = true);
	bool SwitchRangeMs(double start_frame_ms, int64_t end_frame_ms, bool save_change = true);

	/**
	* @brief	切换到上一次的视频区间
	* @return	是否成功
	* @note		记录为空则返回false,切换区间会触发缩略图加载
	*/
	bool SwitchToPreviousRange();

	/**
	* @brief	当前视频区间记录栈是否为空
	* @return	true - 空,false - 非空
	* @note
	*/
	bool IsRangeStackEmpty() { return m_range_stack.size() == 0; }

	/**
	* @brief 获取当前视频开始帧号
	* @return 当前视频开始帧号
	* @note
	*/
	int64_t GetStartFrameNo();
	double GetStartFrameMs();


	/**
	* @brief 获取当前视频结束帧号
	* @return 当前视频结束帧号
	* @note
	*/
	int64_t GetEndFrameNo();
	double GetEndFrameMs();


	/**
	* @brief 切换当前播放倍速
	* @param play_speed 需要切换的播放倍速
	* @return 是否切换成功
	* @note 设置范围[1,16],倍速逻辑为抽帧播放,会跳过中间帧
	*/
	bool SwitchSpeed(int64_t play_speed);

	void SkipFrame(int64_t play_speed);

	/**
	* @brief 获取当前播放倍速
	* @return int64_t 当前播放倍速
	* @note 范围[1,16]
	*/
	int64_t GetSpeed();

	//关键帧相关 

	/**
	* @brief 设置关键帧帧号
	* @param  key_frame_no
	* @note 设置之后如果对应缩略图和图像关键帧状态有变化则会发送更新信号
	*/
	void SetKeyFrameNo(int64_t key_frame_no);

	/**
	* @brief 获取当前的关键帧帧号
	* @return 关键帧帧号
	* @note
	*/
	int64_t GetKeyFrameNo();

	//重置关键帧
	void ResetKeyFrameNo();

	/**
	* @brief 获取视频的关键帧帧号
	* @return 关键帧帧号
	* @note
	*/
	int64_t GetOriginKeyFrameNo();

	//预览关键帧
	void PreviewKeyFrame();

	/**
	* @brief	设置图像亮度
	* @param	luminance 亮度 [0,100]
	* @note		对图像和缩略图都有影响,会更新图像和缩略图
	*/
	void SetLuminance(const int luminance);
	int GetLuminance();

	/**
	* @brief	设置图像对比度
	* @param	contrast 对比度 [0,100]
	* @note		对图像和缩略图都有影响,会更新图像和缩略图
	*/
	void SetContrast(const int contrast);
	int GetContrast();

	/**
	* @brief	设置反色使能
	* @param	enable 反色使能
	* @note		对图像和缩略图都有影响,会更新图像和缩略图
	*/
	void SetAntiColorEnable(const bool enable);
	bool isAntiColorEnable();

	/**
	* @brief	整个视频的总帧数(无视选取区域)
	* @return	总帧数
	*/
	int64_t GetVideoTotalFrameCount()const;

	/**
	* @brief 手动开启加载缩略图动作
	* @note 加载缩略图结束时通过thumbnailLoadingFinished()通知
	*/
	void StartLoadThumbnails();

	/**
	* @brief	触发预览缩略图状态,更新缩略图为中心帧周围的缩略图
	* @param	center_frame_index 中心帧
	* @note		会刷新缩略图显示
	*/
	void PreviewThumbnails(const int64_t center_frame_index);

	/**
	* @brief	在大图上预览单个缩略图
	* @param	display_thumbnail_index 需要显示的缩略图序号
	* @note		刷新大图显示, 需要切回原图直接调用SeekFrame
	*/
	void PreviewSingleThumbnail(const int64_t display_thumbnail_index);

	//当前是否处于预览缩略图(或关键帧)状态
	bool IsPreviewingThumbnail() { return m_thumbnail_previewing; }

	/**
	* @brief	重置缩略图到正常状态
	* @note		会刷新缩略图显示
	*/
	void ResetThumbnails();
	void stopLoadingThumbnails();
	bool isLoadingThumbnails() { return m_thumbnail_loading; }
	void resetSwitchRangeFlag() {
		std::lock_guard<std::mutex> locker(m_switch_range_mutex); m_switch_range_flag = 0; //m_switch_range_condvar.notify_one();
	}

	// add call back function
	void setCrtlCallback(const CrtlCallback callbackFunc) { m_thumbnail_load_finish_func_ = callbackFunc; }

	void setSpeed(int32_t play_speed) {
		if (play_speed == 0) play_speed = 1;
		m_play_speed = play_speed;
	}
signals:

	/**
	* @brief 缩略图更新
	* @param thumbnail_index 需要显示的缩略图索引，有效范围：[0,display_thumbnail_count-1]
	* @param thumbnail_image 缩略图
	*/
	void thumbnailUpdated(int thumbnail_index, const RccFrameInfo & thumbnail_image);
	/**
	* @brief 缩略图载入完成
	* @param ok true-成功，false-失败
	*/
	void thumbnailLoadingFinished(bool ok);

	//通知开启缩略图刷新计时器
	void signalStartLoadingThumbnailTimers();

	//通知外部显示图像和相关信息
	void ImageUpdate(const RccFrameInfo& image);

	//通知外部视频范围变更(用于同步控制sdi范围)
	void signalVideoRangeChanged(int64_t start_frame_index, int64_t end_frame_index);

	/// \brief 高亮缩略图
	/// \param [in] thumbnail_index 缩略图索引
	void signalHighlightThumbnail(int thumbnail_index);

	/**
	* @brief 记录范围变化后的堆栈数量
	* @param nSize 数量
	*/
	void signalRangeStackChange(int nSize);
public slots:

private slots :
	/**
	* @brief 开启缩略图计时器
	*/
	void startThumbnailLoadingTimers();

private:
	void InitData();
	void onThumbnailLoading(CAGBuffer* agFrame);

	/**
	* @brief 刷新图像
	*/
	void doUpdate();

	/**
	* @brief 加载关键帧图像
	*/
	void doLoadKeyFrame();

	/**
	* @brief 停止图像载入
	*/
	//void stopLoadingThumbnails();

	/**
	* @brief 获取缩略图导出间隔
	* @param start_frame_no 起始帧
	* @param end_frame_no 结束帧
	* @param thumbnail_count 缩略图数量
	* @return
	*/
	qint64 getThumbnailExportInterval(qint64 start_frame_no, qint64 end_frame_no, qint64 thumbnail_count);


	//计算缩略图序号队列
	void calcThumbnailIndexes();

	//计算需要显示的缩略图序号队列
	void calcDisplayThumbnailIndexes();

	/**
	* @brief 将需要显示的缩略图发送到界面
	* @param height_light_index 高亮帧
	*/
	void RefreshDisplayThumbnails(int height_light_index = -1);

// 	//刷新缩略图的当前帧状态
// 	void updateCurrentThumbnailState();

	/**
	* @brief 切换播放帧
	* @param frame_no 需要切换的帧号
	* @note 只设置帧号,不刷新图像
	*/
	void setCurrentFrameNo(int64_t frame_no);

	/**
	* @brief 保存状态
	*/
	void saveState();

	/**
	* @brief 恢复状态
	*/
	void restoreState();

	void changeThumbnailsState(bool image_changed);

	void checkTimeOutThumbnails();

private:
	VideoItem m_video_item;//当前视频项

	QWeakPointer<Device> m_device_ptr;//当前视频项对应的设备项

	PlayState m_state{ PlayState::PS_STOP };//当前播放状态
	PlayState m_previous_state{ PlayState::PS_STOP };//之前的播放状态
	std::atomic_bool m_state_saved{ false };//已有状态保存



	//播放相关参数
	qint64 m_video_total_frame_count{ -1 };//总帧数
	qint64 m_start_frame_no{ -1 };//起始帧
	qint64 m_end_frame_no{ -1 };//结束帧
	qint64 m_play_speed{ -1 };//播放速度(抽帧间隔)
	qint64 m_current_frame_no{ -1 };//当前帧号
	qint64 m_last_frame_no{ -1 };//之前一帧
	qint64 m_key_frame_no{ -1 };//关键帧
	qint64 m_last_key_frame_no{ -1 };//之前的关键帧
	qint64 m_origin_key_frame_no{ -1 };//原始的的关键帧


	RccFrameInfo m_image_keyframe;//关键帧数据

	typedef std::pair<qint64, qint64> CSPlayRange;//范围数值对<begin,end>
	std::stack<CSPlayRange> m_range_stack;//记录用户变化范围操作的操作栈

	mutable std::mutex m_playback_mutex;

	std::shared_ptr<CSPlaybackCacher> m_export_cacher_ptr;//视频导出缓存
	PlaybackCacherParams m_cacher_params;//导出缓存相关参数

	std::thread m_update_thread;//图像刷新线程
	std::atomic_bool m_update_running{ true };//正在刷新
	std::condition_variable m_cv_ctrl;//条件变量,用于控制图像刷新
	boost::chrono::milliseconds m_play_period{ 100 };//播放间隔

	//缩略图相关参数
	mutable std::mutex m_thumbnails_mutex;//互斥
	std::vector<int> m_thumbnail_indexes;//需要缓存的的缩略图序号
	//std::vector<RccFrameInfo> m_thumbnails;//缓存的全部缩略图
	//std::vector<RccFrameInfo> m_mini_thumbnails;//缓存的全部小尺寸缩略图
	std::unordered_map<int32_t,RccFrameInfo> m_thumbnails;//缓存的全部缩略图
	std::unordered_map<int32_t,RccFrameInfo> m_mini_thumbnails;//缓存的全部小尺寸缩略图
	int m_thumbnail_count{ 0 };//缓存的缩略图数量
	const int kDefaultThumbnailCount{ 50 };//默认的最大缩略图数量
	int m_previous_thumbnail_center_span{ -1 };//之前缩略图预览中心区间
	std::vector<int> m_display_thumbnail_indexes;//显示在界面上的缩略图序号
	int m_display_thumbnail_count{ 0 };//显示的缩略图数量
#if TRUN_ON_THUMBNAIL_ANIMATION_MODE
	const int kDefaultDisplayThumbnailCount{ 50 };//默认的最大显示缩略图数量
#else
	const int kDefaultDisplayThumbnailCount{ 15 };//默认的最大显示缩略图数量
#endif
	std::shared_ptr<CSPlaybackCacher> m_thumbnail_cacher_ptr;//缩略图导出缓冲
	std::unique_ptr<std::thread> m_thumbnail_thread_ptr;//缩略图线程
	std::atomic_bool m_thumbnail_loading{ false };//正在加载缩略图
	std::atomic_bool m_thumbnail_loading_thrd{ false }; //加载缩率图线程
	std::atomic_bool m_thumbnail_previewing{ false };//正在预览缩略图
	std::atomic_bool m_quit_previewing{ false };//需要退出预览缩略图状态
	RccFrameInfo m_preview_frame;//需要预览的图像
	RccFrameInfo m_current_frame_backup;//当前发送出去的图像
	 //std::thread m_thumbnail_timer;
	QPointer<QTimer> m_thumbnail_timer_ptr;//缩略图刷新计时器
	QElapsedTimer m_thumbnail_elapsed_timer;//缩略图刷新用时计时器
	int m_thumbnail_frame_interval{ 0 };//缩略图间隔(抽帧间隔) 
	const qint64 kThumbnailTimeout{ 10000 };//缩略图刷新超时

	std::atomic_int m_luminance{ 50 };//亮度
	std::atomic_int m_contrast{ 50 };//对比度
	std::atomic_bool m_anti_color{ false };//反色
	std::atomic_bool m_image_changed{ false };//图像参数变更
	int64_t m_fps{ 100 };
	std::atomic_bool m_disable_get_frame{ false };//禁止获取图像

	//std::thread		m_doLoadKeyFrameThrd;

	uint8_t m_export_mode_ = 0;
	uint8_t m_stop_mode_ = 0;
	int		m_test_count = 0;
	int m_interval{ 0 };

	uint8_t	  m_switch_range_flag = 0;
	mutable std::mutex m_switch_range_mutex;
	//std::condition_variable m_switch_range_condvar;
	CrtlCallback m_thumbnail_load_finish_func_ = nullptr;
};

#endif // CSPLAYBACKCONTROLLER_H