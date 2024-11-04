/***************************************************************************************************
** @file: "窗口中心线设置"弹框
** @author: mpp
** @date: 2022-03-01
*
*****************************************************************************************************/

#ifndef CSWINDOWCROSSLINESETTING_H
#define CSWINDOWCROSSLINESETTING_H

#include <QDialog>
#include "Device/device.h"
#include "Device/devicemanager.h"
#include "Device/deviceutils.h"

namespace Ui {
class CSWindowCrossLineSetting;
}

class CSWindowCrossLineSetting : public QDialog
{
    Q_OBJECT

public:
    explicit CSWindowCrossLineSetting(/*QSharedPointer<Device> device, */QWidget *parent = 0);
    ~CSWindowCrossLineSetting();

	/************************
    * @brief: 设置窗窗口中心线交点坐标对应的编辑框文本
    * @param centerpoint: 交点坐标
	* @author: mpp
	*************************/
	void SetCustomCrossLineCenterPointLineEditText(const QPointF& centerpoint);

	void UpdateTranslate();
signals:
	/************************
	* @brief: 窗口中心线设置弹框打开信号
	* @author:mpp
	*************************/
	void SignalWindowCrossLineSettingOpened();

	/************************
	* @brief: 获取中心点坐标信号
	* @author:mpp
	*************************/
	void SignalGetPictureCenterPoint();
private slots:
	void SlotWindowCrossLineSettingOpened();
    void on_CrossLineCenterPoint_X_LineEdit_editingFinished();

    void on_CrossLineCenterPoint_Y_LineEdit_editingFinished();

    void on_CenterLocationBtn_clicked();

	void on_CrossLineCenterPoint_X_LineEdit_textChanged(const QString &arg1);

	void on_CrossLineCenterPoint_Y_LineEdit_textChanged(const QString &arg1);

private:
    /************************
    * @brief: 初始化UI
    * @author: mpp
    *************************/
    void InitUI();

	/**************************
    * @brief: 连接信号槽
	* @author: mpp
	***************************/
	void ConnectSignalSlot();

	/**************************
	* @brief: 获取中心点坐标
	* @return: 中心点坐标
	* @author: mpp
	***************************/
	QPointF GetCrossLineCenterPointF() const;
private:
    Ui::CSWindowCrossLineSetting *ui;
private:
	QPoint _offset{ 0,0 };
};

#endif // CSWINDOWCROSSLINESETTING_H
