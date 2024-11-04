#include "CSMeasureCalibrationDlg.h"
#include "../Device/devicemanager.h"

CSMeasureCalibrationDlg::CSMeasureCalibrationDlg(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::CSMeasureCalibrationDlg)
{
	ui->setupUi(this);
	initUI();

}

CSMeasureCalibrationDlg::~CSMeasureCalibrationDlg()
{
	delete ui;
}

void CSMeasureCalibrationDlg::InitCalibrationInfo()
{
	m_current_device_ptr = DeviceManager::instance().getCurrentDevice();
	if (m_current_device_ptr == nullptr)
	{
		return;
	}
	connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSMeasureCalibrationDlg::slotCurrentDeviceChanged);
	connect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSMeasureCalibrationDlg::slotDeviceStateChanged);
	CMeasureLineManage::instance().setMeasureLineType(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MLT_TWO_CALIBRATION);
	CMeasureLineManage::instance().setMeasureModeType(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
	CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
	CMeasureLineManage::TMesaureLineCal info;
	CMeasureLineManage::instance().getMeasureCal(m_current_device_ptr->getIpOrSn(), info);
	setCalibrateInfo(info);
}

void CSMeasureCalibrationDlg::EscKeyPress()
{
	if (m_current_device_ptr)
	{
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
		m_current_device_ptr->setDrawTypeStatusInfo(Device::DTSI_Noraml);
		emit signalHideSelf();
	}
}

void CSMeasureCalibrationDlg::initUI()
{
	for (int i = int(CMeasureLineManage::MLU_MM); i < CMeasureLineManage::MLU_Count; i++)
	{
		ui->comboBox_Pixel_Unit->addItem(CMeasureLineManage::GetMeasureCalTypeStr((CMeasureLineManage::TMeasureLineUnit)i), i);
	}
	for (int i = int(CMeasureLineManage::MLU_MM); i < CMeasureLineManage::MLU_Count; i++)
	{
		ui->comboBox_distance_Unit->addItem(CMeasureLineManage::GetMeasureCalTypeStr((CMeasureLineManage::TMeasureLineUnit)i), i);
	}
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureCal, this, &CSMeasureCalibrationDlg::slotMeasureCal);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalUpdateMeasureCal, this, &CSMeasureCalibrationDlg::slotUpdateMeasureCal);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalCommonSignal, this, &CSMeasureCalibrationDlg::slotCommonSignal);

	connect(ui->radio_single_pixel, &QRadioButton::clicked, this, &CSMeasureCalibrationDlg::slotRadioChanged);
	connect(ui->radio_two_point, &QRadioButton::clicked, this, &CSMeasureCalibrationDlg::slotRadioChanged);
	setEnabled(true);

	ui->doubleSpinBox_pixel_value->setRange(0.001, constCalMaxValue);
	ui->doubleSpinBox_pixel_value->setValue(0.001);
	ui->doubleSpinBox_distance_value->setRange(0.001, constCalMaxValue);
	ui->doubleSpinBox_distance_value->setValue(0.001);
	ui->toolButton_calibration_scale->setToolTip(tr("Calibration scale"));
	ui->toolButton_calibration_clear->setToolTip(tr("Delete"));
}

void CSMeasureCalibrationDlg::on_pushButton_apply_clicked()
{
	if (!m_current_device_ptr){
	}
	else{
		if (ui->radio_single_pixel->isChecked())
		{
			CMeasureLineManage::TMesaureLineCal info;
			info.type = CMeasureLineManage::TMeasureLineUnit(ui->comboBox_Pixel_Unit->currentData().toInt());
			info.dbUnit = ui->doubleSpinBox_pixel_value->value();
			info.bCalibration = true;
			info.tCalibrationType = CMeasureLineManage::MCT_SINGLE_PIXEL;
			CMeasureLineManage::instance().setMeasureCal(m_current_device_ptr->getIpOrSn(), info);
		}
		if (ui->radio_two_point->isChecked())
		{
			double dbValue = ui->doubleSpinBox_distance_value->value();
			int type = CMeasureLineManage::TMeasureLineUnit(ui->comboBox_distance_Unit->currentData().toInt());
			CMeasureLineManage::instance().gotoMeasureCal(m_current_device_ptr->getIpOrSn(), dbValue, type);
		}
		m_current_device_ptr->setDrawTypeStatusInfo(Device::DTSI_Noraml);
	}
	CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Normal);
	emit signalHideSelf();
}

void CSMeasureCalibrationDlg::on_pushButton_cancel_clicked()
{
	if (m_current_device_ptr)
	{
		QList<QPoint> vctPoints;
		CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(vctPoints));
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Normal);
		disconnect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSMeasureCalibrationDlg::slotDeviceStateChanged);
		disconnect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSMeasureCalibrationDlg::slotCurrentDeviceChanged);
		m_current_device_ptr->setDrawTypeStatusInfo(Device::DTSI_Noraml);
	}
	emit signalHideSelf();
}

