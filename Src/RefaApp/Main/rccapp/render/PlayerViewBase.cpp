#include "PlayerViewBase.h"
#include <QOpenGLWidget>
#include "PlayerViewBase.h"
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "Common/LogUtils/logutils.h"
#include <QException>
#include <QHBoxLayout>
#include "System/SystemSettings/systemsettingsmanager.h"
#include "ImageUtils.h"
#include "Device/devicemanager.h"
#include "Video/DataSavers/csvideosaver.h"
#include "Common/UIUtils/UIHelper.h"

CPlayerViewBase::CPlayerViewBase(int index, uint8_t frame_head_type, QWidget *parent /*= Q_NULLPTR*/)
	:QWidget(parent)
	,m_win_index(index)
{
	frame_header_type_ = frame_head_type;
	qRegisterMetaType<Device::RoiTypes>("RoiTypes");
	qRegisterMetaType<PointInfo>("PointInfo");
	qRegisterMetaType<MultiPointInfo>("MultiPointInfo");
	init();
	bindSignalAndSlot();
	InitViewLabel();
	initMeasureItem();
	if (EXPORT_PREVIEW_PLAYER == m_win_index)
	{
		MouseRightClicked();
		// 现阶段不支持脱靶量计算
		//InitOverlayUI();
	}
	else
	{
		m_rightMenu = new QMenu(this);
		act_close = new QAction(tr("Close"), this);
		act_close_all = new QAction(tr("Close All"), this);

		connect(act_close, &QAction::triggered, this, [this] {
			emit sigAboutToClose(m_win_index);
		});
		connect(act_close_all, &QAction::triggered, this, [this] {
			emit sigAboutToCloseAll();
		});

		m_rightMenu->addAction(act_close);
		m_rightMenu->addAction(act_close_all);
	}

	for (size_t i = 0; i < ViewLabel_Max_COUNT; i++)
	{
		m_mapViewText.insert(ViewShowLabelType(i), "");
		m_mapViewColor.insert(ViewShowLabelType(i), QColor(255, 0, 0));
	}
}

void CPlayerViewBase::SlotUpdateImage(RccFrameInfo img)
{
	if (img.image.isNull()) {//图片为空时，显示背景图
#if 1
		if (m_graphics_view) {
			m_graphics_view->resetMousePress();
		}
		auto target_background_img = m_background_img;
		if (m_graphics_view && m_graphics_view->getRotate() != 0) {
			QMatrix matrix;
			matrix.rotate(-m_graphics_view->getRotate());
			target_background_img = target_background_img.transformed(matrix, Qt::FastTransformation);
		}
		img.image = target_background_img;
#endif // 1
#if 0
		img.image = m_background_img;
#endif // 0
		if (m_graphics_view && m_graphics_scene) {
			m_is_show_background_img = true;
			m_graphics_view->setWheelDisalble(m_is_show_background_img);
			m_graphics_scene->setWheelDisalble(m_is_show_background_img);
		}	
	}
	else {
		if (m_graphics_view && m_graphics_scene) {
			if (m_is_show_background_img) {
				initMeasureLineInfo();
			}
			m_is_show_background_img = false;
			m_graphics_view->setWheelDisalble(m_is_show_background_img);
			m_graphics_scene->setWheelDisalble(m_is_show_background_img);
			if (m_is_size_changed) {
				m_is_size_changed = false;
				FitView();
			}
			
		}
	}
	//if (m_over_exposure) {
	//	UpdateImgPixelColor(img.image, m_exposure_img);
	//	img.image = m_exposure_img;
	//}

#if 0
	if (m_is_show_background_img)
		updateLayout();
	else {
		m_graphics_scene->setBgCoef(1.0);
		m_graphics_scene->setSceneRect(QRectF(0, 0, img.image.width(), img.image.height()));
	}
#endif // 0
#if 1
	m_graphics_scene->setBgCoef(1.0);
	m_graphics_scene->setSceneRect(QRectF(0, 0, img.image.width(), img.image.height()));
	updateViewSceneRect();
#endif // 1


	if ((m_current_image_backup.image.size() != img.image.size()) || m_is_show_background_img) {
		FitView(m_is_show_background_img);
		if (m_is_show_background_img)
		{
			clearMeasureItem();
		}
		if (m_current_image_backup.image.size() != img.image.size())
		{
			m_measureItemManage.setImageRect(QRectF(0, 0, img.image.width(), img.image.height()));
			if (!m_is_show_background_img) {
				initMeasureLineInfo();
			}
			m_device_roi.setImageRect(QRectF(0, 0, img.image.width(), img.image.height()));
			m_intelligent_roi.setImageRect(QRectF(0, 0, img.image.width(), img.image.height()));
			m_auto_exposure_roi.setImageRect(QRectF(0, 0, img.image.width(), img.image.height()));
			m_white_balance_roi.setImageRect(QRectF(0, 0, img.image.width(), img.image.height()));
		}
	}

	QPolygonF view_rect_in_scene_vec = m_graphics_view->mapToScene(m_graphics_view->viewport()->rect());
	m_view_rect_in_scene = QRectF(view_rect_in_scene_vec[0], view_rect_in_scene_vec[2]);
	m_graphics_scene->setCrossLineRect(m_view_rect_in_scene);

	//m_min_coefficient = qMin(abs(m_graphics_scene->sceneRect().width()*1.0 / m_view_rect_in_scene.width()), \
	//	abs(m_graphics_scene->sceneRect().height()*1.0 / m_view_rect_in_scene.height()));

#if 0
	bool wheel_down = (m_min_coefficient < 0.1) ? false : true;
	m_graphics_view->setWheelDownEnable(wheel_down);
	
	bool wheel_up = (m_min_coefficient > 16) ? false : true;
	m_graphics_view->setWheelUpEnable(wheel_up);
#endif // 0


	updateUI();
	if (m_over_exposure && (!m_is_show_background_img)) {//显示背景LOGO时，不进行过曝显示
		UpdateImgPixelColor(img.image, m_exposure_img);
		m_graphics_scene->updateImage(m_exposure_img);
	}
	else {
		m_graphics_scene->updateImage(img.image);
	}
	m_current_image_backup = img;

	if (EXPORT_PREVIEW_PLAYER == m_win_index)
	{
		QString strLT{};
		if (img.playback_info.is_key_frame)
		{
			strLT = QString("%1/%2").arg(img.playback_info.frame_no + 1).arg(img.playback_info.end_frame_no + 1) \
				+ QString(" ") + tr("[Key Frame]") + QString("  ") + img.ip_or_sn;
		}
		else
		{
			strLT = QString("%1/%2").arg(img.playback_info.frame_no + 1).arg(img.playback_info.end_frame_no + 1) + QString("  ")\
				+ img.ip_or_sn;
		}
		SetViewShowLabelInfo(ViewIpLabel, strLT);

		QString strLB;
		if (frame_header_type_ == HEAD_TYPE::eMType) {
			strLB = DeviceUtils::formatNewTimestamp(img.timestamp);
		}
		else if (frame_header_type_ == HEAD_TYPE::eGTypeNs) {
			strLB = DeviceUtils::formatNewTimestampG(img.timestamp);
		}
		else if (frame_header_type_ == HEAD_TYPE::eS1315) {
			strLB = DeviceUtils::formatNewTimestampNs(img.timestamp);
		}
		else {
			strLB = DeviceUtils::formatTimestamp(img.timestamp);
		}
		SetViewShowLabelInfo(ViewTimestampLabel, strLB);
	}
	else
	{
		//输出日志
		bool output_log = false;

		for (auto item : img.osdInfos)
		{
			// Juwc test
			// qDebug() << " ==== Juwc ==== SlotUpdateImage() OsdText: " << item.OsdText;

			if (item.pos_type == OSDInfo::RT)
			{
				if (m_mapViewText[ViewConnectStatusLabel] != item.OsdText)
				{
					output_log = true;//状态切换时输出日志
				}

				SetViewShowLabelInfo(ViewConnectStatusLabel, item.OsdText);
				if (m_graphics_view)
				{
					m_graphics_view->SetViewShowLabelColor(BQGraphicsView::ViewConnectStatusLabel, item.color);
				}
				//m_mapViewColor[ViewConnectStatusLabel] = item.color;
				if (output_log)
				{
					CSLOG_INFO("update statu label info, id:{}, ip:{}, info:{}", m_win_index, m_strDeiveName.toStdString(), item.OsdText.toStdString());
				}
			}
			if (item.pos_type == OSDInfo::LT)
			{
				SetViewShowLabelInfo(ViewIpLabel, item.OsdText);
				//m_mapViewColor[ViewIpLabel] = item.color;
				if (m_graphics_view)
				{
					m_graphics_view->SetViewShowLabelColor(BQGraphicsView::ViewIpLabel, item.color);
				}
			}
			if (item.pos_type == OSDInfo::LB)
			{
				SetViewShowLabelInfo(ViewTimestampLabel, item.OsdText);
				//m_mapViewColor[ViewTimestampLabel] = item.color;
				if (m_graphics_view)
				{
					m_graphics_view->SetViewShowLabelColor(BQGraphicsView::ViewTimestampLabel, item.color);
				}
			}
		}
	}
}

