#ifndef DEVICELICENSE_H
#define DEVICELICENSE_H

/**
 * @brief 设备授权结构体
 */
struct DeviceLicense
{
    int mode { 0 }; // 授权模式：0-软件授权，1-硬件授权
    int state{ 2 }; // 授权状态：0-永久授权，1-试用授权，2-未授权，3-试用授权超期，4-系统时间错误，5-其他错误，6-授权时间存储错误，7-使用时间存储错误
    quint64 remainder{ 0 }; // 剩余授权时间（s），试用授权时有效
};

#endif // DEVICELICENSE_H
