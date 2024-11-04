/***************************************************************************************************
** @file: 菜单栏-帮助-软件版本
** @author: mpp
** @date: 2022/05/10
*
*****************************************************************************************************/
#ifndef CSSOFTWAREVERTION_H_

#include <QtWidgets/QDialog>
#include "ui_cssoftwareversion.h"

class CSSoftwareVersion : public QDialog
{
	Q_OBJECT

public:
	CSSoftwareVersion(QWidget *parent = Q_NULLPTR);
private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/10
	***************************/
	void InitUI();

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/10
	***************************/
	void ConnectSignalAndSlot();
public slots:
	void slotLicense();
private:
	Ui::CSSoftwareVersionClass ui;
};


#define CSSOFTWAREVERTION_H_
#endif
