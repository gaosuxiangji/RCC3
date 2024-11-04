#include "csmenulocalvideo.h"
#include "platform_config/platform_config.h"
#include <QStandardItemModel>
#include <QTableView>
#include <QDir>
#include <thread>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

CSMenuLocalVideo::CSMenuLocalVideo(QWidget *parent)
	: QDialog(parent)
	, m_selectVideoPath("")
	, m_dirPath("")
	, m_videoListParams({})
{
	ui.setupUi(this);
	InitUI();
	ConnectSignalSlot();
	mIsRunning = 1;
}

CSMenuLocalVideo::~CSMenuLocalVideo()
{
	mIsRunning = 0;
	if (mReadVideoThrd.joinable()) mReadVideoThrd.join();
	//if (mSetItemsThrd.joinable()) mSetItemsThrd.join();
}

void CSMenuLocalVideo::SetVideoDir(const QString& videodir)
{
	mIsRunning = 1;
	mLocalVideoPath = videodir;
	mReadVideoThrd = std::thread{ &CSMenuLocalVideo::doReadVideo,this };
	//mSetItemsThrd = std::thread{ &CSMenuLocalVideo::SetVideoItems,this };

	//ReadDirVideoList(videodir);
	//SetVideoListItems(m_videoListParams);
	//if (m_VideoListModel && (m_videoListParams.size() > 0) && (m_VideoListModel->rowCount() > 0) && \
	//	(m_VideoListModel->columnCount() > 0))
	//{//默认选中视频列表中的第一项
	//	ui.Local_Video_TableView->setCurrentIndex(m_VideoListModel->index(0, 0));
	//	m_selectVideoPath = m_dirPath + m_videoListParams[0].strVideoName;
	//}

	//// Add By Juwc -- 2022/7/6 添加视频名称和导出时间Tooltips
	//int VIDNAME = 0;
	//int EXPTIME = 2;
	//int nRow = m_VideoListModel->rowCount();
	//for (int row = 0; row < nRow; row++)
	//{
	//	QStandardItem* nameItem = m_VideoListModel->item(row, VIDNAME);
	//	if (nameItem)
	//		nameItem->setToolTip(nameItem->text());

	//	QStandardItem* timeItem = m_VideoListModel->item(row, EXPTIME);
	//	if (timeItem)
	//		timeItem->setToolTip(timeItem->text());
	//}
}

bool CSMenuLocalVideo::doReadVideo()
{
	ReadDirVideoList(mLocalVideoPath);
	return true;
}

bool CSMenuLocalVideo::SetVideoItems()
{
	while (mIsRunning)
	{
		std::unique_lock<std::mutex> lk(mMtx);
		mCondVar.wait(lk, [this] { return (mLastSetIdx < mCrtIdx) || !mIsRunning; });

		//auto video_item = m_videoListParams[mLastSetIdx];
		lk.unlock();
		emit SignalInsertRow(mLastSetIdx);

		//QList<QStandardItem*> list;
		//list << new QStandardItem(video_item.strVideoName) << new QStandardItem(video_item.strVideoFormat) << \
		//	new QStandardItem(video_item.strExportTime) << new QStandardItem(video_item.strVieoSize);*/
		//
		//m_VideoListModel->insertRow(mLastSetIdx, list);

		//if (mLastSetIdx == 0)
		//{
		//	//默认选中视频列表中的第一项
		//	ui.Local_Video_TableView->setCurrentIndex(m_VideoListModel->index(0, 0));
		//	m_selectVideoPath = m_dirPath + m_videoListParams[0].strVideoName;
		//}
		mLastSetIdx++;
		if (mIsRunning == 2 && mCrtIdx == mLastSetIdx)
		{
			emit SignalInsertFinished();
			//m_VideoListModel->sort(2, Qt::DescendingOrder);
			break;
		}
	}
	return true;
}