void CPlayerViewBase::SlotMeasureT2VUpdateLineType(const QString &ip, const CMeasureLineManage::TMeasureLineType Type)
{	
	if (ip.compare(m_strDeiveName) != 0)
	{
		return;
	}
	setMeasureModeType(CMeasureLineManage::MMT_Add);
	//m_tMeasureModeTye = CMeasureLineManage::MMT_Add;
	m_tMeasureLineTye = Type;
	m_tAddMeasurePoint.curPoint = QPoint();
	m_tAddMeasurePoint.vctPoint.clear();
	switch (Type)
	{
	case CMeasureLineManage::MLT_MORE_POINT:
	case CMeasureLineManage::MLT_AREA_POLYGON:
	{
		m_tAddMeasurePoint.nMaxPointCount = CMeasureLineManage::kMaxPointCount;
		break;
	}
	case CMeasureLineManage::MLT_TWO_CALIBRATION:
	case CMeasureLineManage::MLT_TWO_POINT:
	{
		m_tAddMeasurePoint.nMaxPointCount = 2;
		break;
	}
	default:
		m_tAddMeasurePoint.nMaxPointCount = CMeasureLineManage::kMaxPointCount;
		break;
	}
	m_measureItemManage.updateMeasureType(Type);
}

void CPlayerViewBase::SlotMeasureT2VUpdatModeType(const QString &ip, const CMeasureLineManage::TMeasureModeType Type)
{
	if (ip.compare(m_strDeiveName) != 0)
	{
		return;
	}
	setMeasureModeType(Type);
}

void CPlayerViewBase::slotAddMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_vctMeasureLine.append(info);

	updateMeaureInfo(info);

	m_measureItemManage.addMeasureItem(info, true);
}

void CPlayerViewBase::slotUpdateMeasureLine(const QString strIP, const CMeasureLineManage::TMeasureLineInfo info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}

	updateMeaureInfo(info);
	m_measureItemManage.updateMeasureItem(info);

	int nSize = m_vctMeasureLine.size();
	for (int i = 0; i < nSize; i++)
	{
		if (info.nIndex == m_vctMeasureLine[i].nIndex)
		{
			m_vctMeasureLine[i] = info;
			break;
		}
	}
}

void CPlayerViewBase::slotUpdateMeasureLines(const QString strIP, const QList<CMeasureLineManage::TMeasureLineInfo> info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_vctMeasureLine = info;
}

void CPlayerViewBase::slotDeleteMeasureLines(const QString strIP, const QList<int> vctLine)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	int nSize = vctLine.size();
	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < m_vctMeasureLine.size(); j++)
		{
			if (vctLine[i] == m_vctMeasureLine[j].nIndex)
			{
				m_vctMeasureLine.removeAt(j);
				break;
			}
		}
	}
	m_measureItemManage.deleteMeasureItem(vctLine);
}

void CPlayerViewBase::slotClearMeasureLines(const QString strIP)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_vctMeasureLine.clear();
}

void CPlayerViewBase::slotMeasureModeType(const QString strIP, const CMeasureLineManage::TMeasureModeType info)
{
	m_measure_btn_clicked = false;
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}

	m_graphics_scene->setMeasureModeType(info);

	updateMeasureModeType(info);
	if (CMeasureLineManage::TMeasureModeType::MMT_Modify == info) {
		m_measure_btn_clicked = true;
	}
	else {
		m_measure_btn_clicked = false;
	}
	if (CMeasureLineManage::TMeasureModeType::MMT_Modify == info || CMeasureLineManage::TMeasureModeType::MMT_Add == info)
	{
		updateItemLayer(Device::kMeasureLine);
	}

	m_measureItemManage.updateMeasureModeType(info);
	if (CMeasureLineManage::TMeasureModeType::MMT_Modify == info)
	{
		m_graphics_view->setTranslation(false);
	}
	else
	{
		m_graphics_view->setTranslation(true);
	}
}

void CPlayerViewBase::slotMeasurePointOffset(const QString strIP, const QPoint point)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	m_PointOffset = point;
	m_measureItemManage.setImageOffset(point);

	m_intelligent_roi.setImageOffset(point);
	m_auto_exposure_roi.setImageOffset(point);
	m_device_roi.setImageOffset(point);
}


void CPlayerViewBase::SlotDisplayWidgetAdjust(int type)
{
	m_split_type = type;
#if 0
	updateLayout();
#endif // 0
#if 1
#endif //1
}

void CPlayerViewBase::slotGotoMeasureCal(const QString strIP, double dbValue, int nType)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	// goto save calibration
	QList<QPoint> vctPoints = m_measureItemManage.getCalibrationPoints();
	if (1 < vctPoints.size())
	{
		CMeasureLineManage::TMesaureLineCal info;
		info.type = CMeasureLineManage::TMeasureLineUnit(nType);
		double dbDistance = CMeasureLineManage::PointsDistance(vctPoints);
		if (dbDistance < 0.001)
		{
			dbDistance = 0.001;
		}
		info.dbUnit = dbValue / dbDistance;
		info.dbPoint2Distance = dbValue;
		info.bCalibration = true;
		for (auto pt : vctPoints)
		{
			info.vctPoint.append(pt);
		}
		info.tCalibrationType = CMeasureLineManage::MCT_TWO_POINT;
		CMeasureLineManage::instance().setMeasureCal(m_strDeiveName, info);
	}
}

void CPlayerViewBase::slotMeasureCalMode(const QString strIP, const CMeasureLineManage::TMeasureModeType info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	CMeasureLineManage::TMeasureModeType oldType = m_tMeasureCalMode;
	m_tMeasureCalMode = info;
	CMeasureLineManage::instance().setMeasureLineType(m_strDeiveName, CMeasureLineManage::MLT_TWO_CALIBRATION);
	QList<QPoint> vctPoint;
	switch (info)
	{
	case CMeasureLineManage::MMT_Normal:
	{
		m_measureItemManage.setCalibration(m_tMeasureCalMode);
		if (m_graphics_scene)
		{
			m_graphics_scene->setCalibration(m_tMeasureCalMode);
		}
		setCursor(QCursor(Qt::ArrowCursor));
		break;
	}
	case CMeasureLineManage::MMT_Show:
	{
		m_measureItemManage.setCalibration(m_tMeasureCalMode);
		if (m_graphics_scene)
		{
			m_graphics_scene->setCalibration(m_tMeasureCalMode);
		}
		setCursor(QCursor(Qt::ArrowCursor));
		break;
	}
	case CMeasureLineManage::MMT_Add:
	{
		m_tMeasureLineTye = CMeasureLineManage::MLT_TWO_CALIBRATION;
		m_measureItemManage.setCalibration(m_tMeasureCalMode);
		if (m_graphics_scene)
		{
			m_graphics_scene->setCalibration(m_tMeasureCalMode);
		}
		setCursor(QCursor(Qt::ArrowCursor));
		break;
	}
	case CMeasureLineManage::MMT_Modify:
	{
		m_measureItemManage.setCalibration(m_tMeasureCalMode);
		if (m_graphics_scene)
		{
			m_graphics_scene->setCalibration(m_tMeasureCalMode);
		}
		setCursor(QCursor(Qt::ArrowCursor));
		break;
	}
	default:
		m_measureItemManage.setCalibration(m_tMeasureCalMode);
		if (m_graphics_scene)
		{
			m_graphics_scene->setCalibration(m_tMeasureCalMode);
		}
		setCursor(QCursor(Qt::ArrowCursor));
		break;
	}
	vctPoint = m_measureItemManage.getCalibrationPoints();
	double dbValue = 0.0;
	dbValue = CMeasureLineManage::PointsDistance(vctPoint);
	CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE, QVariant::fromValue(dbValue));

	CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS_SIZE, vctPoint.size());
}

void CPlayerViewBase::slotCommonSignal(const QString strIP, const CMeasureLineManage::TMeasureCommonSignalType commandType, QVariant info)
{
	if (strIP.compare(m_strDeiveName) != 0)
	{
		return;
	}
	switch (commandType)
	{
	case CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED:
	{
		int nIndex = info.toInt();
		m_measureItemManage.setSelectedItem(nIndex);
		break;
	}
	case CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS:
	{
		QList<QPoint> vctPoints = info.value<QList<QPoint>>();
		m_measureItemManage.setCalibrationPoints(vctPoints);
	}
	default:
		break;
	}
}
#include <qtimer.h>
void CPlayerViewBase::updateRoiInfoSlot(const Device::RoiTypes& type)
{
	if (!m_is_acquring) {

		switch (type)
		{
		case Device::RoiTypes::kDeviceRoi:
		{
			emit updateRoiInfoSignal(type, m_device_roi.getRoiRect().toRect());
			//updateItemLayer(type);// 其它几个ROI变化是否也需要更新图层 待后续逐渐验证 若都需要的话 把该语句放到switch代码外执行
			break;
		}
		case Device::kIntelligentTriggerRoi:
		{
			emit updateRoiInfoSignal(type, m_intelligent_roi.getRoiRect().toRect());
			break;
		}
		case Device::kAutoExposureRoi: {
			emit updateRoiInfoSignal(type, m_auto_exposure_roi.getRoiRect().toRect());
			break;
		}
		case Device::kManualWhiteBalance: {
			emit updateRoiInfoSignal(type, m_white_balance_roi.getRoiRect().toRect());
			break;
		}
		default:
			break;
		}
		updateItemLayer(type);
		m_device_roi.setFeaturePointVisible(Device::RoiTypes::kDeviceRoi ==type && m_device_roi.getFeaturePointVisible());
		m_intelligent_roi.setFeaturePointVisible(Device::RoiTypes::kIntelligentTriggerRoi == type && m_intelligent_roi.getFeaturePointVisible());
		m_auto_exposure_roi.setFeaturePointVisible(Device::RoiTypes::kAutoExposureRoi == type && m_auto_exposure_roi.getFeaturePointVisible());
		m_white_balance_roi.setFeaturePointVisible(Device::RoiTypes::kManualWhiteBalance == type && m_white_balance_roi.getFeaturePointVisible());
	}
}

