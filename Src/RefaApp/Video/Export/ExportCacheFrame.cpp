#include "ExportCacheFrame.h"
#include "Device/imageprocessor.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include "opencv2/imgproc.hpp"
#include <thread>
#include <QStaticText>
#include <QPainter>
#include "HscAPIHeader.h"
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
ExportCacheFrame::ExportCacheFrame()
{

}

ExportCacheFrame::ExportCacheFrame(QSharedPointer<Device> device_ptr, int video_num)
	:
	device_ptr_(device_ptr)
{
	m_running = true;
	m_export_thrd = std::thread{ &ExportCacheFrame::doExportProcess,this };
}

ExportCacheFrame::~ExportCacheFrame()
{
	m_running = false;
	//m_condvar.notify_one();
	if (m_export_thrd.joinable()) m_export_thrd.join();
}

bool ExportCacheFrame::doExportProcess()
{
	while (m_running)
	{
		std::unique_lock<std::mutex> lk(m_export_vec_mtx);
		//m_condvar.wait(lk, [this] { return (!m_running || (m_crtidx <m_export_info_vec.size())); });
		if (!m_running)
		{
			lk.unlock();
			break;
		}
		if (m_crtidx >= m_export_info_vec.size())
		{
			lk.unlock();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
			continue;
		}	
		auto export_info = m_export_info_vec[m_crtidx];
		lk.unlock();
		
		doExport(export_info.m_video_id, export_info.m_start_frame_no, export_info.m_end_frame_no, export_info.m_count, export_info.m_interval);
		m_crtidx++;
	}

	return true;
}

void ExportCacheFrame::doExportAsync(int video_id, int64_t start_frame_no, int64_t end_frame_no, int64_t count, int64_t interval)
{
	std::lock_guard<std::mutex> lk(m_export_vec_mtx);
	m_export_info_vec.push_back(ExportInfo(video_id, start_frame_no, end_frame_no, count, interval));
	//if (m_crtidx < 0) m_crtidx = 0;
	m_videoid_to_crtidx[video_id] = m_export_info_vec.size()-1;
	//m_condvar.notify_one();
}

bool ExportCacheFrame::doExport(int video_id, int64_t start_frame_no, int64_t end_frame_no, int64_t count, int64_t interval)
{
	m_exporting = true;
	HscResult res = device_ptr_->startExport(video_id);
	if (res != HSC_OK || !m_running) {
		m_exporting = false;
		return false;
	}

	res = device_ptr_->exportByInterval(video_id, start_frame_no, end_frame_no, count, interval);
	if (res != HSC_OK || !m_running) {
		m_exporting = false;
		return false;
	}

	std::shared_ptr<CAGBuffer> pBuffer(new CAGBuffer);
	auto start_time = std::chrono::high_resolution_clock::now();
	while (1)
	{
		if (!m_running)
		{
			m_exporting = false;
			break;
		}

		bool rslt = device_ptr_->getPlaybackFrame(*pBuffer.get());
		if (rslt)
		{
			std::lock_guard<std::mutex> lk(m_export_map_mtx);
			m_cache_frame_map[video_id] = pBuffer;
			break;
		}
		boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		auto current_time = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() >= eRecordingTimeOutMs) break;
	}

	res = device_ptr_->stopExport();
	m_exporting = false;
	if (res != HSC_OK) return false;
	
	return true;
}


bool ExportCacheFrame::getFrame(int64_t vid, VideoItem &item, cv::Mat &mat)
{
	CAGBuffer *pBuffer = nullptr;
	bool rslt = getFrame(vid, pBuffer);
	if (!rslt || !pBuffer)
	{
		int w = item.getRoi().width();
		int h = item.getRoi().height();
		int RotationType = item.getRotationType();
		if (RotationType == HSC_ROTATION_CW90 || RotationType == HSC_ROTATION_CW270)
		{
			int temp = w;
			w = h;
			h = temp;
		}
		mat = cv::Mat(h, w, CV_8UC1, cv::Scalar(0,0,0));

		int8_t water_mark_enable = (int8_t)item.getProperty(VideoItem::PropType::OsdVisible).toInt();
		if ((water_mark_enable & 0x01) == 0x01) 
		{
			QString record_time = item.getTimeStamp();
			paintDefWatermark(mat, record_time, 0, w, h, mat.channels(), item);
		}
		return true;
	}

	auto contrast = item.getContrast();
	int luminance = item.getLuminance();
	bool anti_color = item.isAntiColorEnable();
	adjustLuminanceAndContrast(pBuffer, luminance, contrast, anti_color, mat);
	int8_t water_mark_enable = item.getProperty(VideoItem::PropType::OsdVisible).toInt();
	if ((water_mark_enable & 0x01) == 0x01) 
	{ 
		QString timestamp = DeviceUtils::formatTimestamp(pBuffer->frame_head.time_stamp);
		paintDefWatermark(mat, timestamp, item.getBeginFrameIndex(), pBuffer->frame_head.rect.width, pBuffer->frame_head.rect.height, mat.channels(), item);
	}
	return true;
}

