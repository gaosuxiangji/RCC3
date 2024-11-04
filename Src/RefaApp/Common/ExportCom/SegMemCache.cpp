#include "SegMemCache.h"

SegMemCache::SegMemCache(int64_t size, int32_t frameCount, int32_t frame_size)
{
	//mFile = std::make_shared<QFile>(filename);
	//mFile->open(QIODevice::ReadWrite);
	//if (mFile->isOpen())
	//{
	//	mFile->resize(size);
	//	mAddress = mFile->map(0, size);
	//	mSize = size;
	//}
	//if (size > eMaxSize) size = eMaxSize;
	mSize = size;
	mFrameSize = frame_size;
	mBlocks.resize(frameCount);
	try {
		reset();
	}
	catch(std::exception &e){
		mSize = 0;
		mFrameSize = 0;
		mBlocks.clear();
		return;
	}
	mInitFinished = true;
}

SegMemCache::SegMemCache() {

}

SegMemCache::~SegMemCache()
{
	for (auto &block : mBlocks) 
	{
		auto data = (char*)block.mBlockAddr;
		if (data) {
			delete[]data;
		}
	}
}

bool SegMemCache::createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount)
{
	/*mFile = std::make_shared<QFile>(path);
	mFile->open(QIODevice::ReadWrite);
	if (mFile->isOpen())
	{
	mFile->resize(fileSize);
	mAddress = mFile->map(0, path);
	mSize = fileSize;
	}
	mBlocks.resize(frameCount);*/
	return false;
}

bool SegMemCache::reset()
{
	std::lock_guard<std::mutex> lock(mBlockMutex);
	int64_t blockAddr = 0;
	auto frameCount = (int32_t)mBlocks.size();
	if (frameCount == 0) {
		return true;
	}
	auto frameSize = mSize / frameCount;

	for (auto& block : mBlocks) {
		auto data = (char*)block.mBlockAddr;
		if (data) {
			delete[]data;
		}

		//char *address = new char[mFrameSize];
		block.mBlockAddr = (uint64_t)static_cast<void*>(getMem());
		block.mFrameNo = 0;
		block.mFrameLen = mFrameSize;
		block.mReady = false;
	}
	mFreeCount = frameCount;
	return true;
}

bool SegMemCache::closeCache()
{
	if (mFile) {
		mFile->close();
	}
	return true;
}

bool SegMemCache::isFull()
{
	return mFreeCount == 0;
}

bool SegMemCache::read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset)
{
	std::lock_guard<std::mutex> lock(mBlockMutex);
	if (block->mReady != eReadyRead) {
		return false;
	}
	if (block->mFrameLen < dataLen) return false;
	memcpy(data, (char*)block->mBlockAddr, dataLen);
	return true;
}

AGBlockInfo* SegMemCache::write(uint8_t* data, int32_t dataLen)
{
	std::lock_guard<std::mutex> lock(mBlockMutex);
	auto block = getFreeBlock();
	if (block == nullptr) {
		return nullptr;
	}
	auto offset = block->mBlockAddr;
    if (block->mFrameLen < dataLen) return nullptr;
	memcpy((char*)block->mBlockAddr, data, dataLen);
	
	if (block->mReady != eReady) {
		block->mReady = eReady;
		mFreeCount--;
	}
	return block;
}

AGBlockInfo* SegMemCache::write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len)
{
	std::lock_guard<std::mutex> lock(mBlockMutex);
	auto block = getFreeBlock();
	if (block == nullptr) {
		return nullptr;
	}

	if (block->mFrameLen < dataLen + info_len) {
		return nullptr;
	}
	memcpy((char*)block->mBlockAddr, info, info_len);
	memcpy(((char*)block->mBlockAddr + info_len), data, dataLen);

	if (block->mReady != eReady) {
		block->mReady = eReady;
		mFreeCount--;
	}
	return block;
}

bool SegMemCache::write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset)
{
	return false;
}

AGBlockInfo* SegMemCache::getFreeBlock()
{
	//std::lock_guard<std::mutex> lock(mBlockMutex);
	for (auto& block : mBlocks) {
		if (block.mReady == eIdle) {
			block.mReady = eBusy;
			return &block;
		}
	}
	return nullptr;
}


void SegMemCache::freeBlock(AGBlockInfo* block) {
	//std::lock_guard<std::mutex> lock(mBlockMutex);
	if (block->mReady != eIdle) {
		block->mReady = eIdle;
		mFreeCount++;
	}
}
