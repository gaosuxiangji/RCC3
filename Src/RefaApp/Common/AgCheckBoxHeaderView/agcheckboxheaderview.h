#ifndef AGCHECKBOXHEADERVIEW_H
#define AGCHECKBOXHEADERVIEW_H

#include <QHeaderView>
#include <QMap>

/**
 * @brief 复选框表头视图类
 */
class AgCheckBoxHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit AgCheckBoxHeaderView(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);

    /**
     * @brief 设置是否可选
     * @param checkable true-可选，false-不可选
     */
    void setSectionCheckable(int logical_index, bool checkable);

    void setModel(QAbstractItemModel *model);

signals:

public slots:

protected slots:
    virtual void dataChanged(const QModelIndex & top_left, const QModelIndex & bottom_right, const QVector<int> &) override;

	void onRowsRemoved(const QModelIndex &parent, int first, int last);

protected:
    virtual void paintSection(QPainter *painter, const QRect &rect, int logical_index) const override;
    virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QRect checkBoxRect(const QStyleOptionHeader & option, const QRect & bounding) const;
    bool rowIntersectsSelection(int row) const;
    bool columnIntersectsSelection(int column) const;
    bool sectionIntersectsSelection(int logical) const;

    void updateCheckbox(int logical_index);
    void updateModel(int logical_index);

	bool clickCheckBox(QMouseEvent *event);

private:
    QMap<int, Qt::CheckState> check_state_map_;
    mutable QMap<int, QRect> section_rect_map_;
    bool block_data_changed{ false };
};

#endif // AGCHECKBOXHEADERVIEW_H
