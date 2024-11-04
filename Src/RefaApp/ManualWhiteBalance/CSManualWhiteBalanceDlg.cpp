#include "CSManualWhiteBalanceDlg.h"
#include "ui_CSManualWhiteBalanceDlg.h"
#include <QDebug>
#include "ImageNS/ImageNS.h"
#define USE_OPENGL 0

uint32_t getFrameDataLen(FrameHead* hscHeader) {
	if (hscHeader == nullptr) {
		return 0;
	}
	if (hscHeader->frame_size != 0) {
		return (uint32_t)hscHeader->frame_size - sizeof(FrameHead);
	}
	auto frameFormat = (AGFrameColor)hscHeader->format;
	switch (frameFormat) {
	case AGFrameColor::RGGB:
	case AGFrameColor::BGGR:
	case AGFrameColor::GRBG:
	case AGFrameColor::GBRG:
	case AGFrameColor::Raw:
		return (uint32_t)(hscHeader->rect.width * hscHeader->rect.height * 1);
	case AGFrameColor::NV21:
		return (uint32_t)(hscHeader->rect.width * hscHeader->rect.height * 1.5);
	case AGFrameColor::I420:
		return (uint32_t)(hscHeader->rect.width * hscHeader->rect.height * 1.5);
	case AGFrameColor::BGR24:
		return (uint32_t)(hscHeader->rect.width * hscHeader->rect.height * 3);
	default:
		return 0;
	}
}


// uint64_t HSCTimeStamp2Microseconds(FrameTimestamp* hscTimeStamp) {
// 	if (hscTimeStamp == nullptr) {
// 		return 0;
// 	}
// 
// 	tm tmHSC = sTmYear;
// 	tmHSC.tm_yday = hscTimeStamp->mDays;
// 	tmHSC.tm_hour = hscTimeStamp->mHours;
// 	tmHSC.tm_min = hscTimeStamp->mMins;
// 	tmHSC.tm_sec = hscTimeStamp->mSeconds;
// 	auto microseconds = ntohl(hscTimeStamp->mMicroseconds);
// 	auto ret = (uint64_t)mktime(&tmHSC) * 1000000 + microseconds;
// 	return ret;
// }

bool CSFrame2AGFrame(CAGBuffer* hscFrame, AGFrame* agFrame) {
	if (hscFrame == nullptr || agFrame == nullptr) {
        return false;
	}
	auto hscHeader = &(hscFrame->frame_head);
	// LOG_I("%s, hscHeader->frameno=[%d].", __CLASS_FUNC__, hscHeader->frameno);
	uint32_t hscDataLen = getFrameDataLen(hscHeader);
	auto hscData = hscFrame->img;
	//auto timestamp = HSCTimeStamp2Microseconds((FrameTimestamp*)(&hscHeader->time_stamp));

	auto agHeader = &(agFrame->mHeader);
	agHeader->mColor = ((AGFrameColor)hscHeader->format);
	//agHeader->mReserved = 0;
	agHeader->mPitch = hscHeader->rect.width;		// pitch
	agHeader->mWidth = hscHeader->rect.width;
	agHeader->mHeight = hscHeader->rect.height;
	agHeader->mLength = (uint32_t)hscDataLen;
	agHeader->mIndex = hscHeader->frameno;
	//agHeader->mTimestamp = timestamp;
	agHeader->mExtent = 0;
	agFrame->mData = hscData;
	return true;
}

static double u16ToDouble(uint8_t type, uint8_t integer, uint8_t decimal, uint16_t fix)
{
	double value = 0.0;
	uint16_t temp = fix& ((((uint16_t)(1) << (type + integer + decimal)) - 1));
	if (0 == fix)
		value = 0.0;
	if (temp& (((uint16_t)(1) << (integer + decimal))))
		value = (double)(((uint16_t)(1) << (/*type +*/ integer + decimal)) - temp) / (double)((uint16_t)(1) << decimal);
	else
		value = (double)((double)temp / (double)((uint16_t)1 << decimal));
	return value;
}

float CSManualWhiteBalanceDlg::changeU2F(uint16_t fix)
{
	float f = (float)u16ToDouble(1, 7, 8, fix);
	return f;
}

