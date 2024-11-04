#ifndef TOUCHCHECKWIDGET_H
#define TOUCHCHECKWIDGET_H

#include <QWidget>

namespace Ui {
class TouchCheckWidget;
}

/**
 * @brief 触摸多选框类
 */
class TouchCheckWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchCheckWidget(QWidget *parent = 0);
    ~TouchCheckWidget();

    /**
     * @brief 设置文本
     * @param text 文本
     */
    void setText(const QString & text);

    /**
     * @brief 获取文本
     * @return 文本
     */
    QString text() const;

    /**
     * @brief 设置选中状态
     * @param checked true-选中，false-未选中
     */
    void setChecked(bool checked);

    /**
     * @brief 获取选中状态
     * @return true-选中，false-未选中
     */
    bool isChecked() const;

signals:
    /**
     * @brief 单击
     * @param checked true-选中，false-未选中
     */
    void clicked(bool checked);

private:
    Ui::TouchCheckWidget *ui;
};

#endif // TOUCHCHECKWIDGET_H
