#include "pathutils.h"

#include <QFileInfo>
#include <QDir>

bool PathUtils::isReadable(const QString &path)
{
    return QFileInfo(path).isReadable();
}

bool PathUtils::isWritable(const QString &path)
{
    QFileInfo file_info(path);
    if (!file_info.isWritable())
    {
        return false;
    }

    if (file_info.isDir())
    {
        QDir dir(path);
        if (!dir.mkdir("tmp"))
        {
            return false;
        }
        dir.rmdir("tmp");
    }

    return true;
}

std::vector<std::string> PathUtils::split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;
	size_t size = str.size();
	for (size_t i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}

	return result;

}
