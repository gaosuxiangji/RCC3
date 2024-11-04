#pragma once

#include "../Video/checkboxdelegate.h"
#include "../Device/device.h"
#include "../Main/rccapp/render/PlayerViewBase.h"
#include "CMeasureLineManage.h"

#include <QWidget>
#include "ui_CSMeasureCalibrationDlg.h"

class CSMeasureCalibrationDlg : public QWidget
{
	Q_OBJECT

public:
	CSMeasureCalibrationDlg(QWidget *parent = Q_NULLPTR);
	~CSMeasureCalibrationDlg();

	void InitCalibrationInfo();
	void EscKeyPress();
private:
	void initUI();

signals:
	void SignalStateChanged(int state);
	void signalHideSelf();

public slots:
	void on_pushButton_apply_clicked();				// 应用
	void on_pushButton_cancel_clicked();			// 取消
	void on_radio_single_pixel_clicked();			// 单像素标定
	void on_radio_two_point_clicked();				// 两点距离;
	void slotRadioChanged();						// 标定类型切换
	void slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info);
	void on_toolButton_calibration_scale_clicked();	// 标定尺隐藏与显示;
	void on_toolButton_calibration_clear_clicked();	// 清除标定尺;


	void slotCurrentDeviceChanged(const QString current_ip);

	void slotDeviceStateChanged(const QString & ip, DeviceState state);

	void slotMeasureCal(const QString strIP, const CMeasureLineManage::TMesaureLineCal info);

	void slotUpdateMeasureCal(const QString strIP, const double dbValue);
	void slotMeasureCalMode(const QString strIP, const CMeasureLineManage::TMeasureModeType info);

	void slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info);

	void setCalibrateInfo(const CMeasureLineManage::TMesaureLineCal info);

	void updateUIValue(DeviceState state);
	void clearLabelInfo();
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;

private:
	//当前的设备
	QSharedPointer<Device> m_current_device_ptr{nullptr};

	const double constCalMaxValue{ 99999999.999f };							 // 标定值最大
private:
	Ui::CSMeasureCalibrationDlg *ui;
};
