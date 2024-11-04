#include "agcheckboxheaderview.h"

#include <QPainter>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QApplication>

AgCheckBoxHeaderView::AgCheckBoxHeaderView(Qt::Orientation orientation, QWidget *parent) : QHeaderView(orientation, parent)
{

}

void AgCheckBoxHeaderView::setSectionCheckable(int logical_index, bool checkable)
{
    if (checkable)
    {
        check_state_map_.insert(logical_index, Qt::Unchecked);
        updateCheckbox(logical_index);
    }
    else
    {
        check_state_map_.remove(logical_index);
    }
}

void AgCheckBoxHeaderView::setModel(QAbstractItemModel *model)
{
	QObject::disconnect(this->model(), &QAbstractItemModel::rowsRemoved, this, &AgCheckBoxHeaderView::onRowsRemoved);

    section_rect_map_.clear();
    QHeaderView::setModel(model);

    for(auto iter = check_state_map_.begin(); iter != check_state_map_.end(); iter++)
    {
        updateCheckbox(iter.key());
    }

	QObject::connect(this->model(), &QAbstractItemModel::rowsRemoved, this, &AgCheckBoxHeaderView::onRowsRemoved);
}

void AgCheckBoxHeaderView::dataChanged(const QModelIndex &top_left, const QModelIndex &bottom_right, const QVector<int> &)
{
    if (block_data_changed)
    {
        return;
    }

    if (orientation() == Qt::Horizontal)
    {
        for (int i = top_left.column(); i <= bottom_right.column(); i++)
        {
            if (check_state_map_.contains(i))
            {
                updateCheckbox(i);
                updateSection(i);
            }
        }
    }
    else if (orientation() == Qt::Vertical)
    {
        for (int i = top_left.row(); i <= bottom_right.row(); i++)
        {
            if (check_state_map_.contains(i))
            {
                updateCheckbox(i);
                updateSection(i);
            }
        }
    }
}

void AgCheckBoxHeaderView::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
	for (auto iter = check_state_map_.begin(); iter != check_state_map_.end(); iter++)
	{
		updateCheckbox(iter.key());
	}
}

void AgCheckBoxHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    section_rect_map_.insert(logicalIndex, rect);

    if (!check_state_map_.contains(logicalIndex))
    {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
    }

    if (!rect.isValid())
        return;

    painter->save();

    // get the state of the section
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    QStyle::State state = QStyle::State_None;
    if (isEnabled())
        state |= QStyle::State_Enabled;
    if (window()->isActiveWindow())
        state |= QStyle::State_Active;
    if (sectionsClickable()) {
        QPoint cursor_pos = mapFromGlobal(cursor().pos());
        bool contains_mouse = rect.contains(cursor_pos);
        if (contains_mouse)
            state |= QStyle::State_MouseOver;
        Qt::MouseButtons mouse_buttons = QApplication::mouseButtons();
        if (contains_mouse && (mouse_buttons.testFlag(Qt::LeftButton) || mouse_buttons.testFlag(Qt::RightButton) ))
            state |= QStyle::State_Sunken;
        else if (highlightSections()) {
            if (sectionIntersectsSelection(logicalIndex))
                state |= QStyle::State_On;
        }

    }
    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex)
        opt.sortIndicator = (sortIndicatorOrder() == Qt::AscendingOrder)
                            ? QStyleOptionHeader::SortDown : QStyleOptionHeader::SortUp;

    // setup the style options structure
    QVariant textAlignment = model()->headerData(logicalIndex, orientation(),
                                                  Qt::TextAlignmentRole);
    opt.rect = rect;
    opt.section = logicalIndex;
    opt.state |= state;
    opt.textAlignment = Qt::Alignment(textAlignment.isValid()
                                      ? Qt::Alignment(textAlignment.toInt())
                                      : defaultAlignment());

    opt.iconAlignment = Qt::AlignVCenter;

    QVariant variant = model()->headerData(logicalIndex, orientation(),
                                    Qt::DecorationRole);
    opt.icon = qvariant_cast<QIcon>(variant);
    if (opt.icon.isNull())
        opt.icon = qvariant_cast<QPixmap>(variant);
    QVariant foregroundBrush = model()->headerData(logicalIndex, orientation(),
                                                    Qt::ForegroundRole);
    if (foregroundBrush.canConvert<QBrush>())
        opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));

    QPointF oldBO = painter->brushOrigin();
    QVariant backgroundBrush = model()->headerData(logicalIndex, orientation(),
                                                    Qt::BackgroundRole);
    if (backgroundBrush.canConvert<QBrush>()) {
        opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
        opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
        painter->setBrushOrigin(opt.rect.topLeft());
    }

    // the section position
    int visual = visualIndex(logicalIndex);
    Q_ASSERT(visual != -1);
    if (count() == 1)
        opt.position = QStyleOptionHeader::OnlyOneSection;
    else if (visual == 0)
        opt.position = QStyleOptionHeader::Beginning;
    else if (visual == count() - 1)
        opt.position = QStyleOptionHeader::End;
    else
        opt.position = QStyleOptionHeader::Middle;
    opt.orientation = orientation();
    // the selected position
    bool previousSelected = sectionIntersectsSelection(this->logicalIndex(visual - 1));
    bool nextSelected =  sectionIntersectsSelection(this->logicalIndex(visual + 1));
    if (previousSelected && nextSelected)
        opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
    else if (previousSelected)
        opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
    else if (nextSelected)
        opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
    else
        opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
    // draw the section
    style()->drawControl(QStyle::CE_Header, &opt, painter, this);

    painter->setBrushOrigin(oldBO);

    painter->restore();

    // 绘制文本
    opt.text = model()->headerData(logicalIndex, orientation(),
                                    Qt::DisplayRole).toString();

    int margin = 2 * style()->pixelMetric(QStyle::PM_HeaderMargin, 0, this);

    const Qt::Alignment headerArrowAlignment = static_cast<Qt::Alignment>(style()->styleHint(QStyle::SH_Header_ArrowAlignment, 0, this));
    const bool isHeaderArrowOnTheSide = headerArrowAlignment & Qt::AlignVCenter;
    if (isSortIndicatorShown() && sortIndicatorSection() == logicalIndex && isHeaderArrowOnTheSide)
        margin += style()->pixelMetric(QStyle::PM_HeaderMarkSize, 0, this);

    if (textElideMode() != Qt::ElideNone)
        opt.text = opt.fontMetrics.elidedText(opt.text, textElideMode() , rect.width() - margin);

    QRect check_rect = checkBoxRect(opt, opt.rect);
    if (orientation() == Qt::Horizontal)
    {
        opt.rect.setLeft(check_rect.right());
    }
    else if (orientation() == Qt::Vertical)
    {
        opt.rect.setTop(check_rect.bottom());
    }
    style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);

    // 绘制复选框
    QStyleOptionButton option;
    option.QStyleOption::operator =(opt);
    option.rect = checkBoxRect(opt, rect);

    Qt::CheckState check_state = check_state_map_.value(logicalIndex);
    switch(check_state)
    {
    case Qt::Checked:
        option.state |= QStyle::State_On;
        break;
    case Qt::Unchecked:
        option.state |= QStyle::State_Off;
        break;
    case Qt::PartiallyChecked:
        option.state |= QStyle::State_NoChange;
        break;
    }
    style()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &option, painter);
}

void AgCheckBoxHeaderView::mousePressEvent(QMouseEvent *event)
{
    QHeaderView::mousePressEvent(event);
}

void AgCheckBoxHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
	if (clickCheckBox(event))
	{
		return;
	}

	QHeaderView::mouseReleaseEvent(event);
}

void AgCheckBoxHeaderView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QHeaderView::mouseDoubleClickEvent(event);
}

