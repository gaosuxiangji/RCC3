#pragma once

#include "IVideoFileReader.h"

#include <memory>

class CAGBufferProcessor;

// Õº∆¨–Ú¡–∂¡»°¿‡
class CImageVideoFileReader : public IVideoFileReader
{
public:
	CImageVideoFileReader();
	~CImageVideoFileReader();

	bool Open(const std::wstring & file_path) override;

	bool GetVideoSegmentInfo(HscVideoClipInfo & video_segment_info) override;

	bool GetFrame(int frame_no, cv::Mat & mat_image, int contrast, int luminance, bool anti_color) override;

	bool Close() override;

private:
	void parseParamFile(const std::string & param_path, HscVideoClipInfo & video_clip_info);

private:
	std::set<VideoFormat> supported_video_formats_{ VIDEO_BMP, VIDEO_JPG, VIDEO_TIF };
	HscVideoClipInfo video_segment_info_;
	std::unique_ptr<CAGBufferProcessor> buffer_processor_ptr_;
	std::vector<std::string> frame_file_paths_;
};

