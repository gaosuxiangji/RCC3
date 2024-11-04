#ifndef VIDEOTOOLBAR_H
#define VIDEOTOOLBAR_H

#include <QWidget>

namespace Ui {
class VideoToolBar;
}
class QToolButton;

/**
 * @brief 视频工具栏类
 */
class VideoToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit VideoToolBar(QWidget *parent = 0);
    ~VideoToolBar();

    /**
     * @brief 设置是否使能
     * @param enabled true-使能，false-禁用
     * @attention 全屏不响应
     */
    void setEnabled(bool enabled);

	/**
	* @brief ROI框选是否使能
	* @return true-使能，false-不使能
	*/
	bool isRoiSelectionEnabled() const;

public slots:
    /**
     * @brief 设置是否选中十字中心线
     * @param bchecked true-选中，false-未选中
     */
	void setFocusLineChecked(bool bchecked);//是否选中窗口中心线

	/**
    * @brief 设置ROI框选是否使能
    * @param benabled true-使能，false-不使能
    */
	void setRoiSelectionEnabled(bool benabled);

signals:
	/**
	*@brief 放大触发信号
	**/
    void zoomInTriggered();

	/**
	*@brief 缩小触发信号
	**/
    void zoomOutTriggered();

	/**
	*@brief 1:1显示触发信号
	**/
    void zoomToOriginalTriggered();

	/**
	*@brief 窗口自适应触发信号
	**/
    void zoomToFitTriggered();

	/**
	*@brief 十字中心线触发信号
	*@param [in] : bchecked : bool,ture-选中显示十字线，false-未选中不显示十字线
	**/
    void focusLineTriggered(bool bchecked);

	/**
	*@brief 全屏触发信号
	*@param [in] : enable : benabled : bool，true-进入全屏，false-退出全屏
	**/
    void fullScreenTriggered(bool enable);

	/**
	*@brief ROI框选触发信号
	**/
    void roiSelectionTriggered();

	/**
	*@brief 快照触发信号
	**/
    void snapshotTriggered();

	/**
	*@brief 有按钮按下时发出信号
	**/
	void buttonClicked();

private slots:
	/**
	*@brief 全屏按钮响应槽函数
	**/
    void on_toolButtonFullscreen_clicked();

private:
    Ui::VideoToolBar *ui;

    QList<QToolButton*> response_enable_buttons_; // 响应使能按钮
};

#endif // VIDEOTOOLBAR_H
