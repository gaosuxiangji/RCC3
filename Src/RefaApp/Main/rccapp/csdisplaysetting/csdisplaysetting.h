#ifndef CSDISPLAYSETTING_H
#define CSDISPLAYSETTING_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class CSDisplaySetting;
}

class CSDisplaySetting : public QWidget
{
    Q_OBJECT
#define POPUP_TITLE QObject::tr("RCC")
public:
    explicit CSDisplaySetting(QWidget *parent = 0);
    ~CSDisplaySetting();

	/**************************
	* @brief: 
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SDIVisible(bool bShow);

	/**************************
	* @brief: 设置反色框状态
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetColorRevertCheckBoxStatus(const bool bChecked);

	/**************************
	* @brief: 设置亮度值
	* @param: value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetluminanceValue(const int value);

	/**************************
	* @brief: 设置对比度
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetContrastValue(const int value);

	/**************************
	* @brief: 设置sdi当前状态
	* @param:bOpened true-已开启 false-未开启
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetSdiCurrentStatus(const bool bOpened);

	/**************************
	* @brief: 设置sdi按钮状态
	* @param: bOpened true-已开启 false-未开启
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetSdiButtonStatus(bool bOpened);

	/**************************
	* @brief: 设置分辨率&播放速率列表
	* @param:strList 分辨率&播放速率列表
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SetSdiFpsResolsList(const QList<QString>& strList);

	/**************************
	* @brief: 设置播放倍速列表
	* @param:strList 播放倍速列表
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void  SetSdiDisplaySpeeds(const QStringList& strList);

	/**************************
	* @brief: 设置当前分辨率&播放速率
	* @param: index 索引
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetCurrentSdiFpsResol(const int index);

	/**************************
	* @brief: 设置当前播放倍速
	* @param: strSpeed - 当前倍速
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetCurrentSdiDisplaySpeed(const QString& strSpeed);

	/**************************
	* @brief: 设置投屏按钮状态
	* @param:bChecked true-选中 false-未选中
	* @return:
	* @author:mpp
	* @date:2022/06/15
	***************************/
	void SetSecondScreenBtnStatus(const bool bChecked);

	/**************************
	* @brief: 清空设置分辨率&播放速率列表
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/28
	***************************/
	void ClearSdiFpsResolsList();

	/**************************
	* @brief: 清空播放倍速列表
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/28
	***************************/
	void ClearSdiDisplaySpeedsList();

	/**************************
	* @brief: 重置参数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/07/07
	***************************/
	void ResetParams();
private:
	/**************************
	* @brief: 初始化UI
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/05/26
	***************************/
	void InitUI();

	/**************************
	* @brief: 隐藏“图像处理”模块中的控件
	* @param: checked 隐藏按钮是否选中
	* @return:
	* @author:mpp
	* @date:2022/05/26
	***************************/
	void HideImageProcessItem(bool checked);

	/**************************
	* @brief: 隐藏“SDI回放”模块中的控件
	* @param: checked 隐藏按钮是否选中
	* @return:
	* @author:mpp
	* @date:2022/05/26
	***************************/
	void HideSDIPlaybackItem(bool checked);

	/**************************
	* @brief: 隐藏“投屏显示”模块中的控件
	* @param: checked 隐藏按钮是否选中
	* @return:
	* @author:mpp
	* @date:2022/05/26
	***************************/
	void HideSecodaryScreenItem(bool checked);

