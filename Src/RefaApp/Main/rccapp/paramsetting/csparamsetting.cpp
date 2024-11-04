#include "csparamsetting.h"
#include "ui_csparamsetting.h"
#include <QToolTip>

int MAX_VALID_VALUE = 255;

CSParamSetting::CSParamSetting(int type, QSharedPointer<Device> device, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CSParamSetting)
	,current_device_(device)
	,m_iType(type)
{
    ui->setupUi(this);
	if (device->IsIntelligentTriggerV4Supported() || device->isSupportHighBitParam())
	{
		//MAX_VALID_VALUE = 4095;
		m_bDevicePixelBit = device->getProperty(Device::PropPixelBitDepth).toInt();
		MAX_VALID_VALUE = pow(2, m_bDevicePixelBit) - 1;
	}

    InitUI(); 
	ConnectSignalSlot();
}

CSParamSetting::~CSParamSetting()
{
    delete ui;
}

void CSParamSetting::SlotIntelligentAvgBright(const QString &ip, const int value)
{
	auto devicePtr = current_device_.lock();
	QString cur_ip = devicePtr->getIp();
	if (cur_ip != ip) return;
	if (m_iType = Device::PropIntelligentTrigger)
	{
		auto temp_val = value;
		if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported())
			&& !devicePtr->isGrabberDevice())//显示百分比
		{
			double percent =/* (m_current_area_mode == 1) ? (100 - Device::CountToPercentage(value, m_current_rect)) :*/ (Device::CountToPercentage(value, m_current_rect));
			ui->label_current_area_show->setText(QString("%1%").arg(QString::number(percent, 'f', 2)));
		}
		else//显示灰度值
		{
			if (devicePtr->IsIntelligentTriggerV4Supported() || devicePtr->isSupportHighBitParam())
			{
				temp_val = Device::ImageBitsChange(value, 16, m_bDevicePixelBit);
			}
			ui->AreaValueLabel->setText(QString::number(temp_val));
		}
	}
}

