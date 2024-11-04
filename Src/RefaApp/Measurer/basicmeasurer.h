#ifndef BASICMEASURER_H
#define BASICMEASURER_H

#include <QString>
#include <QPoint>

/*
基础测量模块类。使用示例如下：
bool exists=BasicMeasurer::CalibrationParamExists();
......
*/

struct BaseCalibrationParam;

class BasicMeasurer
{
public:
    BasicMeasurer();

	/** @brief 判断标定参数是否存在
	@param 
	@return : bool : 标定参数是否存在
	@note
	*/
	static bool CalibrationParamExists();

	/** @brief 移除标定参数
	@param
	@return
	@note
	*/
	static void RemoveParam();

	/** @brief 获取标定参数
	@param
	@return : const BaseCalibrationParam* : 标定参数指针
	@note
	*/
	static const BaseCalibrationParam* GetCalibrationParams();

	/** @brief 插入标定参数
	@param [in] : const BaseCalibrationParam* const param : 标定参数指针
	@return : bool : 插入标定参数是否成功
	@note
	*/
	static bool InsertCalibrationParams(const BaseCalibrationParam* const param);

	/** @brief 加载标定参数文件
	@param [in] : const QString& filePath : 标定参数文件路径
	@return : quint64 : 返回值（错误码）
	@note
	*/
	static quint64 load(const QString& filePath);

	/** @brief 保存标定参数至文件
	@param [in] : const QString& filePath : 标定参数文件路径
	@return : quint64 : 返回值（错误码）
	@note
	*/
	static quint64 save(const QString& filePath);

	/** @brief 获取当前标定参数对应的文件路径
	@param 
	@return : QString : 文件路径
	@note : 只有加载标定参数或保存标定参数至文件时才会影响该值
	*/
	static QString GetCalibrationFilePath();

	/** @brief 像素点转换为世界坐标系空间点
	@param [in] : const QPoint &pos : 像素点
	@return : QString : 世界坐标系点对应的字符串
	@note : 字符串的格式由TansformeMeasureToQString函数决定
	*/
	static QString BasicMeasurer::mapToActual(const QPoint &pos);

	/** @brief 计算两个像素点对应的世界坐标系空间距离
	@param [in] : const QPoint &p1 : 像素点1
	       [in] : const QPoint &p2 : 像素点2
	@return : QString : 距离对应的字符串
	@note : 字符串的格式由TansformeMeasureToQString函数决定
	*/
	static QString BasicMeasurer::actualDistance(const QPoint &p1, const QPoint &p2);

private:
	/** @brief 像素点转换为世界坐标系空间点
	@param [in] : const QPoint &pos : 像素点
	@return : QPoint : 世界坐标系空间点
	@note
	*/
	static QPoint TansformPixelToActual(const QPoint& pos);

	/** @brief 将点序列按照一定的规则转换为字符串
	@param [in] : const QList<QPoint>& measure : 点序列
	@return : QString : 字符串
	@note
	*/
	static QString TansformeMeasureToQString(const QList<QPoint>& measure);

	/** @brief 将距离序列按照一定的规则转换为字符串
	@param [in] : const QList<float>& measure : 点序列
	@return : QString : 字符串
	@note
	*/
	static QString TansformeMeasureToQString(const QList<float>& measure);

private:
	static QString calibrationFilePath;//标定参数对应的文件路径（只有加载标定参数或保存标定参数至文件时才会影响该值）
};

#endif // BASICMEASURER_H