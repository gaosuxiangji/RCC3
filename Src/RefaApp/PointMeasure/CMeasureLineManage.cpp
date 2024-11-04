#include "CMeasureLineManage.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

CMeasureLineManage & CMeasureLineManage::instance()
{
	// TODO: 在此处插入 return 语句
	static CMeasureLineManage inst;
	return inst;
}

CMeasureLineManage::CMeasureLineManage(QObject *parent)
	: QObject(parent)
{
	readCalInfo();
}

CMeasureLineManage::~CMeasureLineManage()
{
	writeCalInfo();
}

double CMeasureLineManage::PointDistance(const QPointF & p1, const QPointF & p2)
{
	return qSqrt(qPow((p1.x() - p2.x()), 2) + qPow((p1.y() - p2.y()), 2));
}

double CMeasureLineManage::PointsDistance(const QList<QPoint>& pts)
{
	int nSize = pts.size();
	double dbLength = 0.0;
	for (int i = 1; i < nSize; i++)
	{
		dbLength += PointDistance(pts[i - 1], pts[i]);
	}
	return dbLength;
}

double CMeasureLineManage::PointsAngle(const QList<QPoint>& pts)
{
	int nSize = pts.size();
	double dbLength = 0.0;
	if (nSize == 3)
	{
		return GetAngleOfThreePoint(pts[0], pts[2], pts[1]);
	}
	else if (nSize == 4)
	{
		return GetAngleOfFourPoint(pts[0], pts[1], pts[2], pts[3]);
	}
	return dbLength;
}

double CMeasureLineManage::GetAngleOfThreePoint(const QPoint &ptStart, const QPoint &ptEnd, const QPoint &ptCenter)
{
	double theta = std::atan2(ptStart.x() - ptCenter.x(), ptStart.y() - ptCenter.y()) - std::atan2(ptEnd.x() - ptCenter.x(), ptEnd.y() - ptCenter.y());
	if (theta > M_PI)
	{
		theta -= 2 * M_PI;
	}
	if (theta < -M_PI)
	{
		theta += 2 * M_PI;
	}
	theta = theta * 180.0 / M_PI;
	return theta;
}

double CMeasureLineManage::GetAngleOfFourPoint(const QPoint &pt1, const QPoint &pt2, const QPoint &pt3, const QPoint &pt4)
{
	long double x = (pt1.x() - pt2.x())*(pt1.x() - pt2.x()) + (pt1.y() - pt2.y())*(pt1.y() - pt2.y());
	long double y = (pt3.x() - pt4.x())*(pt3.x() - pt4.x()) + (pt3.y() - pt4.y())*(pt3.y() - pt4.y());
	long double s1 = std::sqrt(x);
	long double s2 = std::sqrt(y);
	long double s3 = (pt2.x() - pt1.x())*(pt4.x() - pt3.x()) + (pt2.y() - pt1.y())*(pt4.y() - pt3.y());
	long double s4 = std::abs(s3);
	if (s1 == 0.0 || s2 == 0.0)
	{
		return 0.0;
	}
	long double ans2 = s4 / (s1*s2);
	long double ans = std::acos(ans2);
	long double ans3 = (180 * ans) / M_PI;
	return ans3;
}

QPointF CMeasureLineManage::GetRadiusOfThreePoint(const QPoint &pt1, const QPoint &pt2, const QPoint &pt3, double &dbRadius)
{
	double dbX1 = pt1.x(), dbX2 = pt2.x(), dbX3 = pt3.x();
	double dbY1 = pt1.y(), dbY2 = pt2.y(), dbY3 = pt3.y();
	double dbA = dbX1 - dbX2;
	double dbB = dbY1 - dbY2;
	double dbC = dbX1 - dbX3;
	double dbD = dbY1 - dbY3;
	double dbE = ((dbX1 * dbX1 - dbX2 * dbX2) + (dbY1 * dbY1 - dbY2 * dbY2)) / 2.0;
	double dbF = ((dbX1 * dbX1 - dbX3 * dbX3) + (dbY1 * dbY1 - dbY3 * dbY3)) / 2.0;
	double dbDet = dbB * dbC - dbA * dbD;
	if (std::fabs(dbDet) < 1e-5)
	{
		dbRadius = -1;
		return QPointF(0, 0);
	}

	double dbX0 = -(dbD * dbE - dbB * dbF) / dbDet;
	double dbY0 = -(dbA * dbF - dbC * dbE) / dbDet;
	dbRadius = std::hypot(dbX1 - dbX0, dbY1 - dbY0);
	return QPointF(dbX0, dbY0);
}

