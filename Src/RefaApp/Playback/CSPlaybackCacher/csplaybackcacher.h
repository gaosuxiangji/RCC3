#ifndef CSPLAYBACKCACHER_H
#define CSPLAYBACKCACHER_H
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <QSharedPointer>
#include <QTimer>
#include "opencv2/opencv.hpp"
#include "ThreadPool/ThreadPool.h"
#include "cache/FIFOCache.h"
#include "Video/VideoItem/videoitem.h"
#include "Device/csframeinfo.h"
#include "cache/FrameMgrFixed.h"
#include "Common/ExportCom/AGCacheStream.h"

/**
* @brief 回放参数结构体
*/
struct PlaybackCacherParams
{
	qint64 startFrameNo{ -1 };
	qint64 endFrameNo{ -1 };
	qint64 frameInterval{ -1 };
	qint64 curFrameNo{ -1 };
};

//struct ImageInfo
//{
//	int		rows_ = 0;
//	int		cols_ = 0;
//	int		type_ = 0;
//	size_t  step_ = 0;
//
//	ImageInfo(int row, int col, int type, size_t step)
//		: rows_(row), cols_(col), type_(type), step_(step)
//	{
//	};
//
//	ImageInfo() {};
//	
//};

enum State//播放模式
{
	STOP,//停止
	PLAY,//播放
	PAUSE//暂停
};
struct Timestamp { uint8_t timestamp_t_[9]; };

class ImageProcessor;
class Device;
class CSPlaybackCacher
{
private:
	
	struct DataCache
	{
		struct DataCacheInfo
		{
			int32_t		m_pos = -1;
			int32_t		m_next_idx = 0;
			//int8_t		m_state = 0;
		};

		int32_t						m_max_size = 400;
		int64_t						m_start_frame_no = 0;
		//int64_t					m_end_frame_no = 0;
		std::atomic_int32_t			m_interval{0};
		std::atomic_int32_t			m_crt_process_idx{0};
		//std::atomic_int32_t			m_start_idx{0};
		//std::atomic_int32_t			m_end_idx{0};
		std::vector<cv::Mat>		mImages;
		std::vector<DataCacheInfo>	m_data_cache_list;
		std::vector<int32_t>		mIdleList;
        std::atomic_int32_t			mIdleCrtIdx {0};

		DataCache() {}
		DataCache(int64_t start_frame_no, int64_t end_frame_no, int64_t interval)
		:
		m_start_frame_no(start_frame_no),
		m_interval(interval)
		{
			int count = (end_frame_no - start_frame_no + 1) / interval;
			m_data_cache_list.resize(count);
			if (count > m_max_size) count = m_max_size;
			mImages.resize(count);
			mIdleList.resize(count);
			for (int i = 0; i < count - 1; i++){
				mIdleList[i] = i + 1;
			}
			mIdleList[count - 1] = -1;
			/*for (int i = 0; i < count - 1; i++) {
				mProcessIdxList[i].m_next_idx = i + 1;
			}
			mProcessIdxList[count - 1].m_next_idx = 0;*/
		}

		int8_t push_data(int64_t frame_no, cv::Mat &mat)
		{
			int32_t index = (frame_no - m_start_frame_no) / m_interval;
			if (mIdleCrtIdx == -1) return -1; // is full

			// multi thread issue
			int32_t idleidx = mIdleCrtIdx;
			mImages[idleidx] = std::move(mat.clone());
			mIdleCrtIdx = mIdleList[idleidx];

			m_data_cache_list[index].m_pos = idleidx;
			//if (m_end_idx > m_start_idx && m_end_idx < mImages.size()-1) {
			//	mImages[++m_end_idx] = std::move(mat.clone());
			//	mProcessIdxList[index].m_state = 1;
			//	return 0;
			//}
			//if (m_end_idx < m_start_idx && m_end_idx + 1 < m_start_idx) {
			//	mImages[++m_end_idx] = std::move(mat.clone());
			//	mProcessIdxList[index].m_state = 1;
			//	return 0;
			//}
			//if (m_end_idx - m_start_idx + 1 == mImages.size() || m_end_idx - m_start_idx + 1 == 0) {
			//	// mImage is full
			//	return 2;
			//}
		}

		int8_t pop(cv::Mat &mat)
		{
			int idx = m_crt_process_idx;

		}

		int8_t getData(int64_t frame_no, cv::Mat &mat)
		{
			int index = ((frame_no - m_start_frame_no) / m_interval) % mImages.size();
			int pos = (frame_no - m_start_frame_no) / m_interval;
			if (m_data_cache_list.size() <= pos) return -1;
			int pos_ = m_data_cache_list[pos].m_pos;
			if (pos_ == -1 || pos_ >= mImages.size()) return -1;
			swap(mImages[pos],mat);
			mIdleList[pos_] = mIdleCrtIdx;
			mIdleCrtIdx = pos_;
			return 0;
		}
	};

	struct VideoInfo
	{
		int32_t		m_width = 0;
		int32_t		m_height = 0;
		int32_t		m_fps = 0;
		int32_t		m_displaymode = 0;
		int8_t		m_bpp = 0;
		int32_t		m_streamtype = 0;
	};
public:
	struct CachedFrameInfo
	{
		int64_t frameNo{ -1 };
		int64_t rawFrameNo{ -1 }; // 原始帧编号
		std::wstring timestamp;
		cv::Mat matImage;
	};

public:
    CSPlaybackCacher(QSharedPointer<Device> pDevice, int video_segment_id, const VideoItem &video_item, bool auto_cycle = true);
	~CSPlaybackCacher();

