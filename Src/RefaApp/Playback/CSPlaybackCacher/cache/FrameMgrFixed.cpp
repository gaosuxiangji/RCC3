#include "FrameMgrFixed.h"



FrameMgrFixed::FrameMgrFixed()
:
mFrame2Idx(0)
{
}

FrameMgrFixed::~FrameMgrFixed()
{

}

FrameMgrFixed::FrameMgrFixed(int32_t real_count, int32_t frame_size, int count, int interval, const QString &file_name)
:
mCount(real_count),
mFrameSize(frame_size),
mFileSize(real_count * frame_size),
mInterval(interval),
mFrame2Idx(0)
{
	mMaxFileSize = 2000 *  1024 * 1024 ;
	if (mFileSize > mMaxFileSize) mFileSize = mMaxFileSize;
	mMemFile = std::make_shared<MemFile>(mFileSize, file_name);

	mSaveFrameNum = mFileSize / mFrameSize;
	mFrame2Idx.resize(mSaveFrameNum);

	int group_num = count / eMaxGroupSize + 1;
	mStates.resize(group_num);
	mFrameInfos.resize(group_num);
	for (int i = 0; i < group_num; i++)
	{
		if (i < group_num - 1) {
			mStates[i].resize(eMaxGroupSize, 0);
			mFrameInfos[i].resize(eMaxGroupSize);
		}
		else {
			mStates[i].resize(count % eMaxGroupSize, 0);
			mFrameInfos[i].resize(count % eMaxGroupSize);
		}
	}
}


bool FrameMgrFixed::writeData(int64_t frame_no, const char *data, int64_t len, const FrameInfo &info)
{
	if (!mMemFile) return false;
	if (isExist(frame_no)) return true;

	if (mFrameSize > 0 && len != mFrameSize) return false;
	if (mCrtIndex < mCount && mCrtOffset + len <= mFileSize)  // 文件没有写满
	{
		bool rslt = mMemFile->writeData(mCrtOffset, data, len);
		if (!rslt) return false;
	
		int groupidx = -1, pos = -1;
		rslt = getIndex(frame_no, groupidx, pos);
		if (!rslt) return false;

		mStates[groupidx][pos] = 1;
		mFrameInfos[groupidx][pos] = info;
		mFrame2Idx.put(frame_no, mCrtIndex);
		mCrtOffset += len;
		++mCrtIndex;
	}
	else  // 文件写满，覆盖以前的
	{
		int64_t old_frameno = -1;
		int64_t index = -1;
		mFrame2Idx.getFirstKeyValue(old_frameno, index);
		mFrame2Idx.put(frame_no, index);
		uint64_t offset = index * mFrameSize;
		bool rslt = mMemFile->writeData(offset, data, len);

		int groupidx = -1, pos = -1;
		rslt = getIndex(old_frameno, groupidx, pos);
		mStates[groupidx][pos] = 0;

		rslt = getIndex(frame_no, groupidx, pos);
		mStates[groupidx][pos] = 1;
		mFrameInfos[groupidx][pos] = info;
	}
	return true;
}

bool FrameMgrFixed::readData(int64_t frame_no, char *data, int len, FrameInfo &info)
{
	if (!mMemFile || !isExist(frame_no))
	{
		return false;
	}
	if (len != mFrameSize) return false;
	int64_t idx = -1;
	bool rslt = mFrame2Idx.get(frame_no, idx);
	if (!rslt) return false;

	uint64_t offset = idx * mFrameSize;
	rslt = mMemFile->readData(offset, data, len);

	int32_t group_idx = -1;
	int32_t pos = -1;
	rslt = getIndex(frame_no, group_idx, pos);
	if (rslt) info = mFrameInfos[group_idx][pos];
	return rslt;
}

bool FrameMgrFixed::getIndex(int64_t frame_no, int32_t &groupidx, int32_t &pos)
{
	groupidx = (frame_no / mInterval) / eMaxGroupSize;
	if (groupidx < 0 || groupidx >= mStates.size()) return false;

	pos = (frame_no / mInterval) % eMaxGroupSize;
	if (pos < 0 || pos >= mStates[groupidx].size()) return false;
	return true;
}