void CSMenuLocalVideo::InitUI()
{
	//设置弹框的尺寸
	this->setFixedSize(482, 290);

	//隐藏标题栏的问号和图标
	setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);

	//纵向表头不显示
	ui.Local_Video_TableView->verticalHeader()->setVisible(false);
	//设置表头不可点击
	ui.Local_Video_TableView->horizontalHeader()->setSectionsClickable(false);
	//设置选中时为整行选中
	ui.Local_Video_TableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	//不允许在图形界面修改内容
	ui.Local_Video_TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//设置只能选择一行
	ui.Local_Video_TableView->setSelectionMode(QAbstractItemView::SingleSelection);

	//视频列表
	m_VideoListModel = new QStandardItemModel(ui.Local_Video_TableView);
	m_VideoListModel->setColumnCount(4);
	QStringList headList;
	// Modifty by Juwc 2022/7/12
	headList << tr("Video Name") << tr("Recording Format") << tr("Export Time") << tr("Video Size(MB)");
	m_VideoListModel->setHorizontalHeaderLabels(headList);
	ui.Local_Video_TableView->setModel(m_VideoListModel);

	// Add By Juwc -- 2022/7/6 显示表格水平滚动条
	ui.Local_Video_TableView->setColumnWidth(0, 150);
	ui.Local_Video_TableView->setColumnWidth(1, 75);
	ui.Local_Video_TableView->setColumnWidth(2, 135);
	ui.Local_Video_TableView->setColumnWidth(3, 90);
}

void CSMenuLocalVideo::ConnectSignalSlot()
{
	bool ok = connect(ui.Local_Video_TableView, &QTableView::clicked, this, &CSMenuLocalVideo::SlotLocalVideoTableViewClicked);
	ok = connect(ui.CancelBtn, &QPushButton::clicked, this, &CSMenuLocalVideo::reject);
	ok = connect(ui.DisplayBtn, &QPushButton::clicked, this, &CSMenuLocalVideo::accept);
	ok = connect(this, &CSMenuLocalVideo::SignalInsertRow, this, &CSMenuLocalVideo::SlotInsertRow);
	ok = connect(this, &CSMenuLocalVideo::SignalInsertFinished, this, &CSMenuLocalVideo::SlotInsertFinished);
	Q_UNUSED(ok);
}

void CSMenuLocalVideo::SetVideoListItems(const QList<VideoListParam>& items)
{
	for (int i = 0; i < items.size(); ++i)
	{
		QList<QStandardItem*> list;
		list << new QStandardItem(items[i].strVideoName) << new QStandardItem(items[i].strVideoFormat) << \
			new QStandardItem(items[i].strExportTime) << new QStandardItem(items[i].strVieoSize);
		m_VideoListModel->insertRow(i, list);
	}
}

void CSMenuLocalVideo::ReadDirVideoList(const QString& path)
{
	if (path.isEmpty())
	{
		return;
	}

	//设置要遍历的的目录
	QDir fileDir(path);

	//设置文件过滤器
	QStringList nameFilters;

	//设置文件过滤格式
	nameFilters = GetFileFormatList();

	//将过滤后的文件名称存入到files列表中
	QStringList files = fileDir.entryList(nameFilters, QDir::Files | QDir::Dirs | QDir::Readable, QDir::Name);
	QList<VideoListParam> params;
	for (int i = 0; i < files.size(); ++i)
	{
		if (mIsRunning == 0) return;
		QString filePath = path;

		//反斜杠转斜杠
		filePath.replace("\\", "/");

		//清除换行符
		QStringList fileList = filePath.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

		if (fileList.size() > 0)
		{
			filePath = fileList[0];
			if (filePath.length() > 0)
			{
				if (QChar('/') != filePath.at(filePath.length() - 1))
				{
					filePath = filePath + "/";
				}
				m_dirPath = filePath;
				filePath = filePath + files[i];
			}
		}
		
		VideoListParam param = ParseFileNameString(filePath, files[i]);
		param.strVideoName = files[i];

		QFileInfo fileinfo(filePath);
		
		if (fileinfo.exists())
		{
			param.strExportTime = fileinfo.lastModified().toString("yyyyMMdd-hh:mm:ss");

			if (fileinfo.isFile())
			{
				param.strVieoSize = QString::number((double)fileinfo.size() / (1024 * 1024), 'f', 2);
			}
			else if(fileinfo.isDir())
			{
				QDir dir(filePath);
				QFileInfoList list = dir.entryInfoList();
				double fileSize = 0;
				for (int i=0;i<list.count();i++)
				{
					if ((list[i].fileName() != ".") && (list[i].fileName() != ".."))
					{
						fileSize += list[i].size();
					}
				}
				param.strVieoSize = QString::number(fileSize / (1024 * 1024), 'f', 2);
			}
		}
		m_videoListParams.push_back(param);
		emit SignalInsertRow(mCrtIdx);
		mCrtIdx++;
		//mCondVar.notify_one();
	}
	mIsRunning = 2;
	emit SignalInsertFinished();
	//mIsRunning = 2;
	//SortQList(m_videoListParams);
	//SetVideoListItems(m_videoListParams);
}

