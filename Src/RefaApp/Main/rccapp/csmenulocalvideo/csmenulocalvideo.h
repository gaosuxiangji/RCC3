/***************************************************************************************************
** @file: 菜单栏_采集_已导出视频_弹框界面
** @author: mpp
** @date: 2022/03/31
*
*****************************************************************************************************/

#ifndef CSMENULOCALVIDEO_H_
#define CSMENULOCALVIDEO_H_

#include <QtWidgets/QDialog>
#include "ui_csmenulocalvideo.h"
#include <QDateTime>
#include <algorithm>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class QStandardItemModel;

class CSMenuLocalVideo : public QDialog
{
	Q_OBJECT
public:
	/**************************
	* @brief: 视频列表数据参数类型
	* @author:mpp
	* @date:2022/03/31
	***************************/
	typedef struct VideoListParam
	{
		QDateTime time;    //用于排序
		QString strVideoName = "";    //视频名称
		QString strVideoFormat = "";    //视频格式
		QString strExportTime = "";    //录制时间	 -- Modifty by Juwc 2022/7/12 
		QString strVieoSize = "";    //视频大小
	}VideoListParam;
public:
	/**************************
	* @brief: 构造函数
	* @param: parent-父类指针
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	CSMenuLocalVideo(QWidget *parent = Q_NULLPTR);
	~CSMenuLocalVideo();

	/**************************
	* @brief: 设置视频目录
	* @param: videodir 视频目录
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void SetVideoDir(const QString& videodir);

	/**************************
	* @brief: 获取选中视频播放的路径
	* @param:
	* @return: 视频路径
	* @author:mpp
	* @date:2022/03/31
	***************************/
	QString GetDisplayVideoPath() const
	{
		return m_selectVideoPath;
	}
private:
	/**************************
	* @brief: 初始化UI文件
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void InitUI();

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void ConnectSignalSlot();

	/**************************
	* @brief: 设置视频列表数据
	* @param:items-视频列表项
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void SetVideoListItems(const QList<VideoListParam>& items);

	/**************************
	* @brief: 获取文件目录下的视频列表
	* @param: path 目录
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void ReadDirVideoList(const QString& path);

	/**************************
	* @brief: 解析文件名称
	* @param: filename  文件名称
	* @return: 解析字符串后得到的结构体
	* @author:mpp
	* @date:2022/04/01
	***************************/
	CSMenuLocalVideo::VideoListParam ParseFileNameString(const QString& fullPaht, const QString& filename);

	/**************************
	* @brief: 获取视频格式列表
	* @param:
	* @return: 视频格式列表
	* @author:mpp
	* @date:2022/04/06
	***************************/
	QStringList GetFileFormatList()
	{
		QStringList list;
		list << "*.rhvd" << "*.avi" << "*.mp4" << "*.raw" << "bmp*"<< "tif*" << "jpg*";
		return list;
	}

	QStringList GetPicFormatList()
	{
		QStringList list;
		list << "bmp" << "jpg" << "tif";
		return list;
	}

	QStringList GetVideoFormatList()
	{
		QStringList list;
		list << ".rhvd" << ".avi" << ".mp4" << ".raw";
		return list;
	}

	/**************************
	* @brief: QList排序
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/06
	***************************/
	void SortQList(QList<VideoListParam>& list);

	/**************************
	* @brief: 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/04/06
	***************************/
	static bool CompareVideoListParamData(const VideoListParam& param1, const VideoListParam& param2);
private slots:
	/**************************
	* @brief: 点击QTabelView槽函数
	* @param:index-
	* @return:
	* @author:mpp
	* @date:2022/03/31
	***************************/
	void SlotLocalVideoTableViewClicked(QModelIndex modelindex);

	void SlotInsertRow(int index);
	void SlotInsertFinished();
signals:
	void SignalInsertRow(int index);
	void SignalInsertFinished();

private:
	QStandardItemModel* m_VideoListModel;
	QString m_selectVideoPath;    //选中视频播放的路径
	QString m_dirPath;	//本地视频保存文件夹路径
	QList<VideoListParam> m_videoListParams;   //视频列表项

	std::atomic<int>									mIsRunning{0};
	std::thread											mReadVideoThrd;
	std::thread											mSetItemsThrd;
	mutable std::mutex									mMtx;
	std::condition_variable								mCondVar;
	QString												mLocalVideoPath;
	std::atomic<int>									mCrtIdx{0};
	int													mLastSetIdx = 0;

public:
	bool												doReadVideo();
	bool												SetVideoItems();

private:
	Ui::CSMenuLocalVideoClass ui;
};

#endif // CSMENULOCALVIDEO_H_