	bool start(const PlaybackCacherParams & params, const AGStreamCB onAGFrame = nullptr);

	bool start(int64_t start_frame_no, int64_t end_frame_no, int64_t frame_interval, int64_t current_frame_no, const AGStreamCB onAGFrame = nullptr);
	void stop(bool stop_mode = 0);

	bool getFrame(int64_t frame_no, RccFrameInfo & frame_image, bool trigger_cache = true);

	/**
	*@brief 调整图像亮度对比度
	*@param [in/out] : rma_image，图像
	luminance : const int，亮度
	const int，对比度
	anti_color 反色
	**/
	void adjustLuminanceAndContrast(RccFrameInfo & frame_image, const int luminance, const int contrast ,const bool anti_color );

	void pause() {
		in_running_ = PAUSE; 
		if (mCacheStream) mCacheStream->pause();
	}
	void restart() {
		in_running_ = PLAY; if (mCacheStream) mCacheStream->restart();
	}

private:
	int64_t calcCacheEndFrameNo(int64_t cache_start_frame_no, int64_t interval);
	bool isCacheFrameNo(int64_t frame_no);//判断frame_no是不是当前正在缓存的帧
	void doExport();
	void doImageProcess(std::shared_ptr<ImageProcessor> pBufferProcessor, cv::Mat buffer_mat,int64_t frame_no);
	void ExportFrame(int64_t start_frame_no, int64_t end_frame_no, int64_t interval);

	void doExportMode1();
	void doExportMode2();
	void doProcess();
	void doImageProcess1(std::shared_ptr<ImageProcessor> image_processor_ptr, std::shared_ptr<CAGBuffer> &buffer, int64_t frame_no);

	inline bool exportNew() { if (!m_is_thumbnail_ && m_export_mode_) return true; return false; }
	int32_t getFrameSize();

private:
	QSharedPointer<Device> device_ptr_;
	int64_t video_segment_id_;

	AGStreamCB mOnAGFrame = nullptr;

	std::unique_ptr<std::thread> thread_ptr_;
	//std::atomic_bool in_running_ {false};
	std::uint8_t in_running_{ 0 };
	std::atomic_int64_t frame_interval_ {-1};
	std::atomic_int64_t cache_start_frame_no_ {-1};
	std::atomic_int64_t cache_end_frame_no_{-1};
	std::atomic_int64_t play_frame_no_ {0};
	std::atomic_uint8_t triggered_ {0};

	bool auto_cycle_ = true;

	int64_t start_frame_no_ = -1;
	int64_t end_frame_no_ = -1;

	mutable std::mutex mtx_;
	std::condition_variable cv_;

	agile_device::FIFOCache<int64_t, RccFrameInfo> frame_map_;

	agile_device::ThreadPool image_process_executor_{ 2 };

	const int kFirstFrameTimeoutByMs = 5000;
	const int kNonFirstFrameTimeoutByMs = 5000;

	std::unique_ptr<CAGBuffer> buffer_ptr_;

	const int kDefaultFrameCountPerExport = 50;
	int max_frame_count_per_export_{ kDefaultFrameCountPerExport };

	CachedFrameInfo last_cache_frame_info_;
	int32_t max_cache_size_ = 0;

	// for new playback
	int32_t m_count_ = 0;
	int8_t m_export_mode_ = 0;
	uint8_t m_stop_mode_ = 0;
	
	int64_t frame_size_ = 20736640;
	//ImageInfo m_image_info;
	uint8_t m_zero_timestamp[9];
	char *m_data_ = nullptr;
	bool m_is_thumbnail_ = false;
	std::shared_ptr<FrameMgrFixed>  m_framemgr_;
	std::atomic_int8_t m_thread_active_count{ 0 };
	std::atomic_int64_t cache_frame_no_{0};

	uint64_t diff_first_frame_timestamp = 0;
	uint64_t first_frame_timestamp = 0;
	uint64_t lasttimestamp = 0;

	std::unique_ptr<std::thread> m_process_thread_;
	std::atomic_bool m_process_thread_running{ false };
	//std::shared_ptr<DataCache> m_data_cache;

	std::queue<cv::Mat>				m_data_cache_queue;
	mutable std::mutex				m_ProcessQueueMtx;
	std::condition_variable			m_CondVar;
	int32_t							get_frame_no_ = 0;
	int8_t							m_start_export = 0;
	int32_t							mRevCount = 0;
	DeviceHandle					device_handle_;
	std::shared_ptr<AGCacheStream>  mCacheStream;
	std::chrono::time_point<std::chrono::system_clock>		m_last_read_timestamp;

    std::atomic_int32_t				m_put_image_num_ {0};
    std::atomic_int32_t				m_get_image_num_ {0};
    int32_t							m_cache_map_size_ {0};
	mutable std::mutex				m_DataStreamStopMtx;
	std::unordered_map<int64_t, int32_t>	m_get_frame_no_map_;

	VideoInfo						m_videoinfo;

void Assert(bool rslt, const std::string &str)
{
	if (!rslt)
	{
		std::cout << str << std::endl;
	}
}
};

#endif // CSPLAYBACKCACHER_H