CSMenuLocalVideo::VideoListParam CSMenuLocalVideo::ParseFileNameString(const QString& fullPaht, const QString& filename)
{
	QFileInfo fileinfo(fullPaht);
	VideoListParam param;
	QString strFileName = filename;
	if (fileinfo.isFile())
	{
		for (auto &item : GetVideoFormatList())
		{
			if ((filename.length() >= item.length()) && (strFileName.right(item.length()).toLower() == item))
			{
				param.strVideoFormat = item.replace(".", "").toUpper();
			}
		}
	}
	else if(fileinfo.isDir())
	{
		for (auto &item : GetPicFormatList())
		{
			if ((filename.length() >= item.length()) && (strFileName.left(item.length()).toLower() == item))
			{
				param.strVideoFormat = item.toUpper();
			}
		}
	}
	
	return param;
}

bool CSMenuLocalVideo::CompareVideoListParamData(const VideoListParam& param1, const VideoListParam& param2)
{
	bool bRet = false;
	if (param1.time > param2.time)
	{
		bRet = true;
	}
	return bRet;
}

void CSMenuLocalVideo::SortQList(QList<VideoListParam>& list)
{
	for (auto &item : list)
	{
		item.time = QDateTime::fromString(item.strExportTime, "yyyyMMdd-hh:mm:ss");
	}

	qSort(list.begin(), list.end(), CompareVideoListParamData);

}

void CSMenuLocalVideo::SlotLocalVideoTableViewClicked(QModelIndex modelindex)
{
	QString strName = m_VideoListModel->data(m_VideoListModel->index(modelindex.row(), 0)).toString();
	m_selectVideoPath = m_dirPath + strName;
}


void CSMenuLocalVideo::SlotInsertRow(int index)
{
	auto video_item = m_videoListParams[index];
	QList<QStandardItem*> list;
	list << new QStandardItem(video_item.strVideoName) << new QStandardItem(video_item.strVideoFormat) << \
		new QStandardItem(video_item.strExportTime) << new QStandardItem(video_item.strVieoSize);

	m_VideoListModel->insertRow(index, list);
}

void CSMenuLocalVideo::SlotInsertFinished()
{
	while (m_VideoListModel->rowCount() < mCrtIdx || mIsRunning != 2)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
	}
	m_VideoListModel->sort(2, Qt::DescendingOrder);
	SortQList(m_videoListParams);

	//默认选中视频列表中的第一项
	if (m_VideoListModel && m_VideoListModel->columnCount() > 0 && m_VideoListModel->rowCount() > 0 && m_videoListParams.size())
	{
		ui.Local_Video_TableView->setCurrentIndex(m_VideoListModel->index(0, 0));
		m_selectVideoPath = m_dirPath + m_videoListParams[0].strVideoName;
	}
}
