#include "AGCacheStream.h"
#include "Device/device.h"

#include <QtEndian>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/statfs.h>
#endif
#include <stdlib.h>
#include "MemCacheFile.h"
#include "SegMemCache.h"
#include "Device/imageprocessor.h"
#include "Device/devicemanager.h"
#ifndef _WINDOWS
#include "UtilityNS/UtilityNS.h"
#endif
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <memory>

using namespace AGPlugingDef;

ThumbnailTrack sThumbnailTrack;
SeekTrack sSeekTrack;

tm AGPlugingDef::sTmYear;
uint64_t AGPlugingDef::sSecondsSinceThisYear;
uint64_t AGPlugingDef::sTickStart;
void AGPlugingDef::initHSCTimestamp() {
	auto tNow = std::chrono::system_clock::now();
	auto tSeconds = std::chrono::duration_cast<std::chrono::seconds>(tNow.time_since_epoch());
	time_t secNow = tSeconds.count();
#if defined(_MSC_VER)
	localtime_s(&sTmYear, &secNow);
#else
    localtime_r(&secNow, &sTmYear);
#endif
	sSecondsSinceThisYear = secNow;
	sSecondsSinceThisYear -= (sTmYear.tm_yday * 3600 * 24 + sTmYear.tm_hour * 3600 + sTmYear.tm_min * 60 + sTmYear.tm_sec);
	sTickStart = std::chrono::system_clock::now().time_since_epoch().count();
}
uint64_t AGPlugingDef::HSCTimestamp2Microseconds(FrameTimestamp* hscTimeStamp) {
	if (hscTimeStamp == nullptr) {
		return 0;
	}

	tm tmHSC = sTmYear;
	//tmHSC.tm_mday = hscTimeStamp->mDays;
	tmHSC.tm_yday = hscTimeStamp->mDays >> 8 | (hscTimeStamp->mDays & 0x7f) << 8;
	tmHSC.tm_hour = hscTimeStamp->mHours;
	tmHSC.tm_min = hscTimeStamp->mMins;
	tmHSC.tm_sec = hscTimeStamp->mSeconds;
	//auto day = hscTimeStamp->mDays >> 8 | (hscTimeStamp->mDays & 0x7f) << 8;
	auto microseconds = qFromBigEndian<quint32>((void*)&hscTimeStamp->mMicroseconds) ;
	
	//uint64_t ret = microseconds + (hscTimeStamp->mSeconds + hscTimeStamp->mMins * 60 + hscTimeStamp->mHours * 3600 + day * 24 * 3600) * (1000000);
	auto ret = (uint64_t)mktime(&tmHSC) * 1000000 + microseconds;
	return ret;
}

CSingleEvent::CSingleEvent() {
	mNotifyValue = 0;
}
CSingleEvent::~CSingleEvent() {
	notify(-1);
}
int32_t CSingleEvent::wait(uint64_t waitMS) {
	std::unique_lock<std::mutex> lock(mMutex);
	mNotifyValue = -1;
	if (waitMS > 0) {
		auto ret = mWait.wait_for(lock, std::chrono::milliseconds(waitMS));
		return ret == std::cv_status::timeout ? -1 : mNotifyValue;
	}
	else {
		mWait.wait(lock);
		return mNotifyValue;
	}
}
void CSingleEvent::notify(uint32_t notifyValue) {
	mNotifyValue = notifyValue;
	mWait.notify_all();
};


AGCacheIO::AGCacheIO() {
}
AGCacheIO::~AGCacheIO() {
	closeCache();
}

bool AGCacheIO::createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount) {
	closeCache();
    //rgq
#ifdef _WINDOWS
	mFile = ::CreateFile(path, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (mFile == INVALID_HANDLE_VALUE) {
		mFile = nullptr;
		return false;
	}
	mFilePath = path;
	LARGE_INTEGER filePos;
	filePos.QuadPart = fileSize;
	if (::SetFilePointerEx(mFile, filePos, nullptr, FILE_BEGIN)) {
		::SetEndOfFile(mFile);
		::SetFilePointer(mFile, 0, nullptr, FILE_BEGIN);
	}
	else {
		closeCache();
		return false;
	}
#else
    return false;
#endif

	mFileSize = fileSize;
	mBlocks.resize(frameCount);

	CSLOG_INFO("AGCacheIO::createCache CreateFile: fileSize={}, frameCount={}", fileSize, frameCount);
	return reset();
}
bool AGCacheIO::reset() {
	int64_t blockAddr = 0;
	auto frameCount = (int32_t)mBlocks.size();
	if (frameCount == 0) {
		return true;
	}
	auto frameSize = mFileSize / frameCount;
	for (auto& block : mBlocks) {
		block.mBlockAddr = blockAddr;
		block.mFrameNo = 0;
		block.mFrameLen = 0;
		block.mReady = false;
		blockAddr += frameSize;
	}
	mFreeCount = frameCount;
	return true;
}
bool AGCacheIO::closeCache() {
#ifdef _WINDOWS
	if (mFile) {
		CloseHandle(mFile);
		mFile = nullptr;
	}
#endif
	return true;
}
bool AGCacheIO::isFull() {
	return mFreeCount == 0;
}
AGBlockInfo* AGCacheIO::getFreeBlock() {
	std::lock_guard<std::mutex> lock(mBlockMutex);
	for (auto& block : mBlocks) {
		if (block.mReady == eIdle) {
			block.mReady = eBusy;
			return &block;
		}
	}
	return nullptr;
}
AGBlockInfo* AGCacheIO::write(uint8_t* data, int32_t dataLen) {
	CAGBuffer *buffer = (CAGBuffer*)(data);
	auto block = getFreeBlock();
	if (block == nullptr) {
		return nullptr;
	}
	if (!write(block, data, dataLen, 0)) {
		return nullptr;
	}
	return block;
}
AGBlockInfo* AGCacheIO::write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len) {
	return nullptr;
}
bool AGCacheIO::write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset) {
	if (mFile == nullptr) {
		return false;
	}
//rgq
#ifdef _WINDOWS
	LARGE_INTEGER filePos;
	filePos.QuadPart = (LONGLONG)block->mBlockAddr;
	if (!::SetFilePointerEx(mFile, filePos, nullptr, FILE_BEGIN)) {
		return false;
	}
	WriteFile(mFile, data, dataLen, nullptr, nullptr);
#endif

	if (block->mReady == eBusy) {
		block->mReady = eReady;
		mFreeCount--;
	}
	return true;
}
bool AGCacheIO::read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset) {
	if (mFile == nullptr) {
		return false;
	}
	
	{
		std::lock_guard<std::mutex> lock(mBlockMutex);
		if (block->mReady != eReady) {
			return false;
		}
	}

    //rgq
#ifdef _WINDOWS
	LARGE_INTEGER filePos;
	filePos.QuadPart = (LONGLONG)block->mBlockAddr;
	if (!::SetFilePointerEx(mFile, filePos, nullptr, FILE_BEGIN)) {
		return false;
    }
    ReadFile(mFile, data, dataLen, nullptr, nullptr);
#endif

	return true;
}
void AGCacheIO::freeBlock(AGBlockInfo* block) {
	std::lock_guard<std::mutex> lock(mBlockMutex);
	if (block->mReady) {
		block->mReady = eIdle;
		mFreeCount++;
	}
}



