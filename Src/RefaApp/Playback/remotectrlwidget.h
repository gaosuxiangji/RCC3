#ifndef REMOTECTRLWIDGET_H
#define REMOTECTRLWIDGET_H

#include <QWidget>
#include <QSharedPointer>
#include <QVariant>
#include <QList>
#include "Video/VideoItem/videoitem.h"


class Device;

namespace Ui {
class RemoteCtrlWidget;
}

/**
 * @brief 录制回放控制界面类
 */
class RemoteCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteCtrlWidget(QWidget *parent = 0);
    ~RemoteCtrlWidget();

    /**
     * @brief 设置设备
     * @param device_ptr 设备指针
     */
    void setDevice(QSharedPointer<Device> device_ptr);

    /**
     * @brief 获取设备
     * @return 设备指针
     */
    QSharedPointer<Device> getDevice() const;

	/**
	* @brief 获取左设备
	* @return 设备指针
	*/
	QSharedPointer<Device> getLeftDevice() const;

	/**
	* @brief 获取右设备
	* @return 设备指针
	*/
	QSharedPointer<Device> getRightDevice() const;

    /**
     * @brief 切换当前选中的视频
     * @param video_id 视频项id
     */
    void setCurrrentVideo(const QVariant & video_id);

	/**
	 * @brief 获取当前选中视频
	 * @return 视频项ID
	 */
	QVariant getCurrentVideo() const;

	/**
	 * @brief 更新设备列表
	 */
	void updateVideoList();

	/**
	* @brief 更新测量控件
	*/
	void updateMeasureCtrl();

	/**
	*@brief 视频加载完成
	**/
	void videoLoaded();

signals:

    /**
     * @brief 显示缩略图信号
     * @param b_visible 是否显示
     */
    void sigSetThumbnailVisible(bool b_visible);

    /**
     * @brief 显示水印信息
     * @param b_visible 是否显示
     */
	void sigSetOsdVisible(bool b_visible);

    /**
     * @brief 设置亮度对比度信号
     * @param luminance 亮度
     * @param contrast 对比度
     */
    void sigSetLuminanceContrast(int luminance, int contrast );

	/**
	* @brief 亮度对比度修改完成信号信号
	*/
	void sigLuminanceContrastUpdateFinished();

    /**
     * @brief 当前视频项变更
     * @param video_item_id 视频id
     */
    void sigCurrentVideoItemChanged(QVariant video_id);

	/**
	*@brief 单点测量状态变化
	*@param benabled true-使能, false-不使能
	*@return
	**/
	void sigMeasurePoint(bool benabled);

	/**
	*@brief 两点测量状态变化
	*@param benabled true-使能, false-不使能
	*@return
	**/
	void sigMeasureLine(bool benabled);

	/**
	*@brief 清空当前测量模式下的特征
	*@param
	*@return
	**/
	void sigClearCurrentMeasureModeFeatures();

	/**
	* @brief 开始导出
	*/
	void sigStartExport();

private slots:
    /**
     * @brief 转到视频设置根界面
     */
    void toVideoSettingRootWidget();

    /**
     * @brief 转到相机选择界面
     */
    void toCameraSelectionWidget();

	/**
	* @brief 转到其他设置界面
	*/
	void toOtherSettingsWidget();

	/**
	* @brief 转到其他设置界面根页面
	*/
	void toOtherSettingsWidgetRootPage();

    /**
     * @brief 转到亮度对比度设置界面
     */
    void toLuminanceContrastWidget();

    /**
     * @brief 转到协议格式界面
     */
    void toStreamTypeWidget();

    /**
     * @brief 转到视频格式选择界面
     */
    void toVideoformatWidget();

    /**
     * @brief 设置缩略图是否可见
     * @param b_visible true-可见 false-不可见
     */
    void setThumbnailVisible(bool b_visible);

    /**
     * @brief 设置左相机是否选中
     * @param b_selected 是否选中
     */
    void setLeftCameraSelection(bool b_selected);

    /**
     * @brief 设置右相机是否选中
     * @param b_selected 是否选中
     */
	void setRightCameraSelection(bool b_selected);

    /**
     * @brief 设置水印信息是否可见
     * @param b_visible 是否可见
     */
    void setOsdVisible(bool b_visible);

    /**
     * @brief 设置协议格式
     * @param stream_id 协议格式选项
     */
    void setStreamType(const QVariant & stream_id);

    /**
     * @brief 设置视频格式
     * @param format_id 视频格式选项
     */
    void setVideoFormat(const QVariant & format_id);

	/**
	* @brief 设置其他设置显示内容
	*/
	void setOtherSettingsText();

