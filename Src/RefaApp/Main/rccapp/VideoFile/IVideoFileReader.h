#pragma once

#include <string>


#include "HscAPIStructer.h"
#include "opencv2/videoio.hpp"

// ��Ƶ�ļ���ȡ�ӿ���
class IVideoFileReader
{
public:
	IVideoFileReader() = default;
	virtual ~IVideoFileReader() = default;
	IVideoFileReader(const IVideoFileReader&) = delete;
	IVideoFileReader(IVideoFileReader&&) = delete;
	IVideoFileReader & operator=(const IVideoFileReader&) = delete;
	IVideoFileReader & operator=(IVideoFileReader&&) = delete;

	// ��
	virtual bool Open(const std::wstring & filePath) = 0;

	// ��ȡ��ƵƬ����Ϣ
	virtual bool GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo) = 0;

	// ��ȡ֡
	virtual bool GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor) = 0;

	// �ر�
	virtual bool Close() = 0;
};
