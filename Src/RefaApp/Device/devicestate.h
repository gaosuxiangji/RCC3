#ifndef DEVICESTATE_H
#define DEVICESTATE_H

/**
 * @brief 设备状态枚举
 */
enum DeviceState
{
    Unconnected, // 未连接
    Connecting, // 正在连接
    Connected, // 已连接
    ToPreview, // 正在切换预览
    Previewing, // 预览中
    ToAcquire, // 正在切换采集
    Acquiring, // 采集中
    Recording, // 录制中
    ToReplay, // 正在切换回放
    Replaying, // 回放中
    ToExport, // 正在切换导出
    Exporting, // 导出中
    Disconnecting, // 正在断开
	Disconnected, // 已断开
	Reconnecting, // 正在重连
	StandBy,	// 待机
	RecoveringData,	// 正在断电数据恢复
	ToWakeup, // 正在唤醒
};
Q_DECLARE_METATYPE(DeviceState)

#endif // DEVICESTATE_H
