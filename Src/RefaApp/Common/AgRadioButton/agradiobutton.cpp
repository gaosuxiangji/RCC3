#include "agradiobutton.h"

#include <QStyleOptionButton>
#include <QStylePainter>

AgRadioButton::AgRadioButton(QWidget* parent) : QRadioButton(parent)
{
	setAttribute(Qt::WA_StyledBackground);

}

void AgRadioButton::paintEvent(QPaintEvent *e)
{
    QStylePainter p(this);
    QStyleOptionButton opt;
    initStyleOption(&opt);

	QRect rect = style()->subElementRect(QStyle::SE_PushButtonContents, &opt, this);

    // 绘制文本
    if (!opt.text.isEmpty())
    {
        style()->drawItemText(&p, rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic,
                            opt.palette, opt.state & QStyle::State_Enabled, opt.text, QPalette::WindowText);
    }

    // 绘制指示器
    if (opt.state & QStyle::State_On)
    {
        QChar indicator(0x221A);
        opt.palette.setColor(QPalette::WindowText, QColor(51, 153, 255));
        style()->drawItemText(&p, rect, Qt::AlignRight | Qt::AlignVCenter,
                              opt.palette, opt.state & QStyle::State_Enabled, QString(indicator), QPalette::WindowText);
    }
}

bool AgRadioButton::hitButton(const QPoint &pos) const
{
    return rect().contains(pos);
}
