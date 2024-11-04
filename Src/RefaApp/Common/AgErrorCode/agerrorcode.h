#ifndef AGERRORCODE_H
#define AGERRORCODE_H

#include <QString>

#define MAX_ERROR_CODE_DESC (256)   // 错误码描述最大长度

/**
 * @brief 错误码信息
 */
struct AgErrorCodeInfo
{
    uint32_t value; // 错误码值
    int level; // 错误码级别：0-正常，1-警告，2-错误
    QString desc; // 错误码描述
};

class AgErrorCodePrivate;

/**
 * @brief 错误码类
 */
class AgErrorCode
{    
public:
    static AgErrorCode & instance();

    ~AgErrorCode();

    /**
     * @brief 重置
     * @param lang 语言类型：0-中文，1-英文
     * @return true-成功，false-失败
     */
    bool reset(int lang);

    /**
     * @brief 获取错误码信息
     * @param value 错误码值
     * @param info 错误码信息
     * @return true-成功，false-失败
     */
    bool get(uint32_t value, AgErrorCodeInfo & info);

private:
    AgErrorCode();

	AgErrorCode(const AgErrorCode &p) = delete;
	AgErrorCode &operator=(const AgErrorCode &p) = delete;
private:
    AgErrorCodePrivate *d_ptr_;
};

#endif // AGERRORCODE_H
