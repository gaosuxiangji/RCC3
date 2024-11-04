#include "realtimectrlwidget.h"
#include "ui_realtimectrlwidget.h"


#include "Common/AgRadioButton/agradiobutton.h"
#include "Device/device.h"
#include "Device/devicemanager.h"
#include "acqparamdiagram.h"
#include <QDebug>
#include <QLineEdit>
#include <QVariantList>
#include <QButtonGroup>
#include <QSpinBox>
#include <QFontMetrics>
#include "Common/ParamSpinBox/paramspinbox.h"
#include "Device/deviceutils.h"
#include "LogRunner.h"

using namespace FHJD_LOG;

RealtimeCtrlWidget::RealtimeCtrlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealtimeCtrlWidget)
{
	LOG_INFO("Construction.");
    ui->setupUi(this);

	
	widgets_to_enable_
		<< ui->widgetResolution
		<< ui->widgetFPS
		<< ui->widgetExposureTime
		<< ui->widgetOtherSettings
		<< ui->widgetRecordingOffset
		<< ui->widgetRecordingOffsetMode
		<< ui->widgetRecordingLength
		<< ui->widgetRecordingFormat
		<< ui->widgetRecordingUnit
		;


	// 初始化界面
	initUi();
}

RealtimeCtrlWidget::~RealtimeCtrlWidget()
{
    delete ui;
}

void RealtimeCtrlWidget::setDevice(QSharedPointer<Device> device_ptr)
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

		if (!device_ptr)
		{
			SetWidgetsEnable(widgets_to_enable_, false);
			return;
		}
		else
		{
			SetWidgetsEnable(widgets_to_enable_, true);
		}

        // 增加新设备的信号槽关联
        connect(device_ptr.data(), &Device::stateChanged, this, &RealtimeCtrlWidget::onDeviceStateChanged);
        connect(device_ptr.data(), &Device::errorOccurred, this, &RealtimeCtrlWidget::onDeviceErrorOccurred);
		connect(device_ptr.data(), &Device::propertyChanged, this, &RealtimeCtrlWidget::onDevicePropertyChanged);

		//设置其他参数界面
		ui->pageOtherSettings->setDevice(device_ptr);

		//更新当前设备拓扑状态
		updateDeviceTopoState();

        // 更新设备控制界面
        updateDeviceCtrlUi();

		//更新参数设置界面
		updateParamSettingUI();
    }
}

QSharedPointer<Device> RealtimeCtrlWidget::getDevice() const
{
    return device_wptr_.lock();
}

void RealtimeCtrlWidget::setParentDevice(QSharedPointer<Device> device_ptr)
{
	auto old_device_ptr = parent_device_wptr_.lock();
	if (old_device_ptr != device_ptr)
	{
		parent_device_wptr_ = device_ptr;
	}
}

QSharedPointer<Device> RealtimeCtrlWidget::getParentDevice() const
{
	return parent_device_wptr_.lock();
}

void RealtimeCtrlWidget::setChildDevice(QSharedPointer<Device> device_ptr)
{
	auto old_device_ptr = child_device_wptr_.lock();
	if (old_device_ptr != device_ptr)
	{
		child_device_wptr_ = device_ptr;
	}
}

QSharedPointer<Device> RealtimeCtrlWidget::getChildDevice() const
{
    return child_device_wptr_.lock();
}

void RealtimeCtrlWidget::setParentProperties2ChildDevice()
{
	auto parent_device_ptr = getParentDevice();
	auto child_device_ptr = getChildDevice();

	if (parent_device_ptr.isNull() || child_device_ptr.isNull())
	{
		return;
	}
	
	QList<Device::PropType> device_prop_types{	Device::PropFrameRate,Device::PropStreamType,Device::PropTriggerMode,Device::PropExTriggerMode,
												Device::PropRecordingOffsetMode,Device::PropRecordingOffset,Device::PropRecordingLength,
												Device::PropRecordingUnit,Device::PropVideoFormat };

	//判断是否支持模拟增益
	QVariantList analog_values;
	parent_device_ptr->getSupportedProperties(Device::PropAnalogGain,analog_values);
	if (analog_values.count()>1)
	{
		device_prop_types << Device::PropAnalogGain;
	}

	//将父设备的对应参数应用到子设备
	for (auto prop_type : device_prop_types)
	{
		child_device_ptr->setProperty(prop_type, parent_device_ptr->getProperty(prop_type));
	}
}

