#include "realtimeothersettingswidget.h"
#include "ui_realtimeothersettingswidget.h"
#include "Device/deviceutils.h"
#include "Device/device.h"

RealtimeOtherSettingsWidget::RealtimeOtherSettingsWidget(QWidget *parent) 
	: QWidget(parent)
	, ui(new Ui::RealtimeOtherSettingsWidget)
{
	ui->setupUi(this);

	initUi();
}

RealtimeOtherSettingsWidget::~RealtimeOtherSettingsWidget()
{
	delete ui;
}

void RealtimeOtherSettingsWidget::initUi()
{
	// ��ʾ��Ϣ����
	ui->widgetDisplayInfo->setText(tr("Display Info"));
	ui->widgetDisplayInfo->setChecked(true);
	setDisplayInfoVisible(ui->widgetDisplayInfo->isChecked());
	connect(ui->widgetDisplayInfo, &TouchCheckWidget::clicked, this, &RealtimeOtherSettingsWidget::sigSetDisplayInfoVisible);

	//Э���ʽ
	ui->widgetStreamFormat->setTitle(tr("Stream Type"));
	ui->widgetStreamFormat->setCurrentText(DeviceUtils::getStreamTypeText(StreamType::TYPE_RAW8));
	connect(ui->widgetStreamFormat, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toStreamFormatWidget);
	connect(ui->pageStreamType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageStreamType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setStreamFormat);

	//ģ������
	ui->widgetAnalogGain->setTitle(tr("Analog Gain"));
	ui->widgetAnalogGain->setCurrentText(DeviceUtils::getAnalogGainText(ANALOG_GAIN_TYPE::AAG_1));
	connect(ui->widgetAnalogGain, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toAnalogGainWidget);
	connect(ui->pageAnalogGain, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageAnalogGain, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setAnalogGain);

	//������ʽ
	ui->widgetTriggerType->setTitle(tr("Trigger Type"));
	ui->widgetTriggerType->setCurrentText(DeviceUtils::getTriggerModeText(TriggerMode::TRIGGER_INTERNAL));
	connect(ui->widgetTriggerType, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toTriggerTypeWidget);
	connect(ui->pageTriggerType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageTriggerType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setTriggerType);

	//�ⴥ�����巽ʽ
	ui->widgetExTriggerType->setVisible(false);//������ʽΪ�ⴥ��ʱ��ʾ
	ui->widgetExTriggerType->setCurrentText(DeviceUtils::getExternalTriggerModeText(ExternalTriggerMode::TRIGGER_RAISING));
	connect(ui->widgetExTriggerType, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toExTriggerTypeWidget);
	connect(ui->pageExTriggerType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageExTriggerType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setExTriggerType);

	//����
	connect(ui->pushButtonBack, &QPushButton::clicked, this, &RealtimeOtherSettingsWidget::backButtonClicked);
}

QSharedPointer<Device> RealtimeOtherSettingsWidget::getDevice() const
{
	return device_wptr_.lock();
}

void RealtimeOtherSettingsWidget::setDevice(QSharedPointer<Device> device_ptr)
{
	auto old_device_ptr = device_wptr_.lock();
	if (old_device_ptr != device_ptr)
	{
		device_wptr_ = device_ptr;

		if (old_device_ptr)
		{
			// ������豸���źŲ۹���
			disconnect(old_device_ptr.data(), 0, this, 0);
		}

		// �������豸���źŲ۹���
		connect(device_ptr.data(), &Device::stateChanged, this, &RealtimeOtherSettingsWidget::updateOtherSettingsUI);
		connect(device_ptr.data(), &Device::propertyChanged, this, &RealtimeOtherSettingsWidget::updateOtherSettingsUI);

		//���²������ý���
		updateOtherSettingsUI();
	}
}

void RealtimeOtherSettingsWidget::toOthersSettingRootWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageOtherSettingsRoot);
}

void RealtimeOtherSettingsWidget::toStreamFormatWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageStreamType);
}

void RealtimeOtherSettingsWidget::toAnalogGainWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageAnalogGain);
}

void RealtimeOtherSettingsWidget::toTriggerTypeWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageTriggerType);
}

void RealtimeOtherSettingsWidget::toExTriggerTypeWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageExTriggerType);
}

void RealtimeOtherSettingsWidget::setDisplayInfoVisible(bool visible)
{
	//emit sigSetDisplayInfoVisible(visible);
}

void RealtimeOtherSettingsWidget::setTriggerType(const QVariant & trigger_type)
{
	TriggerMode type = TriggerMode(trigger_type.toInt());

	getDevice()->setProperty(Device::PropTriggerMode, type);
}

void RealtimeOtherSettingsWidget::setExTriggerType(const QVariant & ex_trigger_type)
{
	ExternalTriggerMode type = ExternalTriggerMode(ex_trigger_type.toInt());

	getDevice()->setProperty(Device::PropExTriggerMode, type);
}

