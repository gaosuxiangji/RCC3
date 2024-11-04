#ifndef UIUTILS_H
#define UIUTILS_H

#include <QWidget>

/**
 * @brief 界面工具类
 */
class UIUtils
{
public:
    /**
     * @brief 显示提示信息框
     * @param parent 父窗口
     * @param text 文本
     */
    static void showInfoMsgBox(QWidget *parent, const QString & text);

    /**
     * @brief 显示警告信息框
     * @param parent 父窗口
     * @param text 文本
     */
    static void showWarnMsgBox(QWidget *parent, const QString & text);

    /**
     * @brief 显示错误信息框
     * @param parent 父窗口
     * @param text 文本
     */
    static void showErrorMsgBox(QWidget *parent, const QString & text);

    /**
     * @brief 显示疑问信息框
     * @param parent 父窗口
     * @param text 文本
     * @param button_type 类型：0-是/否，1-确定/取消
     * @return
     */
    static bool showQuestionMsgBox(QWidget *parent, const QString & text, int button_type = 0);
};

#endif // UIUTILS_H
