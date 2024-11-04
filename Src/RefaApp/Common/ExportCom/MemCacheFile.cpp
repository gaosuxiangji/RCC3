#include "MemCacheFile.h"

MemCacheFile::MemCacheFile(int64_t size, const QString &filename, int32_t frameCount)
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
	if (filename == "")
	{
		try
		{
			mMemoryData = new char[size];
		}
		catch (std::exception &e)
		{
			mSize = 0;
			return;
			//if (size > eMaxSize) size = eMaxSize;
		}
	}
	else
	{
		mFile = std::make_shared<QFile>(filename);
		mFile->open(QIODevice::ReadWrite);
		if (mFile->isOpen())
		{
			mFile->resize(size);
			mMemoryData = (char*)mFile->map(0, size);
			if (!mMemoryData) return;
		}
	}

	mSize = size;
	mBlocks.resize(frameCount);
	reset();
}

MemCacheFile::MemCacheFile() {

}

MemCacheFile::~MemCacheFile()
{
	if (mFile) {
		mFile->close();
		mFile->unmap((uchar*)mMemoryData);
		mFile->remove();
	}

	else if (mMemoryData) {
		delete[]mMemoryData;
		mMemoryData = nullptr;
	}
}

bool MemCacheFile::readData(int64_t offset, char *data, int len)
{
	if (!mMemoryData) return false;
	if (offset + len> mSize) return false;
	
	memcpy(data, mMemoryData + offset, len);
	return true;
}

bool MemCacheFile::writeData(int64_t offset, const char *data, int64_t len)
{
	if (!mMemoryData || offset + len > mSize) return false;
	memcpy(mMemoryData + offset, data, len);
	//mFile->flush();
	return true;
}

bool MemCacheFile::createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount)
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

bool MemCacheFile::reset()
{
	int64_t blockAddr = 0;
	auto frameCount = (int32_t)mBlocks.size();
	if (frameCount == 0) {
		return true;
	}
	auto frameSize = mSize / frameCount;
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

bool MemCacheFile::closeCache()
{
	if (mFile) {
		mFile->close();
	}
	return true;
}

bool MemCacheFile::isFull()
{
	return mFreeCount == 0;
}

bool MemCacheFile::read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset)
{
	/*if (mFile == nullptr) {
		return false;
	}*/
	if (block->mReady != eReadyRead) {
		return false;
	}
	return readData(block->mBlockAddr, (char*)data, dataLen);
}

AGBlockInfo* MemCacheFile::write(uint8_t* data, int32_t dataLen)
{
	auto block = getFreeBlock();
	if (block == nullptr) {
		return nullptr;
	}
	auto offset = block->mBlockAddr;
	bool rslt = writeData(offset, (char*)data, dataLen);
	if (!rslt) return nullptr;

	if (block->mReady != eReady) {
		block->mReady = eReady;
		mFreeCount--;
	}
	return block;
}

AGBlockInfo* MemCacheFile::write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len)
{
	auto block = getFreeBlock();
	if (block == nullptr) {
		return nullptr;
	}
	auto offset = block->mBlockAddr;
	bool rslt = writeData(offset, (char*)info, info_len);
	if (!rslt) return nullptr;

	rslt = writeData(offset + info_len, (char*)data, dataLen);
	if (!rslt) return nullptr;

	if (block->mReady != eReady) {
		block->mReady = eReady;
		mFreeCount--;
	}
	return block;
}

bool MemCacheFile::write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset)
{
	return false;
}

AGBlockInfo* MemCacheFile::getFreeBlock()
{
	std::lock_guard<std::mutex> lock(mBlockMutex);
	for (auto& block : mBlocks) {
		if (block.mReady == eIdle) {
			block.mReady = eBusy;
			return &block;
		}
	}
	return nullptr;
}


void MemCacheFile::freeBlock(AGBlockInfo* block) {
	std::lock_guard<std::mutex> lock(mBlockMutex);
	if (block->mReady != eIdle) {
		block->mReady = eIdle;
		mFreeCount++;
	}
}