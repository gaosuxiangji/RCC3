#pragma once
#include "Device/device.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include "AGSDK.h"

#include <QDialog>
#include <QSharedPointer>
#include <QTimer>

namespace Ui { class CSManualWhiteBalanceDlg; };

class CSManualWhiteBalanceDlg : public QDialog
{
	Q_OBJECT

		// 颜色通道类型
		enum TCSColorChannel
	{
		COLOR_CHANNEL_RGGB,
		COLOR_CHANNEL_BGGR,
		COLOR_CHANNEL_GRBG,
		COLOR_CHANNEL_GBRG,
		COLOR_CHANNEL_YUV,
		COLOR_CHANNEL_RGB24,
		COLOR_CHANNEL_RGBA = 10,
		COLOR_CHANNEL_BGRA = 11,
		COLOR_CHANNEL_NAL = 20,
	};

public:
	CSManualWhiteBalanceDlg(QSharedPointer<Device> device_ptr, QWidget *parent = Q_NULLPTR);
	~CSManualWhiteBalanceDlg();

	static float changeU2F(uint16_t fix);
	static uint16_t changeF2U(float fix);
	void initUi();

	private slots:

	void on_Slider_R_valueChanged(int value);

	void on_Slider_G_valueChanged(int value);

	void on_Slider_B_valueChanged(int value);

	void on_spinBox_current_R_valueChanged(int value);

	void on_spinBox_current_G_valueChanged(int value);

	void on_spinBox_current_B_valueChanged(int value);

	void on_spinBox_startX_editingFinished();

	void on_spinBox_startY_editingFinished();

	void on_spinBox_width_editingFinished();

	void on_spinBox_height_editingFinished();

	void on_pushButton_calculate_clicked();

	void on_pushButton_resetting_clicked();

	void on_pushButton_auto_param_clicked();

	void on_pushButton_ok_clicked();

	void on_pushButton_cancel_clicked();

	/**
	* @brief 图像更新槽函数
	* @param buffer_ptr 图像帧
	* @param frameInfo 主要包含的OSD信息，显示使用
	*/
	void updateWhiteBalanceFrame(CAGBuffer* buffer_ptr, RccFrameInfo frameInfo);

	/**
	* @brief 图像计算区域变化信号
	* @param type 使用kManualWhiteBalance
	* @param rect 区域
	*/
	void SlotChangeRoi(Device::RoiTypes type, const QRect& rect);
private:
	/**
	* @brief 数据处理函数
	* @param oriImage 原始图像
	* @param processedImage 处理后的图像
	*/
	bool GetImageAndProcess(CAGBuffer * pBufRaw, cv::Mat &oriImage, cv::Mat &processedImage);

	/**
	* @brief 图像帧转换函数
	*/
	void Demosaic24bit_change(BYTE *pRGB, BYTE* pRaw, int X, int Y, int width, int height, TCSColorChannel colorChannel);
	/**
	* @brief 更新白平衡计算区域
	*/
	void updateManualCalArea();

	/**
	* @brief 设置白平衡计算区域
	*/
	void setManualCalArea();

	/**
	* @brief 位置转为参数
	* @param pos 位置
	* @return  参数值
	*/
	float posTofactor(int pos);

	/**
	* @brief 参数转为位置
	* @param factor 参数值
	* @return 位置
	*/
	int factorToPos(float factor);

	QSharedPointer<Device> m_device_ptr;					// 设备指针
	CPlayerViewBase* m_pOriginalPlayer{ nullptr };			// 播放器-原始图像
	CPlayerViewBase* m_pProcessedPlayer{ nullptr };			// 播放器-处理图像
	AGProcessorHandle m_processorHandle;					// 图像处理句柄
	AGFrame m_pProAGFrame;									// 图像处理指针
	AGColorCorrectParam m_pColorCorrectParam;				// 颜色校正指针
	const float constGainBase{ 64.0f };						// 增益计算基数
	QRect m_rcArea;											// 增益计算区域
	CAGBuffer* m_pLastFrame{ nullptr };						// 记录最新的帧信息，做计算使用
	CAGBuffer* m_pTempFrame{ nullptr };						// 临时变量,做图像转换使用
	const double constzMinPSINON{ 0.001 };					// 增益最小值
	QSize m_sizeImage;										// 图像大小
	int m_nAreaMIN{ 32 };									// 计算区域最小值

	Ui::CSManualWhiteBalanceDlg *ui;
};