AGStreamSource::AGStreamSource() {
}

AGStreamSource::~AGStreamSource() {
	//if (mHscHandle) {
	//	HscStopExport(mHscHandle);
	//	mHscHandle = nullptr;
	//}
	stop(true);
}

bool AGStreamSource::start(DeviceHandle hscHandle, int32_t recID, int64_t beginFrame, int64_t endFrame, int32_t interval, const AGStreamCB& onFrame) {
	if (sThumbnailTrack.mInRecState) {
#ifdef _WINDOWS
		sThumbnailTrack.mBeginReqRec = GetTickCount64();
#else
        sThumbnailTrack.mBeginReqRec = UtilityNS::CTimeUtil::getTickCount();
#endif
	}
	else {
#ifdef _WINDOWS
		sThumbnailTrack.mBeginReqThumbnail = GetTickCount64();
#else
		sThumbnailTrack.mBeginReqThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
	}
	stop(false);

	do {
		mOnFrame = onFrame;
		CSLOG_INFO("AGStreamSource::start AGStreamSource::start HscStartExport");
		HscResult res = HscStartExport(hscHandle, recID);
		if (HSC_OK != res) {
			CSLOG_INFO("AGStreamSource::start HscStartExport,fail: recID={}, res={}",recID,res);
			break;
		}
		CSLOG_INFO("AGStreamSource::start HscExportByIntervalEx, begin={},end={},interval={}", beginFrame, endFrame, interval);
		res = HscExportByIntervalEx(hscHandle, recID, (DWORD)beginFrame, (DWORD)endFrame, (int)(endFrame - beginFrame) / interval + 1, interval);
		if (HSC_OK != res) {
			CSLOG_INFO("AGStreamSource::start HscExportByIntervalEx,fail: recID={}, res={}", recID, res);
			break;
		}
		mHscHandle = hscHandle;
		mRecID = recID;
		mBeginFrame = beginFrame;
		mEndFrame = endFrame;
		if (sThumbnailTrack.mInRecState) {
#ifdef _WINDOWS
			sThumbnailTrack.mEndReqRec = GetTickCount64();
#else
			sThumbnailTrack.mEndReqRec = UtilityNS::CTimeUtil::getTickCount();
#endif
			sThumbnailTrack.mReqMS += sThumbnailTrack.mEndReqRec - sThumbnailTrack.mBeginReqRec;
		}
		else {
#ifdef _WINDOWS
			sThumbnailTrack.mEndReqThumbnail = GetTickCount64();
#else
			sThumbnailTrack.mEndReqThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
			sThumbnailTrack.mReqMS += sThumbnailTrack.mEndReqThumbnail - sThumbnailTrack.mBeginReqThumbnail;
		}

		mIsRunning = true;
		mStreamSourceState = ePlay;
		mRecvThread = std::make_unique<std::thread>(&AGStreamSource::onRecvThread, this);
		CSLOG_INFO("AGStreamSource::start AGStreamSource::start call onRecvThread");
		return true;
	} while (false);

	HscStopExport(hscHandle);
	return false;
}

bool AGStreamSource::restart(int64_t beginFrame, int64_t endFrame, int32_t interval)
{
	mBeginFrame = beginFrame;
	mEndFrame = endFrame;
	if (mIsRunning) {
		CSLOG_INFO("AGStreamSource::restart error");
		return false;
	}
	
	CSLOG_INFO("AGStreamSource::restart");
	mIsRunning = false;
	if (mRecvThread && mRecvThread->joinable()) {
		mRecvThread->join();
		mRecvThread.reset();
	}
	mIsRunning = true;
	mStreamSourceState = ePlay;
	mRecvThread = std::make_unique<std::thread>(&AGStreamSource::onRecvThread, this);
	return seek(beginFrame, endFrame, interval);
}

bool AGStreamSource::seek(int64_t beginFrame, int64_t endFrame, int32_t interval)
{
	mBeginFrame = beginFrame;
	mEndFrame = endFrame;
	CSLOG_INFO("AGStreamSource::seek HscExportByIntervalEx:begin={},end={},interval={}", beginFrame, endFrame, interval);
#ifdef _WINDOWS
	sSeekTrack.mBeginReqRec = GetTickCount64();
#else
	sSeekTrack.mBeginReqRec = UtilityNS::CTimeUtil::getTickCount();
#endif
	auto res = HscExportByIntervalEx(mHscHandle, mRecID, (DWORD)beginFrame, (DWORD)endFrame, (int)(endFrame - beginFrame) / interval + 1, interval);
#ifdef _WINDOWS
	sSeekTrack.mEndReqRec = GetTickCount64();
#else
	sSeekTrack.mEndReqRec = UtilityNS::CTimeUtil::getTickCount();
#endif
	//Assert(res == HSC_OK, "");
	if (res != HSC_OK) return false;
	return true;
}

