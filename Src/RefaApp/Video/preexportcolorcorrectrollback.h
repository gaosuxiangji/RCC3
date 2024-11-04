#ifndef PREEXPORTCOLORCORRECTROLLBACK_H
#define PREEXPORTCOLORCORRECTROLLBACK_H


#include "Device/imageprocessor.h"
#include "Video/VideoItem/videoitem.h"
#include <HscAPIStructer.h>
//视频导出前的图像参数修正和导出后的图像参数回滚
class PreExportColorCorrectRollback
{
public:
	PreExportColorCorrectRollback(std::shared_ptr<ImageProcessor> processer)
		:processer_(processer)
	{
		display_mode_ = processer->GetDisplayMode();
		info_back_ = processer->getColorCorrectInfo();
		rotation_type_ = processer->getRotationType();
	}

	PreExportColorCorrectRollback(std::shared_ptr<ImageProcessor> processer, const VideoItem  video_item,bool need_rollback = true)
		:processer_(processer),need_rollback_(need_rollback)
	{
		if (processer)
		{
			display_mode_ = processer->GetDisplayMode();
			if (video_item.getDisplayMode() != 0)
			{
				processer->SetDisplayMode(HscDisplayMode(video_item.getDisplayMode()));
			}

			info_back_ = processer->getColorCorrectInfo();
			HscColorCorrectInfo color_correct_info = video_item.getProperty(VideoItem::PropType::ColorCorrectInfo).value<HscColorCorrectInfo>();
			processer->setWhiteBalanceMode(color_correct_info.awb_mode_);
			if (color_correct_info.awb_mode_ == HscWhiteBalanceMode::HSC_WB_MANUAL_GAIN)
			{
				processer->setManualGain(color_correct_info.r_gain_, color_correct_info.g_gain_, color_correct_info.b_gain_);
			}
			processer->setWbEnv(color_correct_info.awb_env_);
// 			processer->setGammaFactor(color_correct_info.gamma_factor_);
// 
// 			hue_adjust_back_ = processer->getHueAdjust();
// 			processer->setHueAdjust(video_clip_info.hue_adjust);

			rotation_type_ = processer->getRotationType();
			processer->setRotationType((HscRotationType)video_item.getRotationType());

			binning_mode_ = processer->GetBinningModeEnable();
			processer->SetBinningModeEnable(video_item.getBinningModeEnable());

			bpp_ = processer->GetBitsPerPixel();
			auto bpp = ((int)video_item.getValidBitsPerPixel() == 8) ? 8 : 16;
			processer->SetBitsPerPixel(bpp);
			valid_bpp = processer->GetSignificantBitsPerPixel();
			auto valid_bpp = (int)video_item.getValidBitsPerPixel();
			processer->SetSignificantBitsPerPixel(valid_bpp);
		}
	}

	// [2022/9/5 rgq]: 为了在回放预览切换视频时不回滚，特加此函数，需要回滚的不能此函数
	void setRotationType(HscRotationType type)
	{
		rotation_type_ = type;
	}
	// 同上
	void setColorCorrectInfo(HscColorCorrectInfo info)
	{
		info_back_ = info;
	}
	// 同上
	void setDisplayMode(HscDisplayMode info)
	{
		display_mode_ = info;
	}

	~PreExportColorCorrectRollback()
	{
		if (processer_ && need_rollback_)
		{
			processer_->SetDisplayMode(display_mode_);

			processer_->setWhiteBalanceMode(info_back_.awb_mode_);
			if (info_back_.awb_mode_ == HscWhiteBalanceMode::HSC_WB_MANUAL_GAIN) {
				processer_->setManualGain(info_back_.r_gain_, info_back_.g_gain_, info_back_.b_gain_);
			}
			processer_->setWbEnv(info_back_.awb_env_);
// 			processer_->setGammaFactor(info_back_.gamma_factor_);
// 
// 			processer_->setHueAdjust(hue_adjust_back_);

			processer_->setRotationType(rotation_type_);
			processer_->SetBitsPerPixel(bpp_);
			processer_->SetSignificantBitsPerPixel(valid_bpp);
		}
	}

private:
	std::shared_ptr<ImageProcessor> processer_;
	HscColorCorrectInfo info_back_;
	// 	HueAdjust hue_adjust_back_;
	HscRotationType rotation_type_{ HSC_ROTATION_NONE };
	HscDisplayMode display_mode_{ HSC_DISPLAY_MONO };

	bool binning_mode_ = false;
	int bpp_ = 8;
	int valid_bpp = 8;
	bool need_rollback_ = true;
};
#endif // PREEXPORTCOLORCORRECTROLLBACK_H