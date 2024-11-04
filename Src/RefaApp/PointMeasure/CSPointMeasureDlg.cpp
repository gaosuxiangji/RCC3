#include "CSPointMeasureDlg.h"
#include "../Device/devicemanager.h"

CSPointMeasureDlg::CSPointMeasureDlg(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::CSPointMeasureDlg)
{
	ui->setupUi(this);
	initUI();	
}

CSPointMeasureDlg::~CSPointMeasureDlg()
{
	delete ui;
}

void CSPointMeasureDlg::EscKeyPress()
{
	if (m_current_device_ptr)
	{
		CMeasureLineManage::instance().setMeasureModeType(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
	}
}

void CSPointMeasureDlg::initUI()
{
// 	for (int i = int(CMeasureLineManage::MLU_MM); i < CMeasureLineManage::MLU_Count; i++)
// 	{
// 		ui->comboBox_Pixel_Unit->addItem(CMeasureLineManage::GetMeasureCalTypeOrAreaStr((CMeasureLineManage::TMeasureLineUnit)i), i);
// 
// 	}
	ui->toolButtont_twoPoint->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_TWO_POINT));
	ui->toolButtont_twoPoint->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_TWO_POINT));
	ui->toolButtont_twoPoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_morePoint->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_MORE_POINT));
	ui->toolButtont_morePoint->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_MORE_POINT));
	ui->toolButtont_morePoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_angle_3->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_3));
	ui->toolButtont_angle_3->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_3));
	ui->toolButtont_angle_3->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_angle_4->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_4));
	ui->toolButtont_angle_4->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_4));
	ui->toolButtont_angle_4->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_radius->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_RADIUS));
	ui->toolButtont_radius->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_RADIUS));
	ui->toolButtont_radius->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_diameter->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIAMETER));
	ui->toolButtont_diameter->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIAMETER));
	ui->toolButtont_diameter->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_center_distance->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_CENTER_DISTANCE));
	ui->toolButtont_center_distance->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_CENTER_DISTANCE));
	ui->toolButtont_center_distance->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_dimension->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIMENSION));
	ui->toolButtont_dimension->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIMENSION));
	ui->toolButtont_dimension->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_area_radius->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_CENTER));
	ui->toolButtont_area_radius->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_CENTER));
	ui->toolButtont_area_radius->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ui->toolButtont_area_polygon->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_POLYGON));
	ui->toolButtont_area_polygon->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_POLYGON));
	ui->toolButtont_area_polygon->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	initTableView();

	connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSPointMeasureDlg::slotCurrentDeviceChanged);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureCal, this, &CSPointMeasureDlg::slotMeasureCal);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalAddMeasureLine, this, &CSPointMeasureDlg::slotAddMeasureLine);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureModeType, this, &CSPointMeasureDlg::slotMeasureModeType);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureLineCountChange, this, &CSPointMeasureDlg::slotMeasureLineCountChange);
	connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalCommonSignal, this, &CSPointMeasureDlg::slotCommonSignal);
	setCalibrateInfo(CMeasureLineManage::TMesaureLineCal());

	setEnabled(false);
}

