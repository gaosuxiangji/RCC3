#include "checkboxdelegate.h"
#include <QCheckBox>
#include <QApplication>
#include <QMouseEvent>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
	, m_iCheckBoxColumn(0)
{}

CheckBoxDelegate::~CheckBoxDelegate()
{}

// 绘制复选框
void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

    QStyledItemDelegate::paint(painter, viewOption, index);

    if (m_iCheckBoxColumn == index.column())
    {
        bool data = index.model()->data(index, Qt::UserRole).toBool();

        QStyleOptionButton checkBoxStyle;
        checkBoxStyle.state = data ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyle.state |= QStyle::State_Enabled;
        checkBoxStyle.iconSize = QSize(20, 20);
#ifdef _WINDOWS
        checkBoxStyle.rect = option.rect;
#else
        int nX = 0, nY = 0;
        if(option.rect.width() > 20)
        {
            nX = option.rect.x() + (option.rect.width() - 20) / 2;
        }
        if(option.rect.height() > 20)
        {
            nY = option.rect.y() + (option.rect.height() - 20) / 2;
        }
        checkBoxStyle.rect = QRect(nX, nY, 20, 20);
#endif

        QCheckBox checkBox;
		auto appStyle = QApplication::style();
		if (appStyle){
			appStyle->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxStyle, painter, &checkBox);
		}
    }
}

// 响应鼠标事件，更新数据
bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QRect decorationRect = option.rect;

    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (event->type() == QEvent::MouseButtonPress && decorationRect.contains(mouseEvent->pos()))
    {
        if (m_iCheckBoxColumn == index.column())
        {
            bool data = model->data(index, Qt::UserRole).toBool();
			data = !data;
            model->setData(index, data, Qt::UserRole);
			emit signalTableCheck(index,data);
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void CheckBoxDelegate::SlotChangeCheckBoxState(QAbstractItemModel *model, const QModelIndex &index,bool state)
{
	model->setData(index, state, Qt::UserRole);
}