void RealtimeCtrlWidget::slotUpdateDynamicRoi(const QString &ip, const QRect &roi)
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull()|| device_ptr->getIp()!=ip)
	{
		return;
	}
	device_ptr->setProperty(Device::PropRoi, roi);
	updateResolutionUI();
}

void RealtimeCtrlWidget::toParamSettingRootWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRoot);
}

void RealtimeCtrlWidget::toResolutionWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageResolution);
}

void RealtimeCtrlWidget::toFPSWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageFPS);
}

void RealtimeCtrlWidget::toExposureTimeWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageExposureTime);
}

void RealtimeCtrlWidget::toOtherSettingsWidget()
{
	ui->stackedWidget->setCurrentWidget(ui->pageOtherSettings);
}

void RealtimeCtrlWidget::toRecordingOffsetModeWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRecordingOffsetMode);
}

void RealtimeCtrlWidget::toRecordingUnitWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRecordingUnit);
}

void RealtimeCtrlWidget::toRecordingFormatWidget()
{
    ui->stackedWidget->setCurrentWidget(ui->pageRecordingFormat);
}

void RealtimeCtrlWidget::setDisplayInfoVisible(bool visible)
{
	emit sigSetDisplayInfoVisible(visible);
}

void RealtimeCtrlWidget::setTriggerType(const QVariant & trigger_type)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		TriggerMode type = TriggerMode(trigger_type.toInt());
		device_ptr->setProperty(Device::PropTriggerMode, type);
	}
}

void RealtimeCtrlWidget::setExTriggerType(const QVariant & ex_trigger_type)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		ExternalTriggerMode type = ExternalTriggerMode(ex_trigger_type.toInt());
		device_ptr->setProperty(Device::PropExTriggerMode, type);
	}
}

void RealtimeCtrlWidget::setAnalogGain(const QVariant & analog_gain)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		ANALOG_GAIN_TYPE gain = ANALOG_GAIN_TYPE(analog_gain.toInt());
		device_ptr->setProperty(Device::PropAnalogGain, gain);
	}
}

void RealtimeCtrlWidget::setStreamFormat(const QVariant & stream_format)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		StreamType format = StreamType(stream_format.toInt());
		device_ptr->setProperty(Device::PropStreamType, format);
	}
}

void RealtimeCtrlWidget::setExposureTime(const QVariant & exposure_time)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropExposureTime, exposure_time);
	}
}

void RealtimeCtrlWidget::setFPS(const QVariant & fps)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropFrameRate, fps);
	}
}

void RealtimeCtrlWidget::setResolution(const QVariant & resolution)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropRoi, resolution);
	}
}

void RealtimeCtrlWidget::setRecordingOffsetMode(const QVariant & recording_offset_mode)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		RecordMode mode = RecordMode(recording_offset_mode.toInt());
		device_ptr->setProperty(Device::PropRecordingOffsetMode, mode);
	}
}

void RealtimeCtrlWidget::setRecordingOffset()
{
	auto device_ptr = getDevice();
	if (!device_ptr)
	{
		return;
	}

	const auto state = device_ptr->getState();
	//非高采时
	if (state != DeviceState::Acquiring &&
		state != DeviceState::ToAcquire &&
		state != DeviceState::Recording)
	{
		int recording_offset = ui->spinBoxRecordingOffset->value();
		device_ptr->setProperty(Device::PropRecordingOffset, recording_offset);
	}
	ui->spinBoxRecordingOffset->clearFocus();//编辑结束时清除焦点
	setFocus();
}

void RealtimeCtrlWidget::setRecordingLength()
{
	auto device_ptr = getDevice();
	if (!device_ptr)
	{
		return;
	}

	const auto state = device_ptr->getState();
	//非高采时
	if (state != DeviceState::Acquiring &&
		state != DeviceState::ToAcquire &&
		state != DeviceState::Recording)
	{
		int recording_length = ui->spinBoxRecordingLength->value();
		device_ptr->setProperty(Device::PropRecordingLength, recording_length);
	}
	ui->spinBoxRecordingLength->clearFocus();//编辑结束时清除焦点
	setFocus();
}