void RealtimeOtherSettingsWidget::setAnalogGain(const QVariant & analog_gain)
{
	ANALOG_GAIN_TYPE gain = ANALOG_GAIN_TYPE(analog_gain.toInt());

	getDevice()->setProperty(Device::PropAnalogGain, gain);
}

void RealtimeOtherSettingsWidget::setStreamFormat(const QVariant & stream_format)
{
	StreamType format = StreamType(stream_format.toInt());

	getDevice()->setProperty(Device::PropStreamType, format);
}

void RealtimeOtherSettingsWidget::updateOtherSettingsUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())//û���豸������������ҳ�����û�
	{
		toOthersSettingRootWidget();
		setEnabled(false);
		return;
	}

	setEnabled(true);

	//Э���ʽ
	updateStreamTypeUI();

	//ģ������ 
	updateAnalogGainUI();

	//������ʽ
	updateTriggerTypeUI();

	// �ⴥ����ʽ//TODO:�ݲ�֧�������ⴥ����ʽ,��ʱ�������
	//updateExTriggerTypeUI();
}

void RealtimeOtherSettingsWidget::updateStreamTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// ���
	ui->pageStreamType->clearOptions();

	// ���
	QVariantList stream_type_var_list;
	device_ptr->getSupportedProperties(Device::PropStreamType, stream_type_var_list);
	for (auto stream_type_var : stream_type_var_list)
	{
		ui->pageStreamType->addOption(stream_type_var, DeviceUtils::getStreamTypeText(stream_type_var.toInt()));
	}

	// ���õ�ǰ
	QVariant cur_stream_type_var = device_ptr->getProperty(Device::PropStreamType);
	ui->pageStreamType->setCurrentOption(cur_stream_type_var);
	ui->widgetStreamFormat->setCurrentText(ui->pageStreamType->getCurrentOptionText());
}

void RealtimeOtherSettingsWidget::updateAnalogGainUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// ���
	ui->pageAnalogGain->clearOptions();

	// ���
	QVariantList analog_gain_var_list;
	device_ptr->getSupportedProperties(Device::PropAnalogGain, analog_gain_var_list);
	for (auto var : analog_gain_var_list)
	{
		ui->pageAnalogGain->addOption(var, DeviceUtils::getAnalogGainText(var.toInt()));
	}

	// ���õ�ǰ
	QVariant cur_analog_gain_var = device_ptr->getProperty(Device::PropAnalogGain);
	ui->pageAnalogGain->setCurrentOption(cur_analog_gain_var);
	ui->widgetAnalogGain->setCurrentText(ui->pageAnalogGain->getCurrentOptionText());
}

void RealtimeOtherSettingsWidget::updateTriggerTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// ���
	ui->pageTriggerType->clearOptions();

	// ���
	QVariantList trigger_type_var_list;
	device_ptr->getSupportedProperties(Device::PropTriggerMode, trigger_type_var_list);
	for (auto var : trigger_type_var_list)
	{
		ui->pageTriggerType->addOption(var, DeviceUtils::getTriggerModeText(var.toInt()));
	}

	// ���õ�ǰ
	QVariant cur_trigger_type_var = device_ptr->getProperty(Device::PropTriggerMode);
	ui->pageTriggerType->setCurrentOption(cur_trigger_type_var);
	ui->widgetTriggerType->setCurrentText(ui->pageTriggerType->getCurrentOptionText());

	TriggerMode type = TriggerMode(cur_trigger_type_var.toInt());
	//�ⴥ��ʱ��ʾ�ⴥ����ʽ���ð�ť��������ʾ
	ui->widgetExTriggerType->setVisible(/*type == TRIGGER_EXTERNAL*/false);//TODO:�ݲ�֧�������ⴥ����ʽ,��ʱ�������
	ui->line_7->setVisible(ui->widgetExTriggerType->isVisible());
}

void RealtimeOtherSettingsWidget::updateExTriggerTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// ���
	ui->pageExTriggerType->clearOptions();

	// ���
	QVariantList ex_trigger_type_var_list;
	device_ptr->getSupportedProperties(Device::PropExTriggerMode, ex_trigger_type_var_list);
	for (auto var : ex_trigger_type_var_list)
	{
		ui->pageExTriggerType->addOption(var, DeviceUtils::getExternalTriggerModeText(var.toInt()));
	}

	// ���õ�ǰ
	QVariant cur_ex_trigger_type_var = device_ptr->getProperty(Device::PropExTriggerMode);
	ui->pageExTriggerType->setCurrentOption(cur_ex_trigger_type_var);
	ui->widgetExTriggerType->setCurrentText(ui->pageExTriggerType->getCurrentOptionText());
}
