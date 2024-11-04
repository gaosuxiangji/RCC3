#include "localvideoexportpreprocessdelegate.h"
#include "RMAGlobalFunc.h"
#include "Video/Export/exportutils.h"
#include <QImage>
#include <QPainter>
#include <QStaticText>

LocalVideoExportPreprocessDelegate::LocalVideoExportPreprocessDelegate(std::shared_ptr<SingleVideoExportParam> param, QObject *parent /*= Q_NULLPTR*/)
	: SingleVideoExportPreprocessDelegate(param, parent)
{
}

bool LocalVideoExportPreprocessDelegate::GetFrame(FRAME_INDEX frameIndex, RMAImage& image) const
{
	if (!SingleVideoExportPreprocessDelegate::GetFrame(frameIndex, image))
	{
		return false;
	}
	if (image.Empty())
	{
		return false;
	}

	RMAImage img_tmp;
	img_tmp.CopyProperty(image);//ª∫¥ÊÕºœÒµƒ Ù–‘

	const QRect img_rc(0, 0, image.GetWidth(), image.GetHeight());
	if (img_rc != extra_params_.roi)
	{
		if (!img_rc.contains(extra_params_.roi))
		{
			return false;
		}
		cv::Mat mat;
		RMAIMAGE2CVMAT(image, mat);

		cv::Rect roi(extra_params_.roi.x(), extra_params_.roi.y(), extra_params_.roi.width(), extra_params_.roi.height());
		mat = mat(roi).clone();

		image = RMAImage(mat.cols, mat.rows, image.GetPixelFormat(), mat.data);
		image.CopyProperty(img_tmp);//…Ë÷√ÕºœÒ Ù–‘
	}

	if (extra_params_.benabled_watermark)
	{
		paintOSD(image);
		image.CopyProperty(img_tmp);//…Ë÷√ÕºœÒ Ù–‘
	}

	return true;
}

void LocalVideoExportPreprocessDelegate::paintOSD(RMAImage& image) const
{
	if (image.Empty())
	{
		return;
	}

	auto osd = getOsdInfo(image);
	if (osd.isEmpty())
	{
		return;
	}

	QImage img;
	RMAIMAGE2QIMAGE(image, img);
	if (image.GetPixelFormat() == PIXEL_FMT_BGR24)
	{
		img = img.rgbSwapped().copy();
	}

	QString text = osd.join(QString("<br/>"));
	if (text.isEmpty())
		return;
	text = QString("<font color = #FFFFFF>%1</font>").arg(text);

	ExportUtils::paintWatermark2Image(img, text, kWatermarkPadding);

	if (image.GetPixelFormat() == PIXEL_FMT_BGR24)
	{
		img = img.rgbSwapped();
	}
	image = RMAImage(image.GetWidth(), image.GetHeight(), image.GetPixelFormat(), img.bits());
}

QStringList LocalVideoExportPreprocessDelegate::getOsdInfo(const RMAImage& image) const
{
	if (image.Empty())
		return QStringList();
	QStringList osd;
	QString frame_no_text = QString::number(image.GetAbsoluteFrameIndex());

	if (image.GetPixelFormat() == PIXEL_FMT_BGR24)
	{
		frame_no_text = QString("<font color = #FF0000>%1</font>").arg(image.GetAbsoluteFrameIndex());
	}
	osd << QString("%1:%2").arg(tr("Name"), extra_params_.video_name)
		<< QString("%1:%2").arg(tr("Video Frame No")).arg(QString(tr("Frame No %1")).arg(frame_no_text))
		<< QString("%1:%2").arg(tr("Timestamp")).arg(image.GetTimestamp())
		;
	return qMove(osd);
}

void LocalVideoExportPreprocessDelegate::setExtraParams(const LocalExtraParams & params)
{
	extra_params_ = params;
}