bool AGStreamSource::stop(bool need_stop) {
	if (mStreamSourceState == eStop)
	{
		CSLOG_INFO("AGStreamSource::stop, stopped");
		//return true;
	}
	mIsRunning = false;
	if (mRecvThread && mRecvThread->joinable()) {
		mRecvThread->join();
		//mRecvThread.reset();
	}
	if (need_stop && mHscHandle) {
		HscStopExport(mHscHandle);
		mHscHandle = nullptr;
	}
	mStreamSourceState = eStop;
	CSLOG_INFO("StreamSource stop finish");
	return true;
}

bool AGStreamSource::isRunning() {
	return mIsRunning && mStreamSourceState == ePlay;
}

bool AGStreamSource::stopExport()
{
	HscResult res = HSC_ERROR;
	if (mHscHandle)
	{
		res = HscStopExport(mHscHandle);
	}
	return (res == HSC_OK);
}

bool AGStreamSource::startExport(int64_t beginFrame, int64_t endFrame, int32_t interval)
{
	HscResult res = HscStartExport(mHscHandle, mRecID);
	if (HSC_OK != res) {
		return false;
	}
	CSLOG_INFO("AGStreamSource::start HscExportByIntervalEx, begin={},end={},interval={}", beginFrame, endFrame, interval);
	res = HscExportByIntervalEx(mHscHandle, mRecID, (DWORD)beginFrame, (DWORD)endFrame, (int)(endFrame - beginFrame) / interval + 1, interval);
	if (HSC_OK != res) {
		return false;
	}
	return true;
}

bool AGStreamSource::startExport()
{
	HscResult res = HscStartExport(mHscHandle, mRecID);
	if (HSC_OK != res) {
		return false;
	}
	return true;
}

void AGStreamSource::onRecvThread() {
	if (sThumbnailTrack.mInRecState) {
#ifdef _WINDOWS
		sThumbnailTrack.mBeginRecvRec = GetTickCount64();
#else
		sThumbnailTrack.mBeginRecvRec = UtilityNS::CTimeUtil::getTickCount();
#endif
	}
	else {
#ifdef _WINDOWS
		sThumbnailTrack.mBeginRecvThumbnail = GetTickCount64();
#else
		sThumbnailTrack.mBeginRecvThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
	}

	while (mIsRunning) {
		Assert(mThreadCheckFlag == false, "");
		mThreadCheckFlag = true;
#ifdef _WINDOWS
		auto startRecv = GetTickCount64();
#else
		auto startRecv = UtilityNS::CTimeUtil::getTickCount();
#endif

		CAGBuffer* hscFrame = nullptr;
		if (mStreamSourceState == ePlay)
		{
			hscFrame = HscGetFirstFrame(mHscHandle);
		}
		if (hscFrame && mOnFrame && mStreamSourceState == ePlay) {
			mOnFrame(hscFrame);
#ifdef _WINDOWS
			auto curTickcount = GetTickCount64();
#else
			auto curTickcount = UtilityNS::CTimeUtil::getTickCount();
#endif

			if (sSeekTrack.mEndReqRec != 0) {
				if (sSeekTrack.mBeginRecvRec == 0) {
					sSeekTrack.mBeginRecvRec = startRecv;
					sSeekTrack.mFirstRecv = curTickcount;
				}
			}

			if (sThumbnailTrack.mRecvCount == 0) {
				sThumbnailTrack.mFirstRecvMS += curTickcount - sThumbnailTrack.mEndReqThumbnail;
			}
			sThumbnailTrack.mRecvMS += curTickcount - startRecv;
			sThumbnailTrack.mRecvCount++;
		}
		else {
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}
		mThreadCheckFlag = false;
	}
	if (sThumbnailTrack.mInRecState) {
#ifdef _WINDOWS
		sThumbnailTrack.mEndRecvRec = GetTickCount64();
#else
		sThumbnailTrack.mEndRecvRec = UtilityNS::CTimeUtil::getTickCount();
#endif
		sThumbnailTrack.mRecvMS += sThumbnailTrack.mEndRecvRec - sThumbnailTrack.mBeginRecvRec;
	}
	else {
#ifdef _WINDOWS
		sThumbnailTrack.mEndRecvThumbnail = GetTickCount64();
#else
		sThumbnailTrack.mEndRecvThumbnail = UtilityNS::CTimeUtil::getTickCount();
#endif
		sThumbnailTrack.mRecvMS += sThumbnailTrack.mEndRecvThumbnail - sThumbnailTrack.mBeginRecvThumbnail;
	}

	/*if (mHscHandle) {
		HscStopExport(mHscHandle);
		mHscHandle = nullptr;
	}*/
}

void AGStreamSource::pause()
{
	mStreamSourceState = ePause;
}

void AGStreamSource::restart()
{
	mStreamSourceState = ePlay;
}

AGCacheStream::AGCacheStream(QSharedPointer<Device> pDevice, StreamType stream_type)
:
device_ptr_(pDevice)
{
	initHSCTimestamp();
	mCurHSCFrame = new CAGBuffer();
	mOnFrame = [this](CAGBuffer* hscFrame) {
		onStreamFrame(hscFrame);
	};
	mCurBlocks = &(mBlocks[0]);
	mReadyBlocks = &(mBlocks[1]);
	mStreamType = stream_type;
	mHeadFrameType = device_ptr_->getFrameHeadType();
}

AGCacheStream::~AGCacheStream() {
	stop(false);

	if (mCacheIO) mCacheIO->closeCache();
	if (mCurHSCFrame){
		delete mCurHSCFrame;
		mCurHSCFrame = nullptr;
	}
	if (mData) {
		delete[]mData;
		mData = nullptr;
	}
}

