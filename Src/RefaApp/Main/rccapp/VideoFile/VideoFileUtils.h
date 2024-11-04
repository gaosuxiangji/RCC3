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

	// ��ȡ��Ƶ��ʽ���ɹ�����true��ʧ�ܷ���false
	static bool GetVideoFormat(const std::wstring &filePath, VideoFormat & videoFormat);

	// ��ȡ��Ƶ��ʽ�ַ���
	static std::wstring GetVideoFormatStr(const VideoFormat & videoFormat);

	// ��ȡ��Ƶ�ļ���ȡ��
	static IVideoFileReader* GetVideoFileReader(const VideoFormat & videoFormat);
};