void CSParamSetting::InitUI()
{
	//设置弹框的尺寸
	this->setFixedSize(400, 300);
    switch (m_iType)
    {
	case Device::PropIntelligentTrigger:
	{

		auto devicePtr = current_device_.lock();
		if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported())
			&& !devicePtr->isGrabberDevice())
		{
			ShowCtrl(false);
		}
		else
		{
			ShowCtrl(true);
		}
       //设置智能触发弹框标题
       this->setWindowTitle(tr("Intelligent Trigger Parameters"));
       //隐藏控件
       ui->GrayModeLabel->setVisible(false);
       ui->comboBox->setVisible(false);
       ui->LightLabel->setText(tr("Light Threshold"));//亮度阈值
       ui->AreaExplainTextLabel->setText(tr("Brightness Value:"));//当前区域亮度值
       ui->ExplainTextLabel->setText(tr("Threshold exceeded,gather automatically."));//当超出当前设置的亮度阈值后将自动触发采集
	   ui->MaxTriAreaBtn->setToolTip(tr("Maximum Intelligent Trigger Area"));
	   Device::DrawTypeStatusInfo drawStatus = devicePtr->getDrawTypeStatusInfo();
	   if (drawStatus == Device::DTSI_Noraml || drawStatus == Device::DTSI_DrawIntelligentTrigger)
	   {
		   ui->ManualDrawBtn->setEnabled(true);
	   }
	   else
	   {
		   ui->ManualDrawBtn->setEnabled(false);
	   }
       break;
    }
	case Device::PropAutoExposure:
    {
		ShowCtrl(true);
        //设置自动曝光弹框标题
        this->setWindowTitle(tr("Auto Exposure Parameters"));
        ui->LightLabel->setText(tr("Target Gray Value"));//目标灰度值
        ui->AreaExplainTextLabel->setText(tr("Gray Value:"));//当前区域灰度值
		ui->ExplainTextLabel->setText(tr("The value is fixed in range Of 80%~120%."));//所设置的灰度值再80%~120%范围内不做任何调整
		ui->MaxTriAreaBtn->setToolTip(tr("Maximum Auto Exposure Trigger Area"));
		auto devicePtr = current_device_.lock();
		Device::DrawTypeStatusInfo drawStatus = devicePtr->getDrawTypeStatusInfo();
		if (drawStatus == Device::DTSI_Noraml || drawStatus == Device::DTSI_DrawAutoExposure)
		{
			ui->ManualDrawBtn->setEnabled(true);
		}
		else
		{
			ui->ManualDrawBtn->setEnabled(false);
		}
        break;
    }
    default:
        break;
    }


    //隐藏标题栏的问号和图标
    setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::Drawer);

    ui->mainWidget->setObjectName("mainWidget");
    ui->AreaGroupBox->setObjectName("commonLabel");
    ui->OriginCoordinateX->setObjectName("commonLabel");
	ui->OriginCoordinateX->setAlignment(Qt::AlignLeft);
    ui->OriginCoordinateY->setObjectName("commonLabel");
	ui->OriginCoordinateY->setAlignment(Qt::AlignLeft);
    ui->WidthLabel->setObjectName("commonLabel");
	ui->WidthLabel->setAlignment(Qt::AlignLeft);
    ui->HeightLabel->setObjectName("commonLabel");
	ui->HeightLabel->setAlignment(Qt::AlignLeft);
	ui->MaxTriAreaBtn->setObjectName("commonBtn");
	ui->AreaCenterBtn->setObjectName("commonBtn");
	ui->AreaDisplayBtn->setObjectName("commonBtn");
    ui->GrayModeLabel->setObjectName("commonLabel");
    ui->LightLabel->setObjectName("commonLabel");
    ui->AreaExplainTextLabel->setObjectName("commonLabel");
	ui->AreaExplainTextLabel->setAlignment(Qt::AlignLeft);
	ui->label_current_area_show->setObjectName("commonLabel");
    ui->ExplainTextLabel->setObjectName("commonLabel");
	ui->ExplainTextLabel->setAlignment(Qt::AlignLeft);
    ui->AreaValueLabel->setObjectName("commonLabel");
	ui->AreaValueLabel->setAlignment(Qt::AlignLeft);
	ui->Coordinate_X_lineEdit->setObjectName("commonLineEdit");
	ui->Coordinate_Y_lineEdit->setObjectName("commonLineEdit");
	ui->Width_lineEdit->setObjectName("commonLineEdit");
	ui->Height_lineEdit->setObjectName("commonLineEdit");
	ui->Slider_lineEdit->setObjectName("commonLineEdit");
	ui->lineEdit_number->setObjectName("commonLineEdit");
	QFont font;
	font.setFamily("Microsoft YaHei");
	font.setPixelSize(12);
	ui->ExplainTextLabel->setFont(font);
	ui->AreaExplainTextLabel->setFont(font);
	ui->AreaValueLabel->setFont(font);
	ui->ManualDrawBtn->setFont(font);
	ui->comBox_number_condition->setFont(font);
	ui->comboBox->setFont(font);
	//添加颜色按钮
	m_colorBtn = new ColorQPushButton(this);
	ui->horizontalLayout_4->addWidget(m_colorBtn);

	//滑块
	ui->horizontalSlider->setMinimum(0);    //最小值
	ui->horizontalSlider->setMaximum(MAX_VALID_VALUE);    //最大值
	ui->horizontalSlider->setSingleStep(1);    //步长

	ui->lineEdit_number->setRange(0, 100);

	//滑块变化响应编辑框
	QIntValidator* IntValidator = new QIntValidator;
	ui->Slider_lineEdit->setValidator(IntValidator);
	ui->Slider_lineEdit->setText(QString::number(0));

	QRegExp regx("[0-9]{1,9}");
	QRegExpValidator* validator = new QRegExpValidator(regx);
	ui->Coordinate_X_lineEdit->setValidator(validator);
	ui->Coordinate_Y_lineEdit->setValidator(validator);
	ui->Width_lineEdit->setValidator(validator);
	ui->Height_lineEdit->setValidator(validator);

	ui->Coordinate_X_lineEdit->setFocus();

	auto device_ptr = current_device_.lock();
	ui->AreaDisplayBtn->setCheckable(true);
	QRect rect{};
	bool roi_visible = false;
	uint32_t roi_color = qRgb(0, 0, 0);
	int slider_val{};
	if (device_ptr)
	{
		if (Device::PropIntelligentTrigger == m_iType)
		{
			if (device_ptr->isGrabberDevice()) {
				UpdateUI2Grabber(false);
			}

			HscIntelligentTriggerParamV2 param = device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			rect = Device::CameraWindowRect2QRect(param.roi);
			roi_color = param.roi_color;
			roi_visible = param.roi_visible == 1;
			slider_val = param.threshold;
			if (device_ptr->IsIntelligentTriggerV4Supported() || device_ptr->isSupportHighBitParam())
			{
				slider_val = Device::ImageBitsChange(param.threshold_new, 16, m_bDevicePixelBit);
			}

			if ((device_ptr->IsSupportCMOSDigitGain() || device_ptr->IsIntelligentTriggerV5Supported())
				&& !device_ptr->isGrabberDevice())
			{
				slider_val = param.pixel_active_thres;
				//int nModel = device_ptr->getProperty(Device::PropPixelTriggerMode).toInt();
				QVariantList vals;
				device_ptr->getSupportedProperties(Device::PropPixelTriggerMode, vals);
				int i = 0;
				int nSelected = 0;
				for (auto value :vals)
				{
					int nValue = value.toInt();
					if (nValue == param.trigger_type)
					{
						nSelected = i;
					}
					DeviceUtils::getPixelTriggerModel(nValue);
					ui->comBox_number_condition->addItem(DeviceUtils::getPixelTriggerModel(nValue));
					i++;
				}
				ui->comBox_number_condition->setCurrentIndex(nSelected);
				m_current_area_mode = param.trigger_type;//记录当前触发模式
				//int nCurrentValue = device_ptr->getProperty(Device::PropPixelTriggerCurrent).toInt();
				//ui->lineEdit_number->setText(QString::number(nCurrentValue));
				//ui->lineEdit_number->setText(QString::number(param.pixel_number_thres));
				double percent = (m_current_area_mode == 1) ? (100 - Device::CountToPercentage(param.pixel_number_thres, rect)) : (Device::CountToPercentage(param.pixel_number_thres, rect));
				ui->lineEdit_number->setValue(qRound(percent));
				//connect(device_ptr, &Device::signalCurrentAreaPixels, this, &CSParamSetting::SlotCurrentAreaPixels)
			}
		}
		else if (Device::PropAutoExposure == m_iType)
		{
			HscAutoExposureParameter param = device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			rect = Device::CameraWindowRect2QRect(param.autoExpArea);
			roi_color = param.roi_color;
			roi_visible = param.roi_visible == 1;
			QStringList gray_mode_list;
			QVariantList vals;
			device_ptr->getSupportedProperties(Device::PropAutoExposure, vals);
			for (const auto&e : vals)
			{
				gray_mode_list << DeviceUtils::getAutoExposureGrayModeText(e.toInt());

			}
			ui->comboBox->addItems(gray_mode_list);
			ui->comboBox->setCurrentIndex(param.autoExpGrayMode);
			slider_val = param.targetGray;
			if ((device_ptr->IsSupportCMOSDigitGain() || device_ptr->IsIntelligentTriggerV5Supported())
				&& !device_ptr->isGrabberDevice()) slider_val = param.targetGrayNew;
			if (device_ptr->IsIntelligentTriggerV4Supported() || device_ptr->isSupportHighBitParam())
			{
				slider_val = Device::ImageBitsChange(param.targetGrayNew, 16, m_bDevicePixelBit);
			}
		}
	}


	ui->Coordinate_X_lineEdit->setText(QString::number(rect.x()));
	ui->Coordinate_Y_lineEdit->setText(QString::number(rect.y()));
	ui->Width_lineEdit->setText(QString::number(rect.width()));
	ui->Height_lineEdit->setText(QString::number(rect.height()));
	m_colorBtn->SetColor(QColor{ roi_color });
	ui->horizontalSlider->setValue(slider_val);
	ui->AreaDisplayBtn->setChecked(roi_visible);

	ui->Slider_lineEdit->setText(QString::number((slider_val>MAX_VALID_VALUE ? MAX_VALID_VALUE : slider_val)));

	ui->AreaValueLabel->setText(QString::number(0));
}