void CSPointMeasureDlg::initTableView()
{
// 	ui->doubleSpinBox_pixel_value->setRange(0.001, constCalMaxValue);
// 	ui->doubleSpinBox_pixel_value->setValue(0.001);

	m_pModelMeasureTable = new MeasureResultTableModel(this);
	QStringList labels;
	labels << tr("") << tr("Type") << tr("Name") << tr("Result");
	m_pModelMeasureTable->SetListColNum(labels.size());
	m_pModelMeasureTable->SetHeadText(labels);

	QStringList vctListUnit;
	for (int i = CMeasureLineManage::MLU_Pixel; i < CMeasureLineManage::MLU_Count; i++)
	{
		ui->comboBox_showUnit->addItem(CMeasureLineManage::GetMeasureCalTypeOrAreaStr((CMeasureLineManage::TMeasureLineUnit)i));
	}
	ui->comboBox_showUnit->setCurrentText(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(CMeasureLineManage::MLU_Pixel));
	m_pHeader = new TableHeaderView(Qt::Horizontal, this);
	ui->tableViewMeasure->setHorizontalHeader(m_pHeader);

	ui->tableViewMeasure->horizontalHeader()->setFrameShape(QFrame::StyledPanel);
	ui->tableViewMeasure->horizontalHeader()->setVisible(true);
	ui->tableViewMeasure->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->tableViewMeasure->horizontalHeader()->setStyleSheet("border: 0px solid ; border-bottom: 1px solid #d8d8d8;");
	ui->tableViewMeasure->verticalHeader()->setDefaultSectionSize(25);
	//ui->tableViewMeasure->horizontalHeader()->resizeSection(0, 30);
	ui->tableViewMeasure->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableViewMeasure->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableViewMeasure->setModel(m_pModelMeasureTable);
	ui->tableViewMeasure->setColumnWidth(0, 30);
	ui->tableViewMeasure->setColumnWidth(1, 60);
	ui->tableViewMeasure->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableViewMeasure->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
	ui->tableViewMeasure->horizontalHeader()->setStretchLastSection(true);
	connect(ui->tableViewMeasure->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &CSPointMeasureDlg::slotTabViewCurrentSelectChanged);

	CheckBoxDelegate *pDelegate = new CheckBoxDelegate(this);
	ui->tableViewMeasure->setItemDelegate(pDelegate);
	connect(m_pHeader, &TableHeaderView::SignalStateChanged, this, &CSPointMeasureDlg::slotHeaderClick);
	connect(pDelegate, &CheckBoxDelegate::signalTableCheck, this, &CSPointMeasureDlg::slotTableItemCheck);
	connect(this, &CSPointMeasureDlg::SignalStateChanged, m_pHeader, &TableHeaderView::SlotStateChanged);
	connect(m_pModelMeasureTable, &MeasureResultTableModel::signalGetCheckedStatus, this, &CSPointMeasureDlg::slotGetCheckedStatus);
	connect(m_pModelMeasureTable, &MeasureResultTableModel::signalAddMeasureItem, this, &CSPointMeasureDlg::slotAddMeasureItem);
}

void CSPointMeasureDlg::on_pushButton_clear_calibration_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	CMeasureLineManage::instance().clearMeasureCal(m_current_device_ptr->getIpOrSn());
	CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(QList<QPoint>{}));
}

void CSPointMeasureDlg::on_toolButtont_twoPoint_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_TWO_POINT);
}

void CSPointMeasureDlg::on_toolButtont_morePoint_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_MORE_POINT);
}

void CSPointMeasureDlg::on_toolButtont_angle_3_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_ANGLE_3);
}

void CSPointMeasureDlg::on_toolButtont_angle_4_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_ANGLE_4);
}

void CSPointMeasureDlg::on_toolButtont_radius_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_RADIUS);
}

void CSPointMeasureDlg::on_toolButtont_diameter_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_DIAMETER);
}

void CSPointMeasureDlg::on_toolButtont_center_distance_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_CENTER_DISTANCE);
}

void CSPointMeasureDlg::on_toolButtont_dimension_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_DIMENSION);
}

void CSPointMeasureDlg::on_toolButtont_area_radius_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_AREA_CENTER);
}

void CSPointMeasureDlg::on_toolButtont_area_polygon_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureLineType(ip, CMeasureLineManage::MLT_AREA_POLYGON);
}

void CSPointMeasureDlg::on_pushButton_edit_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	QString ip = m_current_device_ptr->getIpOrSn();
	CMeasureLineManage::instance().setMeasureModeType(ip, CMeasureLineManage::MMT_Modify);
}

void CSPointMeasureDlg::on_pushButton_clear_select_clicked()
{
	if (!m_current_device_ptr)
	{
		return;
	}
	if (!m_pModelMeasureTable)
	{
		return;
	}
	QList<int> vctIndex;
	QList<CMeasureLineManage::TMeasureLineInfo> vctInfo = m_pModelMeasureTable->GetSelectInfoList();
	int nSize = vctInfo.size();
	for (int i = 0; i < nSize; i++)
	{
		if (vctInfo[i].bChecked)
		{
			vctIndex.append(vctInfo[i].nIndex);
		}
	}
	if (vctInfo.size())
	{
		CMeasureLineManage::instance().deleteMeasureLines(m_current_device_ptr->getIpOrSn(), vctIndex);
	}
}

