#include "videolisttabelmodel.h"
#include <QColor>
#include <QFont>

VideoListTableModel::VideoListTableModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_iListNum(0)
	, m_listHeadText("")
	, m_bSendAllBtnSignal(false)
{
}

VideoListTableModel::~VideoListTableModel()
{
}

void VideoListTableModel::updateData(QList<VideoInfoRecordList> recordList)
{
	m_recordList = recordList;
	beginResetModel();
	endResetModel();
}

bool VideoListTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
	{
		return false;
	}
	int nColumn = index.column();
	VideoInfoRecordList record = m_recordList.at(index.row());
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::CheckStateRole:
	{
		return false;
	}
	case Qt::EditRole:
	{
		if (LIST_HEAD_VIDEO_INFO_RANGE_FRAMES == nColumn || LIST_HEAD_VIDEO_INFO_RANGE_TIME == nColumn)
		{
			QString strText = value.toString();
			if (strText.isEmpty())
			{
				return false;
			}
			QStringList vctStr = strText.split('-');
			if (vctStr.size() < 2)
			{
				vctStr = strText.split('/');
				if (vctStr.size() < 2)
				{
					vctStr = strText.split(':');
					if (vctStr.size() < 2)
					{
						vctStr = strText.split(';');
						if (vctStr.size() < 2)
						{
							vctStr = strText.split(',');
							if (vctStr.size() < 2)
							{
								vctStr = strText.split(':');
							}
						}
					}
				}
			}

			uint64_t uiStart = record.videoInfo.uiStartFrame;
			uint64_t uiEnd = record.videoInfo.uiEndFrame;
			if (vctStr.size() == 1)
			{
				uiStart = 1;
				if (LIST_HEAD_VIDEO_INFO_RANGE_FRAMES == nColumn)
				{					
					uiEnd = uiStart + strText.toLongLong() - 1;
				}
				if (LIST_HEAD_VIDEO_INFO_RANGE_TIME == nColumn)
				{
                    uiEnd = uiStart + qRound(strText.toDouble() * record.videoInfo.uiFrames / 1000) - 1;
				}			
				if (uiEnd > record.videoInfo.uiTotalFrame)
				{
					uiEnd = record.videoInfo.uiTotalFrame;
				}
			}
			else if (vctStr.size() > 1)
			{
				if (LIST_HEAD_VIDEO_INFO_RANGE_FRAMES == nColumn)
				{
					uiStart = vctStr[0].toLongLong();
					uiEnd = vctStr[1].toLongLong();
				}
				if (LIST_HEAD_VIDEO_INFO_RANGE_TIME == nColumn)
				{
					double starTime = vctStr[0].toDouble();
					double endTime = vctStr[1].toDouble();
                    uiStart = qRound(starTime * record.videoInfo.uiFrames / 1000);
                    uiEnd = qRound(endTime * record.videoInfo.uiFrames / 1000);
				}
				if (uiStart < 1)
				{
					uiStart = 1;
				}
				if (uiEnd > record.videoInfo.uiTotalFrame)
				{
					uiEnd = record.videoInfo.uiTotalFrame;
				}
			}
			else
			{
				return false;
			}
			if (uiStart > 0)
			{
				record.videoInfo.uiStartFrame = uiStart;
			}
			if (uiEnd > 0)
			{
				record.videoInfo.uiEndFrame = uiEnd;
			}
			if (uiStart > uiEnd)
			{
				return false;
			}
			m_recordList.replace(index.row(), record);
			emit SignalDataChanged();
			emit SignalExportRangeDataChanged(record);
			return true;
		}	
	}
	default:
		return false;
	}
	return false;
}

int VideoListTableModel::ReturnSelectNum()
{
	int selectNum = 0;
	return selectNum;
}

void  VideoListTableModel::SetListColNum(int col)
{
	m_iListNum = col;
}

void VideoListTableModel::SetHeadText(const QStringList& headtext)
{
	m_listHeadText = headtext;
}

QList<VideoInfoRecordList> VideoListTableModel::GetSelectInfoList()
{
	return m_recordList;
}

int VideoListTableModel::rowCount(const QModelIndex &parent) const
{
	return m_recordList.count();
}

int VideoListTableModel::columnCount(const QModelIndex &parent) const
{
	return m_iListNum;
}

QVariant VideoListTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int nRow = index.row();
	int nColumn = index.column();
	VideoInfoRecordList record = m_recordList.at(nRow);

	switch (role)
	{
	case Qt::TextColorRole:
		return QColor("#333333");
	case Qt::TextAlignmentRole:
		return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
	case Qt::EditRole:
	case Qt::ToolTipRole:
	case Qt::DisplayRole:
	{
		if ((nColumn >= 0) && (nColumn < 10))
		{
			auto exFrames = record.videoInfo.uiEndFrame - record.videoInfo.uiStartFrame + 1;
			if (LIST_HEAD_VIDEO_INFO_NAME == nColumn)
			{
				return tr("%1").arg(record.videoInfo.strName);
			}
			else if (LIST_HEAD_VIDEO_INFO_RECORD_TIME == nColumn)
			{
				return tr("%1").arg(record.videoInfo.strRecordTime);
			}
			else if (LIST_HEAD_VIDEO_INFO_FRAMES == nColumn)
			{
				return tr("%1").arg(record.videoInfo.uiFrames);
			}
			else if (LIST_HEAD_VIDEO_INFO_RESOLUTION == nColumn)
			{
				return tr("%1").arg(record.videoInfo.strResolution);
			}
			else if (LIST_HEAD_VIDEO_INFO_TOTAL_FRAMES == nColumn)
			{
				return tr("%1").arg(record.videoInfo.uiTotalFrame);
			}
			else if (LIST_HEAD_VIDEO_INFO_TOTAL_TIME == nColumn)
			{
				return tr("%1").arg((record.videoInfo.uiTotalFrame)*(1.0) / record.videoInfo.uiFrames * 1000);
			}
			else if (LIST_HEAD_VIDEO_INFO_RANGE_FRAMES == nColumn)
			{
				return tr("%1-%2").arg(record.videoInfo.uiStartFrame).arg(record.videoInfo.uiEndFrame);
			}
			else if (LIST_HEAD_VIDEO_INFO_RANGE_TIME == nColumn)
			{
				double startTime = record.videoInfo.uiStartFrame * (1.0) / record.videoInfo.uiFrames * 1000;
				double endTime = record.videoInfo.uiEndFrame * (1.0) / record.videoInfo.uiFrames * 1000;
				QString sStartTime = QString::number(QString::number(startTime,'d',1).toDouble(),'g',10);
				QString sEndTime = QString::number(QString::number(endTime, 'd', 1).toDouble(),'g');
				return tr("%1-%2").arg(sStartTime).arg(sEndTime);
			}
			else if (LIST_HEAD_VIDEO_INFO_LENGTH_FRAMES == nColumn)
			{
				return tr("%1").arg(exFrames);
			}
			else if (LIST_HEAD_VIDEO_INFO_LENGTH_TIME == nColumn)
			{			
				double timeLength = QString::number(exFrames* (1.0) / record.videoInfo.uiFrames * 1000, 'd', 1).toDouble();
				QString sTimeLength = QString::number(timeLength, 'g', 10);
				return tr("%1").arg(sTimeLength);
			}
		}
		return "";
	}
	case Qt::FontRole:
	{
		if (LIST_HEAD_VIDEO_INFO_RANGE_FRAMES == nColumn || LIST_HEAD_VIDEO_INFO_RANGE_TIME == nColumn)
		{
			QFont font;
			font.setBold(true);
			return font;
		}
		return false;
	}
	case Qt::UserRole:
	{
	}
	default:
		return QVariant();
	}

	return QVariant();
}

QVariant VideoListTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::TextAlignmentRole:
		return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
	case Qt::DisplayRole:
	case Qt::ToolTipRole:
	{
		if (orientation == Qt::Horizontal)
		{
			if (section < m_listHeadText.size())
			{
				return m_listHeadText[section];
			}
		}
	}
	default:
		return QVariant();
	}

	return QVariant();
}

Qt::ItemFlags VideoListTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index);

	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column() == LIST_HEAD_VIDEO_INFO_RANGE_FRAMES || index.column() == LIST_HEAD_VIDEO_INFO_RANGE_TIME)
	{
		flags |= Qt::ItemIsEditable;
	}

	return flags;
}