static const int64_t TestFileSize = (int64_t)2 * 1024 * 1024 * 1024;	// 2 G
bool AGCacheStream::start(DeviceHandle hscHandle, int32_t recID, int64_t beginFrame, int64_t endFrame, int32_t interval, int32_t maxFrameSize, int32_t fps, const AGStreamCB onUserCB) {
	mOnUserCB = onUserCB;
	stop(false);
	mCallBackState = 0;
	mStopped = ePlay;
	std::lock_guard<std::recursive_mutex> lock(mMutexSource);
	mMaxFrameSize = maxFrameSize;
	mTotalFrameCount = (endFrame - beginFrame) / interval + 1;
	mMaxBlockSize = mTotalFrameCount;
	if (mMaxBlockSize > 100) mMaxBlockSize = mTotalFrameCount / 2;

	mCurFrameNo = 0;
	mInterval = interval;
	mFPS = fps;
	/************for debug**************/
	//if (mStatisticTime.size() < endFrame - beginFrame + 1)
	//{
	//	mStatisticTime.resize(endFrame - beginFrame + 1);
	//	for (uint32_t i = 0; i < mStatisticTime.size(); i++) mStatisticTime[i].resize(7, 0);
	//}
	/**********************************/
	if (mFPS > 0)
	{
		mFrameMircorseconds = 1000000 / mFPS;
		getDiffTimestamp(beginFrame); 
	}
	
	mCrtFrameNo = -1;
	mLastGetFrame = 0;
	mStartFrame = beginFrame;
	mEndFrame = endFrame;
	mFindFirst = false;

	mCurBlocks = &(mBlocks[0]);
	mReadyBlocks = &(mBlocks[1]);

	mFrameCount = (int32_t)(TestFileSize / mMaxFrameSize) >= mMaxBlockSize ? mMaxBlockSize : (int32_t)(TestFileSize / mMaxFrameSize);
	auto fileSize = (int64_t)mFrameCount * mMaxFrameSize;

	if (mState.size() < endFrame - beginFrame + mInterval) mState.resize(endFrame - beginFrame + mInterval, 0);
	else mState.resize(mState.size(), 0);

	QString file_name = getFileName();
	bool rslt = createCache(fileSize, file_name, mFrameCount);
	if (!rslt) return false;
	
	/*if (!mCacheIO)
	{
		mCacheIO = std::make_shared<AGCacheIO>();
		if (!mCacheIO->createCache(filepath.data(), fileSize, frameCount)) {
			mHscHandle = nullptr;
			return false;
		}
	}*/
	mHscHandle = hscHandle;
	mSourceID = recID;

	mData = new char[maxFrameSize];
	m_export_retry_elapsed_timer.restart();
	mStreamSource.start(mHscHandle, mSourceID, beginFrame, endFrame, mInterval, mOnFrame);
	mCacheStart = beginFrame;
	mCacheEnd = endFrame;


	CSLOG_INFO("AGCacheStream::start success.");
	return true;
}

bool AGCacheStream::stop(bool stop) {
	if (mStopped == eStop) {
		CSLOG_INFO("AGCacheStream::stop stopped");
		return true;
	}
	mStopped = eStop;

	mStreamSource.stop(stop);
	while (1)
	{
		if (mImageProcessFinished == 0) break;
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		continue;
	}
	image_process_executor_.reset();

	mWaitFrame.notify();
	if(mCacheIO) mCacheIO->reset();
	std::lock_guard<std::recursive_mutex> lock(mMutexSource);
	mCurBlocks->clear();
	CSLOG_INFO("CacheStream stop finish");
	//mReadyBlocks->clear();
	return true;
}

CAGBuffer* AGCacheStream::getFrame(int64_t frame_no) 
{
	if (mStopped == eStop) {
		return nullptr;
	}
	
	//if (mLastGetFrame != frame_no) CSLOG_INFO("AGCacheStream::getFrame {},{}", mLastGetFrame, frame_no);
	
	//CAGBuffer *cache_buffer = nullptr;
	std::chrono::time_point<std::chrono::system_clock> tpStart = std::chrono::system_clock::now();
	int64_t time_count = 0;

	auto block  = getFrameBlock(frame_no);	
	if (block == nullptr && inRange(frame_no))
	{
		return nullptr;
	}
	//AGBlockInfo *block = nullptr;
	if (block == nullptr) {
		{
			std::lock_guard<std::recursive_mutex> lock(mMutexSource);
			CSLOG_INFO("AGCacheStream::getFrame call seek.");
			swapReadyBlocks();
		}
		seek(frame_no, mEndFrame, mInterval);
		
		mWaitFrameNo = frame_no;
		if (mStopped != eStop) mWaitFrame.wait(5000);
		mCurFrameNo = frame_no;
		if (mWaitFrameNo != -1 || !mCurHSCFrame) return nullptr;
		++mCurReadNum;

		/******************for debug*************/
		auto hscTimestamp = (FrameTimestamp*)(mCurHSCFrame->frame_head.time_stamp);
		auto hscFrameMircorseconds = HSCTimestamp2Microseconds(hscTimestamp);
		/*****************************/
		mLastGetFrame = frame_no;
		return mCurHSCFrame;
	}
	Assert(block->mFrameNo == frame_no, "");
	if (!mCacheIO || !mCacheIO->read(block, (uint8_t*)mCurHSCFrame, mMaxFrameSize, 0)) {
		block->mReady = eReady;
		return nullptr;
	}

	/******************for debug*************/
	auto hscTimestamp = (FrameTimestamp*)(mCurHSCFrame->frame_head.time_stamp);
	auto hscFrameMircorseconds = HSCTimestamp2Microseconds(hscTimestamp);
	/*****************************/
	++mCurReadNum;
	block->mReady = eReady;
	mLastGetFrame = frame_no;
	return mCurHSCFrame;
}

bool AGCacheStream::seek(int64_t start_frame_no, int64_t end_frame_no, int32_t interval, bool need_stop)
{
	if (mFPS > 0) getDiffTimestamp(start_frame_no);
	if (end_frame_no < start_frame_no) end_frame_no = start_frame_no;
	mFindFirst = false;
	mCrtFrameNo = -1;

	if (need_stop)
	{
		mStreamSource.stopExport();
		mStreamSource.startExport();
	}
	bool rslt = mStreamSource.seek(start_frame_no, end_frame_no, interval);
	if (!rslt) return false;

	mCacheStart = start_frame_no;
	mCacheEnd = end_frame_no;
	mForceSeek = false;
	return true;
}

