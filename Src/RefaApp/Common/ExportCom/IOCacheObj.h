#ifndef IO_CACHE_OBJ_
#define IO_CACHE_OBJ_

enum BlockState
{
	eIdle = 0,
	eBusy = 1,
	eReady = 2,
	eReadyRead = 3
};

struct AGBlockInfo {
	uint64_t mBlockAddr = 0;
	int64_t mFrameNo = 0;
	int32_t mFrameLen = 0;
	int8_t mReady = eIdle;
};

namespace IOCacheOBJ
{
	struct FrameInfo
	{
		uint8_t  m_timestamp_[9];
	};
};

class IOCacheObj
{
public:
	virtual bool createCache(const wchar_t* path, int64_t fileSize, int32_t frameCount) = 0;
	virtual bool reset() = 0;
	virtual bool closeCache() = 0;
	virtual bool isFull() = 0;
	virtual uint64_t getMemSize() = 0;

	virtual bool read(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset) = 0;
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen) = 0;
	virtual AGBlockInfo* write(uint8_t* data, int32_t dataLen, uint8_t *info, int32_t info_len) = 0;
	virtual bool write(AGBlockInfo* block, uint8_t* data, int32_t dataLen, int32_t offset) = 0;
	virtual void freeBlock(AGBlockInfo* block) = 0;
};


#endif