void CSMeasureCalibrationDlg::on_radio_single_pixel_clicked()
{
	if (m_current_device_ptr)
	{
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Normal);
	}
	ui->pushButton_apply->setEnabled(true);
}

void CSMeasureCalibrationDlg::on_radio_two_point_clicked()
{
	if (m_current_device_ptr)
	{
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Add);
		QList<QPoint> vctPoints;
		CMeasureLineManage::TMesaureLineCal info;
		CMeasureLineManage::instance().getMeasureCal(m_current_device_ptr->getIpOrSn(), info);
		if (info.bCalibration && info.tCalibrationType == CMeasureLineManage::MCT_TWO_POINT)
		{
			vctPoints.append(info.vctPoint);
		}
		CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(vctPoints));

		double dbValue = 0.0;
		if (info.vctPoint.size() > 1)
		{
			dbValue = CMeasureLineManage::PointsDistance(info.vctPoint);
		}
		slotCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE, QVariant::fromValue(dbValue));
		slotCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS_SIZE, info.vctPoint.size());
	}
}

void CSMeasureCalibrationDlg::slotRadioChanged()
{
	bool bPixelEnable = ui->radio_single_pixel->isChecked();
	ui->doubleSpinBox_pixel_value->setEnabled(bPixelEnable);
	ui->comboBox_Pixel_Unit->setEnabled(bPixelEnable);
	ui->label_one_pixel->setEnabled(bPixelEnable);
	if (bPixelEnable)
	{
		ui->pushButton_apply->setEnabled(true);
	}

	bool bTwoEnable = ui->radio_two_point->isChecked();
	ui->doubleSpinBox_distance_value->setEnabled(bTwoEnable);
	ui->comboBox_distance_Unit->setEnabled(bTwoEnable);
	ui->label_two_distance_px->setEnabled(bTwoEnable);
	ui->label_two_distance->setEnabled(bTwoEnable);
	ui->label_two_point->setEnabled(bTwoEnable);
	ui->toolButton_calibration_scale->setEnabled(bTwoEnable);
	ui->toolButton_calibration_clear->setEnabled(bTwoEnable);
}

void CSMeasureCalibrationDlg::slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info)
{
	switch (commandType)
	{
	case CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE:
	{
		double dbValue = info.toDouble();
		QString strText;
		strText = tr("%1 px").arg(QString::number(dbValue, 'f', 3));
		ui->label_two_distance_px->setText(strText);
		break;
	}
	case CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS_SIZE:
	{
		int nPointSize = info.toInt();
		if (nPointSize > 1)
		{
			ui->pushButton_apply->setEnabled(true);
		}
		else
		{
			if (nPointSize == 0)
			{
				ui->label_two_distance_px->setText(tr("--px"));
			}
			ui->pushButton_apply->setEnabled(false);
		}
		break;
	}
	default:
		break;
	}
}

void CSMeasureCalibrationDlg::on_toolButton_calibration_scale_clicked()
{
	if (m_current_device_ptr)
	{
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Modify);
	}
}

void CSMeasureCalibrationDlg::on_toolButton_calibration_clear_clicked()
{
	if (m_current_device_ptr)
	{
		QList<QPoint> vctPoints;
		CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(vctPoints));
		CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Normal);
	}
}

void CSMeasureCalibrationDlg::slotCurrentDeviceChanged(const QString current_ip)
{
	on_pushButton_cancel_clicked();
}

void CSMeasureCalibrationDlg::slotDeviceStateChanged(const QString &ip, DeviceState state)
{
	on_pushButton_cancel_clicked();
}

void CSMeasureCalibrationDlg::slotMeasureCal(const QString strIP, const CMeasureLineManage::TMesaureLineCal info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	setCalibrateInfo(info);
}

void CSMeasureCalibrationDlg::slotUpdateMeasureCal(const QString strIP, const double dbValue)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	QString strText;
	strText = tr("%1 px").arg(QString::number(dbValue, 'f', 3));
	ui->label_two_distance_px->setText(strText);
}

void CSMeasureCalibrationDlg::slotMeasureCalMode(const QString strIP, const CMeasureLineManage::TMeasureModeType mode)
{
// 	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
// 	switch (mode)
// 	{
// 	case CMeasureLineManage::MMT_Normal:
// 	case CMeasureLineManage::MMT_Show:
// 	case CMeasureLineManage::MMT_Modify:
// 		ui->pushButton_apply->setEnabled(true);
// 		break;
// 	case CMeasureLineManage::MMT_Add:
// 		ui->pushButton_apply->setEnabled(false);
// 		break;
// 	default:
// 		break;
// 	}
}

