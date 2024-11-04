#ifndef CSVIDEOSAVER_H
#define CSVIDEOSAVER_H
#include <opencv2/opencv.hpp>
#include <QString>
#include "Device/device.h"
#include "HscAPIStructer.h"


//视频数据保存类
class CSVideoSaver
{
public:
    CSVideoSaver(const QString save_path,bool with_piv);
	~CSVideoSaver();

	/**
	* @brief 保存快照
	* @param video_info 图像数据和相关信息
	* @return 是否成功
	*/
	bool SaveSnapshot( RccFrameInfo video_info, bool bAnti, uint8_t frame_header_type = eXType);

private:
	bool saveBmpWithExtendInfo(QString image_path,  RccFrameInfo & video_frame);

	/**
	* @brief 保存XJ1310附属信息图像
	* @param [in] image_path 图像路径
	* @param [in] video_frame 视频帧
	* @return true-成功，false-失败
	*/
	bool saveBmpWithXJ1310ExtendInfo(QString image_path,  RccFrameInfo & video_frame);


	bool SaveExcel(QString excel_path, RccFrameInfo& frame);
	bool SaveTiff(QString path, RccFrameInfo& frame);

	QString m_save_path;
	bool m_with_piv_flag = false;
};

#endif // CSVIDEOSAVER_H