void CSPointMeasureDlg::slotTabViewCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous)
{
	if (!current.isValid())
	{
		return;
	}
	if (m_current_device_ptr)
	{
		QList<CMeasureLineManage::TMeasureLineInfo> vctMeasure = m_pModelMeasureTable->GetSelectInfoList();
		CMeasureLineManage::TMeasureLineInfo record = vctMeasure.at(current.row());
		if (record.nIndex >= 0)
		{
			CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(record.nIndex));
		}
	}
}

void CSPointMeasureDlg::slotHeaderClick(Qt::CheckState state)
{
	if (!m_pModelMeasureTable)
	{
		return;
	}
	QList<CMeasureLineManage::TMeasureLineInfo> vctLine = m_pModelMeasureTable->GetSelectInfoList();
	int nSize = vctLine.size();
	bool bChecked = false;
	if (state == Qt::Checked)
	{
		bChecked = true;
	}
	for (int i = 0; i < nSize; i++)
	{
		vctLine[i].bChecked = bChecked;
	}
	m_pModelMeasureTable->updateData(vctLine);

	if (m_current_device_ptr){	
		CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(-1));
	}
}

void CSPointMeasureDlg::slotTableItemCheck(const QModelIndex &index, bool bChecked1)
{
	if (!m_pModelMeasureTable){
		return;
	}
 	bool bChecked = true;
	QList<CMeasureLineManage::TMeasureLineInfo> vctLine = m_pModelMeasureTable->GetSelectInfoList();
	int nSize = vctLine.size();
	if (index.row() < nSize){
		vctLine[index.row()].bChecked = bChecked1;
		m_pModelMeasureTable->updateData(vctLine);
		if (m_current_device_ptr) {
			CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(-1));
		}
	}
	if (nSize == 0){
		bChecked = false;
	}
	Qt::CheckState state = Qt::Unchecked;
	if (bChecked){
		for (int i = 0; i < nSize; i++){
			if (!vctLine[i].bChecked)
			{
				bChecked = false;
				break;
			}
		}
	}
	if (bChecked){
		state = Qt::Checked;
	}
	SignalStateChanged(state);
}

void CSPointMeasureDlg::slotCurrentDeviceChanged(const QString current_ip)
{
	//判断是否跟之前的设备相同 ,不相同的话则断开原设备的信号连接并且跟新设备连接信号,判断是否需要刷新列表,相同的话则返回
	QSharedPointer<Device> new_device_ptr = DeviceManager::instance().getCurrentDevice();
	if (m_current_device_ptr == new_device_ptr)
	{
		return;
	}
	
	//不相同,断开原来的设备型号连接
	if (!m_current_device_ptr.isNull())
	{
		CMeasureLineManage::instance().emitCommonSignal(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS, QVariant::fromValue(QList<QPoint>{}));
		CMeasureLineManage::TMeasureModeType info;
		CMeasureLineManage::instance().getMeasureModeType(m_current_device_ptr->getIpOrSn(), info);
		if (info != CMeasureLineManage::MMT_Show)
		{
			CMeasureLineManage::instance().setMeasureModeType(m_current_device_ptr->getIpOrSn(), CMeasureLineManage::MMT_Show);
		}

		disconnect(m_current_device_ptr.data(), 0, this, 0);
	}
	m_current_device_ptr = new_device_ptr;
	//删除原有列表中的信息
	//m_video_table_model_ptr->removeRows(0, m_video_table_model_ptr->rowCount());

	if (!m_current_device_ptr.isNull())
	{
		m_pModelMeasureTable->SetDeviceName(m_current_device_ptr->getIpOrSn());
		//连接列表刷新信号.
		connect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSPointMeasureDlg::slotDeviceStateChanged);
		connect(m_current_device_ptr.data(), &Device::signalUpdateDrawStatusInfo, this, &CSPointMeasureDlg::slotUpdateDrawStatusInfo);
		updateUIValue(m_current_device_ptr->getState());
	}
	else
	{
		setEnabled(false);
		m_pModelMeasureTable->SetDeviceName("");
		clearLabelInfo();
	}
	Qt::CheckState state = Qt::Unchecked;
	SignalStateChanged(state);
}

