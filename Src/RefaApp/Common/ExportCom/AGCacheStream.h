#pragma once
//#include "HSCHelper.h"
#include "HscAPI.h"
#include <stdint.h>
#include <string>
#include <list>
#include <vector>
#include <mutex>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <QSharedPointer>
#include "Util/HscSystemElapsedTimer.h"
#include "IOCacheObj.h"
#include "../../Playback/CSPlaybackCacher/cache/FIFOCache.h"
#include "Common/LogUtils/logutils.h"
#include "Device/device.h"
#include "../../Playback/CSPlaybackCacher/ThreadPool/ThreadPool.h"

struct ThumbnailTrack {
	bool mInRecState = false;

	int64_t mBeginEnter = 0;
	int64_t mEndEnter = 0;

	int64_t mBeginThumbnail = 0;
	int64_t mEndThumbnail = 0;
	int64_t mBeginReqThumbnail = 0;
	int64_t mEndReqThumbnail = 0;
	int64_t mBeginRecvThumbnail = 0;
	int64_t mEndRecvThumbnail = 0;

	int64_t mBeginReqRec = 0;
	int64_t mEndReqRec = 0;
	int64_t mBeginRecvRec = 0;
	int64_t mEndRecvRec = 0;

	int64_t mEnterMS = 0;
	int64_t mThumbnailMS = 0;
	int64_t mReqMS = 0;
	int64_t mFirstRecvMS = 0;
	int64_t mRecvMS = 0;
	int64_t mProcessMS = 0;

	int64_t mRecvCount = 0;
	int64_t mProccessCount = 0;
	void reset() {
		mInRecState = false;

		mBeginEnter = 0;
		mEndEnter = 0;

		mBeginThumbnail = 0;
		mEndThumbnail = 0;
		mBeginReqThumbnail = 0;
		mEndReqThumbnail = 0;
		mBeginRecvThumbnail = 0;
		mEndRecvThumbnail = 0;

		mBeginReqRec = 0;
		mEndReqRec = 0;
		mBeginRecvRec = 0;
		mEndRecvRec = 0;

		mEnterMS = 0;
		mThumbnailMS = 0;
		mReqMS = 0;
		mFirstRecvMS = 0;
		mRecvMS = 0;
		mProcessMS = 0;

		mRecvCount = 0;
		mProccessCount = 0;
	}
	void printLog() {
		mEnterMS = mEndEnter - mBeginEnter;
		mThumbnailMS = mEndThumbnail - mBeginThumbnail;
		CSLOG_INFO("ThumbnailTrack: EnterMS={}, ThumbnailMS={}, ReqMS={}, FirstRecvMS={}, RecvMS={}, ProcessMS={}",
			mEnterMS, mThumbnailMS, mReqMS, mFirstRecvMS, mRecvMS, mProcessMS);
	}
};
struct SeekTrack {
	int64_t mBeginEnter = 0;
	int64_t mEndEnter = 0;

	int64_t mBeginReqRec = 0;
	int64_t mEndReqRec = 0;

	int64_t mBeginRecvRec = 0;
	int64_t mFirstRecv = 0;
	int64_t mFirstProcess = 0;
	int64_t mFirstRet = 0;

	int64_t mEnterMS = 0;
	int64_t mReqMS = 0;
	int64_t mRecvMS = 0;
	int64_t mProcessMS = 0;

	void reset() {
		mBeginEnter = 0;
		mEndEnter = 0;

		mBeginReqRec = 0;
		mEndReqRec = 0;

		mBeginRecvRec = 0;
		mFirstRecv = 0;
		mFirstProcess = 0;
		mFirstRet = 0;

		mEnterMS = 0;
		mReqMS = 0;
		mRecvMS = 0;
		mProcessMS = 0;
	}
	void printLog() {
		mEnterMS = mEndEnter - mBeginEnter;
		mReqMS = mEndReqRec - mBeginReqRec;
		mRecvMS = mFirstRecv - mBeginRecvRec;
		mProcessMS = mFirstRet - mFirstProcess;
		CSLOG_INFO("SeekTrack: EnterMS={}, ReqMS={}, RecvMS={}, ProcessMS={}",
			mEnterMS, mReqMS, mRecvMS, mProcessMS);
	}
};
extern ThumbnailTrack sThumbnailTrack;
extern SeekTrack sSeekTrack;

