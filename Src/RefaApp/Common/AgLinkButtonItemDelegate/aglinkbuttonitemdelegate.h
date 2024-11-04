#ifndef AGLINKBUTTONITEMDELEGATE_H
#define AGLINKBUTTONITEMDELEGATE_H

#include <QStyledItemDelegate>

/**
 * @brief 链接按钮样式项委托类
 */
class AgLinkButtonItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
    AgLinkButtonItemDelegate(QWidget *parent = nullptr);
    ~AgLinkButtonItemDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    /**
     * @brief 点击连接
     * @param index 模型索引
     */
	void linkButtonClicked(const QModelIndex & index);

protected:
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
	QRect textLayoutBounds(const QStyleOptionViewItem &option) const;
	QRect textRect(const QStyleOptionViewItem &opt) const;
};

#endif // AGLINKBUTTONITEMDELEGATE_H
