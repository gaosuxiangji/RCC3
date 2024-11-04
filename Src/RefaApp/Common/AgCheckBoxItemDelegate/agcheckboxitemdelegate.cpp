#include "agcheckboxitemdelegate.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

AgCheckBoxItemDelegate::AgCheckBoxItemDelegate(QObject *parent) : QItemDelegate(parent)
{
}


AgCheckBoxItemDelegate::~AgCheckBoxItemDelegate()
{
}

QWidget * AgCheckBoxItemDelegate::createEditor(QWidget*, const QStyleOptionViewItem &, const QModelIndex &) const
{
	return nullptr;
}

void AgCheckBoxItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)const
{
	QVariant value = index.data(Qt::CheckStateRole);
	if (value.isValid())
	{
		QStyleOptionViewItem opt = setOptions(index, option);

		// 绘制背景
		drawBackground(painter, opt, index);

		// 绘制复选框
		Qt::CheckState check_state = static_cast<Qt::CheckState>(value.toInt());
		QRect check_rect = checkBoxRect(opt, opt.rect, value);
		drawCheck(painter, opt, check_rect, check_state);
	}
	else
	{
		QItemDelegate::paint(painter, option, index);
	}
}

bool AgCheckBoxItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) 
{
	QVariant value = index.data(Qt::CheckStateRole);
	if (value.isValid())
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			QRect checkRect = checkBoxRect(option, option.rect, Qt::Checked);
			QMouseEvent *me = static_cast<QMouseEvent*>(event);
			if (me->button() == Qt::LeftButton && checkRect.contains(me->pos()))
			{
				Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
				state = ((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
				model->setData(index, state, Qt::CheckStateRole);
			}
		}

		return true;
	}

	return QItemDelegate::editorEvent(event, model, option, index);
}

QRect AgCheckBoxItemDelegate::rect(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const
{
	QVariant value = index.data(role);
	if (role == Qt::CheckStateRole)
		return checkBoxRect(option, option.rect, value);
	else
	{
		return QItemDelegate::rect(option, index, role);
	}
}

QRect AgCheckBoxItemDelegate::checkBoxRect(const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &value) const
{
	if (value.isValid()) {
		QStyleOptionButton opt;
		opt.QStyleOption::operator=(option);
		opt.rect = bounding;
		QRect check_rect;
		const QWidget *widget = option.widget;
		QStyle *style = nullptr;
		if (widget){
			style = widget->style();
		}
		else {
			style = QApplication::style();
		}
		if (style){
			check_rect = style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt, widget);
		}

		int delta_x = (bounding.width() - check_rect.width()) / 2;
		int delta_y = (bounding.height() - check_rect.height()) / 2;

		return QRect(bounding.x() + delta_x, bounding.y() + delta_y, check_rect.width(), check_rect.height());
	}
	else
	{
		return QItemDelegate::doCheck(option, bounding, value);
	}
}
