#include "VideoFileUtils.h"

#include <algorithm>
#include "HscAPIHeader.h"
#include "RhvdVideoFileReader.h"
#include "AviVideoFileReader.h"
#include "ImageVideoFileReader.h"
#include <boost/filesystem.hpp>
std::wstring ToLower(std::wstring & wstr)
{
	std::transform(wstr.begin(), wstr.end(), wstr.begin(), ::towlower);
	return wstr;
}

bool VideoFileUtils::GetVideoFormat(const std::wstring & filePath, VideoFormat & videoFormat)
{
	boost::filesystem::path fspath(filePath);
	if (boost::filesystem::is_directory(fspath))
	{
		std::wstring filename = fspath.filename().wstring();
		std::wstring format = L"bmp";

		auto ext = filename.substr(0, format.length());
		if (filename.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_BMP;
			return true;
		}

		format = L"jpg";
		ext = filename.substr(0, format.length());
		if (filename.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_JPG;
			return true;
		}

		format = L"tif";
		ext = filename.substr(0, format.length());
		if (filename.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_TIF;
			return true;
		}

		format = L"png";
		ext = filename.substr(0, format.length());
		if (filename.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_PNG;
			return true;
		}
	}
	else
	{
		std::wstring format = L".rhvd";
		auto ext = filePath.substr(filePath.length() - format.length());
		if (filePath.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_RHVD;
			return true;
		}

		format = L".avi";
		ext = filePath.substr(filePath.length() - format.length());
		if (filePath.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_AVI;
			return true;
		}

		format = L".mp4";
		ext = filePath.substr(filePath.length() - format.length());
		if (filePath.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_MP4;
			return true;
		}

		format = L".raw";
		ext = filePath.substr(filePath.length() - format.length());
		if (filePath.length() >= format.length() && ToLower(ext) == format)
		{
			videoFormat = VIDEO_RAW;
			return true;
		}
	}

	return false;
}

std::wstring VideoFileUtils::GetVideoFormatStr(const VideoFormat & videoFormat)
{
	std::wstring formatStr(L"unkown");
	switch (videoFormat)
	{
	case VIDEO_RHVD:
		formatStr = L"RHVD";
		break;
	case VIDEO_AVI:
		formatStr = L"AVI";
		break;
	case VIDEO_MP4:
		formatStr = L"MP4";
		break;
	case VIDEO_BMP:
		formatStr = L"BMP";
		break;
	case VIDEO_JPG:
		formatStr = L"JPG";
		break;
	case VIDEO_TIF:
		formatStr = L"TIF";
		break;
	case VIDEO_RAW:
		formatStr = L"RAW";
		break;
	case VIDEO_PNG:
		formatStr = L"PNG";
		break;
	}

	return formatStr;
}

IVideoFileReader* VideoFileUtils::GetVideoFileReader(const VideoFormat & videoFormat)
{
	IVideoFileReader* videoFileReader = nullptr;

	switch (videoFormat)
	{
	case VIDEO_RHVD:
		videoFileReader = new CRhvdVideoFileReader();
		break;
	case VIDEO_AVI:
	case VIDEO_MP4:
		videoFileReader = new CAviVideoFileReader();
		break;
	case VIDEO_BMP:
	case VIDEO_JPG:
	case VIDEO_TIF:
	case VIDEO_PNG:
		videoFileReader = new CImageVideoFileReader();
		break;
	default:
		break;
	}

	return videoFileReader;
}
