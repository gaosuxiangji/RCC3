#include "csedrexposureparamsetting.h"
#include "ui_csedrexposureparamsetting.h"
#include <QStyleOption>
#include <QPainter>

CSEdrexposureParamSetting::CSEdrexposureParamSetting(QSharedPointer<Device> device, const uint32_t value, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::CSEdrExposureParamSetting)
	, m_pCurrentDevice(device)
	, m_mapComBox()
	, m_iThreadValue(value)
{
    ui->setupUi(this);
	InitUI();
	ConnectSignalSlot();
}

CSEdrexposureParamSetting::~CSEdrexposureParamSetting()
{
    delete ui;
}

void CSEdrexposureParamSetting::SlotEdrForbid(bool used)
{
	ui->EdrExposureTimeLineEdit->setEnabled(used);
	ui->EdrComboBox->setEnabled(used);
	ui->EdrThresholdSlider->setEnabled(used);
	ui->EdrThresholdLineEdit->setEnabled(used);
}

void CSEdrexposureParamSetting::InitUI()
{
	m_b_init = true;
    //设置弹框的尺寸
    this->setFixedSize(410, 160);
    //隐藏标题栏的问号和图标
    setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::Drawer);
	auto pDevice = m_pCurrentDevice.lock();

	ui->widget->setStyleSheet(QString::fromUtf8("border:1px solid #CCCCCC"));
	ui->EdrThresholdSlider->setStyleSheet(QString::fromUtf8("border:none"));
	ui->ExposureTimeLabel->setStyleSheet(QString::fromUtf8("border:none"));
	ui->ThresholdLabel->setStyleSheet(QString::fromUtf8("border:none"));

	//滑块
	ui->EdrThresholdSlider->setMinimum(0);    //最小值
	m_nDevicePixelBit = pDevice->getProperty(Device::PropPixelBitDepth).toInt();
	if (m_nDevicePixelBit > 8)
	{
		m_nValueMax = std::pow(2, m_nDevicePixelBit) - 1;
	}
	ui->EdrThresholdSlider->setMaximum(m_nValueMax);    //最大值
	ui->EdrThresholdSlider->setSingleStep(1);    //步长

	uint32_t thread_value = 0;
	if (nullptr != pDevice)
	{
		if (nullptr != pDevice)
		{
			//下拉框
			QVariantList edrVarList;
			pDevice->getSupportedProperties(Device::PropEdrExposureUnit, edrVarList);
			for (auto &item : edrVarList)
			{
				QString strItem = DeviceUtils::getExposureTimeUnitText(item.toInt());
				m_mapComBox.insert(strItem, item.toInt());
				ui->EdrComboBox->addItem(strItem);
			}
		}
		m_b_init = false;

		m_real_unit = pDevice->GetRealExposureTimeUnit();
		m_current_unit = pDevice->getProperty(Device::PropEdrExposureUnit).value<agile_device::capability::Units>();

		//判断是否支持,自动修正
		QVariantList edrVarList;
		pDevice->getSupportedProperties(Device::PropEdrExposureUnit, edrVarList);
		bool is_support = false;
		for (auto &item : edrVarList)
		{
			if (item.toInt() == (int)m_current_unit)
			{
				is_support = true;
			}
		}
		if (!is_support)// 修正参数并且下发
		{
			m_current_unit = (agile_device::capability::Units)edrVarList.first().toInt();
			pDevice->setProperty(Device::PropEdrExposureUnit, QVariant::fromValue(m_current_unit));
		}

		thread_value = pDevice->convertTime(m_iThreadValue, m_real_unit, m_current_unit);
	}
	//变更下拉框选项
	ui->EdrComboBox->setCurrentText(DeviceUtils::getExposureTimeUnitText(m_current_unit));

	//时间编辑框提示（曝光时间必须设置在1与m_iThreadValue之间。）
	ui->EdrExposureTimeLineEdit->setToolTip(tr("Exposure time must be set between 1 and ") + QString::number(thread_value) + tr("microseconds."));

	//阈值编辑框提示（启动阈值必须设置在0与255之间。）
	//ui->EdrThresholdLineEdit->setToolTip(tr("The start threshold must be set between 0 and 255."));
	ui->EdrThresholdLineEdit->setToolTip(tr("The start threshold must be set between %1 and %2.").arg(QString::number(0)).arg(QString::number(m_nValueMax)));
	//滑块变化响应编辑框
	QIntValidator* intValidator = new QIntValidator;
	ui->EdrThresholdLineEdit->setValidator(intValidator);
	ui->EdrThresholdLineEdit->setText(QString::number(0));


	int iUpperLuminance = 0; // 亮度上限阈值
	int iLowerLuminance = 0; // 亮度下限阈值
	int iEdrTime = 0; // 二次曝光时间

	if (nullptr != pDevice)
	{
		if (pDevice->IsIntelligentTriggerV4Supported() || pDevice->isSupportHighBitParam()) {
			HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
			iUpperLuminance = Device::ImageBitsChange(param.upperLuminance, 16, m_nDevicePixelBit);
			iLowerLuminance = Device::ImageBitsChange(param.lowerLuminance, 16, m_nDevicePixelBit);
			iEdrTime = (param.doubleExposureTime > thread_value) ? thread_value : param.doubleExposureTime;
		}
		else
		{
			HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
			iUpperLuminance = param.upperLuminance;
			iLowerLuminance = param.lowerLuminance;
			iEdrTime = (param.doubleExposureTime > thread_value) ? thread_value : param.doubleExposureTime;
		}
	}


	//时间框
	ui->EdrExposureTimeLineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
	ui->EdrExposureTimeLineEdit->setText(QString::number(iEdrTime));
	ui->EdrExposureTimeLineEdit->setFocus();

	//滑块值
	ui->EdrThresholdSlider->setValue(iUpperLuminance);

	//阈值框
	QRegExp regx("[0-9]{1,9}");
	QRegExpValidator* validator = new QRegExpValidator(regx);
	ui->EdrThresholdLineEdit->setValidator(validator);
	ui->EdrThresholdLineEdit->setText(QString::number(iUpperLuminance));
}