double CMeasureLineManage::GetPolygonArea(const QList<QPoint> & pts)
{
	int nSize = pts.size();
	double dbArea = 0.0;
	if (nSize >= 3)
	{
		for (int i = 0; i < nSize; i++)
		{
			QPoint p1 = pts[i];
			QPoint p2 = pts[(i + 1) % nSize];
			double d = p1.x() * p2.y() - p2.x() * p1.y();
			dbArea += d;
		}
		return std::abs(dbArea) / 2;
	}
	return dbArea;
}

QString CMeasureLineManage::GetMeasureCalTypeStr(TMeasureLineUnit info)
{
	QString strText;
	switch (info)
	{
	case CMeasureLineManage::MLU_Pixel:
		strText = tr("px");
		break;
	case CMeasureLineManage::MLU_MM:
		strText = tr("mm");
		break;
	case CMeasureLineManage::MLU_CM:
		strText = tr("cm");
		break;
	case CMeasureLineManage::MLU_M:
		strText = tr("m");
		break;
	default:
		strText = tr("");
		break;
	}
	return qMove(strText);
}

QString CMeasureLineManage::GetMeasureCalTypeOrAreaStr(TMeasureLineUnit info)
{
	QString strText;
	char16_t square = 0xB2;
	QString strSquare = QString::fromUtf16(&square, 1);
	switch (info)
	{
	case CMeasureLineManage::MLU_Pixel:
		strText = tr("px/px%1").arg(strSquare);
		break;
	case CMeasureLineManage::MLU_MM:
		strText = tr("mm/mm%1").arg(strSquare);
		break;
	case CMeasureLineManage::MLU_CM:
		strText = tr("cm/cm%1").arg(strSquare);
		break;
	case CMeasureLineManage::MLU_M:
		strText = tr("m/m%1").arg(strSquare);
		break;
	default:
		strText = tr("");
		break;
	}
	return qMove(strText);
}

QString CMeasureLineManage::GetMeasureLineTypeStr(TMeasureLineType info)
{
	QString strText;
	switch (info)
	{
	case CMeasureLineManage::MLT_TWO_POINT:
		strText = tr("Two distance");
		break;
	case CMeasureLineManage::MLT_MORE_POINT:
		strText = tr("More distance");
		break;
	case CMeasureLineManage::MLT_TWO_CALIBRATION:
		strText = tr("");
		break;
	case CMeasureLineManage::MLT_ANGLE_3:
		strText = tr("Angle(1)");
		break;
	case CMeasureLineManage::MLT_ANGLE_4:
		strText = tr("Angle(2)");
		break;
	case CMeasureLineManage::MLT_RADIUS:
		strText = tr("Radius");
		break;
	case CMeasureLineManage::MLT_DIAMETER:
		strText = tr("Diameter");
		break;
	case CMeasureLineManage::MLT_CENTER_DISTANCE:
		strText = tr("Center distance");
		break;
	case CMeasureLineManage::MLT_DIMENSION:
		strText = tr("Dimension");
		break;
	case CMeasureLineManage::MLT_AREA_CENTER:
		strText = tr("Circle area");
		break;
	case CMeasureLineManage::MLT_AREA_POLYGON:
		strText = tr("Polygon area");
		break;
	default:
		strText = tr("");
		break;
	}
	return qMove(strText);
}

void CMeasureLineManage::addMeasureLine(const QString strIP, const TMeasureLineInfo info)
{
	TMeasureLineInfo temp = info;
	calMeasureLine(strIP, temp);
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		m_mapIP2MeasureLineInfo[strIP].append(temp);
		emit signalAddMeasureLine(strIP, temp);
	}
	else
	{
		QList<TMeasureLineInfo> vctLine;
		vctLine.append(temp);
		m_mapIP2MeasureLineInfo[strIP] = vctLine;
		emit signalAddMeasureLine(strIP, temp);
	}
	emit signalMeasureLineCountChange(strIP, m_mapIP2MeasureLineInfo[strIP].size());
}

