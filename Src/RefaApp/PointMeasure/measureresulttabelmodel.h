#ifndef MEASURERESULTTABLEMODEL_H
#define MEASURERESULTTABLEMODEL_H

/***************************************************************************************************
** @file: 设置列表模块
** @author: 
** @date: 
*
*****************************************************************************************************/
#include "PointMeasure/CMeasureLineManage.h"

#include <QObject>
#include <QAbstractTableModel>

enum LIST_HEAD_MEASURE_RESULT_INFO
{
	LIST_HEAD_MEASURE_RESULT_INFO_CHECK,
	LIST_HEAD_MEASURE_RESULT_INFO_TYPE,
	LIST_HEAD_MEASURE_RESULT_INFO_NAME,
	LIST_HEAD_MEASURE_RESULT_INFO_LENGTH
};

class MeasureResultTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
    MeasureResultTableModel(QObject *parent);
	~MeasureResultTableModel();
public:
	/************************
	* @brief: 更新列表的数据
	* @param recordList: 列表数据
	* @author:
	*************************/
    void updateData(QList<CMeasureLineManage::TMeasureLineInfo> recordList);

	/************************
	* @brief: 设置表格数据
	* @param index: 鼠标选中项的信息，含有行数、列数等
	* @param value: 选中项的信息，比如字符串、是否选中框的bool型等
	* @param role: 选中的是哪一列
	* @author:
	*************************/
    bool setData(const QModelIndex &index, const QVariant &value, int role);

	/************************
	* @brief: 返回选中的项的数目
	* @author:
	*************************/
	int  ReturnSelectNum();

	/************************
	* @brief: 设置列表列数
	* @param col: 列数
	* @author:
	*************************/
	void  SetListColNum(int col);

	/************************
	* @brief: 设置勾选状态
	* @author:
	*************************/
	void setCheckedStatus(bool bChecked);

	/************************
	* @brief: 设置标题头文本
	* @param headtext: 表头文本信息
	* @author:
	*************************/
	void  SetHeadText(const QStringList& headtext);

	/************************
	* @brief: 返回所有的信息
	* @author:
	*************************/
	QList<CMeasureLineManage::TMeasureLineInfo> GetSelectInfoList();

	void SetDeviceName(const QString& devicename)
	{
		m_strDeiveName = devicename;
		m_vctMeasureLine.clear();
		if (!devicename.isEmpty())
		{
			QList<CMeasureLineManage::TMeasureLineInfo> vctInfo;
			CMeasureLineManage::instance().getMeasureLine(devicename, vctInfo);
			m_vctMeasureLine = vctInfo;
			updateInfo();
		}
	};

	void SetShowUnitCoefficient(double value) {
		m_dbShowUnitCoefficient = value; 
		updateInfo();
	}
private:
	/************************
	* @brief: 返回列表数据行数
	* @param parent:
	* @author: 
	*************************/
    int rowCount(const QModelIndex &parent) const;

	/************************
	* @brief: 返回列表数据列数
	* @param parent:
	* @author: 
	*************************/
    int columnCount(const QModelIndex &parent) const;

	/************************
	* @brief: 表格项数据
	* @param index: 鼠标选中项的信息，含有行数、列数等
	* @param role:选中的是哪一列
	* @author: 
	*************************/
    QVariant data(const QModelIndex &index, int role) const;

	/************************
	* @brief: 设置表头数据
	* @param section: 某一列
	* @param orientation: 横向
	* @param role:选中的是哪一行
	* @author: mpp
	*************************/
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

	/************************
	* @brief: 表格可选中、可复选
	* @param index:
	* @author: 
	*************************/
    Qt::ItemFlags flags(const QModelIndex &index) const;
signals:
	/************************
	* @brief: 鼠标点击信号
	* @param QModelIndex: 鼠标点击所在项的信息
	* @author: 
	*************************/
	void SignalIndex(QModelIndex);

	/************************
	* @brief: 鼠标点击信号
	* @param strFileName: 文件名称
	* @author: 
	*************************/
	void SignalButtonClick(QString strFileName);

	/************************
	* @brief: 鼠标点击表头信号
	* @param bchecked: 是否全选
	* @author: 
	*************************/
	void SignalClickHeader(bool bchecked);

	/************************
	* @brief: 
	* @author: 
	*************************/
	void signalGetCheckedStatus();

	/************************
	* @brief:
	* @author:
	*************************/
	void signalAddMeasureItem(int nIndex);

	public slots:
	/************************
	* @brief: 添加所有测量线信息
	* @param QString: strIP
	* @param TMeasureLineInfo: info
	*************************/
	void slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info);

	/************************
	* @brief: 更新单个测量线信息
	* @param QString: strIP
	* @param TMeasureLineInfo: info
	*************************/
	void slotUpdateMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info);

	/************************
	* @brief: 更新所有测量线信息
	* @param QString: strIP
	* @param QList<TMeasureLineInfo>: info
	*************************/
	void slotUpdateMeasureLines(const QString strIP, const QList<CMeasureLineManage::TMeasureLineInfo> info);

	/************************
	* @brief: 删除部分测量线信息
	* @param QString: strIP
	* @param QList<TMeasureLineInfo>: info
	*************************/
	void slotDeleteMeasureLines(const QString strIP, const QList<int> info);

	/************************
	* @brief: 清除测量线信息
	* @param QString: strIP
	*************************/
	void slotClearMeasureLines(const QString strIP);
	void slotMeasurePointOffset(const QString strIP, const QPoint point);
	void updateInfo();
private:
	int m_iListNum;    //列表列数
	QStringList m_listHeadText;    //表头文本信息列表
	bool m_bSendAllBtnSignal;    //是否发送全选按钮信号，防止重复发送 true-发送 false-不发送
	QString m_strDeiveName;
	QList<CMeasureLineManage::TMeasureLineInfo> m_vctMeasureLine;			 // 测量信息
	const int constNameLengthMax{ 64 };										 // 线段名称最大长度
	bool m_bAllChecked{ false };
	double m_dbShowUnitCoefficient{ 1.0 };
	QPoint m_ptOffset{ 0, 0 };
};
#endif //MEASURERESULTTABLEMODEL_H