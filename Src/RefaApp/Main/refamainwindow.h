#ifndef RCCMAINWINDOW_H
#define RCCMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class RefaMainWindow;
}

/**
 * @brief 主界面类
 */
class RefaMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RefaMainWindow(QWidget *parent = 0);
    ~RefaMainWindow();

public slots:
	void setInitStackedWidget();

    /**
     * @brief 主窗口显示后执行的操作(自动搜索)
     */
	void onMainWindowShown();

private slots:
    void on_toolButtonHelp_clicked();
	void setFullScreen(bool benabled);

	/**
	 * @brief 需要自动回放
	 * @param video_id 视频ID
	 */
	void onAutoPlaybackNeeded(const QVariant & video_id);

	/**
	 * @brief 当前页面切换
	 * @param index 页面索引
	 */
	void onCurrentPageChanged(int index);

    /**
     * @brief 切换到实时图像线程完成
     */
    void onSwitchToRealtimeThreadFinished();

    /**
     * @brief 切换到慢速回放线程完成
     */
    void onSwitchToPlaybackThreadFinished();

	/**
	*@brief 设备连接响应槽函数
	*@param [in] : device_ip : const QString &，设备ip
	**/
	void onDeviceConnected(const QString & device_ip);
	
	/**
	* @brief 自动连接设备成功
	*/
	void onAutoConnectDevicesSuccess();

private:
	/**
	 * @brief 初始化UI
	 */
	void initUi();

private:
    Ui::RefaMainWindow *ui;
	int previous_page_index_{ 0 };
};

#endif // RCCMAINWINDOW_H