static uint16_t doubleToU16(uint8_t type, uint8_t integer, uint8_t decimal, double dVal)
{
	uint16_t temp = 0;
	uint16_t dst = 0;
	double dTemp = 0.0;
	dTemp = dVal;
	if (dTemp < 0)
	{
		temp = (uint16_t)(-dVal*(1 << (decimal)));
		dst = (temp | 0x8000);
	}
	else if (dTemp > 0)
		dst = (uint16_t)(dVal*(1 << decimal));
	else
		dst = 0;
	return dst;
}

uint16_t CSManualWhiteBalanceDlg::changeF2U(float fix)
{
	uint16_t u = (uint16_t)doubleToU16(1, 7, 8, fix);
	return u;
}

CSManualWhiteBalanceDlg::CSManualWhiteBalanceDlg(QSharedPointer<Device> device_ptr, QWidget *parent)
	: QDialog(parent)
{
#if USE_OPENGL
	m_processorHandle = AGProcessorCreate(AGProcessorType::ProcessorGL);
#else
	m_processorHandle = AGProcessorCreate(AGProcessorType::ProcessorCPU);
#endif
	m_pColorCorrectParam.mEffect.mEnable = true;
	m_pColorCorrectParam.mEffect.mEnableGain = true;
	m_pColorCorrectParam.mEffect.mEnableCCM = false;
	m_pColorCorrectParam.mEffect.mEnableHue = false;
	m_pColorCorrectParam.mEffect.mEnableGamma = false;
	m_pColorCorrectParam.mEffect.mEnableBrightness = false;
	m_pColorCorrectParam.mEffect.mEnableContrast = false;
	m_pColorCorrectParam.mEffect.mEnableSaturation = false;
	m_pColorCorrectParam.mEffect.mGreyOnly = false;
	m_pColorCorrectParam.mEffect.mEnableMedianBlur = false;
	m_pColorCorrectParam.mEffect.mEnablePseudoColor = false;
	m_pColorCorrectParam.mEffect.mInverseColor = false;
	m_pColorCorrectParam.mEffect.mRGBGain.mR = 1.0;
	m_pColorCorrectParam.mEffect.mRGBGain.mG = 1.0;
	m_pColorCorrectParam.mEffect.mRGBGain.mB = 1.0;
	m_pColorCorrectParam.mEffect.mRGBHue.mR = 1.0;
	m_pColorCorrectParam.mEffect.mRGBHue.mG = 1.0;
	m_pColorCorrectParam.mEffect.mRGBHue.mB = 1.0;
#if USE_OPENGL
	m_pColorCorrectParam.mEffect.mGammma = 0.9;
#else
	m_pColorCorrectParam.mEffect.mGammma = 0.75;
#endif
	m_pColorCorrectParam.mOutputColor = AGFrameColor::BGRA;

	ui = new Ui::CSManualWhiteBalanceDlg();
	ui->setupUi(this);
	setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
	setWindowTitle(tr("Setting Manual parameter"));
	setWindowState(Qt::WindowMaximized);
	if (!device_ptr.isNull())
	{
		m_device_ptr = device_ptr;
		connect(m_device_ptr.data(), &Device::updateWhiteBalanceFrame, this, &CSManualWhiteBalanceDlg::updateWhiteBalanceFrame);
		m_device_ptr->setManualWhiteBalance(true);
	}
	initUi();
}

CSManualWhiteBalanceDlg::~CSManualWhiteBalanceDlg()
{
	if (!m_device_ptr.isNull())
	{
		m_device_ptr->setManualWhiteBalance(false);
		disconnect(m_device_ptr.data(), &Device::updateWhiteBalanceFrame, this, &CSManualWhiteBalanceDlg::updateWhiteBalanceFrame);
	}
	if (m_pTempFrame)
	{
		delete m_pTempFrame;
		m_pTempFrame = nullptr;
	}
	delete ui;
}