namespace AGPlugingDef {
#pragma pack(push, 1)
	struct FrameTimestamp {
		uint16_t mDays;		// 0-1 璺濈鏈勾1鏈?鏃ョ殑澶╂暟
		uint8_t mHours;		// 2 灏忔椂
		uint8_t mMins;		// 3 鍒嗛挓	
		uint8_t mSeconds;	// 4 绉?
		uint32_t mMicroseconds;	// 5-8 寰,楂樹綅鍦ㄥ墠
	};
#pragma pack(pop)

	extern tm sTmYear;
	extern uint64_t sSecondsSinceThisYear;
	extern uint64_t sTickStart;
	void initHSCTimestamp();
	uint64_t HSCTimestamp2Microseconds(FrameTimestamp* hscTimeStamp);
}

class CSingleEvent {
public:
	CSingleEvent();
	~CSingleEvent();

	// waitMS: wait milliseconds, wait infinite if = 0
	// return: timeout if < 0, else return the notifyValue
	int32_t wait(uint64_t waitMS = 0);
	void notify(uint32_t notifyValue = 0);
protected:
	std::mutex mMutex;
	std::condition_variable mWait;
	int32_t mNotifyValue = 0;
};

//struct AGBlockInfo {
//	int64_t mBlockAddr = 0;
//	int64_t mFrameNo = 0;
//	int32_t mFrameLen = 0;
//	bool mReady = false;
//};
class CacheList
{
	enum
	{
		eMaxSize = 50,
		eNone = 0,
		eRead = 0x1,
		eWrite = 0x10,
		eReadWrite = 0x11
	};
	struct FrameStru
	{
		int64_t						mFrameno = -1;
		CAGBuffer					mBuffer;
		int8_t						mState = eNone;
	};

private:
	std::vector<FrameStru>	 mVecBuf;
	int32_t		mCrtIdx = 0;
	std::mutex  mMutex;
	agile_device::FIFOCache<int64_t, int64_t> mFrame2Idx;	// key: frame_no value: crtidx

public:
	CacheList() :mFrame2Idx(0) {
		mFrame2Idx.resize(eMaxSize); 
		mVecBuf.resize(eMaxSize);
	}
	bool add(CAGBuffer* buffer, int len, int64_t frame_no)
	{
		int64_t index = -1;
		if (mCrtIdx == eMaxSize)
		{
			int64_t old_frameno = -1;
			mFrame2Idx.getFirstKeyValue(old_frameno, index);
			mFrame2Idx.put(frame_no, index);
		}
		else
		{
			index = mCrtIdx;
			mFrame2Idx.put(frame_no, index);
			mCrtIdx++;
		}

		std::lock_guard<std::mutex> lock(mMutex);
        memcpy((uint8_t*)(&mVecBuf[index].mBuffer), (uint8_t*)buffer, len);
		mVecBuf[index].mFrameno = frame_no;
		CSLOG_INFO("add buffer:{},{}", frame_no, index);
		return true;
	}

	CAGBuffer* getBuffer(int64_t frame_no)
	{
		int64_t idx = -1;
		bool rslt = mFrame2Idx.get(frame_no, idx);
		if (!rslt) {
            return nullptr;
		}
		//Assert(idx < mVecBuf.size(), "");
		std::lock_guard<std::mutex> lock(mMutex);
		return &mVecBuf[idx].mBuffer;
	}
};

class AGCacheIO : public IOCacheObj {

public:
	AGCacheIO();
	~AGCacheIO();

	virtual bool createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount);
	virtual bool reset();
	virtual bool closeCache();
	virtual bool isFull();
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen);
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len);
	virtual bool write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset);
	virtual bool read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset);
	virtual void freeBlock(AGBlockInfo* block);
protected:
	AGBlockInfo* getFreeBlock();
protected:
	HANDLE mFile = nullptr;
	int64_t mFileSize = 0;
	std::wstring mFilePath;

	std::vector<AGBlockInfo> mBlocks;
	int32_t mFreeCount = 0;
	std::mutex mBlockMutex;
};


using AGStreamCB = std::function<void(CAGBuffer*)>;
class AGStreamSource {
	enum State
	{
		eStop,
		ePlay,
		ePause
	};

public:
	AGStreamSource();
	~AGStreamSource();