int64_t AGCacheStream::frameNoFromMicroseconds(int64_t microseconds) 
{
	if (mFPS > 0) return ceil((microseconds - mFirstMircorseconds) / (1e6/mFPS));
	else return 0;
}

void AGCacheStream::swapReadyBlocks() 
{
	for (int i = 0; i < mCurBlocks->size(); i++)
	{
		popFirstBlock();
	}
	auto temp = mCurBlocks;
	mCurBlocks = mReadyBlocks;
	//mReadyBlocks = temp;
}

AGBlockInfo* AGCacheStream::getFrameBlock(int64_t frameNo) 
{
	std::lock_guard<std::recursive_mutex> lock(mMutexSource);
	//if (!getState(frameNo)) return nullptr;

	if (!mCurBlocks || mCurBlocks->empty()) {
		return nullptr;
	}

	for (auto block : *mCurBlocks) {
		if (block->mFrameNo == frameNo && block->mReady == eReady) {
			block->mReady = eReadyRead;
			return block;
		}
	}
	return nullptr;
}

bool AGCacheStream::popFirstBlock() 
{
	if (!mCurBlocks || mCurBlocks->empty()) return false;
	auto block = *mCurBlocks->rbegin();
	if (!block || block->mReady == eReadyRead) {
		auto state = -1;
		auto frameno = 0;
		if (block) {
			state = block->mReady; 
			frameno = block->mFrameNo;
		}
		//CSLOG_INFO("popFirstBlock no block pop {},{}", frameno, state);
		return false;
	}
	
	auto frameno = block->mFrameNo;
	//if (mLastGetFrame < frameno && mCurBlocks->size() < mMaxBlockSize-10) return true;
	//setState(frameno, 0);

	if (!mCacheIO) return false;
	mForceSeek = true;
	mCacheIO->freeBlock(block);
	mCurBlocks->pop_back();
	// CSLOG_INFO("AGCacheStream::popFirstBlock {},{},{}", mCurFrameNo, frameno, mCurBlocks->size());
	return true;
}

void AGCacheStream::addToCurBlocks(CAGBuffer* hscFrame)
{
	auto image_processor_ptr = device_ptr_->getProcessor();

	if (sSeekTrack.mEndReqRec != 0) {
		if (sSeekTrack.mFirstProcess == 0) {
#ifdef _WINDOWS
			sSeekTrack.mFirstProcess = GetTickCount64();
#else
			sSeekTrack.mFirstProcess = UtilityNS::CTimeUtil::getTickCount();
#endif
        }

	}

	cv::Mat buffer_mat = cv::Mat(1, mMaxFrameSize, CV_8UC1, hscFrame).clone();
	image_process_executor_.commit(std::bind(&AGCacheStream::addToCurBlocksProcess, this, image_processor_ptr, std::make_shared<cv::Mat>(std::move(buffer_mat))));
	
	//根据缓存占用等待一会,最长1秒,减缓写入缓存速度
	uint16_t time_wait_ms = 1000;
	if (mCurBlocks->size() == 0) {
		time_wait_ms = 0;
	}
	else {
		time_wait_ms *= (mCurBlocks->size() - mCurReadNum)*1.0 / mMaxBlockSize;
	}	
	boost::this_thread::sleep_for(boost::chrono::milliseconds(time_wait_ms));
	return;
}

void AGCacheStream::onStreamFrame(CAGBuffer* hscFrame) 
{
	//CSLOG_INFO("AGCacheStream::onStreamFrame recv callback{}", mWaitFrameNo);
	if (mStopped == eStop) {
		onStreamFrameFinished = true;
		return;
	}

	onStreamFrameFinished = false;
	auto tpStart = std::chrono::system_clock::now();
	uint64_t start_time = getTimestamp();
	auto hscTimestamp = (FrameTimestamp*)(hscFrame->frame_head.time_stamp);
	uint64_t hscFrameMircorseconds = 0;
	if (mHeadFrameType == 0)
	{
		hscFrameMircorseconds = DeviceUtils::getTimestampTons(hscFrame->frame_head.time_stamp);
	}
	else
	{
		hscFrameMircorseconds = HSCTimestamp2Microseconds(hscTimestamp);
	}
	
	uint32_t need_use_frame_no = 1;
	//if (mStreamType == TYPE_H264 && mHeadFrameType != 1)  need_use_frame_no = 0;
	uint32_t frameNo = (hscFrame->frame_head.frameno);
	if (!need_use_frame_no)
	{
		if (mFirstMircorseconds == 0 && hscFrame->frame_head.frameno == 1) {
			mFirstMircorseconds = hscFrameMircorseconds - mCacheStart * mFrameMircorseconds;
			mEndMircorseconds = mFirstMircorseconds + mTotalFrameCount * mFrameMircorseconds;
			//CSLOG_INFO("=================================find first frame:{},{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, hscFrame->frame_head.frameno, mCacheStart, mTotalTime);
		}

		auto nIndex = frameNoFromMicroseconds(hscFrameMircorseconds);
		frameNo = nIndex;
		if (nIndex < 0) {
			//CSLOG_INFO("=================================filter data frame err:{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, hscFrame->frame_head.frameno, mCacheStart);
			return;
		}
		if ((frameNo == mCacheStart || approximate(hscFrameMircorseconds, mFirstMircorseconds, mFPS)) && hscFrame->frame_head.frameno == 1) {
			CSLOG_INFO("AGCacheStream::onStreamFrame, find first frame:{},{},{},{},{},{}",
				mFirstMircorseconds, hscFrameMircorseconds, mDiffMax, hscFrame->frame_head.frameno, mCacheStart, mTotalTime);
			mFindFirst = true;
			//mStatisticTime[frameNo][eFirstFrame] = getTimestamp() - mSeekStartTimestamp;
			mTotalTime = 0;
		}
		else if (!mFindFirst)
		{
			//CSLOG_INFO("filter data:{},{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, frameNo, hscFrame->frame_head.frameno, mCacheStart);
			mTotalTime += std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::duration<double>(std::chrono::system_clock::now() - tpStart)).count();
			onStreamFrameFinished = true;
			return;
		}

		if (mCrtFrameNo == -1) mCrtFrameNo = mCacheStart;
		else mCrtFrameNo += mInterval;
		//frameNo = mCrtFrameNo;
		mCrtFrameNo = frameNo;
		hscFrame->frame_head.frameno = frameNo;
	}
	else
	{
		frameNo = frameNo - 1;
		if (mCrtFrameNo == -1 && frameNo == mCacheStart)
		{
			CSLOG_INFO("[PLAYBACK] =================================find first frame:{},{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, hscFrame->frame_head.frameno, mCacheStart, frameNo);
			mFindFirst = true;
			//mStatisticTime[frameNo][eFirstFrame] = getTimestamp() - mSeekStartTimestamp;
			mTotalTime = 0;
		}
		else if (!mFindFirst)
		{
			//CSLOG_INFO("filter data:{},{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, frameNo, hscFrame->frame_head.frameno, mCacheStart);
			mTotalTime += std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::duration<double>(std::chrono::system_clock::now() - tpStart)).count();
			onStreamFrameFinished = true;
			return;
		}
		else if (mCrtFrameNo + mInterval != frameNo)
		{
			CSLOG_WARN("WARN data:timestamp: {}, frameno:{}, head_frame_no:{},crt_frameno:{}, cache_start:{}", hscFrameMircorseconds, frameNo, hscFrame->frame_head.frameno, mCrtFrameNo, mCacheStart);
		}
		mCrtFrameNo = frameNo;
		hscFrame->frame_head.frameno = frameNo;
	}
	//CSLOG_INFO("recv data:{},{},{},{},{},{}", mFirstMircorseconds, hscFrameMircorseconds, mDiffFirstFrameTimestamp, frameNo, hscFrame->frame_head.frameno, mCacheStart);
	addToCurBlocks(hscFrame);
}

