#include "ImageVideoFileReader.h"
#include <boost/filesystem.hpp>
#include "VideoFileUtils.h"
#include "AGBufferProcessor.h"
#include <QString>
#include <QStringList>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <regex>
CImageVideoFileReader::CImageVideoFileReader()
{
}


CImageVideoFileReader::~CImageVideoFileReader()
{
}

bool CImageVideoFileReader::Open(const std::wstring & file_path)
{
	//std::string ascii_file_path = QString::fromWCharArray(file_path.c_str()).toStdString();
	//std::string ascii_file_path;
	//int len = WideCharToMultiByte(CP_ACP, 0, file_path.c_str(), file_path.size(), NULL, 0, NULL, NULL);
	//if (len < 0) return false;
	//char *buffer = new char[len + 1];
	//if (buffer == NULL) return false;
	//WideCharToMultiByte(CP_ACP, 0, file_path.c_str(), file_path.size(), buffer, len, NULL, NULL);
	//buffer[len] = '\0';
	//ascii_file_path.append(buffer);
	//delete[] buffer;
	std::string ascii_file_path;
	try
	{
		boost::filesystem::path_traits::convert(file_path.c_str(), file_path.c_str() + file_path.size(), ascii_file_path, boost::filesystem::path::codecvt());
	}
	catch (const std::exception&)
	{
		return false;
	}

	VideoFormat video_format = VIDEO_RHVD;
	if (!VideoFileUtils::GetVideoFormat(file_path, video_format) || supported_video_formats_.find(video_format) == supported_video_formats_.end())
	{
		return false;
	}

	frame_file_paths_.clear();

	std::map<std::string, std::string> frame_file_path_map;

	boost::filesystem::path fspath(ascii_file_path);
	boost::filesystem::directory_iterator end_iter;
	for (boost::filesystem::directory_iterator file_iter(fspath); file_iter != end_iter; file_iter++)
	{
		if (!boost::filesystem::is_regular_file(*file_iter))
		{
			continue;
		}

		std::string ext = boost::filesystem::extension(file_iter->path());
		if (ext != ".bmp" && ext != ".jpg" && ext != ".tif")
		{
			continue;
		}

		std::string stem = file_iter->path().stem().string();
		std::vector<std::string> splits;
		QString strStem = QString::fromStdString(stem);
		QStringList stringList = strStem.split("-");
		std::string time_str;
		std::regex pattern("[0-9]{8}_[0-9]{6}.[0-9]{6}");

		std::match_results<std::string::const_iterator> result;
		for (auto item : stringList)
		{
			std::string text = item.toStdString();
			splits.push_back(text);
			if (std::regex_match(text, result, pattern)) {
				time_str = *result.begin();
				frame_file_path_map[time_str] = file_iter->path().string();
				break;
			}
		}

		if (!splits.empty() && time_str == "")
		{
			auto str = splits.back();
			bool isNumber = std::all_of(str.begin(), str.end(), [](char c) {return isdigit(c) != 0; });
			if (isNumber)
			{
				uint64_t frame_no = boost::lexical_cast<uint64_t>(splits.back());
				frame_file_path_map[str] = file_iter->path().string();
			}
		}
	}

	video_segment_info_.frame_num = frame_file_path_map.size();
	video_segment_info_.bits_per_pixel = 8;
	video_segment_info_.color_mode = HSC_COLOR_MULTIPLE;
	video_segment_info_.display_mode = HSC_DISPLAY_COLOR;

	std::string param_path = ascii_file_path + "/video_param.json";
	parseParamFile(param_path, video_segment_info_);

	if (!frame_file_path_map.empty())
	{
		std::string first_frame_file_path = frame_file_path_map.begin()->second;
		cv::Mat mat_image = cv::imread(first_frame_file_path);
		if (!mat_image.empty())
		{
			int width = mat_image.cols;
			int height = mat_image.rows;
			video_segment_info_.display_roi.x = 0;
			video_segment_info_.display_roi.y = 0;
			video_segment_info_.display_roi.width = width;
			video_segment_info_.display_roi.height = height;
			video_segment_info_.rect.x = 0;
			video_segment_info_.rect.y = 0;
			video_segment_info_.rect.width = width;
			video_segment_info_.rect.height = height;
			video_segment_info_.sensor_width_max = width;
			video_segment_info_.sensor_height_max = height;
			video_segment_info_.display_width_max = width;
			video_segment_info_.display_height_max = height;
			video_segment_info_.enable_20MP_resolution = false;
			video_segment_info_.rotation_type = HSC_ROTATION_NONE;

			if (mat_image.channels() == 1)
			{
				video_segment_info_.color_mode = HSC_COLOR_MONO;
				video_segment_info_.display_mode = HSC_DISPLAY_MONO;
			}
		}
	}

	for (auto pair : frame_file_path_map)
	{
		frame_file_paths_.push_back(pair.second);
	}

	buffer_processor_ptr_.reset(new CAGBufferProcessor(nullptr));
	if (buffer_processor_ptr_ != nullptr)
	{
		agile_device::capability::ImageFormatControl image_format_control;
		image_format_control.bits_per_pixel = video_segment_info_.bits_per_pixel;
		image_format_control.max_sensor_width = video_segment_info_.sensor_width_max;
		image_format_control.max_sensor_height = video_segment_info_.sensor_height_max;
		image_format_control.max_display_width = video_segment_info_.display_width_max;
		image_format_control.max_display_height = video_segment_info_.display_height_max;
		image_format_control.sensor_rotation_type = video_segment_info_.rotation_type;
		buffer_processor_ptr_->SetCapability(image_format_control);
		buffer_processor_ptr_->SetDisplayMode(HscDisplayMode(video_segment_info_.display_mode));
		buffer_processor_ptr_->SetROI(video_segment_info_.display_roi);
		buffer_processor_ptr_->Enable20MPResolution(video_segment_info_.enable_20MP_resolution);
	}

	return true;
}