void CSParamSetting::ShowCtrl(bool bNoraml)
{
	ui->AreaExplainTextLabel->setVisible(bNoraml);
	ui->ExplainTextLabel->setVisible(bNoraml);
	ui->AreaValueLabel->setVisible(bNoraml);
	ui->label_number_pixel->setVisible(!bNoraml);
	ui->comBox_number_condition->setVisible(!bNoraml);
	ui->lineEdit_number->setVisible(!bNoraml);
	ui->label_tiggeredon->setVisible(!bNoraml);
	ui->label_current_area->setVisible(!bNoraml);
	ui->label_current_area_show->setVisible(!bNoraml);
	ui->label->setVisible(!bNoraml);
	//设置弹框的尺寸
	if (!bNoraml)
	{
		this->setFixedSize(500, 300);
	}
}

void CSParamSetting::ConnectSignalSlot()
{
	bool ok = connect(m_colorBtn, &ColorQPushButton::SignalSelectColor, this, &CSParamSetting::SlotSelectColor);
	ok = connect(ui->Slider_lineEdit, &QLineEdit::textEdited, this, &CSParamSetting::SlotSlider_lineEditChanged);
	ok = connect(ui->Slider_lineEdit, &QLineEdit::editingFinished, this, &CSParamSetting::SlotSliderReleased);
	ok = connect(ui->horizontalSlider, &QSlider::valueChanged, this, [=](int value) {
		ui->Slider_lineEdit->setText(QString::number(value)); }, Qt::QueuedConnection);
	ok = connect(ui->horizontalSlider, &QSlider::valueChanged, this, &CSParamSetting::SlotSliderReleased);
	ok = connect(ui->comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CSParamSetting::SlotComboBox);
	ok = connect(ui->Coordinate_X_lineEdit, &QLineEdit::editingFinished, this, &CSParamSetting::SlotCoordinate_X_lineEditFinished);
	ok = connect(ui->Coordinate_Y_lineEdit, &QLineEdit::editingFinished, this, &CSParamSetting::SlotCoordinate_Y_lineEditFinished);
	ok = connect(ui->Width_lineEdit, &QLineEdit::editingFinished, this, &CSParamSetting::SlotWidth_lineEditFinished);
	ok = connect(ui->Height_lineEdit, &QLineEdit::editingFinished, this, &CSParamSetting::SlotHeight_lineEditFinished);
	ok = connect(this, &CSParamSetting::SignalSetColorRoi, this, &CSParamSetting::SlotSelectColor);
	ok = connect(ui->ManualDrawBtn, &QPushButton::clicked, this, [=]() {
		auto devicePtr = current_device_.lock();
		if (nullptr == devicePtr)
		{
			accept();
			return;
		}
		if (Device::PropIntelligentTrigger == m_iType)
		{
			HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			param.roi_visible = true;
			devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
		}
		else if (Device::PropAutoExposure == m_iType)
		{
			HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			param.roi_visible = true;
			devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		} 
		//accept();
		emit SignalManualDrawRoi();// 向外发送进入ROI绘制请求
	});
	ok = connect(ui->CloseBtn, &QPushButton::clicked, this, &CSParamSetting::reject);
	ok = connect(ui->comBox_number_condition, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &CSParamSetting::SlotComboNumber_condition);
	ok = connect(ui->lineEdit_number, &QSpinBox::editingFinished, this, &CSParamSetting::on_lineEdit_number_editingFinished);
	Q_UNUSED(ok);
}