void CPlayerViewBase::imageRectSlot(const QRect& rect)
{
	m_original_roi_rect = rect;
}

void CPlayerViewBase::sendMousePtInSceneSlot(const QPoint& point)
{
	if (m_is_show_background_img) {
		return;
	}
	cv::Mat dst;
	bool bOutOfImage = false;
	if (point == QPoint{ -1,-1 })
	{
		bOutOfImage = true;
	}
	if (!bOutOfImage && !m_current_image_backup.image.isNull()) {
		QImage temp = m_current_image_backup.image;
		if (m_current_image_backup.raw_image.empty())
		{
			QImage2CvMat(temp, dst, true);
		}
		else
		{
			if (m_current_image_backup.raw_image.depth() == CV_16U)
			{
				dst = m_current_image_backup.raw_image.clone() / std::pow(2, (16 - m_current_image_backup.valid_bits));
				//dst = m_current_image_backup.raw_image.clone() / (std::pow(2, 16) - 1) * (std::pow(2, m_current_image_backup.valid_bits) - 1);
			}
			else
			{
				dst = m_current_image_backup.raw_image.clone();
			}
		}
		emit SignalUpdateGrayOrRGBValue(point, dst);
	}
	else
	{
		bOutOfImage = true;
	}
	if (bOutOfImage)
	{
		emit SignalUpdateGrayOrRGBValue(QPoint(-1, -1), dst);
	}
}

void CPlayerViewBase::updateMeasureItemInfoSlot(const PointInfo& item)
{
	CMeasureLineManage::TMeasureLineInfo info;
	info.nIndex = item.point_index;
	info.vctPoint.append(item.start_point.toPoint());
	info.vctPoint.append(item.end_point.toPoint());
	info.nType = (CMeasureLineManage::TMeasureLineType)item.measure_type;
	info.strName = QString::number(info.nIndex);
	info.bChecked = false;
	CMeasureLineManage::instance().updateOneMeasureLine(m_strDeiveName, info);
}

void CPlayerViewBase::updatemultiMeasureItemInfoSlot(const MultiPointInfo& item)
{
	CMeasureLineManage::TMeasureLineInfo info;
	info.nIndex = item.multi_point_index;
	for (auto pt : item.multi_pt_feature_vec) {
		info.vctPoint.append(pt.toPoint());
	}
	info.nType = (CMeasureLineManage::TMeasureLineType)item.measure_type;
	info.strName = QString::number(info.nIndex);
	info.bChecked = false;
	CMeasureLineManage::instance().updateOneMeasureLine(m_strDeiveName, info);
}

bool CPlayerViewBase::GetTrackRoiEnabled()
{
	return m_roi_drawing;
}

void CPlayerViewBase::SetAntiAliasing(bool bAntiAliased)
{
	if (m_graphics_view) {
		m_bAnti = bAntiAliased;
		m_graphics_view->setRenderHint(QPainter::Antialiasing, m_bAnti);
		m_graphics_view->setRenderHint(QPainter::SmoothPixmapTransform, m_bAnti);
		m_graphics_view->setRenderHint(QPainter::TextAntialiasing, m_bAnti);
	}
}

bool CPlayerViewBase::GetAntiAliasingStatus()
{
	return m_bAnti;
}

void CPlayerViewBase::EnableDrawBorderLine(bool bdraw)
{
	if (m_is_draw_border != bdraw) {	
		m_is_draw_border = bdraw;
		this->update();
	}
}

void CPlayerViewBase::SetOverExposureStatus(bool bOver)
{
	m_over_exposure = bOver;
}

void CPlayerViewBase::setBackgroundImage(const QImage& image)
{
	m_background_img = image;
}

void CPlayerViewBase::cvMat2QImage(const cv::Mat& src, QImage& dst)
{
	if (CV_8UC1 == src.type())
	{
		QImage image(src.cols, src.rows, QImage::Format_Indexed8);
		image.setColorCount(256);

		for (int i = 0; i < 256; ++i)
			image.setColor(i, qRgb(i, i, i));
		uchar * pSrc = src.data;
		if (pSrc)
		{
			for (int row = 0; row < src.rows; ++row)
			{
				uchar* pDst = image.scanLine(row);
				if (pDst)
				{
					memcpy(pDst, pSrc, src.cols);
					pSrc += src.step;
				}
			}
		}
		dst = std::move(image);
		return;
	}
	else if (CV_16UC1 == src.type())
	{
		QImage image(src.cols, src.rows, QImage::Format_Indexed8);
		image.setColorCount(256);

		for (int i = 0; i < 256; ++i)
			image.setColor(i, qRgb(i, i, i));
		for (int row = 0; row < src.rows; ++row)
		{
			const uint16_t* pSrc = src.ptr<uint16_t>(row);
			uchar* pDst = image.scanLine(row);
			if (pSrc && pDst)
			{
				for (int col = 0; col < src.cols; col++)
				{
					pDst[col] = pSrc[col] >> 8;
				}
			}
		}
		dst = std::move(image);
		return;
	}
	else if (CV_16UC3 == src.type())
	{
		cv::Mat temp_mat(src.rows, src.cols, CV_8UC3);

		for (int row = 0; row < src.rows; ++row)
		{
			const cv::Vec3w* pSrc = src.ptr<cv::Vec3w>(row);
			cv::Vec3b* pDst = temp_mat.ptr<cv::Vec3b>(row);
			if (pSrc && pDst)
			{
				for (int col = 0; col < src.cols; col++)
				{
					pDst[col](0) = pSrc[col](0) >> 8;
					pDst[col](1) = pSrc[col](1) >> 8;
					pDst[col](2) = pSrc[col](2) >> 8;
				}
			}
		}
		QImage temp(temp_mat.data, temp_mat.cols, temp_mat.rows, temp_mat.step, QImage::Format_RGB888);

		try {
			dst = std::move(temp.rgbSwapped());
		}
		catch (QException & e)
		{
			CSLOG_ERROR("qDst invalid:{},{},{},{},{}", temp.width(), temp.height(), temp.byteCount(), getpid(), e.what());
			return;
		};
	}
	else if (CV_8UC3 == src.type())
	{
		const uchar* pSrc = (const uchar*)src.data;
		if (pSrc)
		{
			QImage temp(pSrc, src.cols, src.rows, src.step, QImage::Format_RGB888);
			if (temp.format() != QImage::Format_RGB888 || temp.width() > 4096 || temp.height() > 4096)
			{
				CSLOG_ERROR("qDst invalid:{},{},{},{}", temp.width(), temp.height(), temp.byteCount(), getpid());
			}
			try {
				dst = std::move(temp.rgbSwapped());
			}
			catch (QException & e)
			{
				CSLOG_ERROR("qDst invalid:{},{},{},{},{}", temp.width(), temp.height(), temp.byteCount(), getpid(), e.what());
				return;
			};
		}
	}
	else if (CV_8UC4 == src.type())
	{
		const uchar* pSrc = (const uchar*)src.data;
		if (pSrc)
		{
			QImage temp(pSrc, src.cols, src.rows, src.step, QImage::Format_ARGB32);
			dst = std::move(temp.copy());
		}
	}
}

void CPlayerViewBase::QImage2CvMat(QImage& src, cv::Mat& dst, bool bCopy)
{
	switch (src.format())
	{
	case QImage::Format_Grayscale8:
		dst = cv::Mat(src.height(), src.width(), CV_8UC1, (void*)src.bits(), (size_t)src.bytesPerLine());
		break;
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		dst = cv::Mat{ src.height(),src.width(),CV_8UC4,(void*)src.bits(),(size_t)src.bytesPerLine() };
		break;
	case QImage::Format_RGB888:
		if (bCopy)
		{
			dst = cv::Mat{ src.height(),src.width(),CV_8UC3,(void*)src.bits(),(size_t)src.bytesPerLine() }.clone();
		}
		else
		{
			dst = cv::Mat{ src.height(),src.width(),CV_8UC3,(void*)src.bits(),(size_t)src.bytesPerLine() };
		}
		cv::cvtColor(dst, dst, cv::COLOR_BGR2RGB);
		//注意: 该行为会变更src的rgb为bgr
		break;
	case QImage::Format_Indexed8:
		dst = cv::Mat{ src.height(),src.width(),CV_8UC1,(void*)src.bits(),(size_t)src.bytesPerLine() };
		break;
	}
}

void CPlayerViewBase::ZoomIn()
{
	if (this->m_is_show_background_img) // 显示为背景图像时 只进行自适应显示
		return;
	m_mouse_press_point = QPointF(width() / 2.0, height() / 2.0);
	if (m_graphics_view) {
		m_graphics_view->setMousePressPos(m_mouse_press_point);
		m_graphics_view->zoomIn();
	}
}

void CPlayerViewBase::ZoomOut()
{
	if (this->m_is_show_background_img) // 显示为背景图像时 只进行自适应显示
		return;
	if (m_graphics_view) {
		m_graphics_view->zoomOut();
	}
}

