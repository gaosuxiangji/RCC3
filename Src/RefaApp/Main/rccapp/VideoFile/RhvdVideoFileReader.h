#pragma once

#include <string>
#include <fstream>
#include <memory>

#include "IVideoFileReader.h"
#include "HscAPIStructer.h"
#include <ctime>
class CAGBufferProcessor;

//RHVD Header定义
#pragma pack(push,1)
struct RAWVIDEO_HEADER
{
	DWORD mark;                        //待定，4字节
	DWORD version;                      //待定，4字节 0x01020304表示1.2.3.4版本
	DWORD camera_model;                //待定，4字节
	std::time_t create_time;               //创建时间，4字节时间戳
	DWORD sensor_width;                 //传感器宽度，即图像最大宽度，4字节
	DWORD sensor_height;                 //传感器高度，即图像最大高度，4字节
	DWORD blackfield_size;                //暗场数据大小，一般情况为最大高度*最大宽度*sizeof(short)，4字节
	DWORD blackfield_offset;              //暗场数据位置，相对于文件起点的偏移量，4字节
	DWORD grayfield_size;                 //灰场数据大小，一般情况为最大高度*最大宽度*sizeof(short)，4字节
	DWORD grayfield_offset;               //灰场数据位置，相对于文件起点的偏移量，4字节
	DWORD frame_size;                   //一帧数据大小，4字节
	DWORD frame_pitch;                  //一帧数据的偏移量，4字节
	DWORD first_frame_offset;             //起始第一帧数据位置，相对于文件起点的偏移量，4字节
	DWORD fps;                         //图像帧率，4字节
	char  ip[16];						// ip地址
	char  sn[16];						// 序列号
	DWORD play_frame_rate;				// 播放帧率 版本2.0.0.0以后支持的字段
	char remark[32];					// 保留字段
};
#pragma pack(pop)
//RHVD 相机参数定义，留位置

class CRhvdVideoFileReader : public IVideoFileReader
{
public:
	CRhvdVideoFileReader();
	~CRhvdVideoFileReader();

	bool Open(const std::wstring & filePath) override;

	bool GetVideoSegmentInfo(HscVideoClipInfo & videoSegmentInfo) override;

	bool GetFrame(int frameNo, cv::Mat & matImage, int contrast, int luminance, bool antiColor) override;

	bool Close() override;

private:
	std::ifstream file_;
	RAWVIDEO_HEADER video_header_;
	HscVideoClipInfo video_segment_info_;
	std::unique_ptr<CAGBufferProcessor> buffer_processor_ptr_;
	std::unique_ptr<CAGBuffer> buffer_ptr_{ new CAGBuffer() };
};