void CSPointMeasureDlg::slotDeviceStateChanged(const QString &ip, DeviceState state)
{
	if (m_current_device_ptr && ip != m_current_device_ptr->getIpOrSn()) return;
	if (state == Unconnected || state == Disconnected)
	{
		setEnabled(false);
		QList<CMeasureLineManage::TMeasureLineInfo> vctInfo;
		m_pModelMeasureTable->updateData(vctInfo);
	}
	if (m_current_device_ptr.isNull())
	{
		setEnabled(false);
		return;
	}
	if (state == Previewing || state == Acquiring || state == Recording)
	{
		QString strIP = m_current_device_ptr->getIpOrSn();
		QPoint ptOffset(0, 0);
		if (state == Acquiring || state == Recording)
		{
			QRect rc = m_current_device_ptr->GetRoi(Device::kDeviceRoi);
			ptOffset = rc.topLeft();
		}
		CMeasureLineManage::instance().setMeasurePointOffset(strIP, ptOffset);
		setEnabled(true);
		QList<CMeasureLineManage::TMeasureLineInfo> vctInfo;
		CMeasureLineManage::instance().getMeasureLine(strIP, vctInfo);
		m_pModelMeasureTable->updateData(vctInfo);
		CMeasureLineManage::TMeasureModeType info;
		CMeasureLineManage::instance().getMeasureModeType(strIP, info);
		if (info != CMeasureLineManage::MMT_Show)
		{
			CMeasureLineManage::instance().setMeasureModeType(strIP, CMeasureLineManage::MMT_Show);
		}
		CMeasureLineManage::instance().emitCommonSignal(strIP, CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(-1));
	}
	else
	{
		setEnabled(false);
		Qt::CheckState state = Qt::Unchecked;
		SignalStateChanged(state);
	}
	updateUIValue(state);
}

void CSPointMeasureDlg::slotMeasureCal(const QString strIP, const CMeasureLineManage::TMesaureLineCal info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	setCalibrateInfo(info);
}

void CSPointMeasureDlg::slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	ui->tableViewMeasure->scrollToBottom();
}

void CSPointMeasureDlg::slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	if (CMeasureLineManage::MMT_Modify == info)
	{
		ui->pushButton_edit->setEnabled(false);
	}
	else
	{
		ui->pushButton_edit->setEnabled(true);
	}
	if (CMeasureLineManage::MMT_Modify == info || CMeasureLineManage::MMT_Add == info)
	{
		m_current_device_ptr->setDrawTypeStatusInfo(Device::DTSI_DrawMeasure);
	}
	else
	{
		m_current_device_ptr->setDrawTypeStatusInfo(Device::DTSI_Noraml);
	}
}

void CSPointMeasureDlg::slotMeasureLineCountChange(const QString strIP, const int nCount)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;
	if (0 == nCount)
	{
		ui->pushButton_edit->setEnabled(false);
	}
	else
	{
		CMeasureLineManage::TMeasureModeType info;
		CMeasureLineManage::instance().getMeasureModeType(strIP, info);
		if (CMeasureLineManage::MMT_Modify == info)
		{
			ui->pushButton_edit->setEnabled(false);
		}
		else
		{
			QList<CMeasureLineManage::TMeasureLineInfo> vctLine;
			CMeasureLineManage::instance().getMeasureLine(strIP, vctLine);
			if (vctLine.size() == 0)
			{
				ui->pushButton_edit->setEnabled(false);
			}
			else
			{
				ui->pushButton_edit->setEnabled(true);
			}
		}
	}
}

void CSPointMeasureDlg::slotGetCheckedStatus()
{
	if (m_pModelMeasureTable)
	{
		if (m_pHeader)
		{
			m_pModelMeasureTable->setCheckedStatus(m_pHeader->GetChecked());
		}
	}
}

