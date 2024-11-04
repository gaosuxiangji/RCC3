#ifndef AGSWITCHBUTTON_H
#define AGSWITCHBUTTON_H

#include <QCheckBox>
#include <QRect>
#include <QImage>

/**
 * @brief 开关按钮类
 */
class AgSwitchButton : public QCheckBox
{
    Q_OBJECT

public:
    explicit AgSwitchButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
	bool hitButton(const QPoint &pos) const override;

private:
    int indicatorHeight(QStyleOptionButton *opt) const;
	QRect indicatorRect(QStyleOptionButton *opt) const;
    void drawIndicator(QPainter *painter, const QRect & indicator_rect, bool enabled, bool checked);

private:
	QImage enabled_unchecked_image;
	QImage enabled_checked_image;
    QImage disabled_unchecked_image;
    QImage disabled_checked_image;
};

#endif // AGSWITCHBUTTON_H