void CSMeasureCalibrationDlg::slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	if (CMeasureLineManage::MMT_Modify == info)
	{
	}
	else
	{
	}
}

void CSMeasureCalibrationDlg::setCalibrateInfo(const CMeasureLineManage::TMesaureLineCal info)
{
	int nIndex = 0, nDistanceIndex = 0;
	ui->radio_single_pixel->setChecked(false);
	ui->doubleSpinBox_pixel_value->setValue(0.001);
	ui->radio_two_point->setChecked(false);
	ui->doubleSpinBox_distance_value->setValue(0.001);
	ui->label_two_distance_px->setText(tr("--px"));
	if (!info.bCalibration)
	{
		ui->radio_single_pixel->setChecked(true);
	}
	else
	{
		switch (info.tCalibrationType)
		{
		case CMeasureLineManage::MCT_SINGLE_PIXEL:
		{
			ui->radio_single_pixel->setChecked(true);
			QString strText;
			QString strUnit = CMeasureLineManage::GetMeasureCalTypeStr(info.type);
			if (info.type == CMeasureLineManage::MLU_Pixel)
			{
				strUnit.clear();
			}
			int nCount = ui->comboBox_Pixel_Unit->count();
			for (int i = 0; i < nCount; i++)
			{
				if (ui->comboBox_Pixel_Unit->itemData(i).toInt() == (int)info.type)
				{
					nIndex = i;
					break;
				}
			}
			strText = tr("1px = %1%2!").arg(QString::number(info.dbUnit, 'f', 3)).arg(strUnit);
			ui->doubleSpinBox_pixel_value->setValue(info.dbUnit);
			CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Normal);
			break;
		}
		case CMeasureLineManage::MCT_TWO_POINT:
		{
			ui->radio_two_point->setChecked(true);
			QString strText;
			QString strUnit = CMeasureLineManage::GetMeasureCalTypeStr(info.type);
			if (info.type == CMeasureLineManage::MLU_Pixel)
			{
				strUnit.clear();
			}
			int nCount = ui->comboBox_distance_Unit->count();
			for (int i = 0; i < nCount; i++)
			{
				if (ui->comboBox_distance_Unit->itemData(i).toInt() == (int)info.type)
				{
					nDistanceIndex = i;
					break;
				}
			}
			double dbPoint = CMeasureLineManage::PointsDistance(info.vctPoint);
			strText = tr("%1 px").arg(QString::number(dbPoint, 'f', 3));
			ui->label_two_distance_px->setText(strText);
			ui->doubleSpinBox_distance_value->setValue(info.dbPoint2Distance);
			if (m_current_device_ptr)
			{
				CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(info.vctPoint));
				CMeasureLineManage::instance().setMeasureCalMode(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
			}
			break;
		}
		default:
			break;
		}
	}
	ui->comboBox_Pixel_Unit->setCurrentIndex(nIndex);
	ui->comboBox_distance_Unit->setCurrentIndex(nDistanceIndex);
	slotRadioChanged();
}

void CSMeasureCalibrationDlg::updateUIValue(DeviceState state)
{
	if (!m_current_device_ptr)
	{
		setEnabled(false);
	}
// 	else
// 	{
// 		QString strIP = m_current_device_ptr->getIpOrSn();
// 		if (state == Previewing || state == Acquiring || state == Recording)
// 		{
// 			setEnabled(true);
// 			QPoint ptOffset(0, 0);
// 			if (state == Acquiring || state == Recording)
// 			{
// 				QRect rc = m_current_device_ptr->GetRoi(Device::kDeviceRoi);
// 				ptOffset = rc.topLeft();
// 			}
// 			CMeasureLineManage::instance().setMeasurePointOffset(strIP, ptOffset);
// 
// 			CMeasureLineManage::TMesaureLineCal calInfo;
// 			CMeasureLineManage::instance().getMeasureCal(strIP, calInfo);
// 			setCalibrateInfo(calInfo);
// 			
// 			QList<CMeasureLineManage::TMeasureLineInfo> vctLine;
// 			CMeasureLineManage::instance().getMeasureLine(strIP, vctLine);
// 			if (vctLine.size() == 0)
// 			{
// 			}
// 			else
// 			{
// 				CMeasureLineManage::TMeasureModeType type;
// 				CMeasureLineManage::instance().getMeasureModeType(strIP, type);
// 				if (CMeasureLineManage::MMT_Modify == type)
// 				{
// 				}
// 				else
// 				{
// 				}
// 			}
// 		}
// 		else
// 		{
// 			setEnabled(false);
// 		}
// 	}
}

void CSMeasureCalibrationDlg::clearLabelInfo()
{
	ui->doubleSpinBox_pixel_value->setValue(0.001);
	ui->comboBox_Pixel_Unit->setCurrentIndex(0);
}

void CSMeasureCalibrationDlg::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QWidget::changeEvent(event);
}
