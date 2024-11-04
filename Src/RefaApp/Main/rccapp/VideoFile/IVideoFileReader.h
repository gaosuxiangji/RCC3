#pragma once

#include <string>


#include "HscAPIStructer.h"
#include "opencv2/videoio.hpp"

// 视频文件读取接口类
class IVideoFileReader
{
public:
	IVideoFileReader() = default;
	virtual ~IVideoFileReader() = default;
	IVideoFileReader(const IVideoFileReader&) = delete;
	IVideoFileReader(IVideoFileReader&&) = delete;
	IVideoFileReader & operator=(const IVideoFileReader&) = delete;
	IVideoFileReader & operator=(IVideoFileReader&&) = delete;

	// 打开
	virtual bool Open(const std::wstring & filePath) = 0;

	// 获取视频片段信息
	virtual bool GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo) = 0;

	// 获取帧
	virtual bool GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor) = 0;

	// 关闭
	virtual bool Close() = 0;
};
