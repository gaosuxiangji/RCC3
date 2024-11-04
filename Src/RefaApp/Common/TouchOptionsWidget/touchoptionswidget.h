#ifndef TOUCHOPTIONSWIDGET_H
#define TOUCHOPTIONSWIDGET_H

#include <QWidget>

namespace Ui {
class TouchOptionsWidget;
}

class QButtonGroup;
class TouchOptionWidget;

class TouchOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchOptionsWidget(QWidget *parent = 0);
    ~TouchOptionsWidget();

    /**
     * @brief 添加选项
     * @param value 选项值
     * @param text 选项文本
     */
    void addOption(const QVariant & value, const QString & text);

    /**
     * @brief 设置当前选项
     * @param value 当前选项值
     */
    void setCurrentOption(const QVariant & value);

    /**
     * @brief 获取当前选项文本
     * @return 当前选项文本
     */
	QString getCurrentOptionText() const;

	/**
	* @brief 获取当前选项值
	* @return 当前选项值
	*/
	QVariant getCurrentOptionValue() const;

    /**
     * @brief 清除所有选项
     */
    void clearOptions();

    /**
     * @brief 设置自定义控件
     * @param custom_widget 自定义控件
     */
    void setCustomWidget(QWidget *custom_widget,QString tip = QString());

	void setCustomTip(QString tip);

signals:
    /**
     * @brief 返回
     */
    void backButtonClicked();

    /**
     * @brief 当前选项变化
     * @param value
     */
    void currentOptionChanged(const QVariant & value);

private slots:
    void on_pushButtonBack_clicked();
    void onOptionClicked(int id);
    void onSpinBoxValueChanged(int value);

private:
    TouchOptionWidget* getCurrentOptionWidget(int id) const;

private:
    Ui::TouchOptionsWidget *ui;
    QButtonGroup *button_group_options_;
    QWidget* custom_widget_{ nullptr };
};

#endif // TOUCHOPTIONSWIDGET_H
