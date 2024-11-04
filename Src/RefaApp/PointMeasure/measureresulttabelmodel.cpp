#include "measureresulttabelmodel.h"
#include <QColor>
#include <QFont>

MeasureResultTableModel::MeasureResultTableModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_iListNum(0)
	, m_listHeadText("")
	, m_bSendAllBtnSignal(false)
{
	// 绑定信号
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalAddMeasureLine, this, &MeasureResultTableModel::slotAddMeasureLine);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalUpdateMeasureLine, this, &MeasureResultTableModel::slotUpdateMeasureLine);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalUpdateMeasureLines, this, &MeasureResultTableModel::slotUpdateMeasureLines);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalDeleteMeasureLines, this, &MeasureResultTableModel::slotDeleteMeasureLines);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalClearMeasureLines, this, &MeasureResultTableModel::slotClearMeasureLines);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasurePointOffset, this, &MeasureResultTableModel::slotMeasurePointOffset);
}

MeasureResultTableModel::~MeasureResultTableModel()
{
}

void MeasureResultTableModel::updateData(QList<CMeasureLineManage::TMeasureLineInfo> recordList)
{
	m_vctMeasureLine = recordList;
	beginResetModel();
	endResetModel();
}

bool MeasureResultTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
	{
		return false;
	}
	int nColumn = index.column();
	CMeasureLineManage::TMeasureLineInfo record = m_vctMeasureLine.at(index.row());
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::CheckStateRole:
	{
		return false;
	}
	case Qt::EditRole:
	{
		if (LIST_HEAD_MEASURE_RESULT_INFO_NAME == nColumn)
		{
			QString strText = value.toString();
			int nLength = strText.toLocal8Bit().length();
			if (nLength > constNameLengthMax)
			{
				return false;
			}
			record.strName = strText;
			CMeasureLineManage::instance().updateOneMeasureLine(m_strDeiveName, record);
			return true;
		}	
	}
	default:
		return false;
	}
	return false;
}

int MeasureResultTableModel::ReturnSelectNum()
{
	int selectNum = 0;
	return selectNum;
}

void MeasureResultTableModel::SetListColNum(int col)
{
	m_iListNum = col;
}

void MeasureResultTableModel::setCheckedStatus(bool bChecked)
{
	m_bAllChecked = bChecked;
}

void MeasureResultTableModel::SetHeadText(const QStringList& headtext)
{
	m_listHeadText = headtext;
}

QList<CMeasureLineManage::TMeasureLineInfo> MeasureResultTableModel::GetSelectInfoList()
{
	return m_vctMeasureLine;
}

int MeasureResultTableModel::rowCount(const QModelIndex &parent) const
{
	return m_vctMeasureLine.count();
}

int MeasureResultTableModel::columnCount(const QModelIndex &parent) const
{
	return m_iListNum;
}