void CSPointMeasureDlg::slotAddMeasureItem(int nIndex)
{
	if (!m_current_device_ptr) return;
	if (m_pModelMeasureTable)
	{
		QList<CMeasureLineManage::TMeasureLineInfo> vctMeasure = m_pModelMeasureTable->GetSelectInfoList();
		int nCount = vctMeasure.size();
		for (int i = 0; i < nCount; i++) {
			QModelIndex modelIndex = m_pModelMeasureTable->index(i, 0);
			if (modelIndex.isValid())
			{
				CMeasureLineManage::TMeasureLineInfo record = vctMeasure.at(modelIndex.row());
				if (record.nIndex == nIndex)
				{
					ui->tableViewMeasure->setCurrentIndex(modelIndex);
					break;
				}
			}
		}
		
	}
}
void CSPointMeasureDlg::slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info)
{
	if (!m_current_device_ptr || strIP != m_current_device_ptr->getIpOrSn()) return;

	switch (commandType)
	{
	case CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED:
	{
		int nIndex = info.toInt();
		if (nIndex < 0){
			return;
		}
		QList<CMeasureLineManage::TMeasureLineInfo> vctMeasure = m_pModelMeasureTable->GetSelectInfoList();
		QModelIndex modelIndex = ui->tableViewMeasure->currentIndex();
		if (modelIndex.isValid())
		{
			CMeasureLineManage::TMeasureLineInfo record = vctMeasure.at(modelIndex.row());
			if (record.nIndex == nIndex){
				return;
			}
		}
		int nCount = vctMeasure.size();
		for (int i = 0; i < nCount; i++){
			QModelIndex modelIndex = m_pModelMeasureTable->index(i, 0);
			if (modelIndex.isValid())
			{
				CMeasureLineManage::TMeasureLineInfo record = vctMeasure.at(modelIndex.row());
				if (record.nIndex == nIndex)
				{
					ui->tableViewMeasure->setCurrentIndex(modelIndex);
					break;
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

void CSPointMeasureDlg::setCalibrateInfo(const CMeasureLineManage::TMesaureLineCal info)
{
	int nIndex = 0;
	if (!info.bCalibration)
	{
		ui->label_calibrate_status->setText(tr("Not calibrated temporarily"));
		ui->label_calibrate_status->setToolTip(tr("Not calibrated temporarily"));
		//ui->doubleSpinBox_pixel_value->setValue(0.001);
		ui->pushButton_clear_calibration->setEnabled(false);
	}
	else
	{
		QString strText;
		QString strUnit = CMeasureLineManage::GetMeasureCalTypeStr(info.type);
		if (info.type == CMeasureLineManage::MLU_Pixel)
		{
			strUnit.clear();
		}
// 		int nCount = ui->comboBox_Pixel_Unit->count();
// 		for (int i = 0; i < nCount; i++)
// 		{
// 			if (ui->comboBox_Pixel_Unit->itemData(i).toInt() == (int)info.type)
// 			{
// 				nIndex = i;
// 				break;
// 			}
// 		}
		strText = tr("1px = %1%2!").arg(QString::number(info.dbUnit, 'f', 3)).arg(strUnit);
		ui->label_calibrate_status->setText(strText);
		QString strTooptipText = tr("1px = %1%2!").arg(QString::number(info.dbUnit)).arg(strUnit);
		ui->label_calibrate_status->setToolTip(strTooptipText);
		//ui->doubleSpinBox_pixel_value->setValue(info.dbUnit);
		ui->pushButton_clear_calibration->setEnabled(true);
	}
	//ui->comboBox_Pixel_Unit->setCurrentIndex(nIndex);
	if (m_pModelMeasureTable)
	{
		QStringList labels;
		labels << tr("") << tr("Type") << tr("Name") << tr("%1(%2)").arg(tr("Result")).arg(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(info.type));
		m_pModelMeasureTable->SetListColNum(labels.size());
		m_pModelMeasureTable->SetHeadText(labels);
		QString strText = CMeasureLineManage::GetMeasureCalTypeOrAreaStr(info.type);
		QString strOldText = ui->comboBox_showUnit->currentText();
		if (strOldText == strText)
		{
			int nCur = ui->comboBox_showUnit->currentIndex();
			on_comboBox_showUnit_currentIndexChanged(nCur);
		}
		else {
			ui->comboBox_showUnit->setCurrentText(strText);
		}
	}
}

void CSPointMeasureDlg::updateUIValue(DeviceState state)
{
	if (!m_current_device_ptr)
	{
		clearLabelInfo();
		clearTabelView();
		setEnabled(false);
	}
	else
	{
		QString strIP = m_current_device_ptr->getIpOrSn();
		if (state == Previewing || state == Acquiring || state == Recording)
		{
			setEnabled(true);
			QPoint ptOffset(0, 0);
			if (state == Acquiring || state == Recording)
			{
				QRect rc = m_current_device_ptr->GetRoi(Device::kDeviceRoi);
				ptOffset = rc.topLeft();
			}
			CMeasureLineManage::instance().setMeasurePointOffset(strIP, ptOffset);

			CMeasureLineManage::TMesaureLineCal calInfo;
			CMeasureLineManage::instance().getMeasureCal(strIP, calInfo);
			setCalibrateInfo(calInfo);
			
			QList<CMeasureLineManage::TMeasureLineInfo> vctLine;
			CMeasureLineManage::instance().getMeasureLine(strIP, vctLine);
			if (vctLine.size() == 0)
			{
				ui->pushButton_edit->setEnabled(false);
			}
			else
			{
				CMeasureLineManage::TMeasureModeType type;
				CMeasureLineManage::instance().getMeasureModeType(strIP, type);
				if (CMeasureLineManage::MMT_Modify == type)
				{
					ui->pushButton_edit->setEnabled(false);
				}
				else
				{
					ui->pushButton_edit->setEnabled(true);
				}
				CMeasureLineManage::instance().emitCommonSignal(strIP, CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(-1));
			}
		}
		else
		{
			clearLabelInfo();
			clearTabelView();
			setEnabled(false);
		}
	}
}

void CSPointMeasureDlg::clearLabelInfo()
{
	ui->label_calibrate_status->setText(tr("Not calibrated temporarily"));
	ui->label_calibrate_status->setToolTip(tr("Not calibrated temporarily"));
	ui->pushButton_clear_calibration->setEnabled(false);
// 	ui->doubleSpinBox_pixel_value->setValue(0.001);
// 	ui->comboBox_Pixel_Unit->setCurrentIndex(0);
}

void CSPointMeasureDlg::clearTabelView()
{
	if (m_pModelMeasureTable)
	{
		QStringList labels;
		labels << tr("") << tr("Type") << tr("Name") << tr("%1(%2)").arg(tr("Result")).arg(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(CMeasureLineManage::MLU_Pixel));
		m_pModelMeasureTable->SetListColNum(labels.size());
		m_pModelMeasureTable->SetHeadText(labels);
		QList<CMeasureLineManage::TMeasureLineInfo> vctLine;
		m_pModelMeasureTable->updateData(vctLine);
		ui->comboBox_showUnit->setCurrentText(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(CMeasureLineManage::MLU_Pixel));
	}
}

void CSPointMeasureDlg::on_pushButton_go_calibration_clicked()
{
	emit signalHideSelf();
}

void CSPointMeasureDlg::on_comboBox_showUnit_currentIndexChanged(int index)
{
	if (index > -1)
	{
		if (m_pModelMeasureTable)
		{
			if (!m_current_device_ptr)
			{
			}
			else
			{
				QString strIP = m_current_device_ptr->getIpOrSn();
				CMeasureLineManage::TMesaureLineCal info;
				CMeasureLineManage::instance().getMeasureCal(strIP, info);
				CMeasureLineManage::TMeasureLineUnit enumIndex = (CMeasureLineManage::TMeasureLineUnit)index;
				if (info.type != enumIndex)
				{
					double value = 1.0;
					switch (info.type)
					{
					case CMeasureLineManage::MLU_Pixel:
						value = -1.0;
						break;
					case CMeasureLineManage::MLU_MM:
					{
						switch (enumIndex)
						{
						case CMeasureLineManage::MLU_Pixel:
							value = 1.0 / info.dbUnit;
							break;
						case CMeasureLineManage::MLU_MM:
							value = 1.0;
							break;
						case CMeasureLineManage::MLU_CM:
							value = 0.1;
							break;
						case CMeasureLineManage::MLU_M:
							value = 0.001;
							break;
						case CMeasureLineManage::MLU_Count:
							break;
						default:
							break;
						}
					}
						break;
					case CMeasureLineManage::MLU_CM:
					{
						switch (enumIndex)
						{
						case CMeasureLineManage::MLU_Pixel:
							value = 1.0 / info.dbUnit;
							break;
						case CMeasureLineManage::MLU_MM:
							value = 10.0;
							break;
						case CMeasureLineManage::MLU_CM:
							value = 1.0;
							break;
						case CMeasureLineManage::MLU_M:
							value = 0.01;
							break;
						case CMeasureLineManage::MLU_Count:
							break;
						default:
							break;
						}
					}
						break;
					case CMeasureLineManage::MLU_M:
					{
						switch (enumIndex)
						{
						case CMeasureLineManage::MLU_Pixel:
							value = 1.0 / info.dbUnit;
							break;
						case CMeasureLineManage::MLU_MM:
							value = 1000.0;
							break;
						case CMeasureLineManage::MLU_CM:
							value = 100.0;
							break;
						case CMeasureLineManage::MLU_M:
							value = 1;
							break;
						case CMeasureLineManage::MLU_Count:
							break;
						default:
							break;
						}
					}
						break;
					default:
						break;
					}
					m_pModelMeasureTable->SetShowUnitCoefficient(value);
				}
				else
				{
					m_pModelMeasureTable->SetShowUnitCoefficient(1.0);
				}
				QStringList labels;
				labels << tr("") << tr("Type") << tr("Name") << tr("%1(%2)").arg(tr("Result")).arg(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(enumIndex));
				m_pModelMeasureTable->SetListColNum(labels.size());
				m_pModelMeasureTable->SetHeadText(labels);

				ui->tableViewMeasure->setColumnWidth(0, 30);
				ui->tableViewMeasure->setColumnWidth(1, 60);
				ui->tableViewMeasure->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
				ui->tableViewMeasure->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
				ui->tableViewMeasure->horizontalHeader()->setStretchLastSection(true);
			}
		}
	}
}

void CSPointMeasureDlg::slotUpdateDrawStatusInfo(Device::DrawTypeStatusInfo info)
{
	if (DeviceManager::instance().getCurrentDevice() == qobject_cast<Device*>(sender()))
	{
		if (info == Device::DTSI_Noraml || info == Device::DTSI_DrawMeasure)
		{
			setEnabled(true);
		}
		else
		{
			setEnabled(false);
		}
	}
}

void CSPointMeasureDlg::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		if (m_pModelMeasureTable)
		{
			QString strIP;
			CMeasureLineManage::TMesaureLineCal calInfo;
			if (m_current_device_ptr)
			{
				strIP = m_current_device_ptr->getIpOrSn();
				CMeasureLineManage::instance().getMeasureCal(strIP, calInfo);
			}
			QStringList labels;
			labels << tr("") << tr("Type") << tr("Name") << tr("%1(%2)").arg(tr("Result")).arg(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(calInfo.type));
			m_pModelMeasureTable->SetListColNum(labels.size());
			m_pModelMeasureTable->SetHeadText(labels);
			ui->comboBox_showUnit->setCurrentText(CMeasureLineManage::GetMeasureCalTypeOrAreaStr(calInfo.type));
		}
		ui->toolButtont_twoPoint->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_TWO_POINT));
		ui->toolButtont_twoPoint->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_TWO_POINT));
		ui->toolButtont_twoPoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_morePoint->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_MORE_POINT));
		ui->toolButtont_morePoint->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_MORE_POINT));
		ui->toolButtont_morePoint->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_angle_3->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_3));
		ui->toolButtont_angle_3->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_3));
		ui->toolButtont_angle_3->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_angle_4->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_4));
		ui->toolButtont_angle_4->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_ANGLE_4));
		ui->toolButtont_angle_4->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_radius->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_RADIUS));
		ui->toolButtont_radius->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_RADIUS));
		ui->toolButtont_radius->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_diameter->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIAMETER));
		ui->toolButtont_diameter->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIAMETER));
		ui->toolButtont_diameter->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_center_distance->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_CENTER_DISTANCE));
		ui->toolButtont_center_distance->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_CENTER_DISTANCE));
		ui->toolButtont_center_distance->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_dimension->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIMENSION));
		ui->toolButtont_dimension->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_DIMENSION));
		ui->toolButtont_dimension->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_area_radius->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_CENTER));
		ui->toolButtont_area_radius->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_CENTER));
		ui->toolButtont_area_radius->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		ui->toolButtont_area_polygon->setText(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_POLYGON));
		ui->toolButtont_area_polygon->setToolTip(CMeasureLineManage::GetMeasureLineTypeStr(CMeasureLineManage::MLT_AREA_POLYGON));
		ui->toolButtont_area_polygon->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	}

	QWidget::changeEvent(event);
}