	/**************************
	* @brief: 连接信号槽
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void ConnectSignalAndSlot();

	/**************************
	* @brief: 设置投屏当前状态
	* @param:bOpened true-已开启 false-未开启
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SetSecondScreenStatus(const bool bOpened);

	/**************************
	* @brief: 更新帧按钮状态
	* @param: bForbid true-禁用 false-启用
	* @return:
	* @author:mpp
	* @date:2022/07/12
	***************************/
	void UpadateFrameBtnStatus(const bool bForbid);
signals:
	/**************************
	* @brief: 反色复选框状态变化信号
	* @param:bCheced true-被勾选 false-未被勾选
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalRevertCheckBoxStatusChanged(bool bChecked);

	/**************************
	* @brief: 亮度值变化信号
	* @param: value 亮度值
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalLuminanceChanged(const int value);

	/**************************
	* @brief: 对比度值变化信号
	* @param: value 对比度值
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalContrastChanged(const int value);

	/**************************
	* @brief: sdi向前一帧
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalSdiForwardStep();

	/**************************
	* @brief: sdi向后一帧
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalSdiBackwardStep();

	/**************************
	* @brief: sdi播放暂停按钮信号
	* @param:bPlay true-播放  false-暂停
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalSdiDisplayControl(bool bPlay);

	/**************************
	* @brief: sdi开启关闭控制
	* @param:bOpened-true 开启 false-关闭
	* @param:dSpeed 倍速
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalSdiSwitchControl(bool bOpened, const double dSpeed);

	/**************************
	* @brief: 分辨率&播放帧率列表选中索引信号
	* @param:index-索引
	* @return:
	* @author:mpp
	* @date:2022/06/08
	***************************/
	void SignalSdiFpsResolsListIndex(const int index);

	/**************************
	* @brief: 播放倍速值信号
	* @param: value 倍速值
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SignalSdiSpeedValue(const double value);

	/**************************
	* @brief: 开启投屏信号
	* @param: bOpen true-开启 false-关闭
	* @return:
	* @author:mpp
	* @date:2022/06/09
	***************************/
	void SignalSdiSecondScreen(bool bOpen);
private slots:
	/**************************
	* @brief:鼠标释放亮度滑块槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/24
	***************************/
	void SlotLumSliderReleased();

	/**************************
	* @brief: 鼠标释放对比度滑块槽函数
	* @param:
	* @return:
	* @author:mpp
	* @date:2022/06/24
	***************************/
	void SlotContrastSliderReleased();
    void on_image_process_pushButton_clicked(bool checked);

    void on_sdi_playback_title_pushButton_clicked(bool checked);

    void on_secondary_screen_title_pushButton_clicked(bool checked);

    void on_color_revert_checkBox_clicked(bool checked);

    void on_luminance_horizontalSlider_valueChanged(int value);

    void on_luminance_lineEdit_editingFinished();

    void on_contrast_lineEdit_editingFinished();

    void on_resolution_displayRate_comboBox_currentIndexChanged(const QString &arg1);

    void on_forward_frame_pushButton_clicked();

    void on_play_pause_pushButton_clicked(bool checked);

    void on_backward_frame_pushButton_clicked();

    void on_speed_comboBox_currentIndexChanged(const QString &arg1);

    void on_start_sdi_pushButton_clicked(bool checked);

    void on_start_secondary_screen_pushButton_clicked(bool checked);

    void on_contrast_horizontalSlider_valueChanged(int value);

    void on_resolution_displayRate_comboBox_currentIndexChanged(int index);

    void on_speed_comboBox_currentIndexChanged(int index);

    void on_luminance_lineEdit_textChanged(const QString &arg1);

    void on_contrast_lineEdit_textChanged(const QString &arg1);

private:
	QLabel* m_labelImgProcessTitleText;
	QLabel* m_labelImgProcessTitleIcon;
	QLabel* m_labelSDITitleText;
	QLabel* m_labelSDITitleIcon;
	QLabel* m_labelSecondaryScreenTitleText;
	QLabel* m_labelSecondaryScreenTitleIcon;
	QString m_strCurrentSpeed{};    //当前倍速
	bool m_bSpeedInit{ false };    //速度下拉框第一次初始化时触发槽函数，不发送信号
	bool m_bRsolutionInit{ false };    //分辨率下拉框第一次初始化时触发槽函数，不发送信号
private:
    Ui::CSDisplaySetting *ui;
};

#endif // CSDISPLAYSETTING_H