void CPlayerViewBase::FitView(bool bg)
{
	//if (m_graphics_view) {
	//	m_graphics_view->fitView(bg);
	//}
#if 1
	if (m_graphics_view)
		m_graphics_view->fitView(bg);
#endif // 1

#if 0
	if (m_graphics_view) {
		if (bg) {
			this->updateLayout();
			//m_graphics_view->resetTransform();
			//m_graphics_view->fitInView(m_background_img.rect(), Qt::KeepAspectRatio);
			m_graphics_view->fitView(bg);
		}
		else {
			m_graphics_view->fitView(bg);
		}
	}
#endif // 0

}

void CPlayerViewBase::FullView()
{
	if (this->m_is_show_background_img) // 显示为背景图像时 只进行自适应显示
		return;
	m_graphics_view->fullView();
}

void CPlayerViewBase::SetRotateType(ROTATE_TYPE type, bool bclockwise /*= true*/)
{
	if (!m_is_show_background_img) {//判断条件：旋转角度非0，切换到视频播放状态，禁止背景旋转
		FitView();    //旋转角度非0，切换到播放状态，旋转角度到0度，图像显示异常
	}
		
	if (m_graphics_view) {
		m_graphics_view->update();
		bclockwise ? m_graphics_view->rotateRight(type * 90) : m_graphics_view->rotateLeft(type * 90);
#if 1
		if (m_is_show_background_img) {
			auto target_background_img = m_background_img;
			if (m_graphics_view->getRotate() != 0) {
				QMatrix matrix;
				matrix.rotate(-m_graphics_view->getRotate());
				target_background_img = target_background_img.transformed(matrix, Qt::FastTransformation);
			}
			m_graphics_scene->updateImage(target_background_img);
			FitView();
		}
#endif // 1
		if (m_graphics_scene) {
			m_graphics_scene->setRotateValue(m_graphics_view->getRotate());
		}
	}
}

void CPlayerViewBase::SetViewShowLabelInfo(ViewShowLabelType type, QVariant info)
{
	m_mapViewText[type] = info.toString();

	if (m_graphics_view)
	{
		m_graphics_view->SetViewShowLabelInfo((BQGraphicsView::ViewShowLabelType)type, info);
	}
// 	if (m_mapViewLabel.end() != m_mapViewLabel.find(type))
// 	{
// 		if (nullptr != m_mapViewLabel[type])
// 		{
// 			if (ViewShowLabelType::ViewFrameLabel == type)
// 			{
// 				m_mapViewLabel[type]->setVisible(true);
// 			}
// 			m_mapViewLabel[type]->setText(info.toString());
// 			m_mapViewLabel[type]->adjustSize();
// 		}
// 	}
// 	AdjustViewBaseLabel(this->size());
	update();
}

void CPlayerViewBase::SetExportDisplayBtnStatus(bool bPause)
{
	m_bPauseBtnClicked = bPause;
}

void CPlayerViewBase::SetTargetScoringInfo(const bool bShow, const QPointF & fallPoint, const float & dis)
{
	m_overlayImageLabel->setVisible(bShow);
	if (bShow)
	{
		QString strPos = QString("(%1,%2)").arg(fallPoint.x()).arg(fallPoint.y());
		m_fallPosValueLabel->setText(strPos);

		QString strDis = QString("%1m").arg(dis);
		m_distanceValueLabel->setText(strDis);
	}
}

void CPlayerViewBase::SetManualSelectCloseBtnVisible(const bool bShow)
{
	m_bManualSelected = true;
	m_targetBtnClose->setVisible(bShow);
}

void CPlayerViewBase::updateItemLayer(const Device::RoiTypes& type)
{
	if (m_measure_btn_clicked) {
		m_device_roi.stackBefore(&m_measureItemManage);
		m_auto_exposure_roi.stackBefore(&m_measureItemManage);
		m_intelligent_roi.stackBefore(&m_measureItemManage);
		m_white_balance_roi.stackBefore(&m_measureItemManage);
		//m_measureItemManage.setSelected(true);
		return;
	}
	switch (type)
	{
	case Device::kAutoExposureRoi: {
		if (!m_auto_exposure_roi.getFeaturePointVisible()) 
			m_auto_exposure_roi.stackBefore(&m_measureItemManage);
		m_device_roi.stackBefore(&m_auto_exposure_roi);
		m_intelligent_roi.stackBefore(&m_auto_exposure_roi);
		m_white_balance_roi.stackBefore(&m_auto_exposure_roi);
		if (m_auto_exposure_roi.getFeaturePointVisible())
			m_measureItemManage.stackBefore(&m_auto_exposure_roi);
		//m_auto_exposure_roi.setSelected(true);
		break;
	}
	case Device::kIntelligentTriggerRoi:
	{
		if (!m_intelligent_roi.getFeaturePointVisible()) 
			m_intelligent_roi.stackBefore(&m_measureItemManage);
		m_device_roi.stackBefore(&m_intelligent_roi);
		m_auto_exposure_roi.stackBefore(&m_intelligent_roi);
		m_white_balance_roi.stackBefore(&m_intelligent_roi);
		if (m_intelligent_roi.getFeaturePointVisible())
			m_measureItemManage.stackBefore(&m_intelligent_roi);
		//m_intelligent_roi.setSelected(true);
		break;
	}
	case Device::kManualWhiteBalance: {
		if (!m_white_balance_roi.getFeaturePointVisible()) 
			m_white_balance_roi.stackBefore(&m_measureItemManage);
		m_device_roi.stackBefore(&m_white_balance_roi);
		m_auto_exposure_roi.stackBefore(&m_white_balance_roi);
		m_intelligent_roi.stackBefore(&m_white_balance_roi);
		if (m_white_balance_roi.getFeaturePointVisible()) 
			m_measureItemManage.stackBefore(&m_white_balance_roi);
		//m_white_balance_roi.setSelected(true);
		break;
	}
	case Device::kMeasureLine: {
		m_device_roi.stackBefore(&m_measureItemManage);
		m_auto_exposure_roi.stackBefore(&m_measureItemManage);
		m_intelligent_roi.stackBefore(&m_measureItemManage);
		m_white_balance_roi.stackBefore(&m_measureItemManage);
		//m_measureItemManage.setSelected(true);
		break;
	}
	case Device::kDeviceRoi:
	{
		if (!m_device_roi.getFeaturePointVisible())
			m_device_roi.stackBefore(&m_measureItemManage);
		m_intelligent_roi.stackBefore(&m_device_roi);
		m_auto_exposure_roi.stackBefore(&m_device_roi);
		m_white_balance_roi.stackBefore(&m_device_roi);
		if (m_device_roi.getFeaturePointVisible())
			m_measureItemManage.stackBefore(&m_device_roi);
		break;
	}
	case Device::kUnknownRoi:
	default: {
		m_device_roi.stackBefore(&m_measureItemManage);
		m_auto_exposure_roi.stackBefore(&m_measureItemManage);
		m_intelligent_roi.stackBefore(&m_measureItemManage);
		m_white_balance_roi.stackBefore(&m_measureItemManage);
		//m_device_roi.setSelected(true);
		break;
	}
	}
}

void CPlayerViewBase::turn2Display(bool status)
{
	m_is_turn_to_display = status;
}

void CPlayerViewBase::resizeScene()
{
	if (m_graphics_scene) {
		m_graphics_scene->refreshScene(Device::RoiTypes::kUnknownRoi);
	}
}

void CPlayerViewBase::resetParam4DeviceRoiForbid()
{
	m_roi_drawing = false;
	m_device_roi.setFeaturePointVisible(m_roi_drawing);
	m_device_roi.update();
}

void CPlayerViewBase::setDisplayStatus(bool acquiring, const QPointF& pt)
{
	m_is_acquring = acquiring;
	m_device_roi.setVisible(!m_is_show_background_img/* & !m_is_acquring*/);

	if (acquiring) {// 高采模式退出ROI选区调整
		m_device_roi.setFeaturePointVisible(false);
		m_intelligent_roi.setFeaturePointVisible(false);
		m_auto_exposure_roi.setFeaturePointVisible(false);
		m_white_balance_roi.setFeaturePointVisible(false);
	}


	m_graphics_scene->setCrossLineVisible(!m_is_show_background_img&m_cross_line_visible);
	m_graphics_scene->setCenterLineVisible(!m_is_show_background_img&m_center_line_visible);

	BQGraphicsScene::CommonParam param;
	param.center_pt = pt;
	param.is_acquring = acquiring;
	param.device_roi = m_device_roi.getRealRoiRect();
	param.is_draw_center_rect = isDrawCenterRect(pt, param.device_roi);
	param.is_draw_h_line = isDrawCenterHLine(pt, param.device_roi);
	param.is_draw_v_line = isDrawCenterVLine(pt, param.device_roi);

	m_graphics_scene->setCommonParam(param);
}

void CPlayerViewBase::setRoiConstraintCondition(int nConstraintCondition)
{
	m_device_roi.setRoiConstraintCondition(nConstraintCondition);
}

float CPlayerViewBase::GetZoomCoeff() const
{
	return qAbs(m_graphics_view->transform().m11() + m_graphics_view->transform().m12());
	//return m_min_coefficient;
}