void CSParamSetting::SetAreaValue(const int value)
{
	ui->AreaValueLabel->setText(QString::number(value));
}

QRect CSParamSetting::GetRect() const
{
	QRect rect = { QPoint(ui->Coordinate_X_lineEdit->text().toInt() ,ui->Coordinate_Y_lineEdit->text().toInt()) ,QSize(ui->Width_lineEdit->text().toInt(),ui->Height_lineEdit->text().toInt())};
	if (rect.width() == 0)
	{
		rect.setWidth(1);
		ui->Width_lineEdit->setText("1"); 
	}
	if (rect.height() == 0)
	{
		rect.setHeight(1);
		ui->Height_lineEdit->setText("1");
	}
	return rect;
}

void CSParamSetting::UpdateUI2Grabber(bool enable)
{
	//ui->Coordinate_X_lineEdit->setEnabled(enable);
	//ui->Coordinate_Y_lineEdit->setEnabled(enable);
	//ui->Width_lineEdit->setEnabled(enable);
	//ui->Height_lineEdit->setEnabled(enable);
	//ui->MaxTriAreaBtn->setEnabled(enable);
	//ui->ManualDrawBtn->setEnabled(enable);
	//ui->AreaCenterBtn->setEnabled(enable);
	//ui->AreaDisplayBtn->setEnabled(enable); 
	//if (m_colorBtn) {
	//	m_colorBtn->setEnabled(enable);
	//}
}

