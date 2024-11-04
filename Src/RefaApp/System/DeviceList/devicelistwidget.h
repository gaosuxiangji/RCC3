#ifndef DEVICELISTWIDGET_H
#define DEVICELISTWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QPointer>
#include <QMap>

#include "device/devicemanager.h"
#include "Device/device.h"


namespace Ui {
class DeviceListWidget;
}

/**
 * @brief 设备列表界面类
 */
class DeviceListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceListWidget(QWidget *parent = 0);
    ~DeviceListWidget();

    /**
     * @brief 刷新已搜索设备列表
     */
    void updateSearchedDeviceList();

    /**
     * @brief 刷新已连接设备树
     */
    void updateConnectedDeviceTree();

    /**
     * @brief 开始搜索设备
     */
	void SearchDevices();

	/**
	* @brief 断开已连接列表中的所有设备
	*/
	void DisconnectAllDevice();

private slots:
    /**
     * @brief 搜索完成
     * @param device_count 设备个数
     */
    void slotSearchFinished(int device_count);


    /**
     * @brief 有设备状态变化
     * @param state 目前的设备状态
     */
    void slotDevStateChanged(DeviceState state);


	void slotDevErrorOccurred(quint64 err_code);

    /**
     * @brief 已连接的设备选择状态变化
     * @param current
     * @param previous
     */
    void slotCurrentSelectChanged(const QModelIndex &current, const QModelIndex &previous);

	void slotDeviceTreeItemDropped(QModelIndex index);

	/**
	*@brief 设备已连接信号响应槽函数
	*@param ip 设备ip
	**/
	void slotDeviceConnected(const QString & ip);

	/**
	*@brief 设备已断开信号响应槽函数
	*@param ip 设备ip
	**/
	void slotDeviceDisonnected(const QString & ip);

private slots:
    void on_pushButtonSearch_clicked();

    /**
     * @brief 连接按钮单击响应
     * @param index 模型索引
     */
    void onConnectButtonClicked(const QModelIndex &index);

    /**
     * @brief 断开按钮单击响应
     * @param index 模型索引
     */
    void onDisconnectButtonClicked(const QModelIndex &index);

    void on_pushButtonAlterIp_clicked();

    void on_pushButtonDeviceLicense_clicked();

    void on_pushButtonEditDeviceName_clicked();

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化已搜索界面
     */
    void initSearchedUi();

    /**
     * @brief 初始化已连接界面
     */
    void initConnectedUi();

    /**
     * @brief 获取当前设备指针
     * @return 当前设备指针
     */
	QSharedPointer<Device> getCurrentDevicePtr();

    /**
     * @brief 获取设备授权描述
     * @param license 设备授权结构体
     * @return 设备授权描述 NULL-不支持
     */
	QString getLicenseDesc(DeviceLicense license);

	/**
	* @brief 设备授权提示信息窗口
	* @param license 设备授权结构体
	*/
	void licenseMsgBox(const DeviceLicense & license);

	/**
	*@brief 自动连接设备
	**/
	void autoConnectDevices();

	/**
	*@brief 自动连接的设备的结果
	*@param ip 设备ip
	*@param bconnected 是否已连接，true-已连接，false-连接失败
	**/
	void autoConnectDevicesResult(const QString & ip, bool bconnected);

	/**
	*@brief 更新设别列表，包括已连接列表和未连接列表
	**/
	void updateDeviceList();


	/**
	* @brief 断开已连接列表中的设备
	* @param index 模型索引
	*/
	void DisconnectDeviceByIndex(const QModelIndex &index);

signals:
	/**
	*@brief 自动连接设备成功
	**/
	void sigAutoConnectDeviceSuccess();

private:
    Ui::DeviceListWidget *ui;

    QPointer<QStandardItemModel>  searched_model_ptr_;

    QPointer<QStandardItemModel>  connected_model_ptr_;

    QPointer<DeviceManager> dev_manager_ptr_;

	const int secs_of_day_ = 86400;
	const int trial_license_tip_days_{ 3 };//试用授权<3天提示

	const int kConnectedDevicesCountMax{ 1 };//按照市场需求连接设备最大数量从2修改为1

	QMap<QString, bool> auto_connecting_devices_state_;
	bool bauto_connecting_devices_{ false };

	//已连接相机列表列号
	enum class ConnectedDeviceTreeColumn
	{
		kConnectedDeviceName,			//相机名
		kConnectedDeviceModelName,		//型号名
		kConnectedDeviceIP,				//ip
		kConnectedDeviceLicense,		//授权信息
		kConnectedDeviceState,			//状态
		kConnectedDeviceOperate,		//操作
	};

};

#endif // DEVICELISTWIDGET_H