bool CPlayerViewBase::Snapshot()
{
	//准备图像保存路径
	QString save_path = SystemSettingsManager::instance().getWorkingDirectory();
	save_path += '/';
	save_path += SystemSettingsManager::instance().getCurrentExperiment().code;
	save_path += '/';
	save_path += m_current_image_backup.ip_or_sn;
	save_path += '/';

	bool with_piv_flag = false;
	if (!m_current_image_backup.image.isNull())
	{
		auto type = frame_header_type_;
		QSharedPointer<Device> device_ptr = DeviceManager::instance().getDevice(m_current_image_backup.ip_or_sn);
		if (device_ptr)
		{
			//TODO:piv版本相关(前缀后缀)
			//图像旋转
			if (m_win_index != EXPORT_PREVIEW_PLAYER)
			{
				HscRotationType rotate_type = device_ptr->getProperty(Device::PropRotateType).value<HscRotationType>();
				if (rotate_type != HSC_ROTATION_NONE)
				{
					cv::Mat rotate_image;
					QImage2CvMat(m_current_image_backup.image, rotate_image, true);
					ImageUtils::Rotate(rotate_image, rotate_image, rotate_type);
					ImageUtils::Rotate(m_current_image_backup.raw_image, m_current_image_backup.raw_image, rotate_type);
					cvMat2QImage(rotate_image, m_current_image_backup.image);
				}
			}
			if (device_ptr->IsPIVEnabled())
			{
				with_piv_flag = true;
			}
			type = device_ptr->getFrameHeadType();
		}
		CSVideoSaver videoSaver(save_path, with_piv_flag);
		return videoSaver.SaveSnapshot(m_current_image_backup, m_bAnti, type);
	}

	return false;
}

void CPlayerViewBase::SetCustomCrossLineCenterPoint(const QPointF& centerpoint)
{
	if (m_graphics_scene) {
		m_graphics_scene->setCenterLinePt(centerpoint.toPoint());
	}
}

void CPlayerViewBase::EnableDrawCrossLine(bool bdraw)
{
	m_cross_line_visible = bdraw;
}

void CPlayerViewBase::EnableDrawCustomCrossLine(bool bdraw)
{
	m_center_line_visible = bdraw;
}

void CPlayerViewBase::drawRoiRect(const RoiInfo& _roi_info, bool mid)
{
	//if (m_is_acquring) {//高采模式下，禁止拖动、更改roi
	//	return;
	//}
	auto roi_info = _roi_info;
// 	if (m_is_acquring) {//高采模式下传入的ROI区域需要修正回去
// 		roi_info.roi_rect.adjust(+m_PointOffset.x(), +m_PointOffset.y(), +m_PointOffset.x(), +m_PointOffset.y());
// 	}
	switch (roi_info.roi_type)
	{
	case Device::RoiTypes::kDeviceRoi:
	{
		m_device_roi.setRoiType(roi_info.roi_type);
		m_device_roi.setParentScene(m_graphics_scene);
		QRect temp_roi_rect = roi_info.roi_rect;
		if (!m_is_acquring) {		
			if(m_roi_drawing)//非编辑状态下，不重新计算roi区域，否则会导致“预览-高采”相互切换过程中，roi区域显示异常
				reCalRoiRect(temp_roi_rect);
			if (mid) {
				moveRoi2Center(temp_roi_rect);
			}
		}
		m_device_roi.setRoiRect(temp_roi_rect);
		
		m_device_roi.update();
		break;
	}
	case Device::kIntelligentTriggerRoi:
	{
		m_intelligent_roi.setRoiType(roi_info.roi_type);
		m_intelligent_roi.setParentScene(m_graphics_scene);
		if (roi_info.roi_color.isValid())
		{
			m_intelligent_roi.setPainterColor(roi_info.roi_color);
		}
		m_intelligent_roi.setRoiRect(roi_info.roi_rect);
		m_intelligent_roi.update();
		break;
	}
	case Device::kAutoExposureRoi: {
		m_auto_exposure_roi.setRoiType(roi_info.roi_type);
		m_auto_exposure_roi.setParentScene(m_graphics_scene);
		if (roi_info.roi_color.isValid())
		{
			m_auto_exposure_roi.setPainterColor(roi_info.roi_color);
		}
		m_auto_exposure_roi.setRoiRect(roi_info.roi_rect);
		m_auto_exposure_roi.update();
		break;
	}
	case Device::kManualWhiteBalance: {
		m_white_balance_roi.setRoiType(roi_info.roi_type);
		m_white_balance_roi.setParentScene(m_graphics_scene);
		if (roi_info.roi_color.isValid())
		{
			m_white_balance_roi.setPainterColor(roi_info.roi_color);
		}
		m_white_balance_roi.setRoiRect(roi_info.roi_rect);
		m_white_balance_roi.update();
	}
	default:
		break;
	}
	m_graphics_scene->refreshScene(Device::RoiTypes::kUnknownRoi);
}

void CPlayerViewBase::setRoiVisible(Device::RoiTypes type, bool bVisible)
{
	switch (type)
	{
	case Device::kUnknownRoi:
		break;
	case Device::kDeviceRoi:
	{
		m_device_roi.setRoiVisible(bVisible);
		m_device_roi.update();
		break;
	}
	case Device::kAutoExposureRoi: {
		m_auto_exposure_roi.setRoiVisible(bVisible);
		m_auto_exposure_roi.update();
		break;
	}
	case Device::kIntelligentTriggerRoi:
	{
		m_intelligent_roi.setRoiVisible(bVisible);
		m_intelligent_roi.update();
		break;
	}
	case Device::kManualWhiteBalance:
	{
		m_white_balance_roi.setRoiVisible(bVisible);
		m_white_balance_roi.update();
		break;
	}
	default:
		break;
	}
}

void CPlayerViewBase::setFeaturePointVisible(Device::RoiTypes type, bool bVisible)
{
	if (bVisible) {
		if (type != Device::kDeviceRoi)				m_device_roi.setFeaturePointVisible(false);
		if (type != Device::kAutoExposureRoi)		m_auto_exposure_roi.setFeaturePointVisible(false);
		if (type != Device::kIntelligentTriggerRoi)	m_intelligent_roi.setFeaturePointVisible(false);
		if (type != Device::kManualWhiteBalance)	m_white_balance_roi.setFeaturePointVisible(false);
	}
	switch (type)
	{
	case Device::kUnknownRoi:
		break;
	case Device::kDeviceRoi:
	{
		m_roi_drawing = bVisible;
		m_device_roi.setFeaturePointVisible(bVisible);
		break;
	}
	case Device::kAutoExposureRoi: {
		m_auto_exposure_roi.setFeaturePointVisible(bVisible);
		break;
	}
	case Device::kIntelligentTriggerRoi:
	{
		m_intelligent_roi.setFeaturePointVisible(bVisible);
		break;
	}
	case Device::kManualWhiteBalance: {
		m_white_balance_roi.setFeaturePointVisible(bVisible);
		break;
	}
	default:
		break;
	}
	if (bVisible) {
		updateItemLayer(type);
		m_graphics_view->resetMousePress();
	}
	else {
		updateItemLayer(Device::kUnknownRoi);
	}
}

void CPlayerViewBase::setMeasureModeType(CMeasureLineManage::TMeasureModeType type)
{
	if (m_tMeasureModeTye != type)
	{
		CMeasureLineManage::instance().setMeasureModeType(m_strDeiveName, type);
	}
	else
	{
		updateMeasureModeType(type);
	}
}

void CPlayerViewBase::updateMeasureModeType(CMeasureLineManage::TMeasureModeType type)
{
	CMeasureLineManage::TMeasureModeType oldType = m_tMeasureModeTye;
	m_tMeasureModeTye = type;
	switch (m_tMeasureModeTye)
	{
	case CMeasureLineManage::MMT_Add:
	case CMeasureLineManage::MMT_Modify:
	{
		if (CMeasureLineManage::MMT_Add == m_tMeasureModeTye)
		{
			setCursor(QCursor(Qt::CrossCursor));
		}
		else
		{
			setCursor(QCursor(Qt::PointingHandCursor));
		}
		break;
	}
	default:
	{
		m_tAddMeasurePoint.curPoint = QPoint();
		m_tAddMeasurePoint.vctPoint.clear();
		m_tAddMeasurePoint.nMaxPointCount = CMeasureLineManage::kMaxPointCount;
		if (oldType == CMeasureLineManage::MMT_Add || oldType == CMeasureLineManage::MMT_Modify)
		{
			setCursor(QCursor(Qt::ArrowCursor));
		}
		break;
	}
	}

	m_measureItemManage.updateMeasureModeType(type);
}

void CPlayerViewBase::showBackgroundImage(bool is_white_balance)
{
	m_is_show_background_img = true;
	updateUI();
	if (m_graphics_view && m_graphics_scene) {
		m_graphics_scene->setWheelDisalble(m_is_show_background_img);
#if 1
		auto target_background_img = m_background_img;
		if (m_graphics_view->getRotate() != 0) {
			QMatrix matrix;
			matrix.rotate(-m_graphics_view->getRotate());
			target_background_img = target_background_img.transformed(matrix, Qt::FastTransformation);
		}
		m_graphics_scene->updateImage(target_background_img);
		m_graphics_scene->setSceneRect(QRectF(0, 0, target_background_img.width(), target_background_img.height()));
		updateViewSceneRect();
#endif // 1
#if 0
		m_graphics_scene->updateImage(m_background_img);
		if (is_white_balance)
			m_graphics_scene->setSceneRect(QRectF(0, 0, m_background_img.width()*0.7, m_background_img.height()*0.7));
		else
			updateLayout();
#endif // 0
		m_graphics_view->setWheelDisalble(m_is_show_background_img);
		
		clearMeasureItem();
	}
		
}

void CPlayerViewBase::resizeEvent(QResizeEvent *event)
{
	FitView(m_is_show_background_img); //使用 viewport resize event 事件更为准确
	AdjustViewBaseLabel(event->size());
	m_is_size_changed = true;
}