void CSManualWhiteBalanceDlg::initUi()
{
	ROTATE_TYPE nRotate = ROTATE_0;
	if (m_device_ptr)
	{
		nRotate = (ROTATE_TYPE)m_device_ptr->getProperty(Device::PropRotateType).toInt();
	}
	m_pOriginalPlayer = new CPlayerViewBase(INDEX_MANUALWHITEBALANCE_PLAYER, 1, ui->widget_original);
	if (m_pOriginalPlayer)
	{
		m_pOriginalPlayer->setBackgroundImage(QImage(":/image/image/videobackground.png"));
		if (nRotate != ROTATE_0)
		{
			m_pOriginalPlayer->SetRotateType(nRotate);
		}
		m_pOriginalPlayer->showBackgroundImage(false);
		m_pOriginalPlayer->FitView(true);
		connect(m_pOriginalPlayer, &CPlayerViewBase::updateRoiInfoSignal, this, &CSManualWhiteBalanceDlg::SlotChangeRoi);
	}
	QHBoxLayout* displayLayout = new QHBoxLayout(ui->widget_original);
	ui->widget_original->setLayout(displayLayout);
	displayLayout->addWidget(m_pOriginalPlayer);
	m_pProcessedPlayer = new CPlayerViewBase(INDEX_MANUALWHITEBALANCE_PLAYER, 1, ui->widget_processed);
	if (m_pProcessedPlayer)
	{
		m_pProcessedPlayer->setBackgroundImage(QImage(":/image/image/videobackground.png"));
		if (nRotate != ROTATE_0)
		{
			m_pProcessedPlayer->SetRotateType(nRotate);
		}
		m_pProcessedPlayer->showBackgroundImage(false);
		m_pProcessedPlayer->FitView(true);
	}
	QHBoxLayout* displayLayout2 = new QHBoxLayout(ui->widget_processed);
	ui->widget_processed->setLayout(displayLayout2);
	displayLayout2->addWidget(m_pProcessedPlayer);
	
	int nValueR = constGainBase, nValueG = constGainBase, nValueB = constGainBase;
	if(m_device_ptr)
	{
		HscHisiManualAwb param;
		param.mode = AWB_MODE_MANUAL;
		HscResult res = m_device_ptr->GetManualWhiteBalance(param);
		if (res == HSC_OK)
		{
			nValueR = factorToPos(changeU2F(param.r_gain));
			nValueG = factorToPos(changeU2F(param.gr_gain));
			nValueB = factorToPos(changeU2F(param.b_gain));
			m_pColorCorrectParam.mEffect.mRGBGain.mR = posTofactor(nValueR);
			m_pColorCorrectParam.mEffect.mRGBGain.mG = posTofactor(nValueG);
			m_pColorCorrectParam.mEffect.mRGBGain.mB = posTofactor(nValueB);
		}
		HscColorCorrectInfo ColorInfo;
		res = m_device_ptr->getColorCorrectInfo(ColorInfo);
		if (res == HSC_OK)
		{
			m_pColorCorrectParam.mEffect.mEnableGamma = true;
			m_pColorCorrectParam.mEffect.mGammma = ColorInfo.gamma_factor_;
			m_pColorCorrectParam.mEffect.mEnableCCM = true;
			for (int i = 0; i < 9; i++)
			{
				m_pColorCorrectParam.mEffect.mCCMMatrix[i] = ColorInfo.ccm_matrix_[i / 3][i % 3];
			}
		}
	}
	ui->Slider_R->setValue(nValueR);
	ui->Slider_G->setValue(nValueG);
	ui->Slider_B->setValue(nValueB);
	ui->spinBox_init_R->setValue(nValueR);
	ui->spinBox_init_G->setValue(nValueG);
	ui->spinBox_init_B->setValue(nValueB);
	ui->spinBox_init_R->setEnabled(false);
	ui->spinBox_init_G->setEnabled(false);
	ui->spinBox_init_B->setEnabled(false);
	bool bEnable = false;
	if (m_device_ptr)
	{
		if (m_device_ptr->isSupportManualWhiteBalanceAutoMode())
		{
			bEnable = true;
		}
	}
	ui->pushButton_auto_param->setEnabled(bEnable);
}

void CSManualWhiteBalanceDlg::on_Slider_R_valueChanged(int value)
{
	//应用到界面
	ui->spinBox_current_R->setValue(value);
	m_pColorCorrectParam.mEffect.mRGBGain.mR = posTofactor(value);
}

void CSManualWhiteBalanceDlg::on_Slider_G_valueChanged(int value)
{
	ui->spinBox_current_G->setValue(value);
	m_pColorCorrectParam.mEffect.mRGBGain.mG = posTofactor(value);
}

void CSManualWhiteBalanceDlg::on_Slider_B_valueChanged(int value)
{
	ui->spinBox_current_B->setValue(value);
	m_pColorCorrectParam.mEffect.mRGBGain.mB = posTofactor(value);
}

void CSManualWhiteBalanceDlg::on_spinBox_current_R_valueChanged(int value)
{
	if (value == ui->Slider_R->value())
	{
		return;
	}
	ui->Slider_R->setValue(value);
}

