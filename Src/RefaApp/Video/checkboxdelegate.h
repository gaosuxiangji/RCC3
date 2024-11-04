/***************************************************************************************************
** @file: 绘制复选框模块
** @author: mpp
** @date: 2021-12-24
*
*****************************************************************************************************/
#ifndef CHECKBOXDELEGATE_H_
#define CHECKBOXDELEGATE_H_

#include <QObject>
#include <QStyledItemDelegate>

class CheckBoxDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
    CheckBoxDelegate(QObject *parent);
	virtual ~CheckBoxDelegate();

private:
signals:
	void signalTableCheck(const QModelIndex &index,bool bCheck);
private:
	/************************
	* @brief: 绘制复选框
	* @param painter:The QPainter class performs low-level painting on widgets and other paint devices.
	* @param option:The QStyleOptionViewItem class is used to describe the parameters used to draw an item in a view widget
	* @param index:The QModelIndex class is used to locate data in a data model
	* @author: mpp
	*************************/
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	/************************
	* @brief: 响应鼠标事件，更新数据
	* @param event:The QEvent class is the base class of all event classes. Event objects contain event parameters.
	* @param model:The QAbstractItemModel class provides the abstract interface for item model classes
	* @param option:The QStyleOptionViewItem class is used to describe the parameters used to draw an item in a view widget
	* @param index:The QModelIndex class is used to locate data in a data model
	* @author: mpp
	*************************/
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
private:
	const int m_iCheckBoxColumn;    //复选框所在的列

	public slots:
	void SlotChangeCheckBoxState(QAbstractItemModel *model, const QModelIndex &index,bool state);
};
#endif // !CHECKBOXDELEGATE_H_
