/***************************************************************************************************
** @file: 设置列表模块
** @author: 
** @date: 
*
*****************************************************************************************************/

#include <QObject>
#include <QAbstractTableModel>

#define LIST_HEAD_VIDEO_INFO_NAME				0
#define LIST_HEAD_VIDEO_INFO_RECORD_TIME		1
#define LIST_HEAD_VIDEO_INFO_FRAMES				2
#define LIST_HEAD_VIDEO_INFO_RESOLUTION			3
#define LIST_HEAD_VIDEO_INFO_TOTAL_FRAMES		4
#define LIST_HEAD_VIDEO_INFO_TOTAL_TIME			5
#define LIST_HEAD_VIDEO_INFO_RANGE_FRAMES		6
#define LIST_HEAD_VIDEO_INFO_RANGE_TIME			7
#define LIST_HEAD_VIDEO_INFO_LENGTH_FRAMES		8
#define LIST_HEAD_VIDEO_INFO_LENGTH_TIME		9

typedef struct VideoInfoRecord
{
	uint64_t uiIndex;
	QString strName;
	QString strRecordTime;
	uint64_t uiFrames;
	QString strResolution;
	uint64_t uiStartFrame;
	uint64_t uiEndFrame;
	uint64_t uiTotalFrame;

}VideoInfoRecord;

typedef struct VideoInfoRecordList
{
	int nIndex;
	VideoInfoRecord videoInfo;
}VideoInfoRecordList;

class VideoListTableModel : public QAbstractTableModel
{
	Q_OBJECT
public:
    VideoListTableModel(QObject *parent);
	~VideoListTableModel();
public:
	/************************
	* @brief: 更新列表的数据
	* @param recordList: 列表数据
	* @author: 
	*************************/
    void updateData(QList<VideoInfoRecordList> recordList);

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
	* @brief: 设置标题头文本
	* @param headtext: 表头文本信息
	* @author: 
	*************************/
	void  SetHeadText(const QStringList& headtext);

	/************************
	* @brief: 返回选中的项的信息
	* @author: mpp
	*************************/
	QList<VideoInfoRecordList> GetSelectInfoList();
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
	void SignalStateChanged(Qt::CheckState);

	void SignalDataChanged();

	/************************
	* @brief: 修改导出范围信号
	* @param 
	* @author:
	*************************/
	void SignalExportRangeDataChanged(VideoInfoRecordList info);
private:
	QList<VideoInfoRecordList> m_recordList;    //列表中所有行的信息
	int m_iListNum;    //列表列数
	QStringList m_listHeadText;    //表头文本信息列表
	bool m_bSendAllBtnSignal;    //是否发送全选按钮信号，防止重复发送 true-发送 false-不发送
};
