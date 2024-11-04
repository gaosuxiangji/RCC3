#ifndef PATHUTILS_H
#define PATHUTILS_H

#include <QString>
#include <vector>
/**
 * @brief 路径工具类
 */
class PathUtils
{
public:
    /**
     * @brief 是否可读
     * @param path 路径
     * @return true-可读, false-不可读
     */
    static bool isReadable(const QString & path);

    /**
     * @brief 是否可写
     * @param path 路径
     * @return true-可写，false-不可写
     */
    static bool isWritable(const QString & path);

	//分割字符串操作
	static std::vector<std::string> split(std::string str, std::string pattern);
};

#endif // PATHUTILS_H
