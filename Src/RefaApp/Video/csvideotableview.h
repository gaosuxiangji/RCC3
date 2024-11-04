#ifndef CSVIDEOTABLEVIEW_H
#define CSVIDEOTABLEVIEW_H

#include <QTableView>
#include <QObject>
#include <QWidget>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QHeaderView>
/**
 * @brief 设备树QTreeView控件
 */
class CSVideoTableView : public QTableView
{
    Q_OBJECT
public:
	CSVideoTableView(QWidget *parent = nullptr);
    virtual ~CSVideoTableView();

	void init(QStandardItemModel *myModel, QHeaderView *myHeader,QAbstractItemDelegate *myItemDelegate);
signals:
	void tableviewMoveEvent(const QModelIndex& index);
	void tableviewLeaveEvent();
protected:
	virtual void keyPressEvent(QKeyEvent *event);
	void resizeEvent(QResizeEvent *event) override;
	void scrollTo(const QModelIndex & index, ScrollHint hint = EnsureVisible) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void leaveEvent(QEvent *event) override;

private:
	void initFreezeTable();
	void updateFrozenTableGeometry();
	QTableView *m_frozenTableView = nullptr;
	int m_iFreezeColCounts = 1;
	int m_default_section_size = 150;//默认的每列宽
	private slots:
	void updateSectionWidth(int logicalIndex, int oldSize, int newSize);
	void updateSectionHeight(int logicalIndex, int oldSize, int newSize);
	void updateSection(const QModelIndex current);
};

#endif // CSVIDEOTABLEVIEW_H