void RealtimeCtrlWidget::setRecordingUnit(const QVariant & recording_unit)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		RecordType unit = RecordType(recording_unit.toInt());
		device_ptr->setProperty(Device::PropRecordingUnit, unit);
	}
}

void RealtimeCtrlWidget::setRecordingFormat(const QVariant & recording_format)
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		VideoFormat format = VideoFormat(recording_format.toInt());
		device_ptr->setProperty(Device::PropVideoFormat, format);
	}

}

void RealtimeCtrlWidget::onDeviceStateChanged()
{
	// 更新拓扑状态
	updateDeviceTopoState();

	// 更新设备控制界面
	updateDeviceCtrlUi();

	// 更新参数设置界面
	updateParamSettingUI();
}

void RealtimeCtrlWidget::onDevicePropertyChanged()
{
	// 更新拓扑状态
	updateDeviceTopoState();

	// 更新设备控制界面
	updateDeviceCtrlUi();

	// 更新参数设置界面
	updateParamSettingUI();
}

void RealtimeCtrlWidget::onDeviceErrorOccurred()
{
    
}

void RealtimeCtrlWidget::on_pushButtonPreview_clicked()
{
    auto device_ptr = getDevice();
    if (device_ptr)
    {
		//如果是当前设备是父设备,则子设备也预览
		if (cur_device_topo_state_ == ParentDevice)
		{
			auto child_device_ptr = getChildDevice();
			if (child_device_ptr)
			{
				child_device_ptr->preview();
			}
		}
        device_ptr->preview();
    }
}

void RealtimeCtrlWidget::on_pushButtonAcquireTrigger_clicked()
{
    auto device_ptr = getDevice();
    if (device_ptr)
    {
		if (cur_device_topo_state_ == ParentDevice)
		{
			auto child_device_ptr = getChildDevice();
			DeviceState state = device_ptr->getState();
			DeviceState child_state = child_device_ptr->getState();

			if (state == DeviceState::Acquiring && child_state == DeviceState::Acquiring)
			{
				device_ptr->trigger();
				child_device_ptr->trigger();
			}
			else
			{
				//子设备使用父设备的参数
				setParentProperties2ChildDevice();
				device_ptr->acquire();
				child_device_ptr->acquire();
			}
		}
		else
		{
			DeviceState state = device_ptr->getState();
			if (state == DeviceState::Acquiring)
			{
				device_ptr->trigger();
			}
			else
			{
				device_ptr->acquire();
			}
		}

    }
}

void RealtimeCtrlWidget::on_pushButtonStop_clicked()
{
    auto device_ptr = getDevice();
    if (device_ptr)
    {
		//如果是当前设备是父设备,则子设备也停机
		if (cur_device_topo_state_ == ParentDevice)
		{
			auto child_device_ptr = getChildDevice();
			if (child_device_ptr)
			{
				child_device_ptr->stop();
			}
		}
		device_ptr->stop();
    }
}

