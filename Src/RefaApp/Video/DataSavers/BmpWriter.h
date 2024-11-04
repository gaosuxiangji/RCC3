#ifndef _BMP_WRITER_
#define _BMP_WRITER_

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#pragma pack(push, 1)
/**
* @brief XJ520-XG 图像附属信息
*/
struct BitmapExtendInfo
{
	uint16_t year{ 0 }; // 年，自然年
	uint8_t month{ 0 }; // 月，从1开始
	uint8_t day{ 0 }; // 日，从1开始
	uint32_t absolute_time{ 0 }; // 绝对时：0.1ms
	float azimuth = 0; // 方位角度值：1" =方位角/536870912*360
	float pitch = 0; // 俯仰角角度值: 1" =俯仰角/536870912*360
	float distance = 0; // 距离值：1m
	float focal_length = 0; // 焦距：1mm =镜头焦距值/100
	uint32_t frame_rate = 0; // 帧频：0.01帧/s
	uint16_t exposure_time = 0; // 积分时间：μs
	uint8_t res[12];
};

/**
* @brief XJ1310 图像附属信息
*/
struct XJ1310BitmapExtendInfo
{
	uint32_t timestamp = 0; // 拍摄起始时刻
	uint8_t device_state = 0; // 设备状态
	int32_t azimuth = 0; // 方位角
	int32_t pitch = 0; // 俯仰角
	uint32_t focal_length = 0; // 焦距
	uint64_t distance = 0; // 距离
	uint32_t frame_rate = 0; // 帧频
	uint32_t exposure_time = 0; // 曝光时间
	uint16_t camera_state = 0; // 相机状态
	uint8_t res[29]{ 0 }; // 预留
};

typedef struct tagBITMAPFILEHEADER_ {
	uint16_t    bfType;
	uint32_t   bfSize;
	uint16_t    bfReserved1;
	uint16_t    bfReserved2;
	uint32_t   bfOffBits;
} BITMAPFILEHEADER_;

typedef struct tagBITMAPINFOHEADER_ {
	uint32_t      biSize;
	int32_t       biWidth;
	int32_t       biHeight;
	uint16_t       biPlanes;
	uint16_t       biBitCount;
	uint32_t      biCompression;
	uint32_t      biSizeImage;
	int32_t       biXPelsPerMeter;
	int32_t       biYPelsPerMeter;
	uint32_t      biClrUsed;
	uint32_t     biClrImportant;
} BITMAPINFOHEADER_;
#pragma pack(pop)
class CBmpWriter
{
public:
	CBmpWriter();
	~CBmpWriter();

	/**
	* @brief 写入BMP图像：自定义信息
	* @param [in] filename 图像路径
	* @param [in] image 图像数据
	* @param [in] device_index 设备编号
	* @param [in] device_state 设备工作状态
	* @param [in] extend_info 图像附属信息
	* @return true-成功，false-失败
	*/
	static bool write(const std::string & filename, const cv::Mat & image, uint16_t device_index, uint16_t device_state, const XJ1310BitmapExtendInfo & extend_info);

	static bool write(const std::string & filename, const cv::Mat & img, uint16_t device_index, uint16_t device_state, const BitmapExtendInfo & extend_info);

	static bool write(const std::string & filename, const cv::Mat & img, uint16_t bf_res1, uint16_t bf_res2, const std::vector<uint8_t> & custom_data);
};

#endif