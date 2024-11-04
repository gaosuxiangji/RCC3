#ifndef PLAYBACKPLAYERCONTROLLER_H
#define PLAYBACKPLAYERCONTROLLER_H

#include <memory>
#include <mutex>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QPointer>
#include <thread>
#include <atomic>

#include "PlayerControllerInterface.h"
#include "PlaybackCacher/playbackcachertypes.h"

class Device;
class PlaybackCacher;
class PlayerParams;
struct HscVideoClipInfo;
class QTimer;
class PlaybackPlayerController : public PlayerControllerInterface
{
    Q_OBJECT

    enum State
    {
        kStop,
        kPlay,
        kPause
    };

public:
    PlaybackPlayerController(QSharedPointer<Device> device_ptr, QSharedPointer<HscVideoClipInfo> video_segment_ptr);
    ~PlaybackPlayerController();

    virtual void Play() override;
    virtual void Pause() override;
    virtual void Stop() override;
    virtual void NextFrame() override;
    virtual void PreviousFrame() override;
	virtual void SeekFrame(FRAME_INDEX index, bool brel_index = true) override;//默认输入为相对帧，brel_index为false时为绝对帧
	virtual bool GetImage(REL FRAME_INDEX index, RMAImage& image, bool brel_index = true) const override;
    virtual void SpeedTimes(unsigned char times, SpeedTimeMethod method = SPEED_TIME_METHOD_SKIP_FRAME) override;
    virtual TIMESTAMP GetTimeStamp(ABS FRAME_INDEX index) { return 0; }
    virtual std::shared_ptr<PlayerParams> GetPlayerParams() override;
    virtual std::shared_ptr<MediaInfo> GetVideoInfo() const override;
    virtual std::shared_ptr<ISPUtil> GetISPHandle() override { return std::shared_ptr<ISPUtil>(); }
    virtual void Reset() override {}
    virtual void EnableFpsControl(bool enable) override {}

    virtual void SetEditPos(ABS FRAME_INDEX begin, ABS FRAME_INDEX end);
    virtual void EnableLoopPlay(bool enabled);
	virtual void Close() override {};

    /**
     * @brief 开始载入缩略图
     * @param thumbnail_count 缩略图数量
     */
    void startLoadingThumbnails(int thumbnail_count);

	/**
	* @brief 缩略图是否已经载入
	* @return true-已经载入，false-未载入
	*/
	bool isThumbnailsLoaded() const;

	/**
	* @brief 设置当前缩略图
	* @param thumbnail_index 缩略图索引
	*/
	void setCurrentThumbnail(int thumbnail_index);

	void updateThumbnailsLuminanceAndContrast();

	/**
	 * @brief 获取设备
	 * @return 设备指针
	 */
	QSharedPointer<Device> getDevice() const;

	void suspend();

	void resume();

	/**
	*@brief 设置亮度对比度
	*@param [in] : luminance : const int，亮度
				   const int，对比度
	**/
	void setLuminanceAndContrast(const int luminance, const int contrast);

signals:
    /**
     * @brief 缩略图更新
     * @brief thumbnail_index 缩略图索引，有效范围：[0,thumbnail_count-1]
     * @param thumbnail_image 缩略图
     */
    void thumbnailUpdated(int thumbnail_index, const RMAImage & thumbnail_image);

    /**
     * @brief 缩略图载入完成
     * @param ok true-成功，false-失败
     */
    void thumbnailLoadingFinished(bool ok);

private slots:
    /**
     * @brief 载入缩略图
     */
    void onThumbnailLoading();

private:
    qint64 getCurrentFrameNo() const;
    qint64 getPreviousFrameNo() const;
    qint64 getNextFrameNo() const;
    void setCurrentFrameNo(qint64 frame_no);

    void doUpdate();

    /**
     * @brief 停止图像载入
     */
    void stopLoadingThumbnails();

    /**
     * @brief 获取缩略图导出间隔
     * @param start_frame_no 起始帧
     * @param end_frame_no 结束帧
     * @param thumbnail_count 缩略图数量
     * @return
     */
    qint64 getThumbnailExportInterval(qint64 start_frame_no, qint64 end_frame_no, qint64 thumbnail_count);

    /**
     * @brief 保存状态
     */
    void saveState();

    /**
     * @brief 恢复状态
     */
    void restoreState();

private:
	QWeakPointer<Device> device_wptr_;
    std::shared_ptr<MediaInfo> media_info_ptr_;
    std::shared_ptr<PlaybackCacher> export_cacher_ptr_;
    State state_{ State::kStop };

    PlaybackCacherParams cacher_params_;
    std::shared_ptr<PlayerParams> player_params_ptr_;

    qint64 start_frame_no_{ -1 };
    qint64 end_frame_no_{ -1 };
    qint64 play_speed_{ -1 };
    qint64 current_frame_no_{ -1 };
    qint64 last_frame_no_{ -1 };
    mutable std::mutex playback_mutex_;

    std::unique_ptr<RMAImage> rma_image_;
    std::thread ctrl_thread_;
    std::atomic_bool ctrl_running_{ true };
    std::condition_variable cv_ctrl_;
    std::chrono::milliseconds play_period_{ 100 };

	mutable std::mutex thumbnails_mutex_;
	std::vector<RMAImage> thumbnails_;
    std::shared_ptr<PlaybackCacher> thumbnail_cacher_ptr_;
    std::unique_ptr<std::thread> thumbnail_thread_ptr_;
    std::atomic_bool thumbnail_loading_{ false };
    QPointer<QTimer> thumbnail_timer_ptr_;
    QElapsedTimer thumbnail_elapsed_timer_;
    int thumbnail_count_ { 0 };
	int thumbnail_frame_interval_{ 0 };
    const qint64 kThumbnailTimeout{ 5000 };
    State previous_state_ { State::kStop };

	std::atomic_int luminance_{ 50 };
	std::atomic_int contrast_{ 50 };
};

#endif // PLAYBACKPLAYERCONTROLLER_H