void CPlayerViewBase::mousePressEvent(QMouseEvent *event)
{
	if(m_is_show_background_img && this->m_strDeiveName.isEmpty()){
		return;
	}
	if (event->button() == Qt::LeftButton) {
		emit SignalMousePressPointF(m_mouse_press_point);
	}
	emit SignalClicked(m_win_index);
	QWidget::mousePressEvent(event);
}

void CPlayerViewBase::mouseDoubleClickEvent(QMouseEvent* e)
{
#if 0 // 相关执行逻辑放到事件过滤器中执行
	if (m_is_show_background_img && this->GetDeviceName().isEmpty()) {
		return QWidget::mouseDoubleClickEvent(e);
	}
	emit SignalDoubleClicked(m_win_index);
#endif // 0 // 相关执行逻辑放到事件过滤器中执行
	return QWidget::mouseDoubleClickEvent(e);

}

void CPlayerViewBase::wheelEvent(QWheelEvent *event) {
	if (this->m_is_show_background_img) {
		event->accept();
	}
	else
	{
		return QWidget::wheelEvent(event);
	}
}

void CPlayerViewBase::contextMenuEvent(QContextMenuEvent *event)
{
	if ((EXPORT_PREVIEW_PLAYER == m_win_index) && m_bPauseBtnClicked)
	{
		if (!m_current_image_backup.image.isNull()) {
			m_rightMenu->exec(event->globalPos());
		}
	}
	else
	{
		if (!m_strDeiveName.isEmpty())
		{
			if (m_tMeasureModeTye != CMeasureLineManage::MMT_Add && m_tMeasureModeTye != CMeasureLineManage::MMT_Modify)
			{
				m_rightMenu->exec(event->globalPos());
			}
			else
            {
                m_measureItemManage.slotReleassMouse();
				//m_tMeasureModeTye = CMeasureLineManage::MMT_Show;
				setMeasureModeType(CMeasureLineManage::MMT_Show);
			}
		}
	}
	return QWidget::contextMenuEvent(event);
}

void CPlayerViewBase::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.save();
	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewIpLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewIpLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewIpLabel]->text(),
		UIHelper::LT,
		m_locationOffset);

	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewConnectStatusLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewConnectStatusLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewConnectStatusLabel]->text(),
		UIHelper::RT,
		m_locationOffset);
	UIHelper::drawText(
		painter,
		m_mapViewLabel[ViewShowLabelType::ViewTimestampLabel]->font(),
		m_mapViewColor[ViewShowLabelType::ViewTimestampLabel],
		this->rect(),
		m_mapViewLabel[ViewShowLabelType::ViewTimestampLabel]->text(),
		UIHelper::LB,
		m_locationOffset);
	painter.restore();

	if (m_is_draw_border) {
		painter.save();
		QPen pen;
		pen.setColor(QColor("red"));
		pen.setWidth(1);
		painter.setPen(pen);
		painter.drawRect(0, 0, width() - 1, height() - 1);
		painter.restore();
	}
}

void CPlayerViewBase::leaveEvent(QEvent *event)
{
	if (m_is_show_background_img) {
		return;
	}
	cv::Mat dst;
	if (!m_current_image_backup.image.isNull()) {
		emit SignalUpdateGrayOrRGBValue(QPoint(-1, -1), dst);
	}
}

bool CPlayerViewBase::eventFilter(QObject * watched, QEvent * event)
{
	if (watched == m_graphics_view && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
		if (m_is_show_background_img && m_graphics_view) {
			FitView(m_is_show_background_img);
			//m_graphics_view->fitInView(this->m_background_img.rect(), Qt::KeepAspectRatio);
			//m_graphics_view->fitInView(this->m_background_img.rect(), Qt::KeepAspectRatio);
		}
	}else 	if (watched == m_graphics_scene && (event->type() == QEvent::GraphicsSceneMouseDoubleClick) ){
		if (!m_is_show_background_img && (!this->GetDeviceName().isEmpty())) {
			emit SignalDoubleClicked(m_win_index);
		}
	}
	else if (watched == m_graphics_scene && event->type() == QEvent::GraphicsSceneMousePress) {
		QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
		auto mouse_pos = mouseEvent->scenePos().toPoint();
		if (!m_auto_exposure_roi.contains(mouse_pos) && m_auto_exposure_roi.getFeaturePointVisible())
		{
			m_auto_exposure_roi.setFeaturePointVisible(false);
			emit SignalExitDrawRoi();
		}
		if (!m_intelligent_roi.contains(mouse_pos) && m_intelligent_roi.getFeaturePointVisible())
		{
			m_intelligent_roi.setFeaturePointVisible(false);
			emit SignalExitDrawRoi();
		}
	}
	return false;
}

void CPlayerViewBase::bindSignalAndSlot()
{
	bool ok = connect(m_graphics_scene, &BQGraphicsScene::updateRoiInfo, this, &CPlayerViewBase::updateRoiInfoSlot);
	ok = connect(m_graphics_scene, &BQGraphicsScene::imageRectSignal, this, &CPlayerViewBase::imageRectSlot/*, Qt::QueuedConnection*/);
	ok = connect(m_graphics_scene, &BQGraphicsScene::sendMousePtInSceneSignal, this, &CPlayerViewBase::sigCurrentCursorInVideo/*, Qt::QueuedConnection*/);
	ok = connect(m_graphics_scene, &BQGraphicsScene::sendMousePtInSceneSignal, this, &CPlayerViewBase::sendMousePtInSceneSlot/*, Qt::QueuedConnection*/);
	ok = connect(m_graphics_scene, &BQGraphicsScene::SignalCustomCrossLineCenterMovePoint, this, &CPlayerViewBase::SignalCustomCrossLineCenterMovePoint/*, Qt::QueuedConnection*/);
	ok = connect(m_graphics_scene, &BQGraphicsScene::SignalUpdateCustomCrossLineCenterPoint, this, &CPlayerViewBase::SignalUpdateCustomCrossLineCenterPoint/*, Qt::QueuedConnection*/);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalAddMeasureLine, this, &CPlayerViewBase::slotAddMeasureLine);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalUpdateMeasureLine, this, &CPlayerViewBase::slotUpdateMeasureLine);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalUpdateMeasureLines, this, &CPlayerViewBase::slotUpdateMeasureLines);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalDeleteMeasureLines, this, &CPlayerViewBase::slotDeleteMeasureLines);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalClearMeasureLines, this, &CPlayerViewBase::slotClearMeasureLines);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureModeType, this, &CPlayerViewBase::slotMeasureModeType);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasurePointOffset, this, &CPlayerViewBase::slotMeasurePointOffset);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureLineType, this, &CPlayerViewBase::SlotMeasureT2VUpdateLineType);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalGotoMeasureCal, this, &CPlayerViewBase::slotGotoMeasureCal);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalMeasureCalMode, this, &CPlayerViewBase::slotMeasureCalMode);
	ok = connect(&CMeasureLineManage::instance(), &CMeasureLineManage::signalCommonSignal, this, &CPlayerViewBase::slotCommonSignal);
	ok = connect(m_graphics_scene, &BQGraphicsScene::signalMousePress, this, [=] {emit SignalClicked(m_win_index); }/*, Qt::QueuedConnection*/);
	ok = connect(m_graphics_view, &BQGraphicsView::sigUpdateCoefficient, this, [=] {
#if 0
		QPolygonF view_rect_in_scene_vec = m_graphics_view->mapToScene(m_graphics_view->viewport()->rect());
		QRectF view_rect_in_scene = QRectF(view_rect_in_scene_vec[0], view_rect_in_scene_vec[2]);
		m_min_coefficient = qMin(abs(m_graphics_scene->sceneRect().width()*1.0 / view_rect_in_scene.width()), \
			abs(m_graphics_scene->sceneRect().height()*1.0 / view_rect_in_scene.height()));

		if (m_min_coefficient < 0.1) {
			m_min_coefficient = 0.1;
		}
		else if (m_min_coefficient > 16) {
			m_min_coefficient = 16;
		}
		emit sigZoomCoefficientChanged(m_min_coefficient);
		QMatrix mat = m_graphics_view->matrix();
		m_min_coefficient = qMax(abs(mat.m11()), abs(mat.m12()));
		emit sigZoomCoefficientChanged(m_min_coefficient);
#else
		emit sigZoomCoefficientChanged(GetZoomCoeff());
#endif
	}/*, Qt::QueuedConnection*/);
	
}

void CPlayerViewBase::init()
{
	QHBoxLayout *mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(1, 1, 1, 1);
	setStyleSheet("background-color:gray");
	m_graphics_view = new BQGraphicsView(this);
	m_graphics_view->installEventFilter(this);
	mainLayout->addWidget(m_graphics_view);
	m_graphics_scene = new BQGraphicsScene();
	//m_graphics_view->setViewport(new QOpenGLWidget());
	m_graphics_view->setScene(m_graphics_scene);
	m_graphics_scene->installEventFilter(this);
	m_graphics_scene->setBackgroundBrush(Qt::gray);

	m_graphics_scene->addItem(&m_device_roi);
	m_graphics_scene->addItem(&m_intelligent_roi);
	m_graphics_scene->addItem(&m_auto_exposure_roi);
	m_graphics_scene->addItem(&m_white_balance_roi);
}