void RealtimeCtrlWidget::initUi()
{
    // 更新设备控制界面
    updateDeviceCtrlUi();

	//初始显示设置页面
	toParamSettingRootWidget();

    //未连接设备 初始界面置灰 
	SetWidgetsEnable(widgets_to_enable_, false);

	//隐藏拓扑提示
	ui->labelTopoTip->setVisible(false);

    //分辨率
	ui->widgetResolution->setTitle(tr("Resolution"));
	ui->widgetResolution->setCurrentText(ui->pageResolution->getCurrentOptionText());
	connect(ui->widgetResolution, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toResolutionWidget);
	connect(ui->pageResolution, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageResolution, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setResolution);

	//帧率
	ui->widgetFPS->setTitle(tr("FPS"));
	ui->widgetFPS->setCurrentText(ui->pageFPS->getCurrentOptionText());

	connect(ui->widgetFPS, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toFPSWidget);
	connect(ui->pageFPS, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageFPS, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setFPS);

	//曝光时间
	ui->widgetExposureTime->setTitle(QString(tr("Exposure Time(%1s)")).arg(QString(0x03bc)));

	spin_box_exposure_time_ = new ParamSpinBox(this);
	spin_box_exposure_time_->setButtonSymbols(QAbstractSpinBox::NoButtons);
	spin_box_exposure_time_->setFocusPolicy(Qt::ClickFocus);
	ui->pageExposureTime->setCustomWidget(spin_box_exposure_time_);
	spin_box_exposure_time_->clearFocus();
	connect(spin_box_exposure_time_, &QSpinBox::editingFinished, this, [this] {
		spin_box_exposure_time_->clearFocus(); 
	});

	connect(ui->widgetExposureTime, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toExposureTimeWidget);
	connect(ui->pageExposureTime, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageExposureTime, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setExposureTime);

	// 其他设置
	ui->widgetOtherSettings->setTitle(tr("Other Settings"));
	QString other_settings_text = QString("%1, %2, %3").arg(
		DeviceUtils::getStreamTypeText(StreamType::TYPE_RAW8),
		DeviceUtils::getAnalogGainText(ANALOG_GAIN_TYPE::AAG_1),
		DeviceUtils::getTriggerModeText(TriggerMode::TRIGGER_INTERNAL));
	ui->widgetOtherSettings->setCurrentText(other_settings_text);

	connect(ui->widgetOtherSettings, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toOtherSettingsWidget);
	// 显示信息开关
	connect(ui->pageOtherSettings, &RealtimeOtherSettingsWidget::sigSetDisplayInfoVisible, this, &RealtimeCtrlWidget::setDisplayInfoVisible);
	// 返回
	connect(ui->pageOtherSettings, &RealtimeOtherSettingsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);

    //采集参数
    //保存起点方式
	ui->widgetRecordingOffsetMode->setTitle(tr("Recording Offset Mode"));
	ui->widgetRecordingOffsetMode->setCurrentText(DeviceUtils::getRecordingOffsetModeText(RecordMode::RECORD_BEFORE_SHUTTER));

    connect(ui->widgetRecordingOffsetMode, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toRecordingOffsetModeWidget);
	connect(ui->pageRecordingOffsetMode, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageRecordingOffsetMode, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setRecordingOffsetMode);


    //保存起点
	connect(ui->spinBoxRecordingOffset, &QSpinBox::editingFinished, this, &RealtimeCtrlWidget::setRecordingOffset);
	ui->spinBoxRecordingOffset->setAttribute(Qt::WA_InputMethodEnabled, true);
	ui->spinBoxRecordingOffset->clearFocus();

    //保存长度
	connect(ui->spinBoxRecordingLength, &QSpinBox::editingFinished, this, &RealtimeCtrlWidget::setRecordingLength);
	ui->spinBoxRecordingLength->setAttribute(Qt::WA_InputMethodEnabled, true);
	ui->spinBoxRecordingLength->clearFocus();

    //保存单位
    ui->widgetRecordingUnit->setTitle(tr("Recording Unit"));
	ui->widgetRecordingUnit->setCurrentText(DeviceUtils::getRecordingUnitText(RecordType::RECORD_BY_FRAME));

    connect(ui->widgetRecordingUnit, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toRecordingUnitWidget);
	connect(ui->pageRecordingUnit, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageRecordingUnit, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setRecordingUnit);

    //保存格式
	ui->widgetRecordingFormat->setTitle(tr("Recording Format"));
	ui->widgetRecordingFormat->setCurrentText(DeviceUtils::getVideoFormatText(VideoFormat::VIDEO_RHVD));

    connect(ui->widgetRecordingFormat, &TouchComboWidget::clicked, this, &RealtimeCtrlWidget::toRecordingFormatWidget);
	connect(ui->pageRecordingFormat, &TouchOptionsWidget::backButtonClicked, this, &RealtimeCtrlWidget::toParamSettingRootWidget);
	connect(ui->pageRecordingFormat, &TouchOptionsWidget::currentOptionChanged, this, &RealtimeCtrlWidget::setRecordingFormat);

	//外触发提示
	ui->labelExternalTriggerTip->setVisible(false);
}

