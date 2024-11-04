#ifndef ERRORCODEUTILS_H
#define ERRORCODEUTILS_H

#include <QVariant>

class QWidget;

/**
 * @brief 错误码工具类
 */
class ErrorCodeUtils
{
public:
    /**
     * @brief 处理错误码
     * @param parent_ptr 父窗口
     * @param error_code 错误码
     * @param video_id 视频ID
     */
    static void handle(QWidget *parent_ptr, quint64 error_code, const QVariant & video_id=QVariant(),QString extTip = QString());
};

#endif // ERRORCODEUTILS_H
