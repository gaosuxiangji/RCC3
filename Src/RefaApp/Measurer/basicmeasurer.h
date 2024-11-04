#ifndef BASICMEASURER_H
#define BASICMEASURER_H

#include <QString>
#include <QPoint>

/*
��������ģ���ࡣʹ��ʾ�����£�
bool exists=BasicMeasurer::CalibrationParamExists();
......
*/

struct BaseCalibrationParam;

class BasicMeasurer
{
public:
    BasicMeasurer();

	/** @brief �жϱ궨�����Ƿ����
	@param 
	@return : bool : �궨�����Ƿ����
	@note
	*/
	static bool CalibrationParamExists();

	/** @brief �Ƴ��궨����
	@param
	@return
	@note
	*/
	static void RemoveParam();

	/** @brief ��ȡ�궨����
	@param
	@return : const BaseCalibrationParam* : �궨����ָ��
	@note
	*/
	static const BaseCalibrationParam* GetCalibrationParams();

	/** @brief ����궨����
	@param [in] : const BaseCalibrationParam* const param : �궨����ָ��
	@return : bool : ����궨�����Ƿ�ɹ�
	@note
	*/
	static bool InsertCalibrationParams(const BaseCalibrationParam* const param);

	/** @brief ���ر궨�����ļ�
	@param [in] : const QString& filePath : �궨�����ļ�·��
	@return : quint64 : ����ֵ�������룩
	@note
	*/
	static quint64 load(const QString& filePath);

	/** @brief ����궨�������ļ�
	@param [in] : const QString& filePath : �궨�����ļ�·��
	@return : quint64 : ����ֵ�������룩
	@note
	*/
	static quint64 save(const QString& filePath);

	/** @brief ��ȡ��ǰ�궨������Ӧ���ļ�·��
	@param 
	@return : QString : �ļ�·��
	@note : ֻ�м��ر궨�����򱣴�궨�������ļ�ʱ�Ż�Ӱ���ֵ
	*/
	static QString GetCalibrationFilePath();

	/** @brief ���ص�ת��Ϊ��������ϵ�ռ��
	@param [in] : const QPoint &pos : ���ص�
	@return : QString : ��������ϵ���Ӧ���ַ���
	@note : �ַ����ĸ�ʽ��TansformeMeasureToQString��������
	*/
	static QString BasicMeasurer::mapToActual(const QPoint &pos);

	/** @brief �����������ص��Ӧ����������ϵ�ռ����
	@param [in] : const QPoint &p1 : ���ص�1
	       [in] : const QPoint &p2 : ���ص�2
	@return : QString : �����Ӧ���ַ���
	@note : �ַ����ĸ�ʽ��TansformeMeasureToQString��������
	*/
	static QString BasicMeasurer::actualDistance(const QPoint &p1, const QPoint &p2);

private:
	/** @brief ���ص�ת��Ϊ��������ϵ�ռ��
	@param [in] : const QPoint &pos : ���ص�
	@return : QPoint : ��������ϵ�ռ��
	@note
	*/
	static QPoint TansformPixelToActual(const QPoint& pos);

	/** @brief �������а���һ���Ĺ���ת��Ϊ�ַ���
	@param [in] : const QList<QPoint>& measure : ������
	@return : QString : �ַ���
	@note
	*/
	static QString TansformeMeasureToQString(const QList<QPoint>& measure);

	/** @brief ���������а���һ���Ĺ���ת��Ϊ�ַ���
	@param [in] : const QList<float>& measure : ������
	@return : QString : �ַ���
	@note
	*/
	static QString TansformeMeasureToQString(const QList<float>& measure);

private:
	static QString calibrationFilePath;//�궨������Ӧ���ļ�·����ֻ�м��ر궨�����򱣴�궨�������ļ�ʱ�Ż�Ӱ���ֵ��
};

#endif // BASICMEASURER_H