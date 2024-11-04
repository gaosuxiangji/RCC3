#ifndef SETROIWIDGET_H
#define SETROIWIDGET_H

#include <QWidget>
#include <QRect>
#include <QVariant>



namespace Ui {
class SetRoiWidget;
}

//当前所在界面类型
enum CurInterfaceType
{
    RealtimeType = 0,
    RemoteTpye,
    LocalType
};


/**
 * @brief 设置Roi工具面板类
 */
class SetRoiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetRoiWidget(QWidget *parent = 0);
    ~SetRoiWidget();

    /**
     * @brief 设置当前所在界面类型
     * @param type 界面类型
     */
    void setCurInterfaceType(CurInterfaceType type);

public slots:
    /**
     * @brief 实时预览下图像ROI变化
     * @param ip 相机IP
     * @param roi 图像Roi
     */
    void DeviceRoiChanged(const QString &ip, const QRect &roi);

    /**
     * @brief 录制回放和本地回放下图像ROI变化
     * @param id   视频ID
     * @param roi   图像ROI
     */
    void VideoRioChanged(const QVariant& id, const QRect& roi);


signals:

    /**
     * @brief 实时预览ROI设置结束
     * @param b_applyed 是否应用
     * @param ip 设备IP
     * @param roi 图像ROI 不应用时返回原来的Roi
     */
    void sigDeviceRoiChangeFinished(bool b_applyed,const QString &ip, const QRect &roi);

    /**
     * @brief 录制回放和本地回放roi设置结束
     * @param b_applyed 是否应用
     * @param id 视频id
     * @param roi 图像roi 不应用时返回原来的Roi
     */
    void sigVideoRoiChangeFinished(bool b_applyed, const QVariant& id, const QRect& roi);

private slots:
    void on_pushButton_apply_clicked();

    void on_pushButton_cancel_clicked();


private:

	void dataRefresh(QRect roi);

    void roiChanged();

	QRect getDeviceOldRoi(QString ip);

	QRect getVideoOldRoi(QVariant id);

	QString cur_device_ip_;

	QVariant cur_video_id_;

	QRect cur_roi_;

	//宽高最小值
	int width_min{ 256 };
	int height_min{ 128 };

	CurInterfaceType cur_type_{RealtimeType};

    Ui::SetRoiWidget *ui;
};

#endif // SETROIWIDGET_H
