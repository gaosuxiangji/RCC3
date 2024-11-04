#pragma once

#include "measureresulttabelmodel.h"
#include "../Video/tabelheaderview.h"
#include "../Video/checkboxdelegate.h"
#include "../Device/device.h"
#include "../Main/rccapp/render/PlayerViewBase.h"
#include "CMeasureLineManage.h"

#include <QWidget>
#include "ui_CSPointMeasureDlg.h"

class CSPointMeasureDlg : public QWidget
{
	Q_OBJECT

public:
	CSPointMeasureDlg(QWidget *parent = Q_NULLPTR);
	~CSPointMeasureDlg();

	void EscKeyPress();
private:
	void initUI();

	void initTableView();


signals:
	void SignalStateChanged(int state);

	void signalHideSelf();
public slots:
	void on_pushButton_clear_calibration_clicked();	// 清除标定
	void on_toolButtont_twoPoint_clicked();			// 两点间距
	void on_toolButtont_morePoint_clicked();		// 多点间距
	void on_toolButtont_angle_3_clicked();			// 角度1
	void on_toolButtont_angle_4_clicked();			// 角度2
	void on_toolButtont_radius_clicked();			// 半径
	void on_toolButtont_diameter_clicked();			// 直径
	void on_toolButtont_center_distance_clicked();	// 圆心距
	void on_toolButtont_dimension_clicked();		// 标注
	void on_toolButtont_area_radius_clicked();		// 圆面积
	void on_toolButtont_area_polygon_clicked();		// 多边形面积
	void on_pushButton_edit_clicked();				// 编辑

	void on_pushButton_clear_select_clicked();

	void slotTabViewCurrentSelectChanged(const QModelIndex & current, const QModelIndex & previous);

	void slotHeaderClick(Qt::CheckState state);

	void slotTableItemCheck(const QModelIndex &index, bool bCheck);

	void slotCurrentDeviceChanged(const QString current_ip);

	void slotDeviceStateChanged(const QString & ip, DeviceState state);

	void slotMeasureCal(const QString strIP, const CMeasureLineManage::TMesaureLineCal info);

	void slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info);

	void slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info);

	void slotMeasureLineCountChange(const QString strIP, const int nCount);

	void slotGetCheckedStatus();

	void slotAddMeasureItem(int nIndex);

	void setCalibrateInfo(const CMeasureLineManage::TMesaureLineCal info);

	void updateUIValue(DeviceState state);

	void clearLabelInfo();

	void clearTabelView();

	void on_pushButton_go_calibration_clicked();	// 清除标定

	void on_comboBox_showUnit_currentIndexChanged(int index);

	void slotUpdateDrawStatusInfo(Device::DrawTypeStatusInfo info);

	void slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info);
protected:
	/**
	*@brief 变化事件
	*@param [in] : * event : QEvent，事件指针
	**/
	void changeEvent(QEvent *event) override;

private:
	//当前的设备
	QSharedPointer<Device> m_current_device_ptr;

	MeasureResultTableModel *m_pModelMeasureTable{ nullptr };
	TableHeaderView* m_pHeader{ nullptr };    //TableHeaderView对象指针
	QList<CMeasureLineManage::TMeasureLineInfo> m_vctMeasureLine;			 // 测量信息
	const double constCalMaxValue{ 99999999.999f };							 // 标定值最大
private:
	Ui::CSPointMeasureDlg *ui;
};
