#ifndef MEMCACHE_FILE_H_
#define MEMCACHE_FILE_H_
#include <QFile>
#include <memory>
#include <mutex>
#include "IOCacheObj.h"

class MemCacheFile : public IOCacheObj
{
	enum BlockState
	{
		eIdle = 0,
		eBusy = 1,
		eReady = 2,
		eReadyRead = 3
	};
	enum 
	{
		eMaxSize = 2000000000
	};

public:
	~MemCacheFile();
	MemCacheFile(int64_t size, const QString &filename, int32_t frameCount);
	MemCacheFile();

	MemCacheFile(const MemCacheFile &p) = delete;
	MemCacheFile &operator=(const MemCacheFile &p) = delete;


	bool readData(int64_t offset, char *data, int len);
	bool writeData(int64_t offset, const char *data, int64_t len);

	// IOCacheObj interface
	virtual bool createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount);
	virtual bool reset();
	virtual bool closeCache();
	virtual bool isFull();
	virtual uint64_t getMemSize() { return mSize; };

	virtual bool read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset);
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen);
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len);
	virtual bool write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset);
	virtual void freeBlock(AGBlockInfo* block);
	
private:
	std::shared_ptr<QFile> mFile;
	uchar *mAddress = nullptr;
	int64_t mSize = 0;

	std::vector<AGBlockInfo> mBlocks;
	int32_t mFreeCount = 0;

	virtual AGBlockInfo* getFreeBlock();
	std::mutex mBlockMutex;
	char *mMemoryData = nullptr;
};

#endif