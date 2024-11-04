#include "RhvdVideoFileReader.h"
#include "HscAPIStructer.h"
#include "VideoFileUtils.h"
#include "AGBufferProcessor.h"
#include <boost/filesystem.hpp>
CRhvdVideoFileReader::CRhvdVideoFileReader()
{
	memset(&video_header_, 0, sizeof(video_header_));
	memset(&video_segment_info_, 0, sizeof(video_segment_info_));
}


CRhvdVideoFileReader::~CRhvdVideoFileReader()
{
	Close();
}

bool CRhvdVideoFileReader::Open(const std::wstring & filePath)
{
	VideoFormat videoFormat = VIDEO_RHVD;
	VideoFileUtils::GetVideoFormat(filePath, videoFormat);
	if (videoFormat != VIDEO_RHVD)
	{
		return false;
	}
	boost::filesystem::path p(filePath);
	file_.open(p.string(), std::ios_base::in | std::ios_base::binary);
	if (!file_)
	{
		return false;
	}

	file_.read((char*)&video_header_, sizeof(RAWVIDEO_HEADER));
	if (file_.gcount() != sizeof(RAWVIDEO_HEADER))
	{
		Close();
		return false;
	}

	if (video_header_.sensor_height == 0 || video_header_.sensor_width == 0)
	{
		Close();
		return false;
	}

	file_.seekg(1024, std::ios::beg);
	file_.read((char*)&video_segment_info_, sizeof(HscVideoClipInfo));
	if (file_.gcount() != sizeof(HscVideoClipInfo))
	{
		Close();
		return false;
	}

	// 设置图像处理模块
	buffer_processor_ptr_.reset(new CAGBufferProcessor(nullptr));
	if (buffer_processor_ptr_ != nullptr)
	{
		agile_device::capability::ImageFormatControl image_format_control{};
		image_format_control.bits_per_pixel = video_segment_info_.bits_per_pixel;
		image_format_control.max_sensor_width = video_segment_info_.sensor_width_max;
		image_format_control.max_sensor_height = video_segment_info_.sensor_height_max;
		image_format_control.max_display_width = video_segment_info_.display_width_max;
		image_format_control.max_display_height = video_segment_info_.display_height_max;
		image_format_control.offset_x_inc = 1;
		image_format_control.offset_y_inc = 1;
		image_format_control.width_inc = 1;
		image_format_control.height_inc = 1;

		HscDisplayMode display_mode = video_segment_info_.color_mode == HSC_COLOR_MONO ? HSC_DISPLAY_MONO : HSC_DISPLAY_COLOR;
		if (video_segment_info_.display_mode != 0)
		{
			display_mode = HscDisplayMode(video_segment_info_.display_mode);
		}
		buffer_processor_ptr_->SetDisplayMode(display_mode);
		buffer_processor_ptr_->setRotationType(video_segment_info_.rotation_type);
		/*
			X213、X113在做分辨率1296*960-1280*1024转换时，需要启用软裁剪
		*/
		if (
			(DEVICE_X213_ISPSeries == video_header_.camera_model || DEVICE_X113_ISPSeries == video_header_.camera_model || DEVICE_X190 == video_header_.camera_model) &&
			(video_segment_info_.sensor_width_max != video_segment_info_.display_width_max) &&
			(video_segment_info_.sensor_height_max != video_segment_info_.display_height_max)
			)
		{
			image_format_control.enable_width_soft_clipping = true;
		}
		buffer_processor_ptr_->SetROI(video_segment_info_.display_roi);
		buffer_processor_ptr_->Enable20MPResolution(video_segment_info_.enable_20MP_resolution);
		if (buffer_processor_ptr_->GetDisplayMode() == HSC_DISPLAY_COLOR){
			buffer_processor_ptr_->SetWhiteBalanceMode(video_segment_info_.color_correct_info.awb_mode_);
			if (video_segment_info_.color_correct_info.awb_mode_ == HscWhiteBalanceMode::HSC_WB_MANUAL_GAIN)
				buffer_processor_ptr_->setManualGain(video_segment_info_.color_correct_info.r_gain_, video_segment_info_.color_correct_info.g_gain_, video_segment_info_.color_correct_info.b_gain_);
			buffer_processor_ptr_->setWbEnv(video_segment_info_.color_correct_info.awb_env_);
			buffer_processor_ptr_->setGammaFactor(video_segment_info_.color_correct_info.gamma_factor_);
		}

		buffer_processor_ptr_->SetCapability(image_format_control);
	}

	return true;
}

bool CRhvdVideoFileReader::GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo)
{
	if (!file_)
	{
		return false;
	}

	videoSegmentInfo = video_segment_info_;

	return true;
}

bool CRhvdVideoFileReader::GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor)
{
	if (!file_)
	{
		return false;
	}

	if (frameNo < 0 || frameNo >= video_segment_info_.frame_num)
	{
		return false;
	}

	if (!buffer_processor_ptr_)
	{
		return false;
	}

	uint64_t frame_pitch = video_header_.frame_pitch;
	file_.seekg(2 * 1024 + frameNo*frame_pitch, std::ios::beg);
	file_.read((char*)buffer_ptr_.get(), video_header_.frame_size);
	if (file_.gcount() != video_header_.frame_size)
	{
		return false;
	}

	if (buffer_ptr_->frame_head.rect.width != video_segment_info_.rect.width || buffer_ptr_->frame_head.rect.height != video_segment_info_.rect.height)
	{
		return false;
	}

	matImage = buffer_processor_ptr_->cv_process(buffer_ptr_.get(), HSC_BUFFER_PROCESS_IMG, true);
	if (!matImage.empty())
	{
		matImage = buffer_processor_ptr_->cv_process(matImage, contrast, luminance, antiColor);
	}

	return true;
}

bool CRhvdVideoFileReader::Close()
{
	if (file_)
	{
		file_.close();
	}

	return true;
}