void CSParamSetting::SlotAutoExposureAvgGray(const QString &ip, const int value)
{
	auto devicePtr = current_device_.lock();
	QString cur_ip = devicePtr->getIp();
	if (cur_ip != ip) return;
	if (m_iType = Device::PropAutoExposure)
	{
		auto temp_val = value;
		if (devicePtr->IsIntelligentTriggerV4Supported() || devicePtr->isSupportHighBitParam())
		{
			temp_val = Device::ImageBitsChange(value, 16, m_bDevicePixelBit);
		}
		ui->AreaValueLabel->setText(QString::number(temp_val));
	}
}

void CSParamSetting::SlotForbid(bool used)
{
	auto device_ptr = current_device_.lock();
	if (device_ptr && device_ptr->isGrabberDevice()) {
		ui->horizontalSlider->setEnabled(used);
		ui->Slider_lineEdit->setEnabled(used);
		ui->comboBox->setEnabled(used);
	}
	else  {
		ui->Coordinate_X_lineEdit->setEnabled(used);
		ui->Coordinate_Y_lineEdit->setEnabled(used);
		ui->Width_lineEdit->setEnabled(used);
		ui->Height_lineEdit->setEnabled(used);
		ui->MaxTriAreaBtn->setEnabled(used);
		ui->ManualDrawBtn->setEnabled(used);
		ui->AreaCenterBtn->setEnabled(used);
		ui->AreaDisplayBtn->setEnabled(used);
		ui->horizontalSlider->setEnabled(used);
		ui->Slider_lineEdit->setEnabled(used);
		ui->comboBox->setEnabled(used);
		m_colorBtn->setEnabled(used);
	}
}

void CSParamSetting::SetEditValue(Device::RoiTypes type)
{
	auto devicePtr = current_device_.lock();
	if (nullptr != devicePtr)
	{
		ui->Coordinate_X_lineEdit->setText(QString::number(devicePtr->GetRoi(type).x()));
		ui->Coordinate_Y_lineEdit->setText(QString::number(devicePtr->GetRoi(type).y()));
		ui->Width_lineEdit->setText(QString::number(devicePtr->GetRoi(type).width()));
		ui->Height_lineEdit->setText(QString::number(devicePtr->GetRoi(type).height()));
		m_current_rect = GetRect();//记录当前宽高

	}
}

void CSParamSetting::SlotSelectColor(QColor color)
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		param.roi_color = color.rgb();
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.roi_color = color.rgb();
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
	}
}

ColorQPushButton::ColorQPushButton(QWidget* parent)
	:QPushButton(parent)
{
	setAutoFillBackground(true);    //设置自动填充背景
	setFlat(true);    //设置成平面

	//设置默认颜色
	QPalette pal;
	pal.setColor(QPalette::Button, Qt::black);
	this->setPalette(pal);
}

ColorQPushButton::~ColorQPushButton() {}

void ColorQPushButton::SetColor(QColor color)
{
	QPalette pal;
	pal.setColor(QPalette::Button, color);
	setPalette(pal);
}

QColor ColorQPushButton::GetColor() const
{
	return this->palette().color(QPalette::Button);
}

void ColorQPushButton::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		QPalette pal;
		QColor pColor = QColorDialog::getColor(GetColor(), this);
		if (pColor.isValid())
		{
			pal.setColor(QPalette::Button, pColor);
			setPalette(pal);
			emit SignalSelectColor(GetColor());
		}
	}
}

void CSParamSetting::on_AreaDisplayBtn_clicked(bool checked)
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		param.roi_visible = checked?1:0;
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));

		SetEditValue(Device::RoiTypes::kIntelligentTriggerRoi);
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.roi_visible = checked?1:0;
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));

		SetEditValue(Device::RoiTypes::kAutoExposureRoi);
	}
}

