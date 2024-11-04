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
	// 显示信息开关
	ui->widgetDisplayInfo->setText(tr("Display Info"));
	ui->widgetDisplayInfo->setChecked(true);
	setDisplayInfoVisible(ui->widgetDisplayInfo->isChecked());
	connect(ui->widgetDisplayInfo, &TouchCheckWidget::clicked, this, &RealtimeOtherSettingsWidget::sigSetDisplayInfoVisible);

	//协议格式
	ui->widgetStreamFormat->setTitle(tr("Stream Type"));
	ui->widgetStreamFormat->setCurrentText(DeviceUtils::getStreamTypeText(StreamType::TYPE_RAW8));
	connect(ui->widgetStreamFormat, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toStreamFormatWidget);
	connect(ui->pageStreamType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageStreamType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setStreamFormat);

	//模拟增益
	ui->widgetAnalogGain->setTitle(tr("Analog Gain"));
	ui->widgetAnalogGain->setCurrentText(DeviceUtils::getAnalogGainText(ANALOG_GAIN_TYPE::AAG_1));
	connect(ui->widgetAnalogGain, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toAnalogGainWidget);
	connect(ui->pageAnalogGain, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageAnalogGain, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setAnalogGain);

	//触发方式
	ui->widgetTriggerType->setTitle(tr("Trigger Type"));
	ui->widgetTriggerType->setCurrentText(DeviceUtils::getTriggerModeText(TriggerMode::TRIGGER_INTERNAL));
	connect(ui->widgetTriggerType, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toTriggerTypeWidget);
	connect(ui->pageTriggerType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageTriggerType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setTriggerType);

	//外触发具体方式
	ui->widgetExTriggerType->setVisible(false);//触发方式为外触发时显示
	ui->widgetExTriggerType->setCurrentText(DeviceUtils::getExternalTriggerModeText(ExternalTriggerMode::TRIGGER_RAISING));
	connect(ui->widgetExTriggerType, &TouchComboWidget::clicked, this, &RealtimeOtherSettingsWidget::toExTriggerTypeWidget);
	connect(ui->pageExTriggerType, &TouchOptionsWidget::backButtonClicked, this, &RealtimeOtherSettingsWidget::toOthersSettingRootWidget);
	connect(ui->pageExTriggerType, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeOtherSettingsWidget::setExTriggerType);

	//返回
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
			// 解除旧设备的信号槽关联
			disconnect(old_device_ptr.data(), 0, this, 0);
		}

		// 增加新设备的信号槽关联
		connect(device_ptr.data(), &Device::stateChanged, this, &RealtimeOtherSettingsWidget::updateOtherSettingsUI);
		connect(device_ptr.data(), &Device::propertyChanged, this, &RealtimeOtherSettingsWidget::updateOtherSettingsUI);

		//更新参数设置界面
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
	if (device_ptr.isNull())//没有设备，返回设置主页并且置灰
	{
		toOthersSettingRootWidget();
		setEnabled(false);
		return;
	}

	setEnabled(true);

	//协议格式
	updateStreamTypeUI();

	//模拟增益 
	updateAnalogGainUI();

	//触发方式
	updateTriggerTypeUI();

	// 外触发方式//TODO:暂不支持设置外触发方式,暂时禁用入口
	//updateExTriggerTypeUI();
}

void RealtimeOtherSettingsWidget::updateStreamTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// 清除
	ui->pageStreamType->clearOptions();

	// 添加
	QVariantList stream_type_var_list;
	device_ptr->getSupportedProperties(Device::PropStreamType, stream_type_var_list);
	for (auto stream_type_var : stream_type_var_list)
	{
		ui->pageStreamType->addOption(stream_type_var, DeviceUtils::getStreamTypeText(stream_type_var.toInt()));
	}

	// 设置当前
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

	// 清除
	ui->pageAnalogGain->clearOptions();

	// 添加
	QVariantList analog_gain_var_list;
	device_ptr->getSupportedProperties(Device::PropAnalogGain, analog_gain_var_list);
	for (auto var : analog_gain_var_list)
	{
		ui->pageAnalogGain->addOption(var, DeviceUtils::getAnalogGainText(var.toInt()));
	}

	// 设置当前
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

	// 清除
	ui->pageTriggerType->clearOptions();

	// 添加
	QVariantList trigger_type_var_list;
	device_ptr->getSupportedProperties(Device::PropTriggerMode, trigger_type_var_list);
	for (auto var : trigger_type_var_list)
	{
		ui->pageTriggerType->addOption(var, DeviceUtils::getTriggerModeText(var.toInt()));
	}

	// 设置当前
	QVariant cur_trigger_type_var = device_ptr->getProperty(Device::PropTriggerMode);
	ui->pageTriggerType->setCurrentOption(cur_trigger_type_var);
	ui->widgetTriggerType->setCurrentText(ui->pageTriggerType->getCurrentOptionText());

	TriggerMode type = TriggerMode(cur_trigger_type_var.toInt());
	//外触发时显示外触发方式设置按钮和文字提示
	ui->widgetExTriggerType->setVisible(/*type == TRIGGER_EXTERNAL*/false);//TODO:暂不支持设置外触发方式,暂时禁用入口
	ui->line_7->setVisible(ui->widgetExTriggerType->isVisible());
}

void RealtimeOtherSettingsWidget::updateExTriggerTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// 清除
	ui->pageExTriggerType->clearOptions();

	// 添加
	QVariantList ex_trigger_type_var_list;
	device_ptr->getSupportedProperties(Device::PropExTriggerMode, ex_trigger_type_var_list);
	for (auto var : ex_trigger_type_var_list)
	{
		ui->pageExTriggerType->addOption(var, DeviceUtils::getExternalTriggerModeText(var.toInt()));
	}

	// 设置当前
	QVariant cur_ex_trigger_type_var = device_ptr->getProperty(Device::PropExTriggerMode);
	ui->pageExTriggerType->setCurrentOption(cur_ex_trigger_type_var);
	ui->widgetExTriggerType->setCurrentText(ui->pageExTriggerType->getCurrentOptionText());
}