bool AGCacheStream::restart(int64_t beginFrame, int64_t endFrame, int32_t interval, int32_t maxFrameSize, int32_t fps)
{
	mStopped = false;

	mTotalFrameCount = (endFrame - beginFrame) / interval + 1;
	mCurFrameNo = 0;
	mInterval = interval;
	mFPS = fps;
	
	if (mFPS > 0) {
		getDiffTimestamp(beginFrame);
		mFrameMircorseconds = 1000000 / mFPS;
	}

	mLastGetFrame = 0;
	mCrtFrameNo = -1;
	mStartFrame = beginFrame;
	mEndFrame = endFrame;
	mFindFirst = false;

	mCurBlocks = &(mBlocks[0]);
	mReadyBlocks = &(mBlocks[1]);
	auto maxBlockSize = mTotalFrameCount;
	if (maxBlockSize > 100) maxBlockSize = mTotalFrameCount / 2;
	auto frameCount = (int32_t)(TestFileSize / maxFrameSize) >= maxBlockSize ? maxBlockSize : (int32_t)(TestFileSize / maxFrameSize);

	if (mFrameCount < frameCount || maxFrameSize != mMaxFrameSize)
	{
		auto fileSize = frameCount * maxFrameSize;
		if (mState.size() < endFrame - beginFrame + mInterval) mState.resize(endFrame - beginFrame + mInterval, 0);
		else mState.assign(mState.size(), 0);

		mFrameCount = frameCount;
		mMaxBlockSize = maxBlockSize;
		mMaxFrameSize = maxFrameSize;

		QString file_name = getFileName();
		bool rslt = createCache(fileSize, file_name, mFrameCount);
		if (!rslt) return false;
	}
	mStreamSource.stopExport();
	mStreamSource.startExport();
	mStreamSource.restart(beginFrame, endFrame, mInterval);
	mCacheStart = beginFrame;
	mCacheEnd = endFrame;
	mStopped = ePlay;
	return true;
}

bool AGCacheStream::stopStreamSource()
{
	//mStopped = true;
	mWaitFrame.notify();
	mStreamSource.pause();
	//return mStreamSource.stop(true);
	return true;
}

void AGCacheStream::pause()
{
	mStopped = ePause;
	mStreamSource.pause();
	mForceSeek = true;
}

void AGCacheStream::restart()
{
	mStopped = ePlay;
	mStreamSource.restart();
}


