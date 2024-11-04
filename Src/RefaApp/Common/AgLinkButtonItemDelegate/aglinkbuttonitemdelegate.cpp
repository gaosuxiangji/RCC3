#include "aglinkbuttonitemdelegate.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>

AgLinkButtonItemDelegate::AgLinkButtonItemDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{
}


AgLinkButtonItemDelegate::~AgLinkButtonItemDelegate()
{
}

QWidget *AgLinkButtonItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return nullptr;
}

QSize AgLinkButtonItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize size = QStyledItemDelegate::sizeHint(option, index);
	size.setHeight(size.height() * 2);
	return size;
}

void AgLinkButtonItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = opt.widget;
	QStyle *style = nullptr;
	if (widget){
		style = widget->style();
	}
	else {
		style = QApplication::style();
	}
	if (style){
		// 绘制背景
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, widget);

		// 绘制文本
		style->drawItemText(painter, textRect(opt), Qt::AlignCenter, opt.palette, opt.state & QStyle::State_Enabled, opt.text, QPalette::Link);
	}
}

bool AgLinkButtonItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
		QStyleOptionViewItem opt = option;
		initStyleOption(&opt, index);

		//设计优化:按键热区变为整个单元格
        QRect text_rect = textLayoutBounds(opt);
		QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton && text_rect.contains(me->pos()))
        {
            emit linkButtonClicked(index);
        }
    }

    return true;
}

#define QFIXED_MAX (INT_MAX/256)

QRect AgLinkButtonItemDelegate::textLayoutBounds(const QStyleOptionViewItem &option) const
{
	QRect rect = option.rect;
	const bool wrapText = option.features & QStyleOptionViewItem::WrapText;
	switch (option.decorationPosition) {
	case QStyleOptionViewItem::Left:
	case QStyleOptionViewItem::Right:
		rect.setWidth(wrapText && rect.isValid() ? rect.width() : (QFIXED_MAX));
		break;
	case QStyleOptionViewItem::Top:
	case QStyleOptionViewItem::Bottom:
		rect.setWidth(wrapText ? option.decorationSize.width() : (QFIXED_MAX));
		break;
	}

	return rect;
}

QRect AgLinkButtonItemDelegate::textRect(const QStyleOptionViewItem &opt) const
{
    return opt.fontMetrics.boundingRect(opt.rect, opt.displayAlignment, opt.text);
}
