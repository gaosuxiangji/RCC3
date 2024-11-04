#pragma once

#include "measureresulttabelmodel.h"
#include "../Video/tabelheaderview.h"
#include "../Video/checkboxdelegate.h"
#include "../Device/device.h"
#include "../Main/rccapp/render/PlayerViewBase.h"
#include "CMeasureLineManage.h"
#include "CSPointMeasureDlg.h"
#include "CSMeasureCalibrationDlg.h"

#include <QWidget>
#include "ui_CSMeasurePageDlg.h"

class CSMeasurePageDlg : public QWidget
{
	Q_OBJECT

public:
	CSMeasurePageDlg(QWidget *parent = Q_NULLPTR);
	~CSMeasurePageDlg();

	void EscKeyPress();
private:
	void initUI();
	void bindSignalSlot();

signals:
	void SignalStateChanged(int state);
public slots:
	void slotShowMeasureDlg();
	void slotShowCalirationDlg();

protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;

private:
	CSMeasureCalibrationDlg *m_pMeasureCalirationDlg;
	CSPointMeasureDlg *m_pPointMeasureDlg;

private:
	Ui::CSMeasurePageDlg *ui;
};
