#ifndef AGCHECKBOXITEMDELEGATE_H
#define AGCHECKBOXITEMDELEGATE_H

#include <QItemDelegate>

/**
 * @brief 复选框样式项委托类
 */
class AgCheckBoxItemDelegate : public QItemDelegate
{
public:
        AgCheckBoxItemDelegate(QObject *parent = 0);
        ~AgCheckBoxItemDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

	QRect rect(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const;

	QRect checkBoxRect(const QStyleOptionViewItem &option, const QRect &bounding, const QVariant &value) const;
};

#endif // AGCHECKBOXITEMDELEGATE_H