	bool start(DeviceHandle hscHandle, int32_t recID, int64_t beginFrame, int64_t endFrame, int32_t interval, const AGStreamCB& onFrame);
	bool seek(int64_t beginFrame, int64_t endFrame, int32_t interval);
	bool restart(int64_t beginFrame, int64_t endFrame, int32_t interval);
	bool stopExport();
	bool startExport();
	bool startExport(int64_t beginFrame, int64_t endFrame, int32_t interval);
	bool stop(bool need_stop);
	void setStop() { 
		mIsRunning = false; 
	}
	void pause();
	void restart();
	bool isRunning();
	
protected:
	void onRecvThread();

protected:
	std::unique_ptr<std::thread> mRecvThread = nullptr;

	DeviceHandle mHscHandle = nullptr;
	AGStreamCB mOnFrame = nullptr;
    std::atomic_bool mIsRunning {false};
    std::atomic_int8_t mStreamSourceState {eStop};
	int  mRecID = -1;
	int64_t mBeginFrame = 0;
	int64_t mEndFrame = 0;
    std::atomic_bool  mThreadCheckFlag { false};

private:
	void Assert(bool rslt, const std::string &err)
	{
		if (!rslt) {
			std::cout << err << std::endl;
		}
	}
};

/**************************/
/**  AGCacheStream **/
/**************************/
class AGCacheStream{
	enum 
	{
		eTimeout = 3000,
		eLimitSize = 100,
	};

	enum State
	{
		eStop,
		ePlay,
		ePause,
		eStopping,
		eSeek
	};

public:
	struct ImageInfo
	{
		int		rows_ = 0;
		int		cols_ = 0;
		int		type_ = 0;
		size_t  step_ = 0;

		ImageInfo(int row, int col, int type, size_t step)
			: rows_(row), cols_(col), type_(type), step_(step)
		{
		};

		ImageInfo() {};

	};

public:
	AGCacheStream(QSharedPointer<Device> device_ip, StreamType stream_type);
	~AGCacheStream();

	AGCacheStream(const AGCacheStream&) = delete;
	AGCacheStream &operator=(const AGCacheStream &p) = delete;

	bool start(DeviceHandle hscHandle, int32_t recID, int64_t beginFrame, int64_t endFrame, int32_t interval, int32_t maxFrameSize, int32_t fps, const AGStreamCB onUserCB = nullptr);
	bool stop(bool need_stop);
	bool restart(int64_t beginFrame, int64_t endFrame, int32_t interval, int32_t maxFrameSize, int32_t fps);
	//bool pause();

	// CAGBuffer* getNextFrame(int32_t step);
	CAGBuffer* getFrame(int64_t frame_no);
	char* getFrameProcessed(int64_t frame_no, ImageInfo &info);

	void setStart() { mStopped = ePlay; };
	void pause();
	void restart();
	
	uint64_t static getTimestamp()
	{
		auto ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
		return ms.count();
	}

protected:
	int64_t frameNoFromMicroseconds(int64_t microseconds);
	void swapReadyBlocks();
	AGBlockInfo* getFrameBlock(int64_t frameNo);
	bool popFirstBlock();
	void addToCurBlocks(CAGBuffer* hscFrame);
	void addToCurBlocksProcess(std::shared_ptr<ImageProcessor> image_processor_ptr, std::shared_ptr<cv::Mat> mat);
	void onStreamFrame(CAGBuffer* hscFrame);

protected:
	AGStreamCB mOnFrame;
	AGStreamCB mOnUserCB = nullptr;

	std::list<AGBlockInfo*> mBlocks[2];
	std::list<AGBlockInfo*>* mCurBlocks = nullptr;
	std::list<AGBlockInfo*>* mReadyBlocks = nullptr;
	int32_t mMaxFrameSize = 0;
	int64_t mTotalFrameCount = 0;
	int64_t mMaxBlockSize = 0;
	int32_t mFrameCount = 0;
	DeviceHandle mHscHandle = nullptr;
	int32_t mSourceID = 0;

