#ifndef CMEASURELINEMANAGE_H
#define CMEASURELINEMANAGE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QPoint>
#include <qmath.h>
#include <QVariant>

class CMeasureLineManage : public QObject
{
	Q_OBJECT
public:

	enum TMeasureCommonSignalType									// 测量通用信号类型
	{
		MCST_UPDATE_CALIBRATION_VALUE,								// 更新鼠标位置变化导致的标定信息变化
		MCST_UPDATE_CALIBRATION_POINTS_SIZE,						// 更新鼠标位置标定信息点数变化
		MCST_UPDATE_ITEM_SELECTED,									// 更新测量项选择
		MCST_UPDATE_CALIBRATION_POINTS,								// 更新测量项点
	};

	enum TMeasureLineType											// 测量线类型
	{
		MLT_TWO_POINT,												// 两点间距
		MLT_MORE_POINT, 											// 多点间距
		MLT_TWO_CALIBRATION,										// 两点标定
		MLT_ANGLE_3,												// 角度测量 三点
		MLT_ANGLE_4,												// 角度测量 四点
		MLT_RADIUS,													// 半径
		MLT_DIAMETER,												// 直径
		MLT_CENTER_DISTANCE,										// 圆心间距
		MLT_DIMENSION,												// 标注
		MLT_AREA_CENTER,											// 圆面积
		MLT_AREA_POLYGON,											// 多边形面积
	};

	enum TMeasureCalibrationType									// 标定类型
	{
		MCT_SINGLE_PIXEL,											// 单像素标定
		MCT_TWO_POINT, 												// 两点标定
	};

	enum TMeasureModeType											// 测量模式类型
	{
		MMT_Normal,													// 正常类型，不在测量模式 不显示测量线
		MMT_Show,													// 显示测量线
		MMT_Add,													// 测量模式添加
		MMT_Modify,													// 测量模式修改
	};

	struct TMeasureLineInfo
	{
		qint64 nIndex;												// 索引
		QList<QPoint> vctPoint;										// 点集
		bool bChecked;												// 是否选中
		QString strName;											// 名称
		qreal qrLength;												// 距离
		TMeasureLineType nType;										// 类型
	};

	struct TMeasureAddLineInfo
	{
		TMeasureAddLineInfo()
		{
			nMaxPointCount = kMaxPointCount;
		}
		QList<QPoint> vctPoint;										// 点集
		QPoint curPoint;											// 当前点
// 		QString strName;											// 名称
// 		qreal qrLength;												// 距离
		int nMaxPointCount;											// 添加点最大数量
	};

	enum TMeasureLineUnit											// 测量线单位
	{
		MLU_Pixel,													// 像素
		MLU_MM, 													// 毫米
		MLU_CM, 													// 厘米
		MLU_M, 														// 米
		MLU_Count, 													// 总数
	};

	struct TMesaureLineCal											// 测量线计算单位
	{
		TMesaureLineCal()
		{
			bCalibration = false;
			type = MLU_Pixel;
			dbUnit = 1.0;
		}
		bool bCalibration;											// 是否标定
		TMeasureLineUnit type;										// 测量线计算单位
		qreal dbUnit;												// 测量线计算单位
		TMeasureCalibrationType tCalibrationType;					// 标定类型
		QList<QPoint> vctPoint;										// 点集
		qreal dbPoint2Distance;										// 两点对应的长度 dbPoint/PointsDistance(vctPoint )= dbUnit
	};

	static const int kMaxPointCount = 0x7FFFFFFF;					// 点最大数量

	static CMeasureLineManage & instance();
	CMeasureLineManage(QObject *parent = 0);
	~CMeasureLineManage();

    static double PointDistance(const QPointF & p1, const QPointF & p2);
    static double PointsDistance(const QList<QPoint> & pts);
    static QString GetMeasureCalTypeStr(TMeasureLineUnit info);
    static QString GetMeasureCalTypeOrAreaStr(TMeasureLineUnit info);
    static QString GetMeasureLineTypeStr(TMeasureLineType info);
    static double PointsAngle(const QList<QPoint> & pts);
    static double GetAngleOfThreePoint(const QPoint &ptStart, const QPoint &ptEnd, const QPoint &ptCenter);
    static double GetAngleOfFourPoint(const QPoint &pt1, const QPoint &pt2, const QPoint &pt3, const QPoint &pt4);
    static QPointF GetRadiusOfThreePoint(const QPoint &pt1, const QPoint &pt2, const QPoint &pt3, double &dbRadius);
    static double GetPolygonArea(const QList<QPoint> & pts);