bool ExportCacheFrame::getFrame(int64_t video_id, CAGBuffer *&pBuffer)
{
	std::unique_lock<std::mutex> mlk(m_export_map_mtx);
	auto cache_frame_itr = m_cache_frame_map.find(video_id);
	if (cache_frame_itr != m_cache_frame_map.end())
	{
		pBuffer = cache_frame_itr->second.get();
		mlk.unlock();
		return true;
	}
	mlk.unlock();

	std::unique_lock<std::mutex> lk(m_export_vec_mtx);
	auto itr = m_videoid_to_crtidx.find(video_id);
	if (itr == m_videoid_to_crtidx.end())
	{
		lk.unlock();
		return false;
	}
	int idx = itr->second;
	if (idx < m_crtidx) {
		if (idx >= m_export_info_vec.size()) return false;
		auto &info = m_export_info_vec[idx];
		lk.unlock();

		bool rslt = doExport(info.m_video_id, info.m_start_frame_no, info.m_end_frame_no, info.m_count, info.m_interval);
		return rslt;
	}

	if (idx > m_crtidx + 1)
	{	
		if (idx >= m_export_info_vec.size()) 
		{
			lk.unlock();
			return false;
		}

		auto tmp = m_export_info_vec[idx];
		m_export_info_vec[idx] = m_export_info_vec[m_crtidx + 1];
		m_export_info_vec[m_crtidx + 1] = tmp;
		m_videoid_to_crtidx[tmp.m_video_id] = m_crtidx + 1;
		m_videoid_to_crtidx[m_export_info_vec[idx].m_video_id] = idx;
	}
	lk.unlock();
	auto start_time = std::chrono::high_resolution_clock::now();
	while (1)
	{
		std::unique_lock<std::mutex> lk(m_export_map_mtx);
		auto itr = m_cache_frame_map.find(video_id);
		if (itr != m_cache_frame_map.end())
		{
			pBuffer = itr->second.get();
			lk.unlock();
			return true;
		}
		else lk.unlock();

		auto current_time = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count() >= eGetFrameTimeOutMs) {
			return false;
		}
		boost::this_thread::sleep_for(boost::chrono::microseconds(1));
	}
	return true;
}


void ExportCacheFrame::adjustLuminanceAndContrast(CAGBuffer *buffer, const int luminance, const int contrast, const bool anti_color, cv::Mat &des_mat)
{
	auto processor_ptr = device_ptr_->getProcessor();
	if (processor_ptr)
	{
		des_mat = processor_ptr->cv_process(buffer, 1, true);
		des_mat = processor_ptr->cv_process(des_mat, contrast, luminance, anti_color);
	}
}


void ExportCacheFrame::paintDefWatermark(cv::Mat &des_mat, QString &timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, const VideoItem &item)
{
	QString text = getDefWaterMarkText(timestamp, frame_no, width, height, channel, item);
	auto text_list = text.split("\\n");
	double ScalarFont = 0.7;

	int baseline;
	cv::Size text_size;
	if (text_list.size() >= 1) {
		text_size = cv::getTextSize(text_list[0].toStdString(), cv::FONT_HERSHEY_SIMPLEX, ScalarFont, 1, &baseline);
	}
	else {
		text_size = cv::getTextSize(text.toStdString(), cv::FONT_HERSHEY_SIMPLEX, ScalarFont, 1, &baseline);
	}

	int y0 = text_size.height + 5;
	int x = 5;
	int y = des_mat.rows - text_list.size()*y0 + 5;

	double scalar_font = 0.35;
	int i = 0;
	int thick = 1;
	int white_value = ((des_mat.depth() == CV_8U) ? 255 : 65535);
	for (auto elem : text_list)
	{
		//先绘制底层黑色阴影,偏移(1,1)
		cv::putText(des_mat, elem.toStdString(), cv::Point(x + 1, y + 1+ y0*i), cv::FONT_HERSHEY_SIMPLEX, scalar_font, cv::Scalar(0, 0, 0), thick, cv::LINE_8);
		cv::putText(des_mat, elem.toStdString(), cv::Point(x, y + y0*i++), cv::FONT_HERSHEY_SIMPLEX, scalar_font, cv::Scalar(white_value, white_value, white_value), thick, cv::LINE_8);
	}
	std::string ss = std::to_string(frame_no);
	return;

	/*QImage watermark;
	convertTextToImage(text, watermark);
	if (!des_mat.data || !timestamp || width == 0 || height == 0 || channel == 0) return;

	x = 5;
	y = des_mat.rows - watermark.height() - 5;
	addWaterMark(x, y, 1.0, 0.3, 0, des_mat, watermark);*/
}