bool CImageVideoFileReader::GetVideoSegmentInfo(HscVideoClipInfo & video_segment_info)
{
	video_segment_info = video_segment_info_;

	return true;
}

bool CImageVideoFileReader::GetFrame(int frame_no, cv::Mat & mat_image, int contrast, int luminance, bool anti_color)
{
	if (frame_no < 0 || frame_no >= frame_file_paths_.size())
	{
		return false;
	}

	mat_image = cv::imread(frame_file_paths_[frame_no]);
	if (buffer_processor_ptr_)
	{
		mat_image = buffer_processor_ptr_->cv_process(mat_image, contrast, luminance, anti_color);
	}

	return true;
}

bool CImageVideoFileReader::Close()
{
	return true;
}

void CImageVideoFileReader::parseParamFile(const std::string & param_path, HscVideoClipInfo & video_clip_info)
{
	if (!boost::filesystem::exists(param_path))
	{
		return;
	}

	std::ifstream is(param_path);
	std::string json_str((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());

    QJsonParseError parseJsonErr;
	QJsonDocument doc = QJsonDocument::fromJson(json_str.c_str(), &parseJsonErr);

	if (!(parseJsonErr.error == QJsonParseError::NoError))
	{
		printf("parse error\n");
		return;
	}

	QJsonObject jsonObject = doc.object();

	if (jsonObject.contains("name"))
	{
		std::string name = jsonObject["name"].toString().toStdString();
		if (!name.empty())
		{
			std::memcpy(video_clip_info.name, name.c_str(), name.length());
		}
	}

	if (jsonObject.contains("timestamp"))
	{
		video_clip_info.time_stamp = jsonObject["timestamp"].toInt();
	}

	if (jsonObject.contains("width") && jsonObject.contains("height"))
	{
		int width = jsonObject["width"].toInt();
		int height = jsonObject["height"].toInt();
		video_segment_info_.display_roi.x = 0;
		video_segment_info_.display_roi.y = 0;
		video_segment_info_.display_roi.width = width;
		video_segment_info_.display_roi.height = height;
		video_segment_info_.rect.x = 0;
		video_segment_info_.rect.y = 0;
		video_segment_info_.rect.width = width;
		video_segment_info_.rect.height = height;
		video_segment_info_.sensor_width_max = width;
		video_segment_info_.sensor_height_max = height;
		video_segment_info_.display_width_max = width;
		video_segment_info_.display_height_max = height;
	}

	if (jsonObject.contains("frame_rate"))
	{
		video_segment_info_.fps = jsonObject["frame_rate"].toInt();
	}

	if (jsonObject.contains("exposure_time"))
	{
		video_segment_info_.exposure_time = jsonObject["exposure_time"].toInt();
	}

	if (jsonObject.contains("key_frame_no"))
	{
		video_segment_info_.key_frame_no = jsonObject["key_frame_no"].toInt();
	}
}