void RealtimeCtrlWidget::updateDeviceTopoState()
{
	cur_device_topo_state_ = NoTopo;//重置拓扑状态

	QList<QSharedPointer<Device> >devices;//所有已连接设备
	DeviceManager::instance().getConnectedDevices(devices);
	if (devices.count()!=2)//没有两台相机则没有级联状态
	{
		return;
	}
	auto cur_device = getDevice();//当前设备
	if (cur_device.isNull())
	{
		return;
	}

	bool has_cur_device = false;
	for (auto device: devices )
	{
		if (device->getIp() == cur_device->getIp())
		{
			has_cur_device = true;
			break;
		}			
	}
	if (!has_cur_device)//异常情况 当前设备不在已连接设备列表中
	{
		return;
	}
	
	//判断当前设备是不是子设备或父设备
	for (auto device: devices)
	{
		if (cur_device->getParent())
		{
			if (cur_device->getParent()->getIp() == device->getIp())
			{
				//切换拓扑状态,设置父设备和子设备
				cur_device_topo_state_ = ChildDevice;
				setParentDevice(device);
				setChildDevice(cur_device);
				return;
			}
		}

		if (device->getParent())
		{
			if (device->getParent()->getIp() == cur_device->getIp())
			{
				//切换拓扑状态,设置父设备和子设备
				cur_device_topo_state_ = ParentDevice;
				setParentDevice(cur_device);
				setChildDevice(device);
				return;
			}
		}
	}


}

void RealtimeCtrlWidget::updateDeviceCtrlUi()
{
	//根据不同的拓扑状态来刷新控件状态
     bool allows_preview = false;
     bool allows_acquire = false;
     bool allows_trigger = false;
     bool allows_stop = false;
     auto device_ptr = getDevice();
     QString acquire_trigger_text = tr("Acquire");
     if (device_ptr)
     {
		 if (cur_device_topo_state_ == ParentDevice)//当前为父设备
		 {
			 auto child_device_ptr = getChildDevice();
			 if (child_device_ptr)
			 {
				 //父设备和子设备有一个允许就使能
				 allows_preview = device_ptr->allowsPreview() || child_device_ptr->allowsPreview();
				 allows_stop = device_ptr->allowsStop() || child_device_ptr->allowsStop();
				 allows_acquire = device_ptr->allowsAcquire() || child_device_ptr->allowsAcquire();
				 allows_trigger = device_ptr->allowsTrigger() || child_device_ptr->allowsTrigger();
				 
				 //父设备和子设备都进入采集才允许触发
				 DeviceState state = device_ptr->getState();
				 DeviceState child_state = child_device_ptr->getState();
				 if (((state == DeviceState::Acquiring) || (state == DeviceState::Recording)) &&
					 ((child_state == DeviceState::Acquiring)||(child_state == DeviceState::Recording)))
				 {
					 acquire_trigger_text = tr("Trigger");
				 }
			 }
		 }
		 else
		 {
			 allows_preview = device_ptr->allowsPreview();
			 allows_acquire = device_ptr->allowsAcquire();
			 allows_trigger = device_ptr->allowsTrigger();
			 allows_stop = device_ptr->allowsStop();

			 DeviceState state = device_ptr->getState();
			 if ((state == DeviceState::Acquiring) || (state == DeviceState::Recording))
			 {
				 acquire_trigger_text = tr("Trigger");
			 }
		 }
     }

     ui->pushButtonPreview->setEnabled(allows_preview);
     ui->pushButtonAcquireTrigger->setEnabled(allows_acquire || allows_trigger);
     ui->pushButtonAcquireTrigger->setText(acquire_trigger_text);
     ui->pushButtonStop->setEnabled(allows_stop);
}

void RealtimeCtrlWidget::updateParamSettingUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())//没有设备，返回设置主页并且置灰
	{
		toParamSettingRootWidget();
		SetWidgetsEnable(widgets_to_enable_, false);
		return;
	}

	//有设备，刷新参数界面
	SetWidgetsEnable(widgets_to_enable_, true);

	//分辨率
	updateResolutionUI();

	//帧率
	updateFpsUI();

	//曝光时间
	updateExposureTimeUI();

	//其他设置
	updateOtherSettingsWidget();

    //采集参数
	//保存起点方式
	updateRecordingOffsetTypeUI();

	//保存起点
	updateRecordingOffsetUI();

	//保存长度
	updateRecordingLengthUI();

	//保存单位
	updateRecordingUnitUI();

	//保存格式
	updateRecordingFormatUI();

	//示意图刷新
	updateAcqParamDiagram();

	if (device_ptr->getState() != Acquiring && device_ptr->getState() != Recording)
	{
		//高速采集模式相关参数按钮使能
		enableAcqModeRelatedWidgets(true);
		//多相机级联模式相关界面使能
		enableTopoRelatedWidgets(cur_device_topo_state_ != ChildDevice);
	}
	else
	{
		enableTopoRelatedWidgets(cur_device_topo_state_ != ChildDevice);
		enableAcqModeRelatedWidgets(false);
	}

}

