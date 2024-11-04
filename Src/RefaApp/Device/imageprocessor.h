#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <memory>

#include "opencv2/opencv.hpp"

#include "HscAPIStructer.h"

#include <QRect>

class Device;
class CAGBufferProcessor;

/**
 * @brief 图像处理类
 */
class ImageProcessor
{
public:
    ImageProcessor(Device *device_ptr);

    /**
     * @brief 获取显示最大宽度
     * @return
     */
    int GetDisplayWidthMax() const;

    /**
     * @brief 获取显示最大高度
     * @return
     */
    int GetDisplayHeightMax() const;

    /**
     * @brief 图像处理
     * @param buffer_ptr 图像缓存
     * @param process_type 处理类型：0-预览，1-采集，2-导出原始图像（RHVD），3-导出处理图像（BMP、JPG、AVI等）
     * @param bpp_enabled
     * @return 处理图像
     */
    cv::Mat cv_process(CAGBuffer *buffer_ptr, int process_type, bool bpp_enabled = false);

    /**
     * @brief 图像处理
     * @param mat 原图像
     * @param contrast 对比度
     * @param luminance 亮度
     * @param anti_color 反色
     * @return 处理图像
     */
    cv::Mat cv_process(const cv::Mat &mat, int contrast, int luminance, bool anti_color);

	/**
	* @brief 图像处理(去除一些中间过程中的对源数据buffer_ptr内存拷贝，会改变buffer_ptr)
	* @param buffer_ptr 图像缓存
	* @param process_type 处理类型：0-预览，1-采集，2-导出原始图像（RHVD），3-导出处理图像（BMP、JPG、AVI等）
	* @param bpp_enabled
	* @return 处理图像
	*/
	cv::Mat cv_process_no_copy(CAGBuffer *buffer_ptr, int process_type, bool bpp_enabled);

	void SetRoi(QRect roi);

	/**
	* @brief 设置显示模式
	* @param mode 显示模式
	*/
	void SetDisplayMode(HscDisplayMode mode);

	/**
	* @brief 获取显示模式
	* @return 显示模式
	*/
	HscDisplayMode GetDisplayMode() const;

	/**
	* @brief 设置颜色校正参数
	* @param info 颜色校正参数
	*/
	void setColorCorrectInfo(const HscColorCorrectInfo & info);

	/**
	* @brief 获取颜色校正参数
	* @return 颜色校正参数
	*/
	HscColorCorrectInfo getColorCorrectInfo();

	/**
	*@brief 设置旋转类型
	*@param rotation_type 旋转类型
	**/
	void setRotationType(HscRotationType rotation_type);

	/**
	* @brief 获取旋转类型
	* @return 旋转类型
	*/
	HscRotationType getRotationType() const;


	// 白平衡模式
	void setWhiteBalanceMode(HscWhiteBalanceMode wbMode);
	HscWhiteBalanceMode getWhiteBalanceMode() const;

	//手动白平衡
	void setManualGain(float r, float g, float b);
	void getManualGain(float& r, float& g, float& b);

	//拍摄环境
	HScWbEnv setWbEnv(HScWbEnv env);
	HScWbEnv getWbEnv();

	void setGammaFactor(float gamma);
	float getGammaFactor() const;

	//获取有效bbp
	int GetSignificantBitsPerPixel() const;

	void SetSignificantBitsPerPixel(uint8_t bpp);

	void SetBinningModeEnable(bool enable);
	bool GetBinningModeEnable() const;

	//输出bpp
	void SetBitsPerPixel(uint8_t bpp);

	//获取bbp
	uint8_t GetBitsPerPixel() const;

	// 设置帧头格式
	void setFrameHeadType(uint8_t type);

	void setFilerEnable(uint8_t filter_enable);
private:
    Device* device_ptr_;
};

#endif // IMAGEPROCESSOR_H