void CSEdrexposureParamSetting::ConnectSignalSlot()
{
    bool ok = connect(ui->EdrThresholdLineEdit, &QLineEdit::editingFinished, this, &CSEdrexposureParamSetting::on_EdrThresholdLineEdit_editingFinished);
    ok = connect(this, &CSEdrexposureParamSetting::Signal_EdrThresholdLineEdit_textChanged, ui->EdrThresholdSlider, &QSlider::setValue);
    ok = connect(ui->EdrThresholdSlider, &QSlider::valueChanged, this, &CSEdrexposureParamSetting::on_EdrThresholdSlider_valueChanged);
    ok = connect(this, &CSEdrexposureParamSetting::Signal_EdrThresholdSlider_valueChanged, ui->EdrThresholdLineEdit, &QLineEdit::setText);
	ok = connect(ui->EdrExposureTimeLineEdit, &QLineEdit::editingFinished, this, &CSEdrexposureParamSetting::on_EdrExposureTimeLineEdit_editingFinished);
	ok = connect(ui->CloseBtn, &QPushButton::clicked, this, &CSEdrexposureParamSetting::reject);
}

void CSEdrexposureParamSetting::on_EdrThresholdSlider_valueChanged(int value)
{
	auto pDevice = m_pCurrentDevice.lock();
	if (nullptr != pDevice)
	{
		if (pDevice->IsIntelligentTriggerV4Supported() || pDevice->isSupportHighBitParam()) {
			HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
			param.upperLuminance = Device::ImageBitsChange(value, m_nDevicePixelBit, 16);
			pDevice->setProperty(Device::PropEdrExposureV2, QVariant::fromValue(param));
		}
		else {
			HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
			param.upperLuminance = value;
			pDevice->setProperty(Device::PropEdrExposure, QVariant::fromValue(param));
		}
	}

	QString text = QString::number(value);
	emit Signal_EdrThresholdSlider_valueChanged(text);
}

