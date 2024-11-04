#include "AviVideoFileReader.h"

#include "VideoFileUtils.h"
#include "AGBufferProcessor.h"
#include <boost/filesystem.hpp>
//#pragma comment(lib, "comsuppw.lib")

CAviVideoFileReader::CAviVideoFileReader()
{
	
}


CAviVideoFileReader::~CAviVideoFileReader()
{
	Close();
}

bool CAviVideoFileReader::Open(const std::wstring & filePath)
{
	VideoFormat videoFormat = VIDEO_RHVD;
	VideoFileUtils::GetVideoFormat(filePath, videoFormat);
	if (videoFormat != VIDEO_AVI && videoFormat != VIDEO_MP4)
	{
		return false;
	}
	boost::filesystem::path p(filePath);
	cv::String filePathCvStr(p.string());

	if (!video_capture_.open(filePathCvStr))
	{
		return false;
	}

	video_segment_info_.bits_per_pixel = 8;
	video_segment_info_.color_mode = HSC_COLOR_MULTIPLE;
	video_segment_info_.display_mode = HSC_DISPLAY_COLOR;
	if (video_capture_.read(avi_frame_) && avi_frame_.channels() == 1)
	{
		video_segment_info_.color_mode = HSC_COLOR_MONO;
		video_segment_info_.display_mode = HSC_DISPLAY_MONO;
	}
	
	video_segment_info_.frame_num = video_capture_.get(cv::CAP_PROP_FRAME_COUNT);
	video_segment_info_.fps = video_capture_.get(cv::CAP_PROP_FPS);
	int width = video_capture_.get(cv::CAP_PROP_FRAME_WIDTH);
	int height = video_capture_.get(cv::CAP_PROP_FRAME_HEIGHT);
	video_segment_info_.sensor_width_max = width;
	video_segment_info_.sensor_height_max = height;
	video_segment_info_.display_width_max = width;
	video_segment_info_.display_height_max = height;
	video_segment_info_.display_roi.x = 0;
	video_segment_info_.display_roi.y = 0;
	video_segment_info_.display_roi.width = width;
	video_segment_info_.display_roi.height = height;
	video_segment_info_.rect.x = 0;
	video_segment_info_.rect.y = 0;
	video_segment_info_.rect.width = width;
	video_segment_info_.rect.height = height;
	video_segment_info_.enable_20MP_resolution = false;
	video_segment_info_.rotation_type = HSC_ROTATION_NONE;

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

bool CAviVideoFileReader::GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo)
{
	if (!video_capture_.isOpened())
	{
		return false;
	}

	videoSegmentInfo = video_segment_info_;

	return true;
}

bool CAviVideoFileReader::GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor)
{
	if (video_segment_info_.frame_num == 1)
	{
		// 视频仅存在一帧时，只有首次能读出视频帧，故此种情形，直接取首次读出帧
		matImage = avi_frame_;
	}
	else
	{
		if (!video_capture_.isOpened())
		{
			return false;
		}

		if (frameNo < 0 || frameNo >= video_segment_info_.frame_num)
		{
			return false;
		}

		video_capture_.set(cv::CAP_PROP_POS_FRAMES, frameNo);

		if (!video_capture_.read(matImage))
		{
			return false;
		}
	}

	if (buffer_processor_ptr_)
	{
		matImage = buffer_processor_ptr_->cv_process(matImage, contrast, luminance, antiColor);
	}

	return true;
}

bool CAviVideoFileReader::Close()
{
	if (video_capture_.isOpened())
	{
		video_capture_.release();
	}

	return true;
}
