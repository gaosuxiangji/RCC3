#ifndef LOCALVIDEOEXPORTPREPROCESSDELEGATE_H
#define LOCALVIDEOEXPORTPREPROCESSDELEGATE_H

#include "SingleVideoExportPreprocessDelegate.h"
#include <memory>
#include <QRect>
#include <QStringList>

class ISPUtil;

class LocalVideoExportPreprocessDelegate : public SingleVideoExportPreprocessDelegate
{
	Q_OBJECT

public:
	struct LocalExtraParams
	{
		QRect roi;// 图像裁剪roi区域
		bool benabled_watermark{ false };// 是否叠加水印
		QString video_name;// 视频名称
	};

public:
    LocalVideoExportPreprocessDelegate(std::shared_ptr<SingleVideoExportParam> param, QObject *parent = Q_NULLPTR);

	/** @brief 设置其他参数，如裁剪后窗口，标定点等
	@param [in] : params : const LocalExtraParams &，原始参数
	*/
	void setExtraParams(const LocalExtraParams & params);

	/** @brief 获取一帧图像
	@param [in] : FRAME_INDEX frameIndex : 帧号
		   [out] : RMAImage& image : 图像
	@return : bool : 获取图像是否成功
	@note : 该函数会调用ISP模块对图像进行处理
	*/
	bool GetFrame(FRAME_INDEX frameIndex, RMAImage& image) const override;

private:
	/**
	*@brief 绘制osd信息
	*@param [in/out] : RMAImage& image : 图像
	**/
	void paintOSD(RMAImage& image) const;

	/**
	*@brief 获取osd信息
	*@param [in] : const RMAImage& image : 图像
	*@return : QStringList : osd信息
	**/
	inline QStringList getOsdInfo(const RMAImage& image) const;

private:
	LocalExtraParams extra_params_;
	static const int kWatermarkPadding{ 5 };
};

#endif // LOCALVIDEOEXPORTPREPROCESSDELEGATE_H