void CMeasureLineManage::updateOneMeasureLine(const QString strIP, const TMeasureLineInfo info)
{
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		for (auto & temp:m_mapIP2MeasureLineInfo[strIP])
		{
			if (info.nIndex == temp.nIndex)
			{
				temp = info;
				calMeasureLine(strIP, temp);
				emit signalUpdateMeasureLine(strIP, temp);
			}
		}
	}
}

void CMeasureLineManage::updateMeasureLines(const QString strIP, const QList<TMeasureLineInfo> info)
{
	m_mapIP2MeasureLineInfo[strIP] = info;
	calMeasureLines(strIP, m_mapIP2MeasureLineInfo[strIP]);
	emit signalUpdateMeasureLines(strIP, m_mapIP2MeasureLineInfo[strIP]);
	emit signalMeasureLineCountChange(strIP, m_mapIP2MeasureLineInfo[strIP].size());
}

void CMeasureLineManage::deleteMeasureLines(const QString strIP, const QList<TMeasureLineInfo> info)
{
	QList<int> vctIndex;
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		for (auto temp : info)
		{
			int nSize = m_mapIP2MeasureLineInfo[strIP].size();
			for (int i = 0; i < nSize; i++)
			{
				if (temp.nIndex == m_mapIP2MeasureLineInfo[strIP].at(i).nIndex)
				{
					vctIndex.append(temp.nIndex);
					m_mapIP2MeasureLineInfo[strIP].removeAt(i);
					break;
				}
			}
		}
	}
	if (vctIndex.size() > 0)
	{
		emit signalDeleteMeasureLines(strIP, vctIndex);
		emit signalMeasureLineCountChange(strIP, m_mapIP2MeasureLineInfo[strIP].size());
	}
}

void CMeasureLineManage::deleteMeasureLines(const QString strIP, const QList<int> info)
{
	QList<int> vctIndex;
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		for (auto temp : info)
		{
			int nSize = m_mapIP2MeasureLineInfo[strIP].size();
			for (int i = 0; i < nSize; i++)
			{
				if (temp == m_mapIP2MeasureLineInfo[strIP].at(i).nIndex)
				{
					vctIndex.append(temp);
					m_mapIP2MeasureLineInfo[strIP].removeAt(i);
					break;
				}
			}
		}
	}
	if (vctIndex.size() > 0)
	{
		emit signalDeleteMeasureLines(strIP, vctIndex);
		emit signalMeasureLineCountChange(strIP, m_mapIP2MeasureLineInfo[strIP].size());
	}
}
void CMeasureLineManage::clearMeasureLines(const QString strIP)
{
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		m_mapIP2MeasureLineInfo.remove(strIP);
	}
	emit signalClearMeasureLines(strIP);
	emit signalMeasureLineCountChange(strIP, 0);
}

void CMeasureLineManage::getMeasureLine(const QString strIP, QList<TMeasureLineInfo>& info)
{
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		info = m_mapIP2MeasureLineInfo[strIP];
	}
}

int CMeasureLineManage::getFreeMeasureLineIndex(const QString strIP)
{
	if (m_mapIP2MeasureLineInfo.contains(strIP))
	{
		int nSize = m_mapIP2MeasureLineInfo[strIP].size();
		for (int i = 0; i < nSize; i++)
		{
			bool bFind = false;
			for (auto temp: m_mapIP2MeasureLineInfo[strIP])
			{
				if (temp.nIndex == i)
				{
					bFind = true;
					break;
				}
			}
			if (!bFind)
			{
				return i;
			}
		}
		return nSize;
	}
	return 0;
}

void CMeasureLineManage::setMeasureLineType(const QString & ip, const CMeasureLineManage::TMeasureLineType Type)
{
	emit signalMeasureLineType(ip, Type);
}

void CMeasureLineManage::setMeasureCal(const QString strIP, const TMesaureLineCal info)
{
	bool bModify = false;
	if (m_mapIP2MeasureCalInfo.contains(strIP))
	{
		if (m_mapIP2MeasureCalInfo[strIP].dbUnit != info.dbUnit)
		{
			bModify = true;
		}
		else if (m_mapIP2MeasureCalInfo[strIP].type != info.type)
		{
			bModify = true;
		}
	}
	else
	{
		if (info.type != MLU_Pixel)
		{
			bModify = true;
		}
	}
	m_mapIP2MeasureCalInfo[strIP] = info;
	if (bModify)
	{
		QList<TMeasureLineInfo> vctLine;
		getMeasureLine(strIP, vctLine);
		updateMeasureLines(strIP, vctLine);
	}
	emit signalMeasureCal(strIP, info);
}

