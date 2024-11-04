#pragma once

#include <memory>

#include "IVideoFileReader.h"

class CAGBufferProcessor;

// AVI视频文件读取类
class CAviVideoFileReader :	public IVideoFileReader
{
public:
	CAviVideoFileReader();
	~CAviVideoFileReader();

	bool Open(const std::wstring & filePath) override;

	bool GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo) override;

	bool GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor) override;

	bool Close() override;

private:
	cv::VideoCapture video_capture_;
	HscVideoClipInfo video_segment_info_;
	cv::Mat avi_frame_;
	std::unique_ptr<CAGBufferProcessor> buffer_processor_ptr_;
};

