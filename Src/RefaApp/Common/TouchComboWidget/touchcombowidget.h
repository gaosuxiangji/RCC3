#ifndef TOUCHCOMBOWIDGET_H
#define TOUCHCOMBOWIDGET_H

#include <QWidget>

namespace Ui {
class TouchComboWidget;
}

/**
 * @brief 触摸组合框类
 */
class TouchComboWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchComboWidget(QWidget *parent = 0);
    ~TouchComboWidget();

    /**
     * @brief 设置标题
     * @param title 标题
     */
    void setTitle(const QString & title);

    /**
     * @brief 获取标题
     * @return 标题
     */
    QString title() const;

    /**
     * @brief 设备当前文本
     * @param text 文本
     */
    void setCurrentText(const QString & text);

    /**
     * @brief 获取当前文本
     * @return 文本
     */
    QString currentText() const;

signals:
    /**
     * @brief 单击
     */
    void clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::TouchComboWidget *ui;
};

#endif // TOUCHCOMBOWIDGET_H
