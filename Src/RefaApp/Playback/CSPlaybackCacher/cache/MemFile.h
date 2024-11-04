#ifndef MEMFILE_H_
#define MEMFILE_H_
#include <QFile>
#include <memory>


class MemFile
{
private:
	std::shared_ptr<QFile> mFile;
	uchar *mAddress = nullptr;
	int64_t mSize = 0;

public:
	~MemFile();
	MemFile(int64_t size, const QString &filename);

	bool readData(int64_t offset, char *data, int len);
	bool writeData(int64_t offset, const char *data, int64_t len);
};

#endif