void RealtimeCtrlWidget::updateResolutionUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	//删除全部选项
	ui->pageResolution->clearOptions();

	QVariantList resolution_var_list;
	device_ptr->getTypicalProperties(Device::PropRoi, resolution_var_list);

	//保留的自定义选项
	QVariant cur_resolution = device_ptr->getProperty(Device::PropRoi);
	if (!resolution_var_list.contains(cur_resolution))
	{
		resolution_var_list << cur_resolution;
		resolution_dynamic_ == cur_resolution;
	}
	else if (!resolution_dynamic_.isNull())
	{
		resolution_var_list << resolution_dynamic_;
	}

	//添加选项
	for (auto resolution_var : resolution_var_list)
	{
		QRect resolution = resolution_var.toRect();
		ui->pageResolution->addOption(resolution, QString("%1*%2").arg(resolution.width()).arg(resolution.height()));
	}

	//选中按钮,且更新显示
	//QVariant cur_resolution = device_ptr->getProperty(Device::PropRoi);
	ui->pageResolution->setCurrentOption(cur_resolution);
    ui->widgetResolution->setCurrentText(ui->pageResolution->getCurrentOptionText());
}

void RealtimeCtrlWidget::updateFpsUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	//删除所有选项
	ui->pageFPS->clearOptions();

	QVariant cur_fps = device_ptr->getProperty(Device::PropFrameRate);

	//存放典型值到选项列表中
	QVariantList fps_var_list;
	bool has_cur = false;//是否有相同参数
	device_ptr->getTypicalProperties(Device::PropFrameRate, fps_var_list);
	for (auto fps_var : fps_var_list)
	{
		if (cur_fps == fps_var)
		{
			has_cur = true;
		}
		qint64 fps = fps_var.toLongLong();
		ui->pageFPS->addOption(fps, QString("%1").arg(fps));
	}

	//选中按钮， 更新显示
	if (has_cur)
	{
		ui->pageFPS->setCurrentOption(cur_fps);
		ui->widgetFPS->setCurrentText(ui->pageFPS->getCurrentOptionText());
	}
	else//当前选择中没有当前帧率,设置为选项中最大值
	{
		setFPS(fps_var_list.last());
	}
}

void RealtimeCtrlWidget::updateExposureTimeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	QVariantList exposure_time_var_list;
	QVariant cur_exposure_time;
	qint64 exposure_time_min = 0;
	qint64 exposure_time_max = 1000;
	qint64 exposure_time_inc = 1;
	cur_exposure_time = device_ptr->getProperty(Device::PropExposureTime);
	device_ptr->getPropertyRange(Device::PropExposureTime,exposure_time_min,exposure_time_max,exposure_time_inc);
	device_ptr->getTypicalProperties(Device::PropExposureTime, exposure_time_var_list);

	// 更新自定义编辑框范围
	if (spin_box_exposure_time_)
	{
		spin_box_exposure_time_->setRange(exposure_time_min, exposure_time_max);
		spin_box_exposure_time_->setValue(cur_exposure_time.toInt());
		QString str = QString("<font color = #ff0000>*</font>[%1]&le;%2%3").arg(QString(tr("Exposure Time"))).arg(exposure_time_max).arg("us");
		ui->pageExposureTime->setCustomTip(str);
	}

	//删除全部选项
	ui->pageExposureTime->clearOptions();


	//典型值存放到选项中，同时判断是否与当前值相同，不相同则放在最后作为自定义选项
	int index = 0;
	for (auto exposure_time_var : exposure_time_var_list)
	{
		qint64 value = exposure_time_var.toLongLong();
		ui->pageExposureTime->addOption(value, QString("%1").arg(value));
	}

	//选中按钮，更新显示

	if (cur_exposure_time >= exposure_time_min && cur_exposure_time <= exposure_time_max)
	{
	ui->pageExposureTime->setCurrentOption(cur_exposure_time);
    ui->widgetExposureTime->setCurrentText(ui->pageExposureTime->getCurrentOptionText());
	}
	else//不在范围内则使用最大值
	{
		setExposureTime(exposure_time_max);
	}
}