void CSManualWhiteBalanceDlg::on_spinBox_current_G_valueChanged(int value)
{
	if (value == ui->Slider_G->value())
	{
		return;
	}
	ui->Slider_G->setValue(value);
}

void CSManualWhiteBalanceDlg::on_spinBox_current_B_valueChanged(int value)
{
	if (value == ui->Slider_B->value())
	{
		return;
	}
	ui->Slider_B->setValue(value);
}

void CSManualWhiteBalanceDlg::on_spinBox_startX_editingFinished()
{
	int uiValue = ui->spinBox_startX->value();
	int nWidth = m_rcArea.width();
	m_rcArea.setX(uiValue);
	if (uiValue + nWidth > m_sizeImage.width())
	{
		nWidth = m_sizeImage.width() - uiValue;
		ui->spinBox_width->setValue(nWidth);
	}
	m_rcArea.setWidth(nWidth);
	setManualCalArea();
}

void CSManualWhiteBalanceDlg::on_spinBox_startY_editingFinished()
{
	int uiValue = ui->spinBox_startY->value();
	int nHeight = m_rcArea.height();
	m_rcArea.setY(uiValue);
	if (uiValue + nHeight > m_sizeImage.height())
	{
		nHeight = m_sizeImage.height() - uiValue;
		ui->spinBox_height->setValue(nHeight);
	}
	m_rcArea.setHeight(nHeight);
	setManualCalArea();
}

void CSManualWhiteBalanceDlg::on_spinBox_width_editingFinished()
{
	int uiValue = ui->spinBox_width->value();
	int nX = m_rcArea.x();
	if (uiValue + nX > m_sizeImage.width())
	{
		uiValue = m_sizeImage.width() - nX;
		ui->spinBox_width->setValue(uiValue);
	}
	m_rcArea.setWidth(uiValue);
	setManualCalArea();
}

void CSManualWhiteBalanceDlg::on_spinBox_height_editingFinished()
{
	int uiValue = ui->spinBox_height->value();
	int nY = m_rcArea.y();
	if (uiValue + nY > m_sizeImage.height())
	{
		uiValue = m_sizeImage.height() - nY;
		ui->spinBox_height->setValue(uiValue);
	}
	m_rcArea.setHeight(uiValue);
	setManualCalArea();
}

void CSManualWhiteBalanceDlg::on_pushButton_calculate_clicked()
{
	if (!m_pLastFrame)
	{
		return;
	}

	TCSColorChannel colorChannel = TCSColorChannel(m_pLastFrame->frame_head.format);
	uint16_t sensor_roi_y = m_pLastFrame->frame_head.rect.y;
	int height = m_pLastFrame->frame_head.rect.height;
	int width = m_pLastFrame->frame_head.rect.width;
	if (height == 0 || width == 0)
	{
		return;
	}

	cv::Mat matDstOri;
	matDstOri.create(height, width, CV_8UC3);
	Demosaic24bit_change(matDstOri.ptr(), m_pLastFrame->img, m_pLastFrame->frame_head.rect.x, m_pLastFrame->frame_head.rect.y, width, height, colorChannel);
	
	AGFrame lastAGFrame;
	uint32_t hscDataLen = getFrameDataLen(&m_pLastFrame->frame_head);

	auto agHeader = &(lastAGFrame.mHeader);
	agHeader->mColor = AGFrameColor::BGR24;
	//agHeader->mReserved = 0;
	agHeader->mPitch = m_pLastFrame->frame_head.rect.width;		// pitch
	agHeader->mWidth = m_pLastFrame->frame_head.rect.width;
	agHeader->mHeight = m_pLastFrame->frame_head.rect.height;
	agHeader->mLength = (uint32_t)hscDataLen;
	agHeader->mIndex = m_pLastFrame->frame_head.frameno;
	//agHeader->mTimestamp = timestamp;
	agHeader->mExtent = 0;
	lastAGFrame.mData = matDstOri.ptr();

	AGRect refRect;
	refRect.mLeft = m_rcArea.left();
	refRect.mTop = m_rcArea.top();
	refRect.mRight = refRect.mLeft + m_rcArea.width();
	refRect.mBottom = refRect.mTop + m_rcArea.height();

	AGRGBFloat balanceParam;
	AGErrorCode res = AGProcessorGetWB(m_processorHandle, &lastAGFrame, &refRect, &balanceParam);
	if (res == AGErrorCode::AG_OK)
	{
		ui->Slider_R->setValue(factorToPos(balanceParam.mR));
		ui->Slider_G->setValue(factorToPos(balanceParam.mG));
		ui->Slider_B->setValue(factorToPos(balanceParam.mB));
	}
}

