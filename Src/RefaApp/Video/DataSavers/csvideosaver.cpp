#include "csvideosaver.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include <QDir>
#include <QFileInfo>
#include "Video/DataSavers/BmpWriter.h"
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "Common/LogUtils/logutils.h"
#include "ImageNS/ImageNS.h"
#include "tiff.h"
#include "tiffio.h"
using namespace ImageNS;
CSVideoSaver::CSVideoSaver(const QString save_path, bool with_piv)
	:m_save_path(save_path),m_with_piv_flag(with_piv)
{

}

CSVideoSaver::~CSVideoSaver()
{

}

bool CSVideoSaver::SaveSnapshot( RccFrameInfo video_info, bool bAnti, uint8_t frame_header_type)
{
	if (video_info.image.isNull())
	{
		return false;
	}
	QString timestamp_str = DeviceUtils::formatFileTimestamp(video_info.timestamp, frame_header_type);
	QString image_path = m_save_path;
	QString tif_path;
	QString excel_path;
	if (image_path.isEmpty())
	{
		image_path = "./";
	}
	else
	{
		if (!image_path.endsWith('/'))
		{
			image_path.push_back('/');
		}
	}
	QString device_name = video_info.device_name;
	if (device_name.length() > 0)
	{
		image_path += device_name;
		image_path += '-';
	}
	image_path += video_info.ip_or_sn;
	image_path += '_';
	image_path += timestamp_str;
	tif_path = image_path + ".tif";
	excel_path = image_path + ".csv";
	image_path += ".bmp";
	QDir dir;
	CSLOG_INFO("save snapshot to path:{}", image_path.toStdString());
	if (dir.mkpath(QFileInfo(image_path).path()))
	{

		if (video_info.valid_bits != 8)//16bit数据存储对应的参数
		{
			SaveExcel(excel_path, video_info);
			SaveTiff(tif_path, video_info);//该处会改变raw_image
		}
		if (bAnti) {
			return video_info.image.save(image_path, "BMP");
		}
		else {
			return video_info.image.save(image_path, "BMP", 100);
		}
	}
	else
	{
		return false;
	}
}

unsigned long FourBytes2DWORD(const unsigned char bytes[4])
{
	unsigned long dw = 0;
	dw = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
	return dw;
}



bool CSVideoSaver::saveBmpWithExtendInfo(QString image_path,  RccFrameInfo & video_frame)
{
	if (video_frame.extend_info.device_model == DEVICE_XJ1310)
	{
		return saveBmpWithXJ1310ExtendInfo(image_path, video_frame);
	}
	BitmapExtendInfo extend_info;

	std::time_t video_clip_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto local_time = std::localtime(&video_clip_timestamp);
	if (!local_time)
	{
		return false;
	}

	local_time->tm_yday = ((static_cast<uint16_t>(video_frame.timestamp[0]) << 8) | (video_frame.timestamp[1]));
	if (local_time->tm_yday > 365)
		local_time->tm_yday = 0;

	std::time_t timestamp = std::mktime(local_time);

	local_time = std::localtime(&timestamp);
	if (!local_time)
		return false;
	extend_info.year = local_time->tm_year + 1900; // 自然年
	extend_info.month = local_time->tm_mon + 1;
	extend_info.day = local_time->tm_mday;

	uint32_t hour = video_frame.timestamp[2];
	uint32_t min = video_frame.timestamp[3];
	uint32_t sec = video_frame.timestamp[4];
	uint32_t usec = FourBytes2DWORD(&video_frame.timestamp[5]);
	extend_info.absolute_time = (hour * 60 * 60 + min * 60 + sec) * 1000 * 10 + usec / 100;


	extend_info.distance = video_frame.extend_info.distance;

	extend_info.exposure_time = video_frame.extend_info.exposure_time;

	std::string ascii_save_path =image_path.toLocal8Bit().data();

	cv::Mat temp_mat;
	CPlayerViewBase::QImage2CvMat(video_frame.image, temp_mat);
	
	int rows = temp_mat.rows;
	cv::Mat mat_image = cv::Mat(temp_mat, cv::Range(4, rows)); // 去除帧头行

	return CBmpWriter::write(ascii_save_path, mat_image, video_frame.extend_info.device_index, video_frame.extend_info.device_state, extend_info);
}

static uint16_t makeCameraState(const VideoExtendInfo & video_extend_info)
{
	uint8_t gain_value = pow(2, video_extend_info.gain);
	return (gain_value << 8) | (video_extend_info.exposure_mode << 4) | (video_extend_info.time_source << 3) | (video_extend_info.external_clock_status << 2) | video_extend_info.trigger_mode;
}

/**
* @brief 转换角度（XJ130），单位：360/pow(2,31)
* @param value 角度值
*/
static int32_t toXJ1310Angle(double value)
{
	return static_cast<int32_t>(value * 4);
}


