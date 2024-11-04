#ifndef BASICMEASURERWIDGET_H
#define BASICMEASURERWIDGET_H

#include <QWidget>

/*
标定参数管理类。使用示例如下：
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

	/** @brief 更新标定参数路径
	@param [in] : const QString& filePath : 标定参数路径
	@return
	@note
	*/
	void UpdateCalibrationFilePath(const QString& filePath);

	/** @brief 设置视频是否存在的标志
	@param [in] : bool exists : 是否存在
	@return
	@note : 根据视频是否存在，更新按钮状态
	*/
	void SetVideoExistsFlag(bool exists);

protected:
	/** @brief 尺寸变化事件
	@param [in] : QResizeEvent *event : 事件对象
	@return
	@note
	*/
	virtual void resizeEvent(QResizeEvent *event) override;

public Q_SLOTS:
	/** @brief 父窗口的TabWidget切换时触发该信号
	@param [in] : int index : TabWidget的当前索引
	@return
	@note
	*/
	void slotTabWidgetCurrentIndexChanged(int index);

private Q_SLOTS:
	/** @brief 点击删除标定文件按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonDelete_clicked(bool checked = false);

	/** @brief 点击导入标定文件按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonImport_clicked(bool checked = false);

	/** @brief 点击导出标定文件按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_toolButtonExport_clicked(bool checked = false);

	/**
	*@brief 点击两点测量按钮
	*@param [in] : bool checked : 按钮是否被check
	*@return
	**/
	void on_toolButtonLineMeasure_clicked(bool checked);

	/**
	*@brief 点击单点测量按钮
	*@param [in] : bool checked : 按钮是否被check
	*@return
	**/
	void on_toolButtonPointMeasure_clicked(bool checked);

private:
	/** @brief 输出初始化
	@param
	@return
	@note
	*/
	void InitData();

	/** @brief 界面初始化
	@param
	@return
	@note
	*/
	void InitUI();

	/** @brief 更新标定参数文件显示内容
	@param
	@return
	@note
	*/
	void UpdateCalibrationFilePathLineEdit();

	/** @brief 更新与标定参数文件有关的按钮的使能状态
	@param
	@return
	@note
	*/
	void UpdateCalibrationFileButton();

signals:
	/**
	*@brief 单点测量
	*@param [in] : benabled : bool，是否使能
	*@return
	**/
	void sigMeasurePoint(bool benabled);

	/**
	*@brief 两点测量
	*@param [in] : benabled : bool，是否使能
	*@return
	**/
	void sigMeausreLine(bool benabled);

	/**
	*@brief 清空当前测量模式下的特征
	*@param 
	*@return
	**/
	void sigClearCurrentMeasureModeFeatures();

private:
    Ui::BasicMeasurerWidget *ui;
	QString defaultCalibrationFilePathLineEditText;//默认标定参数文件编辑框显示内容
	QString calibrationFilePath;//标定文件路径
};

#endif // BASICMEASURERWIDGET_H