bool ExportCacheFrame::addWaterMark(int x, int y, double alpha, double beta, double gamma, cv::Mat &dst_mat, QImage &image)
{
	cv::Mat mat_watermark;			//需要叠加的水印图像,通常为rgba32
	cv::Mat mat_dst_rgba_img;		//转换为rgba格式的目标图像
	cv::Mat mat_dst_rgba_img_clipped;	//裁切后的rgba图像

	CPlayerViewBase::QImage2CvMat(image, mat_watermark);

	try
	{
		//////////////////////////////////////////////////////////////////////////
		//目标图像转换为rgba
		if (dst_mat.type() != CV_8UC4)
		{
			if (dst_mat.type() == CV_8UC1) {
				cv::cvtColor(dst_mat, mat_dst_rgba_img, cv::COLOR_GRAY2BGRA);
			}
			if (dst_mat.type() == CV_8UC3) {
				cv::cvtColor(dst_mat, mat_dst_rgba_img, cv::COLOR_BGR2BGRA);
			}
		}

		//水印图像转rgba
		if (mat_watermark.type() != CV_8UC4)
		{
			if (mat_watermark.type() == CV_8UC1) {
				cv::cvtColor(mat_watermark, mat_watermark, cv::COLOR_GRAY2BGRA);
			}
			if (mat_watermark.type() == CV_8UC3) {
				cv::cvtColor(mat_watermark, mat_watermark, cv::COLOR_BGR2BGRA);
			}
		}

		//获取水印图像区域
		auto rect = cv::Rect(x, y, mat_watermark.cols, mat_watermark.rows);
		mat_dst_rgba_img_clipped = mat_dst_rgba_img(rect);
		
		//叠加水印到目标上(根据每个像素的alpha通道来计算,不可使用cv::addWeighted,逻辑不一样)
		for (int i = 0; i < mat_dst_rgba_img_clipped.rows; i++)
		{
			for (int j = 0; j < mat_dst_rgba_img_clipped.cols; j++)
			{
				double alpha = (double)mat_watermark.at<cv::Vec4b>(i, j)[3] / 255.0;

				mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[0] =
					mat_watermark.at<cv::Vec4b>(i, j)[0] * alpha +
					mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[0] * (1 - alpha);

				mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[1] =
					mat_watermark.at<cv::Vec4b>(i, j)[1] * alpha +
					mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[1] * (1 - alpha);

				mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[2] =
					mat_watermark.at<cv::Vec4b>(i, j)[2] * alpha +
					mat_dst_rgba_img_clipped.at<cv::Vec4b>(i, j)[2] * (1 - alpha);
			}
		}

		//叠加后的图像转回原图格式
		if (dst_mat.type() != CV_8UC4)
		{
			if (dst_mat.type() == CV_8UC1) {
				cv::cvtColor(mat_dst_rgba_img, dst_mat, cv::COLOR_BGRA2GRAY);
			}
			if (dst_mat.type() == CV_8UC3) {
				cv::cvtColor(mat_dst_rgba_img, dst_mat, cv::COLOR_BGRA2BGR);
			}
		}
		else
		{
			dst_mat = mat_dst_rgba_img;
		}

	}
	catch (cv::Exception &e)
	{
		auto err = e.what();
		std::cout << err << std::endl;
	}
	return true;
}

void ExportCacheFrame::updateWaterMark(int video_id, QImage &image)
{
	WaterMarkInfo info;
	info.image_watermark = image;
	m_watermark_map[video_id] = info;
}

void ExportCacheFrame::deleteWaterMark(int video_id)
{
	auto itr = m_watermark_map.find(video_id);
	if (itr != m_watermark_map.end()) m_watermark_map.erase(itr);
}

bool ExportCacheFrame::hasWaterMark(int video_id)
{
	auto itr = m_watermark_map.find(video_id);
	if (itr != m_watermark_map.end()) return true;
	return false;
}

