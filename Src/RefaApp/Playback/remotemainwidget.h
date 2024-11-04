#ifndef REMOTEPLAYBACKWIDGET_H
#define REMOTEPLAYBACKWIDGET_H

#include "RMAImage.h"
#include <QWidget>
#include <QVariant>
#include <memory>

namespace Ui {
class RemoteMainWidget;
}

class PlaybackPlayerController;

/**
 * @brief 录制回放主界面类
 */
class RemoteMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteMainWidget(QWidget *parent = 0);
    ~RemoteMainWidget();

	/**
	 * @brief 更新设备列表
	 */
	void updateVideoList();

	/**
	* @brief 获取录制回放控制器
	*/
	std::shared_ptr<PlaybackPlayerController> getPlaybackPlayerController() const;

public slots:
	/**
	*@brief 设置全屏显示状态槽函数
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void setFullScreen(bool benable);

	/**
	*@brief 设置十字线槽函数
	*@param [in] : pt : const QPoint &，十字线交点
	**/
	void setFocusPoint(const QPoint &pt);

	/**
	*@brief 设置roi槽函数
	*@param [in] : roi : const QRect &，roi区域
	**/
	void setRoi(const QRect &roi);

	/**
	*@brief 设备连接响应槽函数
	*@param [in] : device_ip : const QString &，设备ip
	**/
	void onDeviceConnected(const QString & device_ip);

	/**
	*@brief 设备断连响应槽函数
	*@param [in] : device_ip : const QString &，设备ip
	**/
	void onDeviceDisconnected(const QString & device_ip);

	/**
	 * @brief 设置当前视频
	 * @param video_id 视频ID
	 */
	void setCurrentVideo(const QVariant & video_id);

	/**
	*@brief 设置播放器暂停
	**/
	void pause();

signals:
	/**
	*@brief 全屏信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void fullScreen(bool benable);

private slots:
	/**
	*@brief 两点测量
	*@param [in] : benabled : bool，是否使能
	*@return
	**/
	void slotMeasureLine(bool benabled);

	/**
	*@brief 单点测量
	*@param [in] : benabled : bool，是否使能
	*@return
	**/
	void slotMeasurePoint(bool benabled);

	/**
	*@brief 截图快照
	*@param [in] : id : const QVariant&，设备id
				   img : const RMAImage&，图像
	*@return
	**/
	void slotSanpshot(const QVariant& id, const RMAImage& img);

	/**
	*@brief 播放器内修改roi
	*@param [in] : id : const QVariant&，设备id
				   rc : const QRect&，修改后的roi
	*@return
	**/
	void slotRoiChanged(const QVariant& id, const QRect& rc);

	/**
	*@brief roi修改完成响应槽函数
	*@param [in] : b_applyed : bool，是否应用，true-应用，false-不应用
	id : const QVariant&，设备id
	roi : const QRect&，应用的roi
	**/
	void slotRoiChangeFinished(bool b_applyed, const QVariant& id, const QRect& roi);

    /**
     * @brief 当前视频变化
     * @param video_id 视频ID
     */
    void onCurrentVideoItemChanged(const QVariant & video_id);

	/**
	* @brief 视频加载完成
	 * @param ok true-成功，false-失败
	**/
	void onVideoLoaded(bool bok = true);

private:
    /**
     * @brief 初始化UI
     */
    void initUi();

    /**
     * @brief 初始化播放器UI
     */
    void initPlayerUi();

    /**
     * @brief 初始化控制面板UI
     */
    void initCtrlUi();

private:
    Ui::RemoteMainWidget *ui;
};

#endif // REMOTEPLAYBACKWIDGET_H