void CPlayerViewBase::reCalRoiRect(QRect& rect)
{
	if (0 == m_original_roi_rect.width() || 0 == m_original_roi_rect.height()) {
		return;
	}
	if (rect.x() < m_original_roi_rect.x()) {
		rect.setLeft(m_original_roi_rect.x());
	}

	if (rect.y() < m_original_roi_rect.y()) {
		rect.setTop(m_original_roi_rect.y());
	}

	if ((rect.width() + rect.x()) > m_original_roi_rect.right()) {
		rect.setRight(m_original_roi_rect.right());
	}

	if ((rect.height() + rect.y()) > m_original_roi_rect.bottom()) {
		rect.setBottom(m_original_roi_rect.bottom());
	}
}

void CPlayerViewBase::moveRoi2Center(QRect& rect)
{
	qreal point_x = (m_graphics_scene->width() - rect.width()) / 2.0;
	qreal point_y = (m_graphics_scene->height() - rect.height()) / 2.0;
	qreal rect_width = rect.width();
	qreal rect_height = rect.height();
	rect.setRect(point_x, point_y, rect_width, rect_height);
}

void CPlayerViewBase::initMeasureLineInfo()
{
	clearMeasureItem();
	QList<CMeasureLineManage::TMeasureLineInfo> vctLine;
	CMeasureLineManage::instance().getMeasureLine(m_strDeiveName, vctLine);
	m_vctMeasureLine = vctLine;
	m_measureItemManage.initMeasureItem(vctLine);
}

void CPlayerViewBase::MouseRightClicked()
{
	m_rightMenu = new QMenu(this);
	QAction* actKeyFrame = new QAction(tr("Set Key Frame"), this);
	QAction* actBeginPos = new QAction(tr("Set Begin Position"), this);
	QAction* actEndPos = new QAction(tr("Set End Position"), this);

	connect(actKeyFrame, &QAction::triggered, this, [this] {

		emit SignalKeyFrame(m_current_image_backup.playback_info.frame_no);
	});
	connect(actBeginPos, &QAction::triggered, this, [this] {
		emit SignalBeginFrame(m_current_image_backup.playback_info.frame_no);
	});
	connect(actEndPos, &QAction::triggered, this, [this] {
		emit SignalEndFrame(m_current_image_backup.playback_info.frame_no);
	});

	m_rightMenu->addAction(actKeyFrame);
	m_rightMenu->addAction(actBeginPos);
	m_rightMenu->addAction(actEndPos);
}

void CPlayerViewBase::UpdateImgPixelColor(QImage &img, QImage& overExploreImg)
{
	//解决“断电重连，偶现相机窗口页面没有刷新过来”；原因：意外断开时，overExploreImg保留之前的过曝图片，
	//而没刷为最新的空图片
	int explore_value = 255;
	overExploreImg = img;

	cv::Mat dst;
	cv::Mat dst_bgra;
	QImage2CvMat(img, dst, true);
	if (dst.empty())
	{
		return;
	}
	int x = dst.channels();
	if (dst.channels() == 1)
	{
		cv::cvtColor(dst, dst_bgra, cv::COLOR_GRAY2BGRA);
	}
	else if (dst.channels() == 3)
	{
		cv::cvtColor(dst, dst_bgra, cv::COLOR_BGR2BGRA);
	}
	else
	{
		dst_bgra = dst;
	}

	for (int i = 0; i < dst_bgra.rows; ++i)
	{
		for (int j = 0; j < dst_bgra.cols; ++j)
		{
			if (dst_bgra.depth() == CV_16U)
			{
				cv::Vec4w vec = dst_bgra.at<cv::Vec4w>(i, j);
				if ((vec[0] >= explore_value) && (vec[1] >= explore_value) && \
					(vec[2] >= explore_value))
				{
					vec[0] = 0;
					vec[1] = 0;
					vec[2] = 255;
					dst_bgra.at<cv::Vec4w>(i, j) = vec;
				}
			}
			else
			{
				int channels = dst_bgra.channels();
				int pos = (i*dst_bgra.cols + j) * channels;
				if (dst_bgra.data[pos] >= explore_value && dst_bgra.data[pos + 1] >= \
					explore_value && dst_bgra.data[pos + 2] >= explore_value)
				{
					dst_bgra.data[pos] = 0;
					dst_bgra.data[pos + 1] = 0;
					dst_bgra.data[pos + 2] = 255;
				}
			}
		}
	}
	cvMat2QImage(dst_bgra, overExploreImg);
}

void CPlayerViewBase::AdjustViewBaseLabel(const QSize& size) {
	for (auto item : m_mapViewLabel.toStdMap())
	{
		if (nullptr != item.second)
		{
			//更新QLabel样式
			UpdateQLabelStyle(item.first, item.second, GetLabelFontSize(size));

			switch (item.first)
			{
			case ViewIpLabel:
				item.second->move(m_locationOffset, m_locationOffset);
				break;
			case ViewConnectStatusLabel:
				if ((size.width() - item.second->width()) > 0)
				{
					item.second->move((size.width() - item.second->width() - m_locationOffset), m_locationOffset);
				}
				break;
			case ViewTimestampLabel:
				if ((size.height() - item.second->height()) > 0)
				{
					item.second->move(m_locationOffset, (size.height() - item.second->height() - m_locationOffset));
				}
				break;
			case ViewFrameLabel:
				item.second->move(0, 0);
				break;
			default:
				break;
			}
		}
	}

	if (m_overlayImageLabel)
	{
		if ((size.width() - m_overlayImageLabel->width()) > 0)
		{
			m_overlayImageLabel->move((size.width() - m_overlayImageLabel->width()), size.height() - m_overlayImageLabel->height());
		}
	}

	if (m_targetBtnClose)
	{
		if ((size.width() - m_targetBtnClose->width()) > 0)
		{
			m_targetBtnClose->move((size.width() - m_targetBtnClose->width() - m_locationOffset), m_locationOffset);
		}
	}
}

void CPlayerViewBase::UpdateQLabelStyle(const ViewShowLabelType& type, QLabel* label, int fontsize)
{
	if (nullptr != label)
	{
		if (ViewShowLabelType::ViewFrameLabel == type)
		{
			label->setStyleSheet(QString::fromUtf8("background-color:white;font-size:12px;font-weight:bold;color:black;"));
		}
		else
		{
			label->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:%1px;font-family:Microsoft YaHei;font-weight:bold;color:#FF0000;").arg(fontsize));
		}
		label->adjustSize();
	}
}

int CPlayerViewBase::GetLabelFontSize(const QSize& size)
{
	int fontSize = 0;

	if (m_viewBaseOriginalSize == size)
	{
		fontSize = 24;
	}
	else
	{
		double scalFactor = ((size.width() / double(m_viewBaseOriginalSize.width())) + (size.height() / double(m_viewBaseOriginalSize.height()))) / 2.0;
		fontSize = qRound(24 * scalFactor);
	}

	return fontSize;
}

void CPlayerViewBase::InitViewLabel()
{
	for (size_t i = 0; i < ViewLabel_Max_COUNT; i++)
	{
		QLabel *pLabel = new QLabel(this);
		if (nullptr != pLabel)
		{
			pLabel->setVisible(false);
			if (ViewFrameLabel == i)
			{
				pLabel->setStyleSheet(QString::fromUtf8("background-color:white;font-size:12px;font-weight:bold;color:black;"));
				pLabel->setVisible(false);
			}
			else
			{
				pLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:24px;font-family:Microsoft YaHei;font-weight:bold;color:#FF0000;"));
			}

			m_mapViewLabel.insert(ViewShowLabelType(i), pLabel);
		}
	}
}

void CPlayerViewBase::updateUI()
{
	if (m_is_turn_to_display) {//切换到播放状态时，图像自适应窗口大小
		FitView();
		m_is_turn_to_display = false;
	}

// 	if (m_mapViewLabel.size() >= ViewShowLabelType::ViewLabel_Max_COUNT) {
// 		m_mapViewLabel[ViewShowLabelType::ViewTimestampLabel]->setVisible(!m_is_show_background_img);
// 	}
	m_graphics_scene->setCrossLineVisible(!m_is_show_background_img&m_cross_line_visible);
	m_graphics_scene->setCenterLineVisible(!m_is_show_background_img&m_center_line_visible);
	m_device_roi.setVisible(!m_is_show_background_img/*&!m_is_acquring*/);
	m_intelligent_roi.setVisible(!m_is_show_background_img);
	m_auto_exposure_roi.setVisible(!m_is_show_background_img);
}

bool CPlayerViewBase::isDrawCenterRect(const QPointF& pt, const QRectF& rt)
{
	if ((pt.x() > rt.x()) && (pt.x() < rt.x() + rt.width()) && (pt.y() > rt.y()) && (pt.y() < rt.y() + rt.height())) {
		return true;
	}
	else {
		return false;
	}
}

bool CPlayerViewBase::isDrawCenterHLine(const QPointF& pt, const QRectF& rt)
{
	if ((pt.y() > rt.y()) && (pt.y() < rt.y() + rt.height())) {
		return true;
	}
	else {
		return false;
	}
}

bool CPlayerViewBase::isDrawCenterVLine(const QPointF& pt, const QRectF& rt)
{
	if ((pt.x() > rt.x()) && (pt.x() < rt.x() + rt.width())) {
		return true;
	}
	else {
		return false;
	}
}