bool CSVideoSaver::saveBmpWithXJ1310ExtendInfo(QString image_path,  RccFrameInfo & video_frame)
{
	XJ1310BitmapExtendInfo extend_info;

	// 拍摄起始时刻
	uint32_t hour = video_frame.timestamp[2];
	uint32_t min = video_frame.timestamp[3];
	uint32_t sec = video_frame.timestamp[4];
	uint32_t usec = FourBytes2DWORD(&video_frame.timestamp[5]);
	extend_info.timestamp = (hour * 60 * 60 + min * 60 + sec) * 1000 * 10 + usec / 100;

	// 设备状态
	extend_info.device_state = video_frame.extend_info.device_state; // RS422

																	 // 方位角
	extend_info.azimuth = toXJ1310Angle(video_frame.extend_info.azimuth); // RS422

																		  // 俯仰角
	extend_info.pitch = toXJ1310Angle(video_frame.extend_info.pitch); // RS422

																	  // 焦距
	extend_info.focal_length = video_frame.extend_info.focal_length; // RS422

																	 // 距离信息
	extend_info.distance = video_frame.extend_info.distance; // 固定值

															 // 帧频
	extend_info.frame_rate = video_frame.extend_info.frame_rate * 100; // 0.01fps

																	   // 曝光时间
	extend_info.exposure_time = video_frame.extend_info.exposure_time;

	// 相机状态
	extend_info.camera_state = makeCameraState(video_frame.extend_info);

	std::string ascii_save_path = image_path.toLocal8Bit().data();

	cv::Mat temp_mat;
	CPlayerViewBase::QImage2CvMat(video_frame.image, temp_mat);

	int rows = temp_mat.rows;
	cv::Mat mat_image = cv::Mat(temp_mat, cv::Range(video_frame.extend_info.frame_header_rows, rows)); // 去除帧头行

	uint16_t device_work_state = 0; // TODO: 待客户提供固定值
	return CBmpWriter::write(ascii_save_path, mat_image, video_frame.extend_info.device_index, device_work_state, extend_info);
}


bool CSVideoSaver::SaveExcel(QString excel_path, RccFrameInfo& frame)
{
	int channels = frame.raw_image.channels();

	cv::Mat bpp16_mat(frame.raw_image.rows, frame.raw_image.cols, CV_16UC(channels));

	bpp16_mat = frame.raw_image;

	int right_shift = 16 - frame.valid_bits;
	{
		std::string csvFile = excel_path.toLocal8Bit().data();
		// 		csvFile += ".csv";
		std::fstream oss;
		oss.open(csvFile, std::ios::out);
		if (oss.is_open())
		{
			for (int i = 0; i < bpp16_mat.rows; ++i)
			{
				std::string rowStr = "";
				for (int j = 0; j < bpp16_mat.cols; ++j)
				{
					std::string rgbStr = "";
					if (channels == 1)
					{
						uint16_t gray = (bpp16_mat.at<uint16_t>(i, j) >> right_shift);
						if (j == 0)
						{
							rgbStr.append(std::to_string(i + 1));
							rgbStr.append(",");
						}
						rgbStr.append(std::to_string(gray));
						if (rowStr.length() > 0) {
							rowStr += ",";
						}
					}
					else if (channels == 3)
					{
						uint16_t b = (bpp16_mat.at<cv::Vec3w>(i, j)[0] >> right_shift);
						uint16_t g = (bpp16_mat.at<cv::Vec3w>(i, j)[1] >> right_shift);
						uint16_t r = (bpp16_mat.at<cv::Vec3w>(i, j)[2] >> right_shift);
						if (j == 0)
						{
							rgbStr.append(std::to_string(i + 1));
							rgbStr.append(",");
						}
						rgbStr.append("(");
						rgbStr.append(std::to_string(r));
						rgbStr.append("|");
						rgbStr.append(std::to_string(g));
						rgbStr.append("|");
						rgbStr.append(std::to_string(b));
						rgbStr.append(")");
						if (rowStr.length() > 0) {
							rowStr += ",";
						}

					}
					rowStr += rgbStr;
				}
				oss << rowStr << std::endl;
			}
			oss.close();
			return true;
		}
	}
	return false;
}

bool CSVideoSaver::SaveTiff(QString path, RccFrameInfo& frame)
{
	if (frame.valid_bits > 8 && frame.valid_bits <= 16)
	{
		int height = frame.raw_image.rows;
		int width = frame.raw_image.cols;
		int channels = frame.raw_image.channels();
		uint8_t dst_bits = frame.valid_bits;//目标文件比特位
		std::string tip = "12bit";
		if (dst_bits == 10)//windows显示10bit图像有问题,暂时使用12bit存储
		{
			dst_bits = 12;
			tip = "10bit";
		}

		TIFF * tif = TIFFOpen(path.toLocal8Bit().data(), "w");
		if (tif)
		{
			TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, dst_bits);//图像位数
			TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, channels);//通道
			TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);//色彩空间解释
			if (channels==3)
			{
				TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);//色彩空间解释
				cv::cvtColor(frame.raw_image, frame.raw_image, cv::ColorConversionCodes::COLOR_BGR2RGB);
			}
			TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
			TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);//压缩算法
			TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, tip.c_str());//备注图像实际位深
			TIFFSetField(tif, TIFFTAG_XRESOLUTION, 100);
			TIFFSetField(tif, TIFFTAG_YRESOLUTION, 100);
			TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);//分辨率单位
			TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
			TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
			TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);


			frame.raw_image *= (std::pow(2,dst_bits)-1)/ (std::pow(2, 16)-1);//改为低位存储

			if (dst_bits == 10)
			{
				ImageNS::Grey16_10(frame.raw_image.data, frame.raw_image.data, width * channels, height);
			}
			else if (dst_bits == 12)
			{
				ImageNS::Grey16_12(frame.raw_image.data, frame.raw_image.data, width * channels, height);
			}

			uint8_t*  data_packed = frame.raw_image.data;
			int32_t line_size = TIFFScanlineSize(tif);
			for (uint32_t row = 0; row < height; row++)
			{

				if (TIFFWriteScanline(tif, data_packed, row) < 0)
				{
					break;
				}
				data_packed += line_size;
			}

			TIFFClose(tif);
			return true;
		}
	}

	return false;
}