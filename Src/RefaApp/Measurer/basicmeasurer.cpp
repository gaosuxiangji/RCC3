#include "basicmeasurer.h"
#include "ProjectController.h"
#include "ControllerManager.h"
//#include "CalibrationDataManagerInterface.h"
#include "CalibrationController.h"
#include "ReconstructAlgorithm.h"

QString BasicMeasurer::calibrationFilePath;

BasicMeasurer::BasicMeasurer()
{

}

bool BasicMeasurer::CalibrationParamExists()
{
	return ControllerManager::GetController<ProjectController>()->Calibrated();
}

void BasicMeasurer::RemoveParam()
{
	ControllerManager::GetController<CalibrationController>()->RemoveCalibrationParam();
}

const BaseCalibrationParam* BasicMeasurer::GetCalibrationParams()
{
	return ControllerManager::GetController<CalibrationController>()->GetCalibrationParams();
}

bool BasicMeasurer::InsertCalibrationParams(const BaseCalibrationParam* const param)
{
	return ControllerManager::GetController<CalibrationController>()->InsertCalibrationParams(param);
}

quint64 BasicMeasurer::load(const QString& filePath)
{
	auto res=ControllerManager::GetController<CalibrationController>()->ImportCalibrationParams(filePath);
	if (0 == res)
	{
		calibrationFilePath = filePath;
	}

	return res;
}

quint64 BasicMeasurer::save(const QString& filePath)
{
	auto res=ControllerManager::GetController<CalibrationController>()->ExportCalibrationParams(filePath);
	if (0 == res)
	{
		calibrationFilePath = filePath;
	}

	return res;
}

QString BasicMeasurer::GetCalibrationFilePath()
{
	return calibrationFilePath;
}

QString BasicMeasurer::mapToActual(const QPoint &pos)
{
	QList<QPoint> temp({ pos });

	if (!CalibrationParamExists())
	{
		return TansformeMeasureToQString(temp);
	}

	QPoint dst = TansformPixelToActual(pos);
	temp.push_back(dst);

	return TansformeMeasureToQString(temp);
}

QString BasicMeasurer::actualDistance(const QPoint &p1, const QPoint &p2)
{
	float dist_px = std::sqrt(std::pow(p1.x() - p2.x(), 2) + std::pow(p1.y() - p2.y(), 2));
	QList<float> temp({ dist_px });

	if (!CalibrationParamExists())
	{
		return TansformeMeasureToQString(temp);
	}

	QPoint pos_actual_1 = TansformPixelToActual(p1);
	QPoint pos_actual_2 = TansformPixelToActual(p2);

	float dist_actual = std::sqrt(std::pow(pos_actual_1.x() - pos_actual_2.x(), 2) + std::pow(pos_actual_1.y() - pos_actual_2.y(), 2));
	temp.push_back(dist_actual);

	return TansformeMeasureToQString(temp);
}

QPoint BasicMeasurer::TansformPixelToActual(const QPoint& pos)
{
	auto param = ControllerManager::GetController<CalibrationController>()->GetCalibrationParams();
	//QPoint temp(pos.x(), param->imageSize.height - pos.y());
	//const cv::Mat& homographyMatrix = param->homographyMatrix_camera_2_plane;
	cv::Point2f dst;
	//cv::perspectiveTransform(std::vector<cv::Point2f>(1, cv::Point2f(pos.x(), pos.y())), dst, homographyMatrix);
	ReconstructAlgorithm::Reconstruct(cv::Point2f(pos.x(), pos.y()), param, dst);

	return QPoint(dst.x, dst.y);
}

QString BasicMeasurer::TansformeMeasureToQString(const QList<QPoint>& measure)
{
	QString temp = QString("(%1Px,%2Px)").arg(QString::number(measure.front().x(),'f',3), QString::number(measure.front().y(),'f',3));

	if (2 == measure.size())
	{
		temp.append('\n').append(QString("(%1mm,%2mm)").arg(QString::number(measure.back().x(),'f',3), QString::number(measure.back().y(),'f',3)));
	}

	return temp;
}

QString BasicMeasurer::TansformeMeasureToQString(const QList<float>& measure)
{
	QString temp = QString("%1Px").arg(QString::number(measure.front(),'f',3));

	if (2 == measure.size())
	{
		temp.append(", ").append(QString("%1mm").arg(QString::number(measure.back(),'f',3)));
	}

	return temp;
}