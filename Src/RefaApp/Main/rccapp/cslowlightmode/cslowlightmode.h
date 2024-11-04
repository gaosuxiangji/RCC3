#pragma once

#include <QtWidgets/QDialog>
#include "ui_cslowlightmode.h"

class CSLowLightMode : public QDialog
{
	Q_OBJECT

public:
	CSLowLightMode(QWidget *parent = Q_NULLPTR);
public:
	void setlightValue(int value);
signals:
	void signalLowLightValue(int value);
public slots:
	void slotCameraDisconnected();
private:
	void initUI();
	void bind();
	void setTitleName(const QString& title_name);
private slots:
	void slotLightLineEditFinished();
	void slotLightSliderMoved(int value);
private:
	const int m_const_value_max{ 5000 };
	const int m_const_value_min{ 1 };
private:
	Ui::CSLowLightModeClass ui;
};
