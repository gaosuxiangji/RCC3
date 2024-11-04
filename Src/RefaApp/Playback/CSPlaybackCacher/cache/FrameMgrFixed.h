#ifndef FRAMEMGR_FIXED_H_
#define FRAMEMGR_FIXED_H_
#include "MemFile.h"
#include "./FIFOCache.h"
#include <vector>


struct FrameInfo
{
	uint8_t  m_timestamp_[9];
};

class FrameMgrFixed
{
	enum
	{
		//eMaxFileSize = 1024 * 1024 * 1024 * 2,
		eMaxGroupSize = 100000,
	};


private:
	std::shared_ptr<MemFile>		mMemFile = nullptr;
	
	int64_t							m_start_frame_no_ = 0;
	int64_t							m_end_frame_no_ = 0;
	int32_t							mCount = 0;
	int32_t							mFrameSize = 0;
	uint64_t						mFileSize = 0;
	int32_t							mInterval = 0;

	int64_t							mSaveFrameNum = 0;
	uint64_t						mCrtOffset = 0;
	int64_t							mCrtIndex = 0;
	uint64_t						mMaxFileSize = 0;

	agile_device::FIFOCache<int64_t, int64_t> mFrame2Idx;	// key: frame_no value: crtidx
	std::vector<std::vector<uint8_t>>	mStates;
	std::vector<std::vector<FrameInfo>>	mFrameInfos;

public:
	FrameMgrFixed();
	~FrameMgrFixed();
	FrameMgrFixed(int32_t real_count, int32_t frame_size, int count, int interval, const QString &file_name);

	bool readData(int64_t frame_no, char *data, int len, FrameInfo &info);
	bool writeData(int64_t frame_no, const char *data, int64_t len, const FrameInfo &info);

private:
	inline bool isExist(int64_t frame_no) {
		int32_t group_idx = (frame_no / mInterval) / eMaxGroupSize;
		if (group_idx < 0 || group_idx >= mStates.size()) return false;

		int32_t pos = (frame_no/ mInterval) % eMaxGroupSize;
		if (pos < 0 || pos >= mStates[group_idx].size() || !mStates[group_idx][pos]) return false;
		return true;
	};

	bool getIndex(int64_t frame_no, int32_t &groupidx, int32_t &pos);

};

#endif