QRect AgCheckBoxHeaderView::checkBoxRect(const QStyleOptionHeader &option, const QRect &bounding) const
{
    QStyleOptionButton opt;
    opt.QStyleOption::operator =(option);
    opt.rect = bounding;

	const QWidget *widget = parentWidget();
	QStyle *style = nullptr;
	if (widget) {
		style = widget->style();
	}
	else {
		style = QApplication::style();
	}
	QRect check_rect;
	if (style){
		check_rect = style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt, widget);
	}

	int delta_x = (bounding.width() - check_rect.width()) / 2;
	int delta_y = (bounding.height() - check_rect.height()) / 2;

	return QRect(bounding.x() + delta_x, bounding.y() + delta_y, check_rect.width(), check_rect.height());
}

bool AgCheckBoxHeaderView::rowIntersectsSelection(int row) const
{
    return (selectionModel() ? selectionModel()->rowIntersectsSelection(row, rootIndex()) : false);
}

bool AgCheckBoxHeaderView::columnIntersectsSelection(int column) const
{
    return (selectionModel() ? selectionModel()->columnIntersectsSelection(column, rootIndex()) : false);
}

bool AgCheckBoxHeaderView::sectionIntersectsSelection(int logical) const
{
    return (orientation() == Qt::Horizontal ? columnIntersectsSelection(logical) : rowIntersectsSelection(logical));
}

void AgCheckBoxHeaderView::updateCheckbox(int logical_index)
{
	QAbstractItemModel* model_ptr = model();
	if (!model_ptr)
	{
		return;
	}

    bool all_checked = true;
    bool all_unchecked = true;

    if (orientation() == Qt::Horizontal)
    {
        for (int i =0; i < model_ptr->rowCount(rootIndex()); i++)
        {
            if (model_ptr->index(i, logical_index, rootIndex()).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            {
                all_unchecked = false;
            }
            else
            {
                all_checked = false;
            }
        }
    }
    else if (orientation() == Qt::Vertical)
    {
        for (int i =0; i < model_ptr->columnCount(rootIndex()); i++)
        {
            if (model_ptr->index(logical_index, i, rootIndex()).data(Qt::CheckStateRole).toInt() == Qt::Checked)
            {
                all_unchecked = false;
            }
            else
            {
                all_checked = false;
            }
        }
    }

    if (all_unchecked)
    {
        check_state_map_.insert(logical_index, Qt::Unchecked);
    }
    else if (all_checked)
    {
        check_state_map_.insert(logical_index, Qt::Checked);
    }
    else
    {
        check_state_map_.insert(logical_index, Qt::PartiallyChecked);
    }
}

void AgCheckBoxHeaderView::updateModel(int logical_index)
{
    Qt::CheckState check_state = check_state_map_.value(logical_index);

    if (orientation() == Qt::Horizontal)
    {
        for (int i = 0; i < model()->rowCount(rootIndex()); i++)
        {
            model()->setData(model()->index(i, logical_index, rootIndex()), check_state, Qt::CheckStateRole);
        }
    }
    else if (orientation() == Qt::Vertical)
    {
        for (int i = 0; i < model()->columnCount(rootIndex()); i++)
        {
            model()->setData(model()->index(logical_index, i, rootIndex()), check_state, Qt::CheckStateRole);
        }
    }
}

bool AgCheckBoxHeaderView::clickCheckBox(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
	{
		return false;
	}


	int logical_index = logicalIndexAt(event->pos());

	QStyleOptionHeader opt;
	initStyleOption(&opt);
	QRect check_rect = checkBoxRect(opt, section_rect_map_[logical_index]);
	if (!check_rect.contains(event->pos()))
	{
		return false;
	}

	if (!check_state_map_.contains(logical_index))
	{
		return false;
	}

	block_data_changed = true;

	Qt::CheckState check_state = check_state_map_.value(logical_index);
	if (check_state == Qt::Checked)
	{
		check_state_map_.insert(logical_index, Qt::Unchecked);
	}
	else
	{
		check_state_map_.insert(logical_index, Qt::Checked);
	}

	updateModel(logical_index);

	block_data_changed = false;

	updateCheckbox(logical_index);
	updateSection(logical_index);

	return true;
}