void CMeasureLineManage::clearMeasureCal(const QString strIP)
{
	bool bModify = false;
	if (m_mapIP2MeasureCalInfo.contains(strIP))
	{
		if (m_mapIP2MeasureCalInfo[strIP].type != MLU_Pixel)
		{
			bModify = true;
		}
		else if (m_mapIP2MeasureCalInfo[strIP].dbUnit != 1.0)
		{
			bModify = true;
		}
		m_mapIP2MeasureCalInfo.remove(strIP);
	}
	if (bModify)
	{
		QList<TMeasureLineInfo> vctLine;
		getMeasureLine(strIP, vctLine);
		updateMeasureLines(strIP, vctLine);
	}

	emit signalMeasureCal(strIP, TMesaureLineCal());
}

void CMeasureLineManage::getMeasureCal(const QString strIP, TMesaureLineCal & info)
{
	if (m_mapIP2MeasureCalInfo.contains(strIP))
	{
		info =  m_mapIP2MeasureCalInfo[strIP];
	}
	else
	{
		info = TMesaureLineCal();
	}
}

void CMeasureLineManage::updateMeasureCalInfo(const QString strIP, const TMeasureLineInfo info)
{
	double dbValue = PointsDistance(info.vctPoint);
	emit signalUpdateMeasureCal(strIP, dbValue);
}

void CMeasureLineManage::gotoMeasureCal(const QString strIP, double dbValue, int nType)
{
	emit signalGotoMeasureCal(strIP, dbValue, nType);
}

void CMeasureLineManage::setMeasureCalMode(const QString strIP, const TMeasureModeType info)
{
	emit signalMeasureCalMode(strIP, info);
}

void CMeasureLineManage::emitCommonSignal(const QString strIP, const TMeasureCommonSignalType commandType, QVariant info)
{
	emit signalCommonSignal(strIP, commandType, info);
}

void CMeasureLineManage::calMeasureLine(const QString strIP, TMeasureLineInfo &info)
{
	TMesaureLineCal calInfo;
	getMeasureCal(strIP, calInfo);
	switch (info.nType)
	{
	case MLT_TWO_POINT:
	case MLT_MORE_POINT:
	case MLT_TWO_CALIBRATION:
	{
		double dbValue = PointsDistance(info.vctPoint);
		if (calInfo.type != MLU_Pixel)
		{
			dbValue *= calInfo.dbUnit;
		}
		info.qrLength = dbValue;
		break;
	}
	case MLT_ANGLE_3:
	case MLT_ANGLE_4:
	{
		double dbValue = PointsAngle(info.vctPoint);
		info.qrLength = dbValue;
		break;
	}
	case MLT_RADIUS:
	{
		if (info.vctPoint.size() >= 3)
		{
			double dbRadius = -1;
			QPointF center = GetRadiusOfThreePoint(info.vctPoint[0], info.vctPoint[1], info.vctPoint[2], dbRadius);
			if (dbRadius > 0)
			{
				if (calInfo.type != MLU_Pixel)
				{
					dbRadius *= calInfo.dbUnit;
				}
				info.qrLength = dbRadius;
			}
		}
		break;
	}
	case MLT_DIAMETER:
	{
		if (info.vctPoint.size() >= 3)
		{
			double dbRadius = -1;
			QPointF center = GetRadiusOfThreePoint(info.vctPoint[0], info.vctPoint[1], info.vctPoint[2], dbRadius);
			if (dbRadius > 0)
			{
				if (calInfo.type != MLU_Pixel)
				{
					dbRadius *= calInfo.dbUnit;
				}
				info.qrLength = 2*dbRadius;
			}
		}
		break;
	}
	case MLT_CENTER_DISTANCE:
	{
		if (info.vctPoint.size() >= 6)
		{
			double dbRadius1 = -1;
			QPointF center1 = GetRadiusOfThreePoint(info.vctPoint[0], info.vctPoint[1], info.vctPoint[2], dbRadius1);
			if (dbRadius1 > 0)
			{
				double dbRadius2 = -1;
				QPointF center2 = GetRadiusOfThreePoint(info.vctPoint[3], info.vctPoint[4], info.vctPoint[5], dbRadius2);
				double dbValue = PointDistance(center1, center2);
				if (calInfo.type != MLU_Pixel)
				{
					dbValue *= calInfo.dbUnit;
				}
				info.qrLength = dbValue;
			}
		}
		break;
	}
	case MLT_DIMENSION:
	{
		double dbValue = 1.0;
		if (calInfo.type != MLU_Pixel)
		{
			dbValue = calInfo.dbUnit;
		}
		info.qrLength = dbValue;
		break;
	}
	case MLT_AREA_CENTER:
	{
		if (info.vctPoint.size() >= 3)
		{
			double dbRadius = -1;
			QPointF center = GetRadiusOfThreePoint(info.vctPoint[0], info.vctPoint[1], info.vctPoint[2], dbRadius);
			if (dbRadius > 0)
			{
				dbRadius = dbRadius*dbRadius*M_PI;
				if (calInfo.type != MLU_Pixel)
				{
					dbRadius *= std::pow(calInfo.dbUnit, 2);
				}
				info.qrLength = dbRadius;
			}
		}
		break;
	}
	case MLT_AREA_POLYGON:
	{
		if (info.vctPoint.size() >= 3)
		{
			double dbArea = GetPolygonArea(info.vctPoint);
			if (dbArea > 0)
			{
				if (calInfo.type != MLU_Pixel)
				{
					dbArea *= std::pow(calInfo.dbUnit, 2);
				}
				info.qrLength = dbArea;
			}
		}
		break;
	}
	default:
		break;
	}
}