char* AGCacheStream::getFrameProcessed(int64_t frame_no, ImageInfo &info)
{
	uint64_t t1 = getTimestamp();
	uint64_t t2 = 0;

	if (mStopped == eStop) {
		CSLOG_INFO("AGCacheStream::getFrameProcessed {},{},{}", mLastGetFrame, frame_no, mStopped);
		return nullptr;
	}

	std::chrono::time_point<std::chrono::system_clock> tpStart = std::chrono::system_clock::now();
	int64_t time_count = 0;

	auto block = getFrameBlock(frame_no);
	if (block == nullptr && inRange(frame_no)) {

		if (!m_thumbnail_elapsed_timer.hasExpired(1000)) return nullptr;

	}
	if (block == nullptr) {

		//CSLOG_INFO("[PLAYBACK] AGCacheStream::getFrame start request: {},{},{},{}", mLastGetFrame, frame_no, mCacheStart, mCacheEnd);
		{
			std::lock_guard<std::recursive_mutex> lock(mMutexSource);
			CSLOG_INFO("AGCacheStream::getFrameProcessed call seek.");
			swapReadyBlocks();
		}
		if (!mStreamSource.isRunning())
		{
			std::lock_guard<std::recursive_mutex> lock(mMutexSource);
			int32_t i = 0;
			for (i = 0; i < 100; i++)
			{
				bool rslt = popFirstBlock();
				if (!rslt) break;
			}
			mCurReadNum = 0;
			mStreamSource.restart();
		}
		if (mStreamSource.isRunning())
		{
			std::lock_guard<std::recursive_mutex> lock(mMutexSource);
			int32_t i = 0;
			for (i = 0; i < mCurReadNum; i++)
			{
				bool rslt = popFirstBlock();
				if (!rslt) break;
			}
			mCurReadNum = 0;
		}
		if (mEndFrame < frame_no) mEndFrame = frame_no;
		auto stop_state = mStopped;
		mStopped = eSeek;
		while (1)
		{
			if (mImageProcessFinished == 0) break;
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
			continue;
		}
		image_process_executor_.reset();
		seek(frame_no, mEndFrame, mInterval, m_export_retry_elapsed_timer.hasExpired(10000));
		mStopped = stop_state;

		mWaitFrameNo = frame_no;
		if (mStopped != eStop && mStreamSource.isRunning()) mWaitFrame.wait(5000);
		mCurFrameNo = frame_no;
		if (mWaitFrameNo != -1 || !mData) return nullptr;
		++mCurReadNum;
	
		t2 = getTimestamp();
		//mStatisticTime[frame_no][eGetOneFrame] = t2 - t1;
		//CSLOG_INFO("[PLAYBACK] AGCacheStream::getFrame return response: {},{}, total_time:", mLastGetFrame, frame_no, t2 - t1);
		mLastGetFrame = frame_no;
		info = m_image_info;
		m_thumbnail_elapsed_timer.restart();
		return mData;
	}

	Assert(block->mFrameNo == frame_no, "block frame no is invalid");
	if (!mCacheIO || !mCacheIO->read(block, (uint8_t*)mData, mMaxFrameSize, 0)) {
		block->mReady = eReady;
		CSLOG_ERROR("read from cache failed:{},{}", frame_no, block->mBlockAddr);
		return nullptr;
	}

	//缓存降到一定比例再继续取流
	if (!mStreamSource.isRunning())
	{
		float restart_buffer_rate = 0.8;
		if (mCurBlocks->size() <= mFrameCount * restart_buffer_rate)
		{
			CSLOG_INFO("has space to cache, so restart, current cache size:{},mMaxBlockSize:{} \n", mCurBlocks->size(), mMaxBlockSize);

			mStreamSource.restart();
		}
	}

	++mCurReadNum;
	block->mReady = eReady;
	mLastGetFrame = frame_no;
	info = m_image_info;

	/*********for debug*******/
	auto hscTimestamp = (FrameTimestamp*)(mData);
	auto hscFrameMircorseconds = HSCTimestamp2Microseconds(hscTimestamp);
	/****************/
	t2 = getTimestamp();
	//mStatisticTime[frame_no][eGetOneFrame] = t2 - t1;
	//CSLOG_INFO("[PLAYBACK] AGCacheStream::getFrame from cache: {},{}, total_time:", frame_no, block->mBlockAddr, t2 - t1);
	m_thumbnail_elapsed_timer.restart();
	m_export_retry_elapsed_timer.restart();
	return mData;
}


void AGCacheStream::addToCurBlocksProcess(std::shared_ptr<ImageProcessor> image_processor_ptr, std::shared_ptr<cv::Mat> mat){
	mImageProcessFinished++;
	if (mStopped == eStop || mStopped == eSeek) {
		mImageProcessFinished--;
		return;
	}
#ifdef _WINDOWS
	auto startProcess = GetTickCount64();
#else
	auto startProcess = UtilityNS::CTimeUtil::getTickCount();
#endif
	auto tp = getTimestamp();
	
	CAGBuffer *hscFrameBuf = (CAGBuffer*)(mat->data);
	if (!hscFrameBuf){
		mImageProcessFinished--;
		CSLOG_ERROR("image_mat is invalid ");
		return;
	}

	//mStatisticTime[hscFrameBuf->frame_head.frameno][eStartProcess] = tp;

	cv::Mat image_mat = image_processor_ptr->cv_process(hscFrameBuf, 3, true);

	int32_t mat_size = image_mat.cols * image_mat.rows * image_mat.channels() *((image_mat.depth() ==CV_16U)?2:1);

//	int32_t data_len = mMaxFrameSize - sizeof(IOCacheOBJ::FrameInfo);
// 	if (data_len != mat_size) {
// 		mImageProcessFinished--;
// 		CSLOG_ERROR("image is invalid: mat_size:{}, data_len:{}", mat_size, data_len);
// 		return;
// 	}

	//Assert(image_mat.data != nullptr, "");
	if (m_image_info.cols_ == 0 || image_mat.type() != m_image_info.type_) {
		m_image_info = ImageInfo(image_mat.rows, image_mat.cols, image_mat.type(), image_mat.step);
	}
	auto frame_no = hscFrameBuf->frame_head.frameno;
	if (frame_no == mWaitFrameNo){
		memcpy(mData, hscFrameBuf->frame_head.time_stamp, sizeof(IOCacheOBJ::FrameInfo));
		memcpy(mData + sizeof(IOCacheOBJ::FrameInfo), image_mat.data, mat_size);

		mWaitFrameNo = -1;
		mWaitFrame.notify();
		if (sSeekTrack.mEndReqRec != 0) {
			if (sSeekTrack.mFirstRet == 0) {
#ifdef _WINDOWS
				sSeekTrack.mFirstRet = GetTickCount64();
#else
				sSeekTrack.mFirstRet = UtilityNS::CTimeUtil::getTickCount();
#endif
			}
		}
	}
	{
		std::lock_guard<std::recursive_mutex> lock(mMutexSource);
		if (!mCacheIO) {
			mImageProcessFinished--;
			return;
		}
		if ((int32_t)mCurBlocks->size() >= mMaxBlockSize || mCacheIO->isFull()) {
			// no space to cache, so stop
			if (mCurReadNum <= 0 || mStopped == ePause)	{
				mStreamSource.pause();
				mImageProcessFinished--;
				CSLOG_INFO("no space to cache, so stop, current cache size:{},mMaxBlockSize:{} \n", mCurBlocks->size(), mMaxBlockSize);
				//stopStreamSource();
				return;
			}

			int32_t i = 0;
			for (i = 0; i < mCurReadNum; i++){
				bool rslt = popFirstBlock();
				if (!rslt) break;
			}
			CSLOG_INFO("clean cache bolcks:{} \n", mCurReadNum);
			mCurReadNum -= (i + 1);
		}
	}

	IOCacheOBJ::FrameInfo info;
	memcpy(info.m_timestamp_, hscFrameBuf->frame_head.time_stamp, sizeof(info.m_timestamp_));
	Assert(frame_no == hscFrameBuf->frame_head.frameno, "");
	auto block = mCacheIO->write((uint8_t*)image_mat.data, mat_size, (uint8_t*)&info, sizeof(info));
	if (block == nullptr) {
		// no space to cache, so stop
		//mWaitFrame.notify();
		//mStreamSource.stop(true);
		CSLOG_WARN("write data error: {}", frame_no);
		stopStreamSource();
		mImageProcessFinished--;
		return;
	}

	Assert(frame_no == hscFrameBuf->frame_head.frameno, "");
	block->mFrameNo = hscFrameBuf->frame_head.frameno;
	block->mFrameLen = (int32_t)mMaxFrameSize;
	{
		std::lock_guard<std::recursive_mutex> lock(mMutexSource);
		mCurBlocks->emplace_front(block);
		 CSLOG_INFO("thumbnail - mCurBlocks->emplace_front:{},{},{}", frame_no, mCurBlocks->size(), block->mBlockAddr);
		//mLastGetFrame = frameNo;
		//setState(frame_no, 1);
	}

#ifdef _WINDOWS
	sThumbnailTrack.mProcessMS += GetTickCount64() - startProcess;
#else
	sThumbnailTrack.mProcessMS += UtilityNS::CTimeUtil::getTickCount() - startProcess;
#endif
	sThumbnailTrack.mProccessCount++;
	mImageProcessFinished--;
	//CSLOG_INFO("[PLAYBACK] save to cache:{},{},{}", frame_no, mCurBlocks->size(), block->mBlockAddr);
	if (mOnUserCB && mStopped != eStop && mFindFirst) {
		mOnUserCB(hscFrameBuf);
	}

}


