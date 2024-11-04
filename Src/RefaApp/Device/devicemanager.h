#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QList>
#include <QSharedPointer>
#include <QMutex>
#include "HscAPI.h"
#include "Main/rccapp/csglobaldefine.h"
#include <QMap>
#include <QTreeWidget>
#include <QTreeWidgetItem>
class Device;
/*class HscDeviceInfo;*/
#ifdef CSRCCAPP

static const int kSupportedMaxDeviceCount = 64;//最大搜索设备数
static const int kSupportedMaxAddedDeviceCount = 18;//最大设备连接数
#else

static const int kSupportedMaxDeviceCount = 64;//最大搜索设备数
static const int kSupportedMaxAddedDeviceCount = 2;//最大设备连接数
#endif // CSRCCAPP

enum TreeWidgetItemType {
	GROUP,
	DEVICE,
	SYN_CONTROLLER
};

typedef struct TreeWidgetItemInfo {
	TreeWidgetItemType item_type{ GROUP };
	QSharedPointer<Device> p_device{ nullptr };
	QString group_name{};
	QString device_model_type{}; // 设备型号名称
	int device_model{0};
}TreeWidgetItemInfo;

/**
 * @brief 设备管理类
 */
class DeviceManager : public QObject
{
    Q_OBJECT
public:
    static DeviceManager& instance();

    /**
     * @brief 搜索
     */
    void search();


    /**
     * @brief 创建已搜索到的设备
     * 需要在设备搜索结束时调用
     */
	void createDevices();

	void createDevice(const HscDeviceInfo& info);
	

    /**
     * @brief 根据ip获取设备
     * @param ip 设备IP
     */
    QSharedPointer<Device> getDevice(const QString & ip) const;
	QSharedPointer<Device> getNewDevice(const QString & ip, const DeviceModel& dm);

    /**
     * @brief 获取设备列表
     * @param devices 设备列表
     */
    void getDevices(QList<QSharedPointer<Device>> & devices) const;
	void getManageDevices(QList<QSharedPointer<Device>> & devices) const;
	QList<TreeWidgetItemInfo> getTreeWidgetItemInfo();

	/**
	* @brief 设备列表是否存在设备
	*/
	bool hasDevices()const;

    /**
     * @brief 获取已连接设备数
     * @return 已连接设备数
     */
    int getConnectedDeviceCount() const;

    /**
     * @brief 获取已连接设备列表
     * @param devices 设备列表
     */
	void getConnectedDevices(QList<QSharedPointer<Device>> & devices)const;

    /**
     * @brief 新增已添加设备
     * @param device 设备
     */
    void appendAddedDevice(QSharedPointer<Device>  device);

    /**
     * @brief 移除已添加设备
     * @param device 设备
     */
    void removeAddedDevice(QSharedPointer<Device>  device);

    /**
     * @brief 设置当前设备
     * @param device 设备
     */
    void SetCurrentDevice(QSharedPointer<Device>  device);

    /**
     * @brief 获取当前设备
     * @return 当前设备指针
     */
    QSharedPointer<Device> getCurrentDevice() const;

    /**
     * @brief 获取已添加设备列表
     * @param devices 已添加设备列表
     */
    void getAddedDevices(QList<QSharedPointer<Device>> & devices)const;

    /**
     * @brief 断开所有设备
     */
    void disconnectAll();

	/**
	* @brief 所有已连接设备从本地重新加载属性
	*/
	void ReloadAllDeviceConfigFromLocal();

	// 是否有相机处于高采
	bool isAnyCameraHighMode() const;

	/**
	* @brief 是否允许全部设备软触发
	* @return 是/否
	*/
	bool AllowSoftTriggerAllDevice();

	/**
	* @brief 软触发全部正在高采中的相机
	*/
	void SoftTriggerAllDevice();

	void addManageItem(const TreeWidgetItemInfo&  iteminfo);
	void appendManageItem(const TreeWidgetItemInfo&  iteminfo);
	void removeManageDevice(const QString& cur_ip);
	bool findManageDevice(QSharedPointer<Device>  device);

	void removeManageGroup(const QString& group_name);
	void reviseManageGroup(const QString& group_name, const QString& group_new_name);
	bool findManageGroup(const QString& group_name);
signals:
    /**
     * @brief 搜索进度变化
     * @param value 进度值，有效范围[0,100]
     */
    void searchProcessChanged(int value);

    /**
     * @brief 搜索完成
     * @param device_count 设备个数
     */
    void searchFinished(int device_count);

    /**
     * @brief 设备已连接
     * @param device_ip 设备IP
     */
    void deviceConnected(const QString & device_ip);

    /**
     * @brief 设备已断开
     * @param device_ip 设备IP
     */
    void deviceDisconnected(const QString & device_ip);

    /**
     * @brief 当前设备切换
     * @param device_ip 设备IP
     */
    void currentDeviceChanged(const QString & device_ip);

	/**
	* @brief 需要自动回放
	* @param video_id 视频ID
	*/
	void autoPlaybackNeeded(const QVariant & video_id);

	/**
	* @brief 自动导出视频
	* @param  video_id 视频ID
	*/
	void autoExportVideo(const QVariant & video_id);

	/**
	* @brief 自动导出视频并且自动触发
	* @param  video_id 视频ID
	*/
	void autoExportVideoAndTrigger(const QVariant & video_id);

	void signalAddDevice(const TreeWidgetItemInfo&  iteminfo);
	void signalUpdateTree(const QString& ip);
	void signalCurrentGroupChanged(const QString group_name, const QString device_num);
private:
	/**
	 * @brief 设备录制完成
	 * @param video_id 视频ID
	 */
	void onRecordingFinished(const QVariant & video_id, bool berforeStopRecing, uint8_t state);

private:
    explicit DeviceManager(QObject *parent = 0);

    /**
     * @brief 根据ip获取设备
     * @param ip 设备IP
     * @return
     */
    QSharedPointer<Device> getDeviceByIp(const QString &ip) const;


private:
	HscDeviceInfo device_infos_[kSupportedMaxDeviceCount];//搜索到的设备信息
	int device_count_ = kSupportedMaxDeviceCount;//搜索到的设备个数
	bool search_res{false};//搜索结果

	QSharedPointer<Device> current_device_ ; //当前选中设备
	QList<QSharedPointer<Device>> added_devices_; //已添加的设备列表(点击连接后添加的设备,最多2个)

    QList<QSharedPointer<Device>> devices_; // 设备列表
	QList<QSharedPointer<Device>> m_manage_devices{}; //管理设备列表
	QList<TreeWidgetItemInfo> m_manage_devices_item_info{};
    mutable QMutex mutex_;
};

#endif // DEVICEMANAGER_H