void ExportCacheFrame::paintSelfDefWatermark(int video_id, cv::Mat &mat)
{
	auto itr = m_watermark_map.find(video_id);
	if (itr == m_watermark_map.end()) return;

	auto image_water_mark = itr->second;
	addWaterMark(image_water_mark.x, image_water_mark.y, image_water_mark.alpha, image_water_mark.beta, image_water_mark.gamma, mat, image_water_mark.image_watermark);
}

//QImage ExportCacheFrame::getDefWaterMarkImage(uint8_t * timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, VideoItem &video_itme)
//{
//	QString text = getDefWaterMarkText(timestamp, frame_no, width, height, channel, video_itme);
//	return convertTextToImage(text);
//}

QString ExportCacheFrame::getDefWaterMarkText(QString &timestamp, uint32_t frame_no, uint32_t width, uint32_t height, uint32_t channel, const VideoItem &video_itme)
{
	QString text;
	//老采集软件水印格式
	//帧编号/总帧数 宽x高@帧率fps 曝光时间us
	//时间戳

	QString frame_no_str = QString::number(frame_no + 1);
	if (3 == channel)
	{
		frame_no_str = QString("%1").arg(frame_no + 1);
	}
	else if (4 == channel)
	{
		frame_no_str = QString("%1").arg(frame_no + 1);
	}
	int count = video_itme.getEndFrameIndex() - video_itme.getBeginFrameIndex() + 1;
	qint64 nFrameStep = video_itme.getProperty(VideoItem::PropType::FrameStep).toInt();
	nFrameStep += 1;
	if (nFrameStep < 1)
	{
		nFrameStep = 1;
	}
	else if (nFrameStep > count)
	{
		nFrameStep = count;
	}
	if (nFrameStep > 1)
	{
		count = 1 + (count - 1) / nFrameStep;
	}
	QString strTotalFrame = QString::number(count);
	if (device_ptr_)
	{
		agile_device::capability::Units nUnit = device_ptr_->GetRealExposureTimeUnit();
		if (agile_device::capability::Units::kUnit100ns == nUnit)
		{
			float dbexposure_time = (float)video_itme.getExposureTime() / 10.0f;
			text.append(QString("%1/%2 %3@%4fps %5us")
				.arg(frame_no_str).arg(strTotalFrame).arg(video_itme.getResolution()).arg(video_itme.getFrameRate()).arg(QString::asprintf("%.2f", dbexposure_time)));
		}
		else if (agile_device::capability::Units::kUnit10ns == nUnit)
		{
			float dbexposure_time = (float)video_itme.getExposureTime() / 100.0f;
			text.append(QString("%1/%2 %3@%4fps %5us")
				.arg(frame_no_str).arg(strTotalFrame).arg(video_itme.getResolution()).arg(video_itme.getFrameRate()).arg(QString::asprintf("%.2f", dbexposure_time)));
		}
		else
		{
			float dbexposure_time = (float)video_itme.getExposureTime();
			if (device_ptr_->isGrabberDevice()) {//CXP相机能力集是微秒，需要除以10
				dbexposure_time = (float)video_itme.getExposureTime() / 10.0f;
			}
			text.append(QString("%1/%2 %3@%4fps %5us")
				.arg(frame_no_str).arg(strTotalFrame).arg(video_itme.getResolution()).arg(video_itme.getFrameRate()).arg(QString::asprintf("%.0f", dbexposure_time)));
		}
	}
	else
	{
		text.append(QString("%1/%2 %3@%4fps %5us")
			.arg(frame_no_str).arg(strTotalFrame).arg(video_itme.getResolution()).arg(video_itme.getFrameRate()).arg(video_itme.getExposureTime()));
	}
	text += "\\n";
	//text.append(QString("%1").arg(DeviceUtils::formatTimestamp(timestamp)));
	text.append(QString("%1").arg(timestamp));
	return text;
}


void ExportCacheFrame::convertTextToImage(const QString &text, QImage &image)
{
	QStaticText static_text(text);
	QSizeF static_text_rc = static_text.size();

	image = QImage(static_text_rc.width(), static_text_rc.height(), QImage::Format::Format_Grayscale8);
	image.fill(QColor(255));
	QPainter p;
	p.begin(&image);
	p.setPen(Qt::white);
	p.drawText(QRect(0, 0, static_text_rc.width(), static_text_rc.height()), text);
	p.end();
}


bool ExportCacheFrame::stop()
{
	return true;
	m_running = false;
	while (m_exporting)
	{
		boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
	}

	HscResult res = device_ptr_->stopExport();
	if (res != HSC_OK) return false;
	return true;
}