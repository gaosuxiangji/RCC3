#pragma once

#include <string>

#include "HscAPIHeader.h"

class IVideoFileReader;

class VideoFileUtils
{
public:
	VideoFileUtils() = default;
	~VideoFileUtils() = default;
	VideoFileUtils(const VideoFileUtils&) = delete;
	VideoFileUtils(VideoFileUtils&&) = delete;
	VideoFileUtils & operator=(const VideoFileUtils&) = delete;
	VideoFileUtils & operator=(VideoFileUtils&&) = delete;

	// 获取视频格式，成功返回true，失败返回false
	static bool GetVideoFormat(const std::wstring &filePath, VideoFormat & videoFormat);

	// 获取视频格式字符串
	static std::wstring GetVideoFormatStr(const VideoFormat & videoFormat);

	// 获取视频文件读取器
	static IVideoFileReader* GetVideoFileReader(const VideoFormat & videoFormat);
};

