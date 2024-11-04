#ifndef AGRADIOBUTTON_H
#define AGRADIOBUTTON_H

#include <QRadioButton>

/**
 * @brief 单选按钮类，对号样式
 */
class AgRadioButton : public QRadioButton
{
public:
    AgRadioButton(QWidget* parent=nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

    bool hitButton(const QPoint &) const override;
};

#endif // AGRADIOBUTTON_H