	void addMeasureLine(const QString strIP, const TMeasureLineInfo info);					// 添加一个线段
	void updateOneMeasureLine(const QString strIP, const TMeasureLineInfo info);			// 更新一个线段
	void updateMeasureLines(const QString strIP, const QList<TMeasureLineInfo> info);		// 更新为所有线段
	void deleteMeasureLines(const QString strIP, const QList<TMeasureLineInfo> info);		// 删除线段
	void deleteMeasureLines(const QString strIP, const QList<int> info);					// 删除线段
	void clearMeasureLines(const QString strIP);											// 删除所有线段
	void getMeasureLine(const QString strIP, QList<TMeasureLineInfo>& info);				// 获取线段
	int getFreeMeasureLineIndex(const QString strIP);										// 获取新线段索引
	void setMeasureLineType(const QString &ip, const CMeasureLineManage::TMeasureLineType Type);// 设置绘制的点类型

	void setMeasureCal(const QString strIP, const TMesaureLineCal info);					// 设置标定信息
	void clearMeasureCal(const QString strIP);												// 删除标定信息
	void getMeasureCal(const QString strIP, TMesaureLineCal &info);							// 获取标定信息
	void updateMeasureCalInfo(const QString strIP, const TMeasureLineInfo info);			// 更新标定信息 一般情况是界面上添加点或移动标定点才去更新
	void gotoMeasureCal(const QString strIP, double dbValue, int nType);					// 将要去标定，管理界面没有标定信息，此函数发出信号去告诉绘制界面，获取绘制的标定信息以便去标定
	void setMeasureCalMode(const QString strIP, const TMeasureModeType info);				// 设置测量类型

	void emitCommonSignal(const QString strIP, const TMeasureCommonSignalType commandType, QVariant info);// 设置测量类型

	void calMeasureLine(const QString strIP, TMeasureLineInfo & info);						// 测量线计算
	void calMeasureLines(const QString strIP, QList<TMeasureLineInfo>& info);				// 多个测量线计算

	void setMeasureModeType(const QString strIP, const TMeasureModeType info);				// 设置测量类型
	void getMeasureModeType(const QString strIP, TMeasureModeType &info);					// 获取测量类型
	void readCalInfo();																		// 读取标定数据
	void writeCalInfo();																	// 写入标定数据
	void setMeasurePointOffset(const QString strIP, const QPoint info);						// 设置偏移量
signals:
	// 测量线添加信号
	void signalAddMeasureLine(const QString strIP, const TMeasureLineInfo info);
	// 测量线变化信号
	void signalUpdateMeasureLine(const QString strIP, const TMeasureLineInfo info);
	// 测量线更新信号
	void signalUpdateMeasureLines(const QString strIP, const QList<TMeasureLineInfo> info);
	// 测量线删除信号
	void signalDeleteMeasureLines(const QString strIP, const QList<int> info);
	// 测量线清除信号
	void signalClearMeasureLines(const QString strIP);
	void signalMeasureLineCountChange(const QString strIP, const int nCount);
	// 测量类型变化信号
	void signalMeasureModeType(const QString strIP, const TMeasureModeType modeType);
	// 绘制点偏移量
	void signalMeasurePointOffset(const QString strIP, const QPoint point);
	// 绘制测量线类型变化信号
	void signalMeasureLineType(const QString &ip, const CMeasureLineManage::TMeasureLineType Type);

	// 测量标定变化信号
	void signalMeasureCal(const QString strIP, const TMesaureLineCal info);
	// 测量标定点变化信号
	void signalUpdateMeasureCal(const QString strIP, const double dbValue);
	// 将要去标定的信号
	void signalGotoMeasureCal(const QString strIP, double dbValue, int nType);
	// 测量标定点模式变化信号
	void signalMeasureCalMode(const QString strIP, TMeasureModeType mode);

	// 测量通用信号变化
	void signalCommonSignal(const QString strIP, const TMeasureCommonSignalType commandType, QVariant info);

private:
	const QString constKeyName{ "MeasureCalibrateInfo" };							// 保存测量标定的Key 
	const QString constMeasureLineFile{ "/MeasureLine.json" };
	const QString constMeasureRoot{ "measureline" };
	const QString constMeasureip{ "ip" };
	const QString constMeasurecalibration{ "calibration" };
	const QString constMeasureunit{ "unit" };
	const QString constMeasurecalibrationtype{ "calibrationtype" };
	const QString constMeasurecalibrationpoints{ "calibrationpoints" };
	const QString constMeasureX{ "x" };
	const QString constMeasureY{ "y" };
	const QString constMeasurecalibrationvalue{ "calibrationvalue" };
	
	QString m_strMeasureFile;
	QMap<QString, QList<TMeasureLineInfo>> m_mapIP2MeasureLineInfo;
	QMap<QString, TMesaureLineCal> m_mapIP2MeasureCalInfo;
	QMap<QString, TMeasureModeType> m_mapIP2MeasureModeType;
};

#endif //CMEASURELINEMANAGE_H