	std::recursive_mutex mMutexSource;
	CSingleEvent mWaitFrame;
	CAGBuffer* mCurHSCFrame = nullptr;
	int64_t mCurFrameNo = 0;
	int32_t	mInterval = 0;
	int64_t mWaitFrameNo = -1;
	std::atomic_uint64_t mLastGetFrame{ 0 };
	int64_t mEndFrame = 0;
	int64_t mStartFrame = 0;

	int32_t mFPS = 0;
	int64_t mFrameMircorseconds = 0;
	int64_t mFirstMircorseconds = 0;
	int64_t mEndMircorseconds = 0;
	int64_t mDiffFirstFrameTimestamp = 0;
	bool mFindFirst = false;

	std::shared_ptr<IOCacheObj> mCacheIO;
	AGStreamSource mStreamSource;

	std::vector<int8_t>		mState;
	std::int8_t		mStopped{ eStop };

	//CacheList						mCache;
	int8_t							mCallBackState = -1;
	std::atomic<int32_t>			mCurReadNum = 0;
	int32_t							mReadyReadNum = 0;

	// for debug
	int64_t							mTotalTime = 0;
	int64_t							mCacheStart = 0;
	int64_t							mCacheEnd = 0;
	bool							onStreamFrameFinished = false;
	std::atomic_int64_t				mLastReadFrame{0};
	int64_t							mCrtFrameNo = -1;
	QSharedPointer<Device>			device_ptr_;

	char							*mData = nullptr;
	agile_device::ThreadPool		image_process_executor_{ 4 };
	ImageInfo						m_image_info;
	StreamType						mStreamType = TYPE_H264;
	uint8_t							mHeadFrameType = 1;

	HscSystemElapsedTimer			m_thumbnail_elapsed_timer;

	//导出重试计时器
	HscSystemElapsedTimer			m_export_retry_elapsed_timer;

	// check state thread
    std::atomic_int32_t				mImageProcessFinished {0};
	bool							mForceSeek = false;
	int32_t							mLastRequestFrameno = 0;
	int32_t							mLastRequestFrameCount = 0;

	// for debug
	enum TIMETYPE
	{
		eFirstFrame = 0,
		eStartProcessThread,
		eEndProcessThread,
		eStartProcesser,
		eStartProcess,
		eEndProcess,
		eGetOneFrame
	};
	std::vector<std::vector<uint64_t>>		mStatisticTime;
	uint64_t								mSeekStartTimestamp = 0;
	int										mColorMode = -1;


	int8_t getState(int64_t frame_no)
	{
		if (frame_no >= (int64_t)mState.size()) return 0;
		else return mState[frame_no];
	}

	void Assert(bool rslt, const std::string &err)
	{
		if (!rslt) {
			CSLOG_ERROR(err);
		}
	}

	bool inRange(int64_t frame_no)
	{
		//if (mLastGetFrame == 0 && frame_no >= mCacheStart && frame_no <= mCacheEnd) return true;
		//return false;
		if (mForceSeek) return false;
		if (mInterval == 0) return false;
		int32_t diff = (int32_t)((frame_no - mLastGetFrame) / mInterval);
		if (mLastGetFrame == 0) diff = 0;
		if (diff >= 0 && diff <= 50 && frame_no >= mCacheStart && frame_no <= mCacheEnd) return true;
		return false;
	}

	bool seek(int64_t start_frame_no, int64_t end_frame_no, int32_t interval, bool need_stop = false);
	bool stopStreamSource();

	inline bool approximate(int64_t hscFrameMircorseconds, int64_t firstMircorseconds,int fps)
	{
		if (fps == 0) return  false;
		auto diff = (hscFrameMircorseconds - firstMircorseconds);
		if (mDiffMin <= diff && diff <= mDiffMax) return true;
		//auto difftime = abs(hscFrameMircorseconds - (firstMircorseconds + diffTimestamp));
		//if (difftime < 1e6 / fps) return true;
		return false;
	}

	bool createCache(int64_t size, QString &filename, int32_t frameCount);

	QString getFileName();

    int64_t getLinuxMemoryFree();

	uint64_t mDiffMax = 0;
	uint64_t mDiffMin = 0;
	void getDiffTimestamp(uint32_t beginFrame) {
		if (mFPS == 0) return;  mDiffMax = ((1000000 / mFPS) + 1) * beginFrame; mDiffMin = ((1000000 / mFPS)) * beginFrame;
	}
};

