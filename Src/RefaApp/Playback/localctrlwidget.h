#ifndef LOCALCTRLWIDGET_H
#define LOCALCTRLWIDGET_H

#include "RMAImage.h"
//#include "chessboardpixelsizecalibrator.h"
#include <QWidget>
#include <QVariant>
#include <memory>

namespace Ui {
class LocalCtrlWidget;
}

class BaseCalibrator;

/**
 * @brief 本地回放控制界面类
 */
class LocalCtrlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LocalCtrlWidget(QWidget *parent = 0);
    ~LocalCtrlWidget();

public Q_SLOTS:
	/** @brief 外部通知该对象标定视频是否存在
	@param [in] : bool exists : 标定文件是否存在
	@return
	@note
	*/
	void slotCalibrationVideoExists(bool exists);

	/** @brief 获取当前视频帧
	@param
	@return
	@note
	*/
	void slotReceiveCurrentFrame(const RMAImage& image);

signals:
    /**
     * @brief 当前视频变化
     * @param video_id 当前视频ID
     */
    void currentVideoItemChanged(const QVariant & video_id);

    /**
     * @brief OSD可见变化
     * @param visible true-可见, false-不可见
     */
    void osdVisibleChanged(bool visible);

    /**
     * @brief 亮度对比度变化
     * @param luminance 亮度
     * @param contrast 对比度
     */
    void luminanceAndContrastChanged(int luminance, int contrast);

	/** @brief 通知外部对象获取当前视频帧
	@param 
	@return
	@note
	*/
	void sigRequestCurrentFrame();

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
	*@brief 清空测量特征
	*@param
	*@return
	**/
	void sigClearMeasureFeatures();

	/**
	*@brief 开始标定前发送该信号
	*@param
	*@return
	**/
	void sigBeginCalibrate();

	/**
	*@brief 结束标定后发送该信号
	*@param
	*@return
	**/
	void sigEndCalibrate();

private slots:
    /**
     * @brief 转到视频设置根界面
     */
    void toVideoSettingsRootWidget();

	/**
	* @brief 转到其他设置界面
	*/
	void toOtherSettingsWidget();

	/**
	* @brief 转到其他设置界面根页面
	*/
	void toOtherSettingsWidgetRootPage();

    /**
     * @brief 转到亮度对比度界面
     */
    void toLuminanceContrastWidget();

	/**
	* @brief 转到视频格式界面
	*/
	void toVideoFormatWidget();

	/**
	 * @brief 设置OSD是否可见
	 * @param visible true-可见，false-不可见
	 */
	void setOsdVisible(bool visible);

	/**
	 * @brief 设置视频格式
	 * @param format 视频格式
	 */
	void setVideoFormat(const QVariant & format);

    void on_pushButtonImportVideo_clicked();

    void on_pushButtonImportImageSequence_clicked();

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

    /**
     * @brief 亮度变化
     * @param value 亮度值
     */
    void onLuminanceChanged(int value);

    /**
     * @brief 对比度变化
     * @param value 对比度值
     */
    void onContrastChanged(int value);

    void on_pushButtonExport_clicked();

	/** @brief 点击标定按钮
	@param [in] : bool checked : 按钮是否被check
	@return
	@note
	*/
	void on_pushButtonCalibrate_clicked(bool checked = false);

	/**
	* @brief 设置其他设置显示内容
	*/
	void setOtherSettingsText();

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

	/**
	 * @brief 初始化视频列表界面
	 */
	void initVideoListUi();

    /**
     * @brief 更新视频列表界面
     */
    void updateVideoListUi();

    /**
     * @brief 初始化亮度/对比度界面
     */
    void initLuminanceAndContrastUi();

    /**
     * @brief 更新亮度/对比度界面
     * @param video_id 视频ID
     * @param luminance 亮度
     * @param contrast 对比度
     */
    void updateLuminanceAndContrastUi(const QVariant & video_id, int luminance, int contrast);

    /**
     * @brief 更新亮度/对比度组合框
     * @param video_id 视频ID
     * @param luminance 亮度
     * @param contrast 对比度
     */
    void updateLuminanceAndContrastCombo(const QVariant & video_id, int luminance, int contrast);

    /**
     * @brief 更新亮度标签
     * @param luminance 亮度
     */
    void updateLuminanceLabel(int luminance);

    /**
     * @brief 更新对比度标签
     * @param contrast 对比度
     */
    void updateContrastLabel(int contrast);

    /**
     * @brief 初始化水印信息界面
     */
    void initOsdUi();

    /**
     * @brief 更新水印信息界面
     * @param video_id 视频ID
     * @param osd_visible 水印是否显示
     */
    void updateOsdUi(const QVariant & video_id, bool osd_visible);

    /**
     * @brief 初始化视频格式界面
     */
    void initVideoFormatUi();

    /**
     * @brief 更新视频格式界面
     * @param video_id 视频ID
     * @param video_format 视频格式
     */
    void updateVideoFormatUi(const QVariant & video_id, int video_format);

	/**
	* @brief 更新视频格式组合框
	* @param video_id 视频ID
	* @param video_format 视频格式
	*/
	void updateVideoFormatCombo(const QVariant & video_id, int video_format);

    /**
     * @brief 初始化导出界面
     */
    void initExportUi();

	/** @brief 初始化标定参数界面消息绑定
	@param
	@return
	@note
	*/
	void InitCalibrationUiConnect();

    /**
     * @brief 更新导出界面
     * @param selected_video_id_list 选中的视频ID列表
     */
    void updateExportUi(const QVariantList & selected_video_id_list);

	/** @brief 初始化测量界面消息绑定
	@param
	@return
	@note
	*/
	void InitBasicMeasureUiConnect();

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::LocalCtrlWidget *ui;

    QVariant current_video_id_; // 视频ID

	RMAImage calibrationImage;//标定图像
	//double squareSizeValue = 0.;//方格间距值
	std::shared_ptr<BaseCalibrator> calibrator;//表定器
	FEATURE_ID chessboardFeatureID = -1;
};

#endif // LOCALCTRLWIDGET_H
