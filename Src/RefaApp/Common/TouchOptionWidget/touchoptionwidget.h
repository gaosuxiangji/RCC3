#ifndef TOUCHOPTIONWIDGET_H
#define TOUCHOPTIONWIDGET_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class TouchOptionWidget;
}
class QAbstractButton;

class TouchOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchOptionWidget(QWidget *parent = 0);
    ~TouchOptionWidget();

    /**
     * @brief 设置文本
     * @param text 文本
     */
    void setText(const QString & text);

    /**
     * @brief 设置选项值
     * @param user_data 选项值
     */
    void setValue(const QVariant & value);

    /**
     * @brief 获取选项值
     * @return 选项值
     */
    QVariant value() const;

    /**
     * @brief 获取按钮
     * @return 按钮指针
     */
    QAbstractButton* button() const;

private:
    Ui::TouchOptionWidget *ui;

    QVariant value_;
};

#endif // TOUCHOPTIONWIDGET_H
