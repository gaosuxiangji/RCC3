#ifndef DEVICEFRAME_H
#define DEVICEFRAME_H

#include <QRect>
#include <opencv2/opencv.hpp>

/**
 * @brief 设备帧结构体
 */
struct DeviceFrame
{
    int format{ 0 }; // 0-RGGB, 1-BGGR, 2-GRBG, 3-GBRG, 4-YUV, 5-BGR24, 6-Raw
    QRect roi; // ROI
    quint64 frame_no{ 0 }; // 帧编号
    QStringList osd; // OSD
    cv::Mat image; // 图像数据
};

Q_DECLARE_METATYPE(DeviceFrame)

#endif // DEVICEFRAME_H
