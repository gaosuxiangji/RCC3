#ifndef DEVICESYSTEMWIDGET_H
#define DEVICESYSTEMWIDGET_H

#include <QWidget>

namespace Ui {
class DeviceSystemWidget;
}

/**
 * @brief 设备系统界面类
 */
class DeviceSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSystemWidget(QWidget *parent = 0);
    ~DeviceSystemWidget();

    /**
     * @brief 主窗口显示后执行的操作(自动搜索)
     */
	void onMainWindowShown();

	/**
	* @brief 断开所有设备并且重新搜索(切换工控机IP时)
	*/
	void onHostIpChanged();


private:
    /**
     * @brief 初始化界面
     */
    void initUi();

signals:
	/**
	*@brief 自动连接设备成功
	**/
	void sigAutoConnectDeviceSuccess();

private:
    Ui::DeviceSystemWidget *ui;
};

#endif // DEVICESYSTEMWIDGET_H