void CSParamSetting::on_AreaCenterBtn_clicked()
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		auto rect = devicePtr->getCenterRoi(Device::RoiTypes::kIntelligentTriggerRoi);
		param.roi = Device::QRectTCameraWindowRect(rect);
		if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
		{
			param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
		}
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));

		//更改编辑框的值
		SetEditValue(Device::RoiTypes::kIntelligentTriggerRoi);
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.autoExpArea = Device::QRectTCameraWindowRect(devicePtr->getCenterRoi(Device::RoiTypes::kAutoExposureRoi));
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		SetEditValue(Device::RoiTypes::kAutoExposureRoi);
	}
}

void CSParamSetting::on_MaxTriAreaBtn_clicked()
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		auto rect = devicePtr->GetMaxRoi(Device::RoiTypes::kIntelligentTriggerRoi);
		param.roi = Device::QRectTCameraWindowRect(rect);
		if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
		{
			param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
		}
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
		SetEditValue(Device::RoiTypes::kIntelligentTriggerRoi);
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.autoExpArea = Device::QRectTCameraWindowRect(devicePtr->GetMaxRoi(Device::RoiTypes::kAutoExposureRoi));
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		SetEditValue(Device::RoiTypes::kAutoExposureRoi);
	}
}

void CSParamSetting::SlotSlider_lineEditChanged(const QString &text)
{
	int value = text.toInt();
	value = value > MAX_VALID_VALUE ? MAX_VALID_VALUE : value;
	ui->Slider_lineEdit->setText(QString::number(value));
	ui->horizontalSlider->setValue(value);
}

void CSParamSetting::SlotSliderReleased()
{
	int value = ui->horizontalSlider->value();
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}
	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		if (devicePtr->IsIntelligentTriggerV4Supported() || devicePtr->isSupportHighBitParam())
		{
			param.threshold_new = Device::ImageBitsChange(value, m_bDevicePixelBit, 16);
		}
		else
		{
			param.threshold = value;		
		}
		if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) 
			&& !devicePtr->isGrabberDevice()) param.pixel_active_thres = value;
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		if (devicePtr->IsIntelligentTriggerV4Supported() || devicePtr->isSupportHighBitParam())
		{
			param.targetGrayNew = Device::ImageBitsChange(value, m_bDevicePixelBit, 16);
		}
		else if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported())
			&& !devicePtr->isGrabberDevice())
		{
			param.targetGrayNew = value;
		}
		else
		{
			param.targetGray = value;
		}

		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
	}
}

void CSParamSetting::SlotComboBox(int index)
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}
	HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
	param.autoExpGrayMode = AutoExposureGrayMode(index);
	devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
}

void CSParamSetting::SlotComboNumber_condition(int index)
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}
 	HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
 	param.trigger_type = index;
	m_current_area_mode = param.trigger_type;//记录当前触发模式

	double percent = (m_current_area_mode == 1) ? (100 - Device::CountToPercentage(param.pixel_number_thres, m_current_rect)) : (Device::CountToPercentage(param.pixel_number_thres, m_current_rect));
	ui->lineEdit_number->setValue(qRound(percent));
 	devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
}

void CSParamSetting::on_lineEdit_number_editingFinished()
{
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}
	int value = ui->lineEdit_number->value();
 	HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
	ui->lineEdit_number->setValue(value);
	if (m_current_area_mode == 1)
	{
		value = 100 - value;
	}

	if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
	{
		param.pixel_number_thres = Device::PercentageToCount(value, m_current_rect);
	}
	devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
}

