#include "agswitchbutton.h"

#include <QStylePainter>
#include <QStyleOptionButton>
#include <QtMath>

AgSwitchButton::AgSwitchButton(QWidget *parent) : QCheckBox(parent)
{
    setTristate(false);

    enabled_unchecked_image.load(":/switchbutton/images/enabled_unchecked.png");
    enabled_checked_image.load(":/switchbutton/images/enabled_checked.png");
    disabled_unchecked_image.load(":/switchbutton/images/disabled_unchecked.png");
    disabled_checked_image.load(":/switchbutton/images/disabled_checked.png");
}

void AgSwitchButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);

    // 绘制文本
    if (!opt.text.isEmpty())
    {
        style()->drawItemText(&p, opt.rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                            opt.palette, opt.state & QStyle::State_Enabled, opt.text, QPalette::WindowText);
    }

    // 绘制指示器
	QRect indicator_rect = indicatorRect(&opt);
    drawIndicator(&p, indicator_rect, opt.state & QStyle::State_Enabled, opt.state & QStyle::State_On);
}

bool AgSwitchButton::hitButton(const QPoint &pos) const
{
	QStyleOptionButton opt;
	initStyleOption(&opt);

    //return indicatorRect(&opt).contains(pos);
	//设计优化:按钮热区变更为整个选项界面
	return opt.rect.contains(pos);
}

int AgSwitchButton::indicatorHeight(QStyleOptionButton *opt) const
{
    return opt->fontMetrics.boundingRect(opt->text).height();
}

QRect AgSwitchButton::indicatorRect(QStyleOptionButton *opt) const
{
    int height = indicatorHeight(opt);
    int width = qCeil(qreal(enabled_unchecked_image.width()) * height / enabled_unchecked_image.height());
	return QRect(opt->rect.right() - width, opt->rect.top() + (opt->rect.height() - height) / 2, width, height);
}

void AgSwitchButton::drawIndicator(QPainter *painter, const QRect & indicator_rect, bool enabled, bool checked)
{
    QImage image;
    if (enabled)
    {
        image = checked ? enabled_checked_image : enabled_unchecked_image;
    }
    else
    {
        image = checked ? disabled_checked_image : disabled_unchecked_image;
    }

    image = image.scaled(indicator_rect.width(), indicator_rect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->drawImage(indicator_rect, image);
}