void CPlayerViewBase::InitOverlayUI() {
	m_overlayImageLabel = new QLabel(this);
	m_overlayImageLabel->hide();
	m_fallPosNameLabel = new QLabel(m_overlayImageLabel);
	m_fallPosValueLabel = new QLabel(m_overlayImageLabel);
	m_distanceNameLabel = new QLabel(m_overlayImageLabel);
	m_distanceValueLabel = new QLabel(m_overlayImageLabel);

	m_targetBtnClose = new QToolButton(this);
	m_targetBtnClose->setStyleSheet("QToolButton{border:none;background-color:transparent;color:white;}");
	m_targetBtnClose->setIconSize(QSize(57, 55));
	m_targetBtnClose->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	m_targetBtnClose->setCursor(Qt::ArrowCursor);

	QIcon icon;
	icon.addFile(QStringLiteral(":/image/image/manual_select_close.png"), QSize(), QIcon::Normal, QIcon::Off);
	m_targetBtnClose->setIcon(icon);
	QFont font;
	font.setPixelSize(10);
	m_targetBtnClose->setFont(font);
	m_targetBtnClose->setText(tr("Close manual select"));
	bool ok = connect(m_targetBtnClose, &QToolButton::clicked, this, [=]() {
		setCursor(Qt::ArrowCursor);
		m_bManualSelected = false;
		m_targetBtnClose->hide();
		emit SignalCloseManualSelect();
	});
	Q_UNUSED(ok);
	m_targetBtnClose->hide();


	m_overlayImageLabel->setStyleSheet(QString::fromUtf8("background-color:white;color:black;"));
	m_fallPosNameLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:14px;color:black;"));
	m_fallPosValueLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:24px;font-weight:bold;color:black;"));
	m_distanceNameLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:14px;color:black;"));
	m_distanceValueLabel->setStyleSheet(QString::fromUtf8("background-color:transparent;font-size:24px;font-weight:bold;color:black;"));

	m_overlayImageLabel->setFixedSize(204, 138);
	m_fallPosNameLabel->setText(tr("Fall Position Coordinate"));
	m_distanceNameLabel->setText(tr("Distance From Fall Position To Target"));

	m_overlayImageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_fallPosNameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_fallPosValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_distanceNameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_distanceValueLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QVBoxLayout* vPosLayout = new QVBoxLayout;
	vPosLayout->setContentsMargins(0, 0, 0, 0);
	vPosLayout->setSpacing(1);
	vPosLayout->addWidget(m_fallPosValueLabel);
	vPosLayout->addWidget(m_fallPosNameLabel);

	QVBoxLayout* vDisLayout = new QVBoxLayout;
	vDisLayout->setContentsMargins(0, 0, 0, 0);
	vDisLayout->setSpacing(1);
	vDisLayout->addWidget(m_distanceValueLabel);
	vDisLayout->addWidget(m_distanceNameLabel);

	QVBoxLayout* vMainLayout = new QVBoxLayout(m_overlayImageLabel);
	m_overlayImageLabel->setLayout(vMainLayout);
	vMainLayout->setContentsMargins(0, 3, 0, 3);
	vMainLayout->setSpacing(6);
	vMainLayout->addLayout(vPosLayout);
	vMainLayout->addLayout(vDisLayout);
}

#if 0
void CPlayerViewBase::updateLayout()
{
	//qreal coef = 1.0 / m_split_type;
	qreal coef = 1.0;
	if (m_is_show_background_img) {
		m_graphics_scene->setBgCoef(coef);
		m_graphics_scene->setSceneRect(QRectF(0, 0, m_background_img.width()*coef, m_background_img.height()*coef));
	}
}
#endif // 0
void CPlayerViewBase::updateViewSceneRect()
{
	if (m_graphics_scene && m_graphics_view) {
		auto scene_rect = m_graphics_scene->sceneRect();
		scene_rect.adjust(-32768, -32768, 32768, 32768);// 从1024修改到32768， 因为用户体验任意拖拽效果不佳
		m_graphics_view->setSceneRect(scene_rect);
	}
}

void CPlayerViewBase::initMeasureItem()
{
	m_graphics_scene->addItem(&m_measureItemManage);
	connect(m_graphics_scene, &BQGraphicsScene::signalPressOnePt, &m_measureItemManage, &CSMeasureItemManage::slotPressOnePt, Qt::QueuedConnection);
	connect(m_graphics_scene, &BQGraphicsScene::signalMoveOnePt, &m_measureItemManage, &CSMeasureItemManage::slotMoveOnePt, Qt::QueuedConnection);
	connect(m_graphics_scene, &BQGraphicsScene::signalReleassMouse, &m_measureItemManage, &CSMeasureItemManage::slotReleassMouse, Qt::QueuedConnection);
	connect(&m_measureItemManage, &CSMeasureItemManage::signalAddItemForPoint, this, &CPlayerViewBase::slotAddItemForPoint);
	connect(&m_measureItemManage, &CSMeasureItemManage::signalItemPointsMove, this, &CPlayerViewBase::slotItemPointsMove);
	connect(&m_measureItemManage, &CSMeasureItemManage::signalMeasureItemModified, this, &CPlayerViewBase::slotMeasureItemModified);
	connect(&m_measureItemManage, &CSMeasureItemManage::signalMeasureItemClicked, this, &CPlayerViewBase::slotMeasureItemClicked);
}

void CPlayerViewBase::updateMeaureInfo(const CMeasureLineManage::TMeasureLineInfo& info)
{
}

void CPlayerViewBase::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		act_close->setText(tr("Close"));
		act_close_all->setText(tr("Close All"));
	}

	QWidget::changeEvent(event);
}

void CPlayerViewBase::slotAddItemForPoint(QList<QPoint> vctPoint)
{
	bool bAdd = false;
	bool bMoreAdd = false;
	int nCount = vctPoint.size();
	if (m_tMeasureLineTye == CMeasureLineManage::MLT_TWO_CALIBRATION)
	{
		double dbValue = 0.0;
		QList<QPoint> vctPoint = m_measureItemManage.getCalibrationPoints();
		dbValue = CMeasureLineManage::PointsDistance(vctPoint);
		CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE, QVariant::fromValue(dbValue));
		CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS_SIZE, QVariant::fromValue(vctPoint.size()));
		CMeasureLineManage::instance().setMeasureCalMode(m_strDeiveName, CMeasureLineManage::MMT_Show);
		return;
	}
	if (nCount > 0)
	{
		switch (m_tMeasureLineTye)
		{
		case CMeasureLineManage::MLT_TWO_POINT:
		case CMeasureLineManage::MLT_MORE_POINT:
			if (nCount > 1)
			{
				bAdd = true;
				if (m_tMeasureLineTye == CMeasureLineManage::MLT_MORE_POINT)
				{
					bMoreAdd = true;
				}
			}
			break;
		case CMeasureLineManage::MLT_TWO_CALIBRATION:
		{
			double dbValue = 0.0;
			QList<QPoint> vctPoint = m_measureItemManage.getCalibrationPoints();
			dbValue = CMeasureLineManage::PointsDistance(vctPoint);
			CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE, QVariant::fromValue(dbValue));
			CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_POINTS_SIZE, QVariant::fromValue(vctPoint.size()));
			CMeasureLineManage::instance().setMeasureCalMode(m_strDeiveName, CMeasureLineManage::MMT_Show);
			break;
		}
		case CMeasureLineManage::MLT_ANGLE_3:
			if (nCount > 2)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_ANGLE_4:
			if (nCount > 3)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_RADIUS:
			if (nCount > 2)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_DIAMETER:
			if (nCount > 2)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_CENTER_DISTANCE:
			if (nCount > 5)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_DIMENSION:
			if (nCount > 0)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_AREA_CENTER:
			if (nCount > 2)
			{
				bAdd = true;
			}
			break;
		case CMeasureLineManage::MLT_AREA_POLYGON:
			if (nCount > 2)
			{
				bAdd = true;
				bMoreAdd = true;
			}
			break;
		default:
			break;
		}
		if (bAdd)
		{
			CMeasureLineManage::TMeasureLineInfo info;
			info.nIndex = CMeasureLineManage::instance().getFreeMeasureLineIndex(m_strDeiveName);
			for (auto tmp : vctPoint)
			{
				info.vctPoint.append(tmp + m_PointOffset);
			}
			info.nType = m_tMeasureLineTye;
			info.strName = QString::number(info.nIndex + 1);
			info.bChecked = false;
			CMeasureLineManage::instance().addMeasureLine(m_strDeiveName, info);
		}
	}
	// 多点距离是通过右键结束绘制的，所以在此时不能修改类型为show，不然右键会被当作其他用处
	if (!bMoreAdd)
	{
		setMeasureModeType(CMeasureLineManage::MMT_Show);
	}
}

void CPlayerViewBase::slotItemPointsMove(QList<QPoint> vctPoint)
{
	double dbValue = 0.0;
	dbValue = CMeasureLineManage::PointsDistance(vctPoint);
	CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_CALIBRATION_VALUE, QVariant::fromValue(dbValue));
}

void CPlayerViewBase::slotMeasureItemModified(const CMeasureLineManage::TMeasureLineInfo& ptInfo)
{
	if (ptInfo.nIndex >= 0)
	{
		CMeasureLineManage::instance().updateOneMeasureLine(m_strDeiveName, ptInfo);
	}
}

void CPlayerViewBase::slotMeasureItemClicked(const int nIndex)
{
	CMeasureLineManage::instance().emitCommonSignal(m_strDeiveName, CMeasureLineManage::MCST_UPDATE_ITEM_SELECTED, QVariant::fromValue(nIndex));
}
void CPlayerViewBase::clearMeasureItem()
{
	if (m_graphics_scene)
	{
		QList<QGraphicsItem *> vct = m_measureItemManage.childItems();
		for (auto item : vct)
		{
			m_graphics_scene->removeItem(item);
		}
		m_measureItemManage.clearMeasureItem();
	}
}