bool AGCacheStream::createCache(int64_t fileSize, QString &file_name, int32_t frameCount)
{
	//try
	//{
	//	mCacheIO = std::make_shared<SegMemCache>(fileSize, frameCount, mMaxFrameSize);
	//}
	//catch (std::exception &err)
	//{
	//	CSLOG_ERROR("AGCacheStream::createCache create SegMemCache erro:{},{},{}", fileSize, frameCount, mMaxFrameSize);
	//	return false;
	//}

	//return true;

	// 检查可用内存
	uint8_t flag = 0;
#ifdef _WINDOWS
	MEMORYSTATUSEX memoryStateEx;
	memoryStateEx.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memoryStateEx);
	if (memoryStateEx.ullAvailPhys < TestFileSize * 2)
	{
		flag = 2;
		if (file_name != "")
		{
			LPCWSTR lpcwstrFile = (LPCWSTR)(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).utf16();
			ULARGE_INTEGER liFreeBytes, liTotalBytes, liTotalFreeBytes;
			bool rslt = GetDiskFreeSpaceEx(lpcwstrFile, &liFreeBytes, &liTotalBytes, &liTotalFreeBytes);
			if (!rslt) flag = 2;

			if (liTotalFreeBytes.QuadPart < TestFileSize * 1.5) flag = 2;
			else flag = 1;
		}
	}
	
	if (flag == 2 || flag == 0)
	{
		file_name = "";
		if (flag == 2) fileSize = mMaxFrameSize * 10 < memoryStateEx.ullAvailPhys / 2 ? mMaxFrameSize * 10 : memoryStateEx.ullAvailPhys / 2;
    }
#else
    //rgq:
    uint64_t uiAvailableDisk = getLinuxMemoryFree();
    if(uiAvailableDisk < TestFileSize * 2)
    {
        fileSize = mMaxFrameSize * 10 < uiAvailableDisk / 2 ? mMaxFrameSize * 10 : uiAvailableDisk / 2;
    }
#endif

	//mCacheIO = std::make_shared<MemCacheFile>(fileSize, file_name, mFrameCount);
	mCacheIO = std::make_shared<SegMemCache>(fileSize, frameCount, mMaxFrameSize);
	if (!mCacheIO) {
		CSLOG_ERROR("AGCacheStream::createCache new Cache failed: size:{}, filename:{}, count:{}", fileSize, file_name.toStdString(), mFrameCount);
		return false;
	}

	auto size = mCacheIO->getMemSize();
	if (size > 0) {
		CSLOG_INFO("AGCacheStream::createCache new Cache finished size:{}, filename:{}, count:{}", fileSize, file_name.toStdString(), mFrameCount);
		return true;
	}

	CSLOG_ERROR("AGCacheStream::createCache new Cache failed: size:{}, filename:{}, count:{}", fileSize, file_name.toStdString(), mFrameCount);
	return false;
}


QString AGCacheStream::getFileName()
{
	QString file_name = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
	auto ms = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	file_name += "/playBackCache_";
	file_name += QString::number(ms);
	return file_name;
}


int64 AGCacheStream::getLinuxMemoryFree()
{
    int64 nRet = -1;
#ifndef _WINDOWS
#if 0
    FILE *file = fopen("/proc/meminfo", "r");
    if(file == nullptr)
    {
        return nRet;
    }
    char str1[20];
    char str2[20];
    char str3[20];
    int64_t nMemToal = 0, nMemAvaillable = 0, nMemFree = 0;
    fscanf(file, "MemTotal: %s kB\n", str1);
    nMemToal = atol(str1) / 1000;

    fscanf(file, "MemFree: %s kB\n", str3);
    nMemFree = atol(str3) / 1000;

    fscanf(file, "MemAvailable: %s kB\n", str2);
    nMemAvaillable= atol(str2) / 1000;
    fclose(file);
    return nMemFree;
#else
    std::ifstream meminof("/proc/meminfo");
    std::string strLine;
    while(std::getline(meminof, strLine))
    {
        if (strLine.compare(0, 7, "MemFree") == 0)
        {
            auto nStart = strLine.find_first_of("0123456789");
            auto nEnd = strLine.find_last_of("0123456789");
            std::string strValue = strLine.substr(nStart, nEnd);
            return atol(strValue.c_str()) / 1000;
        }
    }
#endif

#endif
    return nRet;
}