void CSParamSetting::SlotCoordinate_X_lineEditFinished()
{
	int value_ = ui->Coordinate_X_lineEdit->text().toInt();
	ui->Coordinate_X_lineEdit->setText(QString::number(value_));
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiXRange(range, Device::RoiTypes::kIntelligentTriggerRoi))
		{
			//m_coordinate_X_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nValue = GetRect().x();
			int nWidth = GetRect().width();
			if (range.min <= nValue && range.max >= nValue)
			{
				if (nValue + nWidth >= range.max)
				{
					nWidth = range.max - nValue + 1;
					ui->Width_lineEdit->setText(QString::number(nWidth));
				}
				HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				QRect rect = GetRect();
				rect.setLeft(nValue);
				rect.setWidth(nWidth);
				if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
				{
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
				}
				param.roi = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };
				devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int xValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi).x();
				ui->Coordinate_X_lineEdit->setText(QString::number(xValue));
				QString xTip = QString(tr("The starting point coordinate X must be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Coordinate_X_lineEdit->mapToGlobal(QPoint(0,0)), xTip);
			}
		}	
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiXRange(range, Device::RoiTypes::kAutoExposureRoi))
		{
			//m_coordinate_X_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nValue = GetRect().x();
			int nWidth = GetRect().width();
			if (range.min <= nValue && range.max >= nValue)
			{
				if (nValue + nWidth >= range.max)
				{
					nWidth = range.max - nValue + 1;
					ui->Width_lineEdit->setText(QString::number(nWidth));
				}
				HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				QRect rect = GetRect();
				rect.setLeft(nValue);
				rect.setWidth(nWidth);
				param.autoExpArea = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };;
				devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int xValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>().autoExpArea).x();
				ui->Coordinate_X_lineEdit->setText(QString::number(xValue));
				QString xTip = QString(tr("The starting point coordinate X must be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Coordinate_X_lineEdit->mapToGlobal(QPoint(0, 0)), xTip);
			}
		}
	}
}

void CSParamSetting::SlotCoordinate_Y_lineEditFinished()
{
	int value_ = ui->Coordinate_Y_lineEdit->text().toInt();
	ui->Coordinate_Y_lineEdit->setText(QString::number(value_));
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiYRange(range, Device::RoiTypes::kIntelligentTriggerRoi))
		{
			//m_coordinate_Y_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nValue = GetRect().y();
			int nHeight = GetRect().height();
			if (range.min <= nValue && range.max >= nValue)
			{
				if (nValue + nHeight >= range.max)
				{
					nHeight = range.max - nValue + 1;
					ui->Height_lineEdit->setText(QString::number(nHeight));
				}
				HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				QRect rect = GetRect();
				rect.setTop(nValue);
				rect.setHeight(nHeight);
				if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
				{
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
				}
				param.roi = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };
				devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int yValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi).y();
				ui->Coordinate_Y_lineEdit->setText(QString::number(yValue));
				QString yTip =  QString(tr("The starting point coordinate Y must be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Coordinate_Y_lineEdit->mapToGlobal(QPoint(0, 0)), yTip);
			}
		}
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiYRange(range, Device::RoiTypes::kAutoExposureRoi))
		{
			//m_coordinate_Y_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nValue = GetRect().y();
			int nHeight = GetRect().height();
			if (range.min <= nValue && range.max >= nValue)
			{
				if (nValue + nHeight >= range.max)
				{
					nHeight = range.max - nValue + 1;
					ui->Height_lineEdit->setText(QString::number(nHeight));
				}
				HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				QRect rect = GetRect();
				rect.setTop(nValue);
				rect.setHeight(nHeight);
				param.autoExpArea = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };;
				devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int yValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>().autoExpArea).y();
				ui->Coordinate_Y_lineEdit->setText(QString::number(yValue));
				QString yTip = QString(tr("The starting point coordinate Y must be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Coordinate_Y_lineEdit->mapToGlobal(QPoint(0, 0)), yTip);
			}
		}
	}
}

void CSParamSetting::SlotWidth_lineEditFinished()
{
	int value_ = ui->Width_lineEdit->text().toInt();
	ui->Width_lineEdit->setText(QString::number(value_));
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiWidthRange(GetRect().x(), range, Device::RoiTypes::kIntelligentTriggerRoi))
		{
			//m_width_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nWidth = GetRect().width();
			if (nWidth > 0 && range.min <= nWidth && range.max >= nWidth)
			{
				HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				QRect rect = GetRect();
				if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
				{
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
				}
				param.roi = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };
				devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int widthValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi).width();
				ui->Width_lineEdit->setText(QString::number(widthValue));
				QString widthTip = QString(tr("The width be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Width_lineEdit->mapToGlobal(QPoint(0, 0)), widthTip);
			}
		}
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiWidthRange(GetRect().x(), range, Device::RoiTypes::kAutoExposureRoi))
		{
			//m_width_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nWidth = GetRect().width();
			if (nWidth > 0 && range.min <= nWidth && range.max >= nWidth)
			{
				HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				QRect rect = GetRect();
				param.autoExpArea = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };;
				devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int widthValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>().autoExpArea).width();
				ui->Width_lineEdit->setText(QString::number(widthValue));
				QString widthTip = QString(tr("The width be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Width_lineEdit->mapToGlobal(QPoint(0, 0)), widthTip);
			}
		}
	}
}