private slots:

    void onDeviceStateChanged();

    void onDevicePropertyChanged();

    void onDeviceErrorOccurred();

	/**
	* @brief 当前视频变化
	* @param video_id 视频ID
	*/
	void onCurrentVideoItemChanged(const QVariant & video_id);

	/**
	* @brief 选中视频变化
	* @param video_id_list 视频ID列表
	*/
	void onSelectedVideoItemsChanged(const QVariantList & video_id_list);

	/**
	* @brief 删除视频
	* @param video_id_list 视频ID列表
	*/
	void onVideoItemsRemoved(const QVariantList & video_id_list);

    void on_sliderLuminance_valueChanged(int value);

    void on_sliderContrast_valueChanged(int value);

    void on_pushButtonExport_clicked();

	/**
	 * @brief 更新设备列表
	 */
	void onVideoSegmentListUpdated();

private:
	/** @brief 界面初始化
	@param 
	@return
	@note 
	*/
    void InitUI();

    /**
     * @brief 视频设置界面初始化
     */
    void InitVideoSettingUI();


    /**
     * @brief 更新视频设置界面(相机选择和视频列表)
     */
	void updateVideoSettingUI();

    /**
     * @brief 更新相机选择界面
     */
    void updateCameraSelectionUI();

    /**
     * @brief 更新设备列表界面
     */
    void updateVideoListUI();

	/**
	* @brief 更新其他设置界面
	*/
	void updateOtherSettingsUI();

    /**
     * @brief 更新亮度对比度界面
     */
    void updateLuminanceContrastUI();

    /**
     * @brief 更新OSD界面
     */
    void updateOsdUI();

    /**
     * @brief 更新协议格式界面
     */
    void updateStreamTypeUI();

    /**
     * @brief 更新视频格式界面
     */
    void updateVideoFormatUI();

	QVariantList getSupportedVideoFormat(QVariant cur_stream);

    /**
     * @brief 更新导出按钮状态
     */
    void updateExportButtonUI();

    /**
     * @brief 获取当前视频项
     * @return
     */
	VideoItem getCurVideoItem();

	/** @brief 初始化测量界面消息绑定
	@param
	@return
	@note
	*/
	void InitBasicMeasureUiConnect();

    /**
     * @brief 设置控件列表使能
     * @param widgets_to_enable 控件列表
     * @param b_enbable 使能状态
     */
	void SetWidgetsEnable(QList<QWidget*> widgets_to_enable,bool b_enbable);
	
private:
    Ui::RemoteCtrlWidget *ui;
		
	QWeakPointer<Device> device_wptr_;

	QWeakPointer<Device> left_device_wptr_;
	QWeakPointer<Device> right_device_wptr_;

	QVariant current_video_id_;

	//刷新相机选择界面时的默认选项
	bool  left_camera_old_select_{ true };
	bool  right_camera_old_select_{ true };

	bool  left_camera_select_{ false };
	bool  right_camera_select_{ false };

    QList<QWidget*> widgets_to_enable_;//需要控制使能的控件列表

	bool needs_update_video_list_{ false };
	bool badded_new_video_segment_{ false };//是否添加了新的视频，用于判断是否跳转至最后一个视频
};

#endif // REMOTECTRLWIDGET_H
