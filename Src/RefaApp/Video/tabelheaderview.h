/***************************************************************************************************
** @file: 导出文件列表-表头模块
** @author: mpp
** @date: 2021-12-24
*
*****************************************************************************************************/
#ifndef TABELHEADERVIEW_H_
#define TABELHEADERVIEW_H_

#include <QHeaderView>
#include <QObject>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>

class TableHeaderView : public QHeaderView
{
	Q_OBJECT
public:
	TableHeaderView(Qt::Orientation orientation, QWidget *parent);
	virtual ~TableHeaderView();
private:
	/************************
	* @brief: 绘制复选框
	* @author: mpp
	*************************/
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

	/************************
	* @brief: 鼠标按下表头
	* @author: mpp
	*************************/
    void mousePressEvent(QMouseEvent *event);

	/************************
	* @brief: 鼠标从表头释放，发送信号，更新model数据
	* @author: mpp
	*************************/
    void mouseReleaseEvent(QMouseEvent *event);

	/************************
	* @brief: 鼠标滑过、离开，更新复选框状态
	* @author: mpp
	*************************/
    bool event(QEvent *event);

public:
	bool GetChecked()
	{
		return m_bChecked;
	}
public:
signals :
	void SignalStateChanged(Qt::CheckState);
public slots:
	/************************
	* @brief: 槽函数，用于更新复选框状态
	* @author: mpp
	*************************/
    void SlotStateChanged(int state);
private:
	bool m_bPressed;    //
	bool m_bChecked;    //
	bool m_bTristate;     //
	bool m_bNoChange;    //
	bool m_bMoving;    //
private:
	const int m_iCheckBoxColumn;    //复选框所在的列
};

#endif // !TABELHEADERVIEW_H_
