#ifndef BASICMEASURERWIDGET_H
#define BASICMEASURERWIDGET_H

#include <QWidget>

/*
�궨���������ࡣʹ��ʾ�����£�
BasicMeasurerWidget basicMeasurerWidget;
......
*/

namespace Ui {
class BasicMeasurerWidget;
}

class BasicMeasurerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BasicMeasurerWidget(QWidget *parent = 0);
    ~BasicMeasurerWidget();

	/** @brief ���±궨����·��
	@param [in] : const QString& filePath : �궨����·��
	@return
	@note
	*/
	void UpdateCalibrationFilePath(const QString& filePath);

	/** @brief ������Ƶ�Ƿ���ڵı�־
	@param [in] : bool exists : �Ƿ����
	@return
	@note : ������Ƶ�Ƿ���ڣ����°�ť״̬
	*/
	void SetVideoExistsFlag(bool exists);

protected:
	/** @brief �ߴ�仯�¼�
	@param [in] : QResizeEvent *event : �¼�����
	@return
	@note
	*/
	virtual void resizeEvent(QResizeEvent *event) override;

public Q_SLOTS:
	/** @brief �����ڵ�TabWidget�л�ʱ�������ź�
	@param [in] : int index : TabWidget�ĵ�ǰ����
	@return
	@note
	*/
	void slotTabWidgetCurrentIndexChanged(int index);

private Q_SLOTS:
	/** @brief ���ɾ���궨�ļ���ť
	@param [in] : bool checked : ��ť�Ƿ�check
	@return
	@note
	*/
	void on_toolButtonDelete_clicked(bool checked = false);

	/** @brief �������궨�ļ���ť
	@param [in] : bool checked : ��ť�Ƿ�check
	@return
	@note
	*/
	void on_toolButtonImport_clicked(bool checked = false);

	/** @brief ��������궨�ļ���ť
	@param [in] : bool checked : ��ť�Ƿ�check
	@return
	@note
	*/
	void on_toolButtonExport_clicked(bool checked = false);

	/**
	*@brief ������������ť
	*@param [in] : bool checked : ��ť�Ƿ�check
	*@return
	**/
	void on_toolButtonLineMeasure_clicked(bool checked);

	/**
	*@brief ������������ť
	*@param [in] : bool checked : ��ť�Ƿ�check
	*@return
	**/
	void on_toolButtonPointMeasure_clicked(bool checked);

private:
	/** @brief �����ʼ��
	@param
	@return
	@note
	*/
	void InitData();

	/** @brief �����ʼ��
	@param
	@return
	@note
	*/
	void InitUI();

	/** @brief ���±궨�����ļ���ʾ����
	@param
	@return
	@note
	*/
	void UpdateCalibrationFilePathLineEdit();

	/** @brief ������궨�����ļ��йصİ�ť��ʹ��״̬
	@param
	@return
	@note
	*/
	void UpdateCalibrationFileButton();

signals:
	/**
	*@brief �������
	*@param [in] : benabled : bool���Ƿ�ʹ��
	*@return
	**/
	void sigMeasurePoint(bool benabled);

	/**
	*@brief �������
	*@param [in] : benabled : bool���Ƿ�ʹ��
	*@return
	**/
	void sigMeausreLine(bool benabled);

	/**
	*@brief ��յ�ǰ����ģʽ�µ�����
	*@param 
	*@return
	**/
	void sigClearCurrentMeasureModeFeatures();

private:
    Ui::BasicMeasurerWidget *ui;
	QString defaultCalibrationFilePathLineEditText;//Ĭ�ϱ궨�����ļ��༭����ʾ����
	QString calibrationFilePath;//�궨�ļ�·��
};

#endif // BASICMEASURERWIDGET_H
