#include "imageprocessor.h"

#include "AGBufferProcessor.h"
#include "Device/device.h"

ImageProcessor::ImageProcessor(Device *device_ptr) : device_ptr_(device_ptr)
{

}

int ImageProcessor::GetDisplayWidthMax() const
{
    auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
    if (processor_ptr)
    {
        return processor_ptr->GetDisplayWidthMax();
    }

    return 0;
}

int ImageProcessor::GetDisplayHeightMax() const
{
    auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
    if (processor_ptr)
    {
        return processor_ptr->GetDisplayHeightMax();
    }

    return 0;
}

cv::Mat ImageProcessor::cv_process(CAGBuffer *buffer_ptr, int process_type, bool bpp_enabled)
{
    auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
    if (processor_ptr)
    {
        return processor_ptr->cv_process(buffer_ptr, HscBufferProcessType(process_type), bpp_enabled);
    }

    return cv::Mat();
}

cv::Mat ImageProcessor::cv_process(const cv::Mat &mat, int contrast, int luminance, bool anti_color)
{
    auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
    if (processor_ptr)
    {
        return processor_ptr->cv_process(mat, contrast, luminance, anti_color);
    }

    return cv::Mat();
}

cv::Mat ImageProcessor::cv_process_no_copy(CAGBuffer *buffer_ptr, int process_type, bool bpp_enabled)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->cv_process_no_copy(buffer_ptr, HscBufferProcessType(process_type), bpp_enabled);
	}

	return cv::Mat();
}

void ImageProcessor::SetRoi(QRect roi)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		CameraWindowRect rect;
		rect.x = roi.x();
		rect.y = roi.y();
		rect.width = roi.width();
		rect.height = roi.height();
		processor_ptr->SetROI(rect);
	}
}

void ImageProcessor::SetDisplayMode(HscDisplayMode mode)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->SetDisplayMode(mode);
	}
}

HscDisplayMode ImageProcessor::GetDisplayMode() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->GetDisplayMode();
	}
	return HSC_DISPLAY_MONO;
}

void ImageProcessor::setColorCorrectInfo(const HscColorCorrectInfo & info)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->setColorCorrectInfo(info);
	}
}

HscColorCorrectInfo ImageProcessor::getColorCorrectInfo()
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->getColorCorrectInfo();
	}
	return HscColorCorrectInfo();
}

void ImageProcessor::setRotationType(HscRotationType rotation_type)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->setRotationType(rotation_type);
	}
}

HscRotationType ImageProcessor::getRotationType() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->getRotationType();
	}
	return HSC_ROTATION_NONE;
}

void ImageProcessor::setWhiteBalanceMode(HscWhiteBalanceMode wbMode)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->SetWhiteBalanceMode(wbMode);
	}
}

HscWhiteBalanceMode ImageProcessor::getWhiteBalanceMode() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->GetWhiteBalanceMode();
	}
	return HSC_WB_NONE;
}

void ImageProcessor::setManualGain(float r, float g, float b)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->setManualGain(r, g, b);
	}
}

void ImageProcessor::getManualGain(float& r, float& g, float& b)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->getManualGain(r, g, b);
	}

}

HScWbEnv ImageProcessor::setWbEnv(HScWbEnv env)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->setWbEnv(env);
	}
	return HSC_WB_ENV_LIGHT;
}

HScWbEnv ImageProcessor::getWbEnv()
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->getWbEnv();
	}
	return HSC_WB_ENV_LIGHT;
}

uint8_t ImageProcessor::GetBitsPerPixel() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->GetBitsPerPixel();
	}
	return 8;
}
int ImageProcessor::GetSignificantBitsPerPixel() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->GetSignificantBitsPerPixel();
	}
	return 8;

}

void ImageProcessor::SetSignificantBitsPerPixel(uint8_t bpp)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->SetSignificantBitsPerPixel(bpp);
	}
}

void ImageProcessor::SetBinningModeEnable(bool enable)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->SetBinningMode(enable);
	}
}

bool ImageProcessor::GetBinningModeEnable() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->GetBinningMode();
	}
	return false;
}

void ImageProcessor::SetBitsPerPixel(uint8_t bpp)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->SetBitsPerPixel(bpp);
	}
}

void ImageProcessor::setGammaFactor(float gamma)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->setGammaFactor(gamma);
	}
}

float ImageProcessor::getGammaFactor() const
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		return processor_ptr->getGammaFactor();
	}
	return 0.0;
}

void ImageProcessor::setFrameHeadType(uint8_t type)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->setFrameHeadType(type);
	}
}


void ImageProcessor::setFilerEnable(uint8_t filter_enable)
{
	auto processor_ptr = CAGBufferProcessor::GetProcessor(device_ptr_->device_handle_);
	if (processor_ptr)
	{
		processor_ptr->setFilerEnable(filter_enable);
	}
}