void CSEdrexposureParamSetting::on_EdrExposureTimeLineEdit_editingFinished()
{
	int iTimeValue = 0;
	auto pDevice = m_pCurrentDevice.lock();
	uint32_t thread_value = 0;
	if (nullptr != pDevice)
	{
		thread_value = pDevice->convertTime(m_iThreadValue, m_real_unit, m_current_unit);
	}
	QString timeText = ui->EdrExposureTimeLineEdit->text();
	if (ui->EdrExposureTimeLineEdit->text().length() > 8)
	{
		timeText = ui->EdrExposureTimeLineEdit->text().left(8);
	}

	if (timeText.toULongLong() > thread_value)
	{
		iTimeValue = thread_value;
		ui->EdrExposureTimeLineEdit->setText(QString::number(iTimeValue));
	}
	else if (timeText.toULongLong() < 1)
	{
		iTimeValue = 1;
		ui->EdrExposureTimeLineEdit->setText(QString::number(iTimeValue));
	}
	else
	{
		iTimeValue = ui->EdrExposureTimeLineEdit->text().toLongLong();
		ui->EdrExposureTimeLineEdit->setText(QString::number(iTimeValue));
	}

	if (nullptr != pDevice)
	{
		if (pDevice->IsIntelligentTriggerV4Supported() || pDevice->isSupportHighBitParam()) {
			HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
			param.doubleExposureTime = iTimeValue;
			pDevice->setProperty(Device::PropEdrExposureV2, QVariant::fromValue(param));
		}
		else {
			HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
			param.doubleExposureTime = iTimeValue;
			pDevice->setProperty(Device::PropEdrExposure, QVariant::fromValue(param));
		}
	}
}

void CSEdrexposureParamSetting::on_EdrThresholdLineEdit_editingFinished()
{
	int iThrehold = 0;

	if (ui->EdrThresholdLineEdit->text().toInt() > m_nValueMax)
	{
		iThrehold = m_nValueMax;
		ui->EdrThresholdLineEdit->setText(QString::number(iThrehold));
	}
	else if (ui->EdrThresholdLineEdit->text().toInt() < 0)
	{
		iThrehold = 0;
		ui->EdrThresholdLineEdit->setText(QString::number(iThrehold));
	}
	else
	{
		iThrehold = ui->EdrThresholdLineEdit->text().toInt();
	}

	emit Signal_EdrThresholdLineEdit_textChanged(iThrehold);

	auto pDevice = m_pCurrentDevice.lock();
	if (nullptr != pDevice)
	{
		if (pDevice->IsIntelligentTriggerV4Supported()||pDevice->isSupportHighBitParam()) {
			HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
			param.upperLuminance = Device::ImageBitsChange(iThrehold, m_nDevicePixelBit, 16);
			pDevice->setProperty(Device::PropEdrExposureV2, QVariant::fromValue(param));
		}
		else {
			HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
			param.upperLuminance = iThrehold;
			pDevice->setProperty(Device::PropEdrExposure, QVariant::fromValue(param));
		}
	}
}

void CSEdrexposureParamSetting::on_EdrComboBox_currentTextChanged(const QString &arg1)
{
	if (m_b_init) {
		return;
	}
	if (m_mapComBox.end() != m_mapComBox.find(arg1))
	{
		auto pDevice = m_pCurrentDevice.lock();
		if (nullptr != pDevice)
		{
			pDevice->setProperty(Device::PropEdrExposureUnit, QVariant::fromValue(m_mapComBox[arg1]));
			m_current_unit = (agile_device::capability::Units)m_mapComBox[arg1];
			uint32_t doubleTime = 0;
			if (pDevice->IsIntelligentTriggerV4Supported() || pDevice->isSupportHighBitParam()) {
				HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
				doubleTime = param.doubleExposureTime;
			}
			else {
				HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
				doubleTime = param.doubleExposureTime;
			}
			ui->EdrExposureTimeLineEdit->setText(QString::number(doubleTime));

			auto thread_value = pDevice->convertTime(m_iThreadValue, m_real_unit, m_current_unit);
			ui->EdrExposureTimeLineEdit->setToolTip(tr("Exposure time must be set between 1 and ") + QString::number(thread_value) + tr("microseconds."));
		}
	}
}
