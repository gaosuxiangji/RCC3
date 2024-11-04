#ifndef AGERRORCODEPRIVATE_H
#define AGERRORCODEPRIVATE_H

#include <QMap>
#include <QSharedPointer>

#include "../agerrorcode.h"

/**
 * @brief 错误码私有类
 */
class AgErrorCodePrivate
{
public:
    AgErrorCodePrivate();

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
	bool isArmError(uint32_t error_code) const;
    QMap<uint32_t, QSharedPointer<AgErrorCodeInfo>> error_code_map_;
};

#endif // AGERRORCODEPRIVATE_H
