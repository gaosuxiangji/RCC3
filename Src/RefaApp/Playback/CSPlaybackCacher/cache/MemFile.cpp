#include "MemFile.h"

MemFile::MemFile(int64_t size, const QString &filename)
{
	mFile = std::make_shared<QFile>(filename);
	mFile->open(QIODevice::ReadWrite);
	if (mFile->isOpen())
	{
		mFile->resize(size);
		mAddress = mFile->map(0, size);
		mSize = size;
	}
}

MemFile::~MemFile()
{
	if (mFile) {
		mFile->close();
		mFile->unmap(mAddress);
		mFile->remove();
	}
}

bool MemFile::readData(int64_t offset, char *data, int len)
{
	if (!mAddress) return false;
	if (offset + len>= mSize) return false;
	
	memcpy(data, mAddress + offset, len);
	return true;
}

bool MemFile::writeData(int64_t offset, const char *data, int64_t len)
{
	if (!mAddress || offset + len > mSize) return false;
	memcpy(mAddress + offset, data, len);
	//mFile->flush();
	return true;
}