QVariant MeasureResultTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int nRow = index.row();
	int nColumn = index.column();
	CMeasureLineManage::TMeasureLineInfo record = m_vctMeasureLine.at(nRow);

	switch (role)
	{
	case Qt::TextColorRole:
		return QColor("#333333");
	case Qt::TextAlignmentRole:
		return QVariant(Qt::AlignCenter | Qt::AlignVCenter);
	case Qt::EditRole:
	case Qt::ToolTipRole:
		if (LIST_HEAD_MEASURE_RESULT_INFO_CHECK == nColumn)
		{
			return tr("%1").arg(record.strName);
		}
	case Qt::DisplayRole:
	{
		if ((nColumn >= 0) && (nColumn < 10))
		{
			if (LIST_HEAD_MEASURE_RESULT_INFO_CHECK == nColumn)
			{
				return Qt::Checked;
			}
			else if (LIST_HEAD_MEASURE_RESULT_INFO_NAME == nColumn)
			{
				return tr("%1").arg(record.strName);
			}
			else if (LIST_HEAD_MEASURE_RESULT_INFO_TYPE == nColumn)
			{
				QString strType = CMeasureLineManage::GetMeasureLineTypeStr(record.nType);
				return tr("%1").arg(strType);
			}
			else if (LIST_HEAD_MEASURE_RESULT_INFO_LENGTH == nColumn)
			{
				if (record.nType == CMeasureLineManage::MLT_DIMENSION)
				{
					qreal dbXValue = record.vctPoint[0].x() - m_ptOffset.x();
					qreal dbYValue = record.vctPoint[0].y() - m_ptOffset.y();
					dbXValue *= record.qrLength;
					dbYValue *= record.qrLength;
					if (m_dbShowUnitCoefficient < 0)
					{
						return "";
					}
					dbXValue *= m_dbShowUnitCoefficient;
					dbYValue *= m_dbShowUnitCoefficient;
					if (Qt::ToolTipRole == role)
					{
						return tr("%1,%2").arg(QString::number(dbXValue, 'f')).arg(QString::number(dbYValue, 'f'));
					}
					return tr("%1,%2").arg(QString::number(dbXValue, 'f', 3)).arg(QString::number(dbYValue, 'f', 3));
				}
				else if (record.nType == CMeasureLineManage::MLT_ANGLE_3 || record.nType == CMeasureLineManage::MLT_ANGLE_4)
				{
					if (Qt::ToolTipRole == role)
					{
						return tr("%1%2").arg(QString::number(record.qrLength, 'f')).arg(QStringLiteral("°"));
					}
					return tr("%1%2").arg(QString::number(record.qrLength, 'f', 3)).arg(QStringLiteral("°"));
				}
				else if (record.nType == CMeasureLineManage::MLT_AREA_CENTER || record.nType == CMeasureLineManage::MLT_AREA_POLYGON)
				{
					qreal lengthValue = record.qrLength;
					if (m_dbShowUnitCoefficient < 0)
					{
						return "";
					}
					lengthValue *= std::pow(m_dbShowUnitCoefficient, 2);
					if (Qt::ToolTipRole == role)
					{
						return tr("%1").arg(QString::number(lengthValue, 'f'));
					}
					return tr("%1").arg(QString::number(lengthValue, 'f', 3));
				}
				qreal lengthValue = record.qrLength;
				if (m_dbShowUnitCoefficient < 0)
				{
					return "";
				}
				lengthValue *= m_dbShowUnitCoefficient;
				if (Qt::ToolTipRole == role)
				{
					return tr("%1").arg(QString::number(lengthValue, 'f'));
				}
				return tr("%1").arg(QString::number(lengthValue, 'f', 3));
			}
		}
		return "";
	}
	case Qt::FontRole:
	{
		if (LIST_HEAD_MEASURE_RESULT_INFO_NAME == nColumn)
		{
			QFont font;
			font.setBold(true);
			return font;
		}
		return false;
	}
	case Qt::UserRole:
	{
		if (nColumn == LIST_HEAD_MEASURE_RESULT_INFO_CHECK)
		{
			return record.bChecked;
		}
	}
	default:
		return QVariant();
	}

	return QVariant();
}

QVariant MeasureResultTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags MeasureResultTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index);

	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	if (index.column() == LIST_HEAD_MEASURE_RESULT_INFO_NAME)
	{
		flags |= Qt::ItemIsEditable;
	}

	return flags;
}

void MeasureResultTableModel::slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	emit signalGetCheckedStatus();
	m_vctMeasureLine.append(info);
	m_vctMeasureLine[m_vctMeasureLine.size() - 1].bChecked = m_bAllChecked;
	updateInfo();
	emit signalAddMeasureItem(info.nIndex);
}

void MeasureResultTableModel::slotUpdateMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	int nSize = m_vctMeasureLine.size();
	for (int i = 0; i < nSize; i++)
	{
		if (info.nIndex == m_vctMeasureLine[i].nIndex)
		{
			bool bChecked = m_vctMeasureLine[i].bChecked;
			m_vctMeasureLine[i] = info;
			m_vctMeasureLine[i].bChecked = bChecked;
			break;
		}
	}
	updateInfo();
}

void MeasureResultTableModel::slotUpdateMeasureLines(const QString strIP, const QList<CMeasureLineManage::TMeasureLineInfo> info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	emit signalGetCheckedStatus();
	QList<CMeasureLineManage::TMeasureLineInfo> oldInfo = m_vctMeasureLine;
	m_vctMeasureLine = info;
	bool bReplaceEnd = false;
	if (m_vctMeasureLine.size() == oldInfo.size())
	{
		bReplaceEnd = true;
	}
	int nSize = m_vctMeasureLine.size();
	for (int i = 0; i < nSize; i++)
	{
		if (bReplaceEnd)
		{
			m_vctMeasureLine[i].bChecked = oldInfo[i].bChecked;
		}
		else
		{
			m_vctMeasureLine[i].bChecked = m_bAllChecked;
		}
	}
	updateInfo();
}

void MeasureResultTableModel::slotDeleteMeasureLines(const QString strIP, const QList<int> vctLine)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	int nSize = vctLine.size();
	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < m_vctMeasureLine.size(); j++)
		{
			if (vctLine[i] == m_vctMeasureLine[j].nIndex)
			{
				m_vctMeasureLine.removeAt(j);
				break;
			}
		}
	}
	updateInfo();
}

void MeasureResultTableModel::slotClearMeasureLines(const QString strIP)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_vctMeasureLine.clear();
	updateInfo();
}

void MeasureResultTableModel::slotMeasurePointOffset(const QString strIP, const QPoint point)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_ptOffset = point;
	updateInfo();
}

void MeasureResultTableModel::updateInfo()
{
	beginResetModel();
	endResetModel();
}