void CSManualWhiteBalanceDlg::on_pushButton_resetting_clicked()
{
	ui->Slider_R->setValue(ui->spinBox_init_R->value());
	ui->Slider_G->setValue(ui->spinBox_init_G->value());
	ui->Slider_B->setValue(ui->spinBox_init_B->value());
}

void CSManualWhiteBalanceDlg::on_pushButton_auto_param_clicked()
{
	if (m_device_ptr)
	{
		HscHisiManualAwb param;
		param.mode = AWB_MODE_AUTO;
		HscResult res = m_device_ptr->GetManualWhiteBalance(param);
		if (res == HSC_OK)
		{
			int nValueR = constGainBase, nValueG = constGainBase, nValueB = constGainBase;
			nValueR = factorToPos(changeU2F(param.r_gain));
			nValueG = factorToPos(changeU2F(param.gr_gain));
			nValueB = factorToPos(changeU2F(param.b_gain));
			m_pColorCorrectParam.mEffect.mRGBGain.mR = posTofactor(nValueR);
			m_pColorCorrectParam.mEffect.mRGBGain.mG = posTofactor(nValueG);
			m_pColorCorrectParam.mEffect.mRGBGain.mB = posTofactor(nValueB);
			ui->Slider_R->setValue(nValueR);
			ui->Slider_G->setValue(nValueG);
			ui->Slider_B->setValue(nValueB);
		}
	}
}

void CSManualWhiteBalanceDlg::on_pushButton_ok_clicked()
{
	if (m_device_ptr)
	{
		HscHisiManualAwb param;
		param.mode = AWB_UPDATE_MANUAL_PARAM;
		param.r_gain = changeF2U(posTofactor(ui->spinBox_current_R->value()));
		param.gr_gain = changeF2U(posTofactor(ui->spinBox_current_G->value()));
		param.gb_gain = changeF2U(posTofactor(ui->spinBox_current_G->value()));
		param.b_gain = changeF2U(posTofactor(ui->spinBox_current_B->value()));
		HscResult res = m_device_ptr->SetManualWhiteBalance(param);
		if (res == HSC_OK)
		{
			//应用参数
			if (m_device_ptr->isGetParamsFromDevice())
			{
				HscColorCorrectInfo info;
				if (m_device_ptr->getColorCorrectInfo(info) != HSC_OK)
				{
					return;
				}
				info.awb_mode_ = HSC_WB_MANUAL_GAIN;
				info.r_gain_ = changeU2F(param.r_gain);
				info.g_gain_ = changeU2F(param.gr_gain);
				info.b_gain_ = changeU2F(param.b_gain);
				if (m_device_ptr->setColorCorrectInfo(info) != HSC_OK)
				{
					return;
				}
			}
		}
	}
}

void CSManualWhiteBalanceDlg::on_pushButton_cancel_clicked()
{
	close();
}

void CSManualWhiteBalanceDlg::updateWhiteBalanceFrame(CAGBuffer * buffer_ptr, RccFrameInfo Info)
{
	if (buffer_ptr)
	{
		auto bpp = buffer_ptr->frame_head.bpp;
		if (bpp == 12 || bpp == 10)
		{
			if (!m_pTempFrame)
			{
				m_pTempFrame = new CAGBuffer();
			}
			m_pTempFrame->frame_head = buffer_ptr->frame_head;
			memcpy(m_pTempFrame->img, buffer_ptr->img,/* getFrameDataLen(&m_pTempFrame->frame_head)*/ImgBufferSize());
			
			//转换位深到8位
			cv::Mat temp(m_pTempFrame->frame_head.rect.height, m_pTempFrame->frame_head.rect.width, CV_16UC1, m_pTempFrame->img);

			if (bpp == 12)
			{
				temp *= (255.0/4095);
				//ImageNS::Grey16_8(m_pTempFrame->img, buffer_ptr->img, buffer_ptr->frame_head.rect.width, buffer_ptr->frame_head.rect.height);
			}
			else if ( bpp == 10 )
			{
				temp *= (255.0/1023);
				//ImageNS::Grey16_8(m_pTempFrame->img, buffer_ptr->img, buffer_ptr->frame_head.rect.width, buffer_ptr->frame_head.rect.height);
			}
			ImageNS::Grey16_8_t(m_pTempFrame->img, buffer_ptr->img, buffer_ptr->frame_head.rect.width, buffer_ptr->frame_head.rect.height);

		}
		cv::Mat oriImage, processImage;
		if (GetImageAndProcess(buffer_ptr, oriImage, processImage))
		{
			QImage qt_oriimage, qt_processImage;
			CPlayerViewBase::cvMat2QImage(oriImage, qt_oriimage);
			CPlayerViewBase::cvMat2QImage(processImage, qt_processImage);

			RccFrameInfo frameInfo{};
			frameInfo = Info;
			frameInfo.image = qt_oriimage;
			m_pOriginalPlayer->SlotUpdateImage(frameInfo);
			if (!m_rcArea.isValid())
			{
				int nWidth = frameInfo.image.width();
				int nHeight = frameInfo.image.height();
				m_sizeImage = QSize(nWidth, nHeight);

				m_rcArea = QRect(nWidth / 4, nHeight / 4, nWidth / 2, nHeight / 2);
				RoiInfo roi_info;
				roi_info.roi_type = Device::RoiTypes::kManualWhiteBalance;
				roi_info.roi_rect = m_rcArea;
				roi_info.roi_color = Qt::red;
				m_pOriginalPlayer->setRoiVisible(Device::RoiTypes::kManualWhiteBalance, true);
				m_pOriginalPlayer->drawRoiRect(roi_info, false);
				m_pOriginalPlayer->setFeaturePointVisible(Device::kManualWhiteBalance, true);
				ui->spinBox_startX->setRange(0, nWidth - m_nAreaMIN);
				ui->spinBox_startY->setRange(0, nHeight - m_nAreaMIN);
				ui->spinBox_width->setRange(m_nAreaMIN, nWidth);
				ui->spinBox_height->setRange(m_nAreaMIN, nHeight);
				updateManualCalArea();
			}
			RccFrameInfo frameInfoProcess{};
			frameInfoProcess = Info;
			frameInfoProcess.image = qt_processImage;
			m_pProcessedPlayer->SlotUpdateImage(frameInfoProcess);

			m_pLastFrame = buffer_ptr;
		}
	}
}

bool CSManualWhiteBalanceDlg::GetImageAndProcess(CAGBuffer * pBufRaw, cv::Mat &oriImage, cv::Mat &processedImage)
{
	if (!pBufRaw)
	{
		return false;
	}

	TCSColorChannel colorChannel = TCSColorChannel(pBufRaw->frame_head.format);
	uint16_t sensor_roi_y = pBufRaw->frame_head.rect.y;
	int height = pBufRaw->frame_head.rect.height;
	int width = pBufRaw->frame_head.rect.width;
	if (height == 0 || width == 0)
	{
		return false;
	}

	cv::Mat matDstOri;
	matDstOri.create(height, width, CV_8UC3);
	Demosaic24bit_change(matDstOri.ptr(), pBufRaw->img, pBufRaw->frame_head.rect.x, pBufRaw->frame_head.rect.y, width, height, colorChannel);
	oriImage = matDstOri;

	if (!CSFrame2AGFrame(pBufRaw, &m_pProAGFrame))
	{
		return false;
	}
	AGColorCorrectParam correctParam = m_pColorCorrectParam;

#if USE_OPENGL
#else
	switch (m_pColorCorrectParam.mOutputColor)
	{
	case AGFrameColor::BGRA:
	{
		auto frameFormat = (AGFrameColor)pBufRaw->frame_head.format;
		switch (frameFormat) {
		case AGFrameColor::GRBG:
		case AGFrameColor::GBRG:
		{
			correctParam.mEffect.mRGBGain.mB = m_pColorCorrectParam.mEffect.mRGBGain.mR;
			correctParam.mEffect.mRGBGain.mR = m_pColorCorrectParam.mEffect.mRGBGain.mB;
			break;
		}
		case AGFrameColor::BGGR:
		case AGFrameColor::RGGB:
		case AGFrameColor::Raw:
		default:
			break;
		}
	}
		break;
	default:
		break;
	}
#endif
	auto pFrame = AGProcessorColorCorrect(m_processorHandle, &m_pProAGFrame, &correctParam);
	if (!pFrame)
	{
		return false;
	}
	processedImage = cv::Mat(pFrame->mHeader.mHeight, pFrame->mHeader.mWidth, CV_8UC4, pFrame->mData).clone();
	if (processedImage.empty())
	{
		return false;
	}

	return true;
}

void CSManualWhiteBalanceDlg::Demosaic24bit_change(BYTE *pRGB, BYTE* pRaw, int X, int Y, int width, int height, TCSColorChannel colorChannel)
{
	cv::Mat matSrc(height, width, CV_8UC1, pRaw);
	int nType = CV_8UC3;
	if (colorChannel == COLOR_CHANNEL_RGBA || colorChannel == COLOR_CHANNEL_BGRA)
	{
		nType = CV_8UC4;
	}
	cv::Mat matDst(height, width, nType, pRGB);
	switch (colorChannel)
	{
	case COLOR_CHANNEL_RGGB:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerBG2BGR);
		break;
	case COLOR_CHANNEL_BGGR:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerRG2BGR);
		break;
	case COLOR_CHANNEL_GRBG:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerGB2BGR);
		break;
	case COLOR_CHANNEL_GBRG:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerGR2BGR);
		break;
	case COLOR_CHANNEL_RGBA:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerGB2BGR);
		break;
	case COLOR_CHANNEL_BGRA:
		cv::demosaicing(matSrc, matDst, cv::COLOR_BayerGR2BGR);
		break;
	default:
		break;
	}
}

void CSManualWhiteBalanceDlg::SlotChangeRoi(Device::RoiTypes type, const QRect & rect)
{
	if (Device::RoiTypes::kManualWhiteBalance != type)
		return;

	if (m_rcArea.isValid())
	{
		if (m_rcArea == rect)
		{
			return;
		}
	}
	m_rcArea = rect;
	if (m_rcArea.width() < m_nAreaMIN)
	{
		m_rcArea.setWidth(m_nAreaMIN);
	}
	if (m_rcArea.height() < m_nAreaMIN)
	{
		m_rcArea.setHeight(m_nAreaMIN);
	}
	if (m_rcArea.x() > m_sizeImage.width()- m_nAreaMIN)
	{
		m_rcArea.setX(m_sizeImage.width() - m_nAreaMIN);
	}
	if (m_rcArea.x() < 0)
	{
		m_rcArea.setX(0);
	}
	if (m_rcArea.y() > m_sizeImage.height() - m_nAreaMIN)
	{
		m_rcArea.setY(m_sizeImage.height() - m_nAreaMIN);
	}
	if (m_rcArea.y() < 0)
	{
		m_rcArea.setY(0);
	}
	if (m_rcArea.x() + m_rcArea.width() > m_sizeImage.width())
	{
		m_rcArea.setWidth(m_sizeImage.width() - m_rcArea.x());
	}
	if (m_rcArea.y() + m_rcArea.height() > m_sizeImage.height())
	{
		m_rcArea.setHeight(m_sizeImage.height() - m_rcArea.y());
	}
	setManualCalArea();
	updateManualCalArea();
}

void CSManualWhiteBalanceDlg::updateManualCalArea()
{
	ui->spinBox_startX->setValue(m_rcArea.x());
	ui->spinBox_startY->setValue(m_rcArea.y());
	ui->spinBox_width->setValue(m_rcArea.width());
	ui->spinBox_height->setValue(m_rcArea.height());
}

void CSManualWhiteBalanceDlg::setManualCalArea()
{
	if (m_rcArea.isValid())
	{
		if (m_pOriginalPlayer)
		{
			RoiInfo roi_info;
			roi_info.roi_type = Device::RoiTypes::kManualWhiteBalance;
			roi_info.roi_rect = m_rcArea;
			roi_info.roi_color = Qt::red;
			m_pOriginalPlayer->setRoiVisible(Device::RoiTypes::kManualWhiteBalance, true);
			m_pOriginalPlayer->drawRoiRect(roi_info, false);
		}
	}
}

float CSManualWhiteBalanceDlg::posTofactor(int pos)
{
	float factor = 0.0;
	factor = (float)(pos+1) / constGainBase;
	if (factor >= -constzMinPSINON && factor <= constzMinPSINON)
	{
		factor = constzMinPSINON;
	}
	return factor;
}


int CSManualWhiteBalanceDlg::factorToPos(float factor)
{
	int nValue = factor * constGainBase;
	if (nValue > 0)
	{
		nValue--;
	}
	return nValue;
}