void CMeasureLineManage::calMeasureLines(const QString strIP, QList<TMeasureLineInfo> &info)
{
	int nSize = info.size();
	TMesaureLineCal calInfo;
	getMeasureCal(strIP, calInfo);
	for (int i = 0; i < nSize; i++)
	{
		switch (info[i].nType)
		{
		case MLT_TWO_POINT:
		case MLT_MORE_POINT:
		case MLT_TWO_CALIBRATION:
		{
			double dbValue = PointsDistance(info[i].vctPoint);
			if (calInfo.type != MLU_Pixel)
			{
				dbValue *= calInfo.dbUnit;
			}
			info[i].qrLength = dbValue;
			break;
		}
		case MLT_ANGLE_3:
		case MLT_ANGLE_4:
		{
			info[i].qrLength = PointsAngle(info[i].vctPoint);
			break;
		}
		case MLT_RADIUS:
		{
			if (info[i].vctPoint.size() >= 3)
			{
				double dbRadius = -1;
				QPointF center = GetRadiusOfThreePoint(info[i].vctPoint[0], info[i].vctPoint[1], info[i].vctPoint[2], dbRadius);
				if (dbRadius > 0)
				{
					if (calInfo.type != MLU_Pixel)
					{
						dbRadius *= calInfo.dbUnit;
					}
					info[i].qrLength = dbRadius;
				}
			}
			break;
		}
		case MLT_DIAMETER:
		{
			if (info[i].vctPoint.size() >= 3)
			{
				double dbRadius = -1;
				QPointF center = GetRadiusOfThreePoint(info[i].vctPoint[0], info[i].vctPoint[1], info[i].vctPoint[2], dbRadius);
				if (dbRadius > 0)
				{
					if (calInfo.type != MLU_Pixel)
					{
						dbRadius *= calInfo.dbUnit;
					}
					info[i].qrLength = 2 * dbRadius;
				}
			}
			break;
		}
		case MLT_CENTER_DISTANCE:
		{
			if (info[i].vctPoint.size() >= 6)
			{
				double dbRadius1 = -1;
				QPointF center1 = GetRadiusOfThreePoint(info[i].vctPoint[0], info[i].vctPoint[1], info[i].vctPoint[2], dbRadius1);
				if (dbRadius1 > 0)
				{
					double dbRadius2 = -1;
					QPointF center2 = GetRadiusOfThreePoint(info[i].vctPoint[3], info[i].vctPoint[4], info[i].vctPoint[5], dbRadius2);
					double dbValue = PointDistance(center1, center2);
					if (calInfo.type != MLU_Pixel)
					{
						dbValue *= calInfo.dbUnit;
					}
					info[i].qrLength = dbValue;
				}
			}
			break;
		}
		case MLT_DIMENSION:
		{
			double dbValue = 1.0;
			if (calInfo.type != MLU_Pixel)
			{
				dbValue = calInfo.dbUnit;
			}
			info[i].qrLength = dbValue;
			break;
		}
		case MLT_AREA_CENTER:
		{
			if (info[i].vctPoint.size() >= 3)
			{
				double dbRadius = -1;
				QPointF center = GetRadiusOfThreePoint(info[i].vctPoint[0], info[i].vctPoint[1], info[i].vctPoint[2], dbRadius);
				if (dbRadius > 0)
				{
					dbRadius = dbRadius*dbRadius*M_PI;
					if (calInfo.type != MLU_Pixel)
					{
						dbRadius *= std::pow(calInfo.dbUnit, 2);
					}
					info[i].qrLength = dbRadius;
				}
			}
			break;
		}
		case MLT_AREA_POLYGON:
		{
			if (info[i].vctPoint.size() >= 3)
			{
				double dbArea = GetPolygonArea(info[i].vctPoint);
				if (dbArea > 0)
				{
					if (calInfo.type != MLU_Pixel)
					{
						dbArea *= std::pow(calInfo.dbUnit, 2);
					}
					info[i].qrLength = dbArea;
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void CMeasureLineManage::setMeasureModeType(const QString strIP, const TMeasureModeType info)
{
	if (m_mapIP2MeasureModeType.contains(strIP))
	{
	}
	else
	{
	}
	m_mapIP2MeasureModeType[strIP] = info;
	emit signalMeasureModeType(strIP, m_mapIP2MeasureModeType[strIP]);
}

void CMeasureLineManage::getMeasureModeType(const QString strIP, TMeasureModeType & info)
{
	if (m_mapIP2MeasureModeType.contains(strIP))
	{
		info = m_mapIP2MeasureModeType[strIP];
	}
	else
	{
		info = MMT_Normal;
	}
}

void CMeasureLineManage::setMeasurePointOffset(const QString strIP, const QPoint point)
{
	emit signalMeasurePointOffset(strIP, point);
}

void CMeasureLineManage::readCalInfo()
{
	QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	QDir fileDir(appDataPath);
	if (!fileDir.exists())
	{
		if (!fileDir.mkdir(appDataPath))
		{
			return;
		}
	}
	appDataPath += constMeasureLineFile;
	m_strMeasureFile = appDataPath;
	QFile file(appDataPath);
	if (!file.exists())
	{
		return;
	}
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}
	QByteArray byteArr = file.readAll();
	QJsonParseError err;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(byteArr, &err);
	if (err.error != QJsonParseError::NoError)
	{
		return;
	}
	QJsonObject rootObj = jsonDoc.object();
	QJsonValue root = rootObj.value(constMeasureRoot);
	if (!root.isNull())
	{
		QJsonArray vctJson = root.toArray();
		int nCount = vctJson.count();
		for (int i = 0; i < nCount; i++)
		{
			QJsonValue oneJson = vctJson.at(i);
			QJsonObject tmp = oneJson.toObject();
			if (tmp.isEmpty())
			{
				continue;
			}
			QString strIP;
			QJsonValue jsonIP = tmp.value(constMeasureip);
			if (!jsonIP.isNull())
			{
				strIP = jsonIP.toString();
			}
			if (strIP.isEmpty())
			{
				continue;
			}
			TMesaureLineCal calInfo;
			QJsonValue jsonCalibration = tmp.value(constMeasurecalibration);
			if (!jsonCalibration.isNull())
			{
				calInfo.dbUnit = jsonCalibration.toDouble();
			}
			QJsonValue jsonUnit = tmp.value(constMeasureunit);
			if (!jsonUnit.isNull())
			{
				int nType = jsonUnit.toInt();
				if (nType <= MLU_Pixel || nType > MLU_Count)
				{
					continue;
				}
				else
				{
					calInfo.type = (TMeasureLineUnit)jsonUnit.toInt();
				}
			}
			if (calInfo.dbUnit < 0.001)
			{
				calInfo.dbUnit = 0.001;
			}
			calInfo.bCalibration = true;
			QJsonValue jsonCalibrationType = tmp.value(constMeasurecalibrationtype);
			if (!jsonCalibrationType.isNull())
			{
				int nType = jsonCalibrationType.toInt();
				if (nType < MCT_SINGLE_PIXEL || nType > MCT_TWO_POINT)
				{
					nType = MCT_SINGLE_PIXEL;
				}
				calInfo.tCalibrationType = (TMeasureCalibrationType)nType;
				if (nType == MCT_TWO_POINT)
				{
					QJsonValue jsoncalibrationpoints = tmp.value(constMeasurecalibrationpoints);
					if (!jsoncalibrationpoints.isNull())
					{
						if (jsoncalibrationpoints.isArray())
						{
							QJsonArray vctJson = jsoncalibrationpoints.toArray();
							for (int i = 0; i < vctJson.size(); i++)
							{
								QJsonValue oneJson = vctJson.at(i);
								QJsonObject onePoint = oneJson.toObject();
								if (onePoint.isEmpty())
								{
									continue;
								}
								QJsonValue jsonPointX = onePoint.value(constMeasureX);
								QJsonValue jsonPointY = onePoint.value(constMeasureY);
								if (!jsonPointX.isNull() && !jsonPointY.isNull())
								{
									QPoint point;
									point.setX(jsonPointX.toDouble());
									point.setY(jsonPointY.toDouble());
									calInfo.vctPoint.append(point);
								}
							}
						}
					}
					QJsonValue jsoncalibrationdistance = tmp.value(constMeasurecalibrationvalue);
					if (!jsoncalibrationdistance.isNull())
					{
						calInfo.dbPoint2Distance = jsoncalibrationdistance.toDouble();
						double dbValue = PointsDistance(calInfo.vctPoint);
						if (dbValue < 0.001)
						{
							dbValue = 0.001;
						}
						calInfo.dbUnit = calInfo.dbPoint2Distance / dbValue;
					}
				}
			}
			m_mapIP2MeasureCalInfo[strIP] = calInfo;
		}
	}
}

void CMeasureLineManage::writeCalInfo()
{
	if (m_strMeasureFile.isEmpty())
	{
		QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
		QDir fileDir(appDataPath);
		if (!fileDir.exists())
		{
			if (!fileDir.mkdir(appDataPath))
			{
				return;
			}
		}
		appDataPath += constMeasureLineFile;
		m_strMeasureFile = appDataPath;
	}
	QFile file(m_strMeasureFile);
	if (!file.open((QIODevice::WriteOnly)))
	{
		return;
	}

	QJsonArray jsonVct;
	for (auto it = m_mapIP2MeasureCalInfo.begin(); it != m_mapIP2MeasureCalInfo.end(); it++)
	{
		TMesaureLineCal calInfo = it.value();
		QString strIP = it.key();
		QJsonObject jsonObject;
		jsonObject.insert(constMeasureip, strIP);
		jsonObject.insert(constMeasurecalibration, calInfo.dbUnit);
		jsonObject.insert(constMeasureunit, (int)calInfo.type);
		jsonObject.insert(constMeasurecalibrationtype, (int)calInfo.tCalibrationType);
		if (calInfo.tCalibrationType == MCT_TWO_POINT)
		{
			int nPointSize = calInfo.vctPoint.size();
			if (nPointSize > 1)
			{
				QJsonArray jsonPointVct;
				for (int i = 0; i < nPointSize; ++i)
				{
					QJsonObject jsonPoint;
					jsonPoint.insert(constMeasureX, calInfo.vctPoint[i].x());
					jsonPoint.insert(constMeasureY, calInfo.vctPoint[i].y());
					jsonPointVct.append(jsonPoint);
				}
				jsonObject.insert(constMeasurecalibrationpoints, jsonPointVct);
				jsonObject.insert(constMeasurecalibrationvalue, calInfo.dbPoint2Distance);
			}
		}
		jsonVct.append(jsonObject);
	}
	if (jsonVct.size() > 0)
	{
		QJsonObject jsonRoot;
		jsonRoot.insert(constMeasureRoot, jsonVct);
		QJsonDocument jsonDoc;
		jsonDoc.setObject(jsonRoot);
		file.write(jsonDoc.toJson());
	}
}
