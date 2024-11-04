#ifndef LOCALWIDGET_H
#define LOCALWIDGET_H

#include "RMAImage.h"
#include <QWidget>
#include <QVariant>

namespace Ui {
class LocalMainWidget;
}

/**
 * @brief 本地回放主界面类
 */
class LocalMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LocalMainWidget(QWidget *parent = 0);
    ~LocalMainWidget();

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
	void setFocusPoint(const QPoint & pt);

	/**
	*@brief 设置roi槽函数
	*@param [in] : roi : const QRect &，roi区域
	**/
	void setRoi(const QRect & roi);

signals:
	/**
	*@brief 全屏显示信号
	*@param [in] : benabled : bool，true-进入全屏，false-退出全屏
	**/
	void fullScreen(bool benable);

	/** @brief 通知外部对象标定视频是否存在
	@param [in] : bool exists : 标定文件是否存在
	       [in] : const QString& filePath : 标定文件路径
	@return
	@note
	*/
	void sigCalibrationVideoExists(bool exists);

	/** @brief 发送当前视频帧
	@param
	@return
	@note
	*/
	void sigSendCurrentFrame(const RMAImage& image);

private slots:
	/**
	*@brief 选中视频变化响应槽函数
	*@param [in] : id :const QVariant&，设备id
	**/
	void onCurrentVideoItemChanged(const QVariant & video_id);

	/** @brief 外部通知该对象获取当前视频帧
	@param
	@return
	@note
	*/
	void slotRequestCurrentFrame();

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
	*@param [in] : id :const QVariant&，设备id
				   img : const RMAImage&，图像
	*@return
	**/
	void slotSanpshot(const QVariant& id, const RMAImage& img);

	/**
	*@brief 开始标定
	*@param 
	*@return
	**/
	void slotBeginCalibrate();

	/**
	*@brief 播放器内修改roi
	*@param [in] : id : const QVariant&，设备id
				   roi : const QRect&，修改后的roi
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

private:
	/**
	 * @brief 初始化界面
	 */
	void initUi();

	/**
	 * @brief 初始化播放器界面
	 */
	void initPlayerUi();

	/**
	 * @brief 初始化控制界面
	 */
	void initCtrlUi();

private:
    Ui::LocalMainWidget *ui;
};

#endif // LOCALWIDGET_H