void RealtimeCtrlWidget::updateRecordingOffsetTypeUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// 清除
	ui->pageRecordingOffsetMode->clearOptions();

	// 添加
	QVariantList offset_mode_var_list;
	device_ptr->getSupportedProperties(Device::PropRecordingOffsetMode, offset_mode_var_list);
	for (auto var : offset_mode_var_list)
	{
		ui->pageRecordingOffsetMode->addOption(var, DeviceUtils::getRecordingOffsetModeText(var.toInt()));
	}

	// 设置当前
	QVariant cur_offset_mode_var = device_ptr->getProperty(Device::PropRecordingOffsetMode);
	ui->pageRecordingOffsetMode->setCurrentOption(cur_offset_mode_var);
	ui->widgetRecordingOffsetMode->setCurrentText(ui->pageRecordingOffsetMode->getCurrentOptionText());
}

void RealtimeCtrlWidget::updateRecordingOffsetUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	qint64 cur_offset = 0;
	qint64 offset_min = 0;
	qint64 offset_max = 1000;
	qint64 offset_inc = 1;

	//非高采时
	if (device_ptr->getState() != DeviceState::Acquiring &&
		device_ptr->getState() != DeviceState::ToAcquire &&
		device_ptr->getState() != DeviceState::Recording)
	{
		cur_offset = device_ptr->getProperty(Device::PropRecordingOffset).toLongLong();
		device_ptr->getPropertyRange(Device::PropRecordingOffset, offset_min, offset_max, offset_inc);

		//更新编辑框数值和范围
		ui->spinBoxRecordingOffset->setValue(cur_offset);
		ui->spinBoxRecordingOffset->setRange(offset_min, offset_max);
	}

}

void RealtimeCtrlWidget::updateRecordingLengthUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	qint64 cur_length = 1;
	qint64 length_min = 0;
	qint64 length_max = 1000;
	qint64 length_inc = 1;

	//非高采时
	if (device_ptr->getState() != DeviceState::Acquiring &&
		device_ptr->getState() != DeviceState::ToAcquire &&
		device_ptr->getState() != DeviceState::Recording )
	{
		cur_length = device_ptr->getProperty(Device::PropRecordingLength).toLongLong();
		device_ptr->getPropertyRange(Device::PropRecordingLength, length_min, length_max, length_inc);

		//更新编辑框数值和范围
		ui->spinBoxRecordingLength->setValue(cur_length);
		ui->spinBoxRecordingLength->setRange(length_min, length_max);
	}
}

void RealtimeCtrlWidget::updateOtherSettingsWidget()
{
	auto device_ptr = getDevice();
	if (device_ptr)
	{
		TriggerMode trig_type = TriggerMode(device_ptr->getProperty(Device::PropTriggerMode).toInt());
		QString other_settings_text = QString("%1, %2, %3").arg(
			DeviceUtils::getStreamTypeText(device_ptr->getProperty(Device::PropStreamType).toInt()),
			DeviceUtils::getAnalogGainText(device_ptr->getProperty(Device::PropAnalogGain).toInt()),
			DeviceUtils::getTriggerModeText(trig_type));
		ui->widgetOtherSettings->setCurrentText(other_settings_text);
		ui->labelExternalTriggerTip->setVisible(trig_type == TRIGGER_EXTERNAL);
	}
}

void RealtimeCtrlWidget::updateRecordingUnitUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// 清除
	ui->pageRecordingUnit->clearOptions();

	// 添加
	QVariantList unit_var_list;
	device_ptr->getSupportedProperties(Device::PropRecordingUnit, unit_var_list);
	for (auto var : unit_var_list)
	{
		ui->pageRecordingUnit->addOption(var, DeviceUtils::getRecordingUnitText(var.toInt()));
	}

	// 设置当前
	QVariant cur_unit_var = device_ptr->getProperty(Device::PropRecordingUnit);
	ui->pageRecordingUnit->setCurrentOption(cur_unit_var);
	ui->widgetRecordingUnit->setCurrentText(ui->pageRecordingUnit->getCurrentOptionText());
}