void CSParamSetting::SlotHeight_lineEditFinished()
{
	int value_ = ui->Height_lineEdit->text().toInt();
	ui->Height_lineEdit->setText(QString::number(value_));
	auto devicePtr = current_device_.lock();
	if (nullptr == devicePtr)
	{
		return;
	}
	if (Device::PropIntelligentTrigger == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiHeightRange(GetRect().y(), range, Device::RoiTypes::kIntelligentTriggerRoi))
		{
			//m_height_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nHeight = GetRect().height();
			if (nHeight > 0 && range.min <= nHeight && range.max >= nHeight)
			{
				HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				QRect rect = GetRect();
				if ((devicePtr->IsSupportCMOSDigitGain() || devicePtr->IsIntelligentTriggerV5Supported()) && !devicePtr->isGrabberDevice())
				{
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, m_current_rect), rect);
				}
				param.roi = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };
				devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int heightValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>().roi).height();
				ui->Height_lineEdit->setText(QString::number(heightValue));
				QString heightTip = QString(tr("The height be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Height_lineEdit->mapToGlobal(QPoint(0, 0)), heightTip);
			}
		}
	}
	else if (Device::PropAutoExposure == m_iType)
	{
		HscIntRange range;
		if (devicePtr->getRoiHeightRange(GetRect().y(), range, Device::RoiTypes::kAutoExposureRoi))
		{
			//m_height_Validator->setRange(range.min, range.max);
			// [2022/8/31 rgq]: 与老采集软件保持一致，不设置输入范围
			int nHeight = GetRect().height();
			if (nHeight > 0 && range.min <=  nHeight && range.max >= nHeight)
			{
				HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				QRect rect = GetRect();
				param.autoExpArea = CameraWindowRect{ quint16(rect.x()),quint16(rect.y()),quint16(rect.height()),quint16(rect.width()) };;
				devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
				m_current_rect = rect;//记录当前宽高

			}
			else
			{
				int heightValue = Device::CameraWindowRect2QRect(devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>().autoExpArea).height();
				ui->Height_lineEdit->setText(QString::number(heightValue));
				QString heightTip = QString(tr("The height be set between %1 and %2.").arg(range.min).arg(range.max));
				QToolTip::showText(ui->Height_lineEdit->mapToGlobal(QPoint(0, 0)), heightTip);
			}
		}
	}
}

// void CSParamSetting::SlotCurrentAreaPixels(const uint64_t value)
// {
// 	if (m_iType = Device::PropIntelligentTrigger)
// 	{
// 		ui->label_current_area_show->setText(QString::number(value, 'f', 0));
// 	}
// }

void CSParamSetting::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	auto device_ptr = current_device_.lock();
	Device::DrawTypeStatusInfo drawStatus = device_ptr->getDrawTypeStatusInfo();
	if (drawStatus == Device::DTSI_Noraml && !device_ptr->isGrabberDevice()) {
		ui->ManualDrawBtn->setEnabled(true);
	}
	QRect rect;
	if (device_ptr)
	{
		if (Device::PropIntelligentTrigger == m_iType)
		{
			HscIntelligentTriggerParamV2 param = device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			rect = Device::CameraWindowRect2QRect(param.roi);
		}
		else if (Device::PropAutoExposure == m_iType)
		{
			HscAutoExposureParameter param = device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			rect = Device::CameraWindowRect2QRect(param.autoExpArea);
		}
	}
	ui->Coordinate_X_lineEdit->setText(QString::number(rect.x()));
	ui->Coordinate_Y_lineEdit->setText(QString::number(rect.y()));
	ui->Width_lineEdit->setText(QString::number(rect.width()));
	ui->Height_lineEdit->setText(QString::number(rect.height()));

	m_current_rect = rect;//记录当前宽高
}