void RealtimeCtrlWidget::updateRecordingFormatUI()
{
	auto device_ptr = getDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	// 清除
	ui->pageRecordingFormat->clearOptions();

	// 添加
	QVariantList format_var_list;
	device_ptr->getSupportedProperties(Device::PropVideoFormat, format_var_list);
	for (auto var : format_var_list)
	{
		ui->pageRecordingFormat->addOption(var, DeviceUtils::getVideoFormatText(var.toInt()));
	}

	// 设置当前
	QVariant cur_format_var = device_ptr->getProperty(Device::PropVideoFormat);
	ui->pageRecordingFormat->setCurrentOption(cur_format_var);
	ui->widgetRecordingFormat->setCurrentText(ui->pageRecordingFormat->getCurrentOptionText());
}

void RealtimeCtrlWidget::updateAcqParamDiagram()
{
	//刷新采集参数文字 
	QString offset_mode = ui->pageRecordingOffsetMode->getCurrentOptionText();
	QString record_unit = ui->pageRecordingUnit->getCurrentOptionText();
	QString record_offset = QString().setNum(ui->spinBoxRecordingOffset->value());
	QString record_length = QString().setNum(ui->spinBoxRecordingLength->value());
	QString acq_info = QString("%1: %2%3, %4: %5%3").arg(offset_mode).arg(record_offset).arg(record_unit).arg(tr("Recording Length")).arg(record_length);

	QFontMetrics font_width(ui->labelAcqParams->font());
	QString elide_str = font_width.elidedText(acq_info, Qt::ElideRight, ui->labelAcqParams->width());

	ui->labelAcqParams->setText(elide_str);

	//刷新采集示意图
	QVariant record_mode_var = ui->pageRecordingOffsetMode->getCurrentOptionValue();
	RecordMode record_mode = RecordMode(record_mode_var.toInt());
	AcqParamDiagram::RecordOffsetMode record_offset_mode;
	if (record_mode == RECORD_BEFORE_SHUTTER)
	{
		record_offset_mode = AcqParamDiagram::BEFORE_SHUTTER;
	}
	else
	{
		record_offset_mode = AcqParamDiagram::AFTER_SHUTTER;
	}

	ui->widgetAcqDiagram->drawDiagram(record_offset_mode , record_offset.toInt(),record_length.toInt());
}

void RealtimeCtrlWidget::enableAcqModeRelatedWidgets(bool b_enable)
{
	//  分辨率 帧率	协议格式 模拟增益 触发方式 保存起点方式 保存起点 保存长度 保存单位 保存格式
	QList<QWidget*> widgets;
	widgets
		<< ui->widgetResolution
		<< ui->widgetFPS
		<< ui->widgetRecordingOffsetMode
		<< ui->widgetRecordingOffset
		<< ui->widgetRecordingLength
		<< ui->widgetRecordingUnit
		<< ui->widgetRecordingFormat
		<< ui->widgetOtherSettings
		;
	SetWidgetsEnable(widgets, b_enable);
}

void RealtimeCtrlWidget::enableTopoRelatedWidgets(bool b_enable)
{
	//   帧率 协议格式 模拟增益 触发方式 保存起点方式 保存起点 保存长度 保存单位 保存格式 设备控制
	QList<QWidget*> widgets;
	widgets
		<< ui->widgetFPS
		<< ui->widgetRecordingOffsetMode
		<< ui->widgetRecordingOffset
		<< ui->widgetRecordingLength
		<< ui->widgetRecordingUnit
		<< ui->widgetRecordingFormat
		<< ui->frameCtrl
		<< ui->widgetOtherSettings
		;
	SetWidgetsEnable(widgets, b_enable);

	//拓扑提示
	ui->labelTopoTip->setVisible(!b_enable);
}

void RealtimeCtrlWidget::SetWidgetsEnable(QList<QWidget*> widgets_to_enable, bool b_enbable)
{
	for (auto widget : widgets_to_enable)
	{
		widget->setEnabled(b_enbable);
	}
}


