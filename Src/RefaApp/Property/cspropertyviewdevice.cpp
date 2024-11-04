#include "cspropertyviewdevice.h"
#include "ui_cspropertyviewdevice.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/UIUtils/uiutils.h"
#include "Property/csintcomboproperty.h"
#include "Device/devicemanager.h"
#include "Device/device.h"
#include "Device/deviceutils.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "Device/csdlgprogressformat.h"

CSPropertyViewDevice::CSPropertyViewDevice(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CSPropertyViewDevice)
{
	ui->setupUi(this);
	m_device_magager_ptr = &DeviceManager::instance();

	m_property_manager = new CSVariantPropertyManager(ui->widgetPropertyBrowser);
	m_property_editor = new CSVariantEditorFactory(ui->widgetPropertyBrowser);
	ui->widgetPropertyBrowser->setFactoryForManager(m_property_manager, m_property_editor);//设置属性可编辑

	m_map_props = {
		{ kGroupBasicProps, std::make_tuple("STRID_GN_BASIC_INFO", "", kRoot, true) },
		{ kGroupDeviceProps, std::make_tuple("STRID_GN_DEVICE_PARAM", "", kRoot, true) },
		{ kGroupRecordingProps, std::make_tuple("STRID_GN_ACQUISITION_PARAM", "", kRoot, true) },
		{ kGroupSdiParam, std::make_tuple("STRID_SDI_PARAM", "TIPID_SDI_PARAM", kRoot, true) },
		{ kGroupAdvancedProps, std::make_tuple("STRID_ADVANCED_PARAM", "", kRoot, true) },
		{ kPropModel, std::make_tuple("STRID_PN_MODEL", "STRID_PD_MODEL", kGroupBasicProps, false) },
		{ kPropIP, std::make_tuple("STRID_PN_ID", "TIPID_IP", kGroupBasicProps, false) },
		{ kPropSN, std::make_tuple("STRID_SN", "TIPID_SN", kGroupBasicProps, false) },
		{ kPropDeviceName, std::make_tuple("STRID_PN_NAME", "STRID_PD_NAME", kGroupBasicProps, false) },
		{ kPropDeviceIndex, std::make_tuple("STRID_DEVICE_INDEX", "TIPID_DEVICE_INDEX", kGroupBasicProps, false) },
		{ kPropStationName, std::make_tuple("STRID_STATION_NAME", "TIPID_STATION_NAME", kGroupBasicProps, false) },
		{ kPropStationAlias, std::make_tuple("STRID_STATION_ALIAS", "TIPID_STATION_ALIAS", kGroupBasicProps, false) },
		{ kPropStatus, std::make_tuple("STRID_PN_STATUS", "STRID_PD_STATUS", kGroupBasicProps, false) },
		{ kPropHardwareStandby, std::make_tuple("STRID_PN_STANDBY", "STRID_PD_STANDBY", kGroupBasicProps, false) },
		{ kPropPosition, std::make_tuple("STRID_POSITION", "STRID_POSITION", kGroupBasicProps, false) },
		{ kPropGropName, std::make_tuple("STRID_GROUP_NAME", "STRID_GROUP_NAME", kGroupBasicProps, false) },
		{ kPropGropDeviceNum, std::make_tuple("STRID_GROUP_DEVICE_NUM", "STRID_GROUP_DEVICE_NUM", kGroupBasicProps, false) },
		{ kGroupRoi, std::make_tuple("STRID_GROUP_ROI", "", kGroupDeviceProps, true) },
		{ kPropResolution, std::make_tuple("STRID_RESOLUTION", "TIPID_RESOLUTION", kGroupRoi, false) },
		{ kPropRoiOffsetX, std::make_tuple("STRID_OFFSET_X", "TIPID_OFFSET_X", kGroupRoi, false) },
		{ kPropRoiOffsetY, std::make_tuple("STRID_OFFSET_Y", "TIPID_OFFSET_Y", kGroupRoi, false) },
		{ kPropRoiWidth, std::make_tuple("STRID_WIDTH", "TIPID_WIDTH", kGroupRoi, false) },
		{ kPropRoiHeight, std::make_tuple("STRID_HEIGHT", "TIPID_HEIGHT", kGroupRoi, false) },
		{ kGroupTrigger, std::make_tuple("STRID_GROUP_TRIGGER", "", kGroupDeviceProps, true) },
		{ kPropTriggerMode, std::make_tuple("STRID_PN_TRIGGER_MODE", "STRID_PD_TRIGGER_MODE", kGroupTrigger, false) },
		{ kPropSpacebarTrigger, std::make_tuple("STRID_SPACEBAR_TRIGGER", "TIPID_SPACEBAR_TRIGGER", kGroupTrigger, false) },
		{ kPropExternalTriggerMode, std::make_tuple("STRID_PN_EXTERNAL_TRIGGER_MODE", "STRID_PD_EXTERNAL_TRIGGER_MODE", kGroupTrigger, false) },
		{ kPropJitterEliminationLength, std::make_tuple("STRID_JITTER_ELIMINATION_LENGTH", "TIPID_JITTER_ELIMINATION_LENGTH", kGroupTrigger, false) },
		{ kGroupSync, std::make_tuple("STRID_GROUP_SYNC", "", kGroupDeviceProps, true) },
		{ kPropSyncSource, std::make_tuple("STRID_PN_SYNC_MODE", "STRID_PD_SYNC_MODE", kGroupSync, false) },
		{ kPropPulseWidth, std::make_tuple("STRID_PN_PULSE_WIDTH", "STRID_PD_PULSE_WIDTH", kGroupSync, false) },
		{ kPropPulseWidth100Ns, std::make_tuple("STRID_PN_PULSE_WIDTH_100NS", "STRID_PD_PULSE_WIDTH", kGroupSync, false) },
		{ kPropStreamType,  std::make_tuple("STRID_STREAM_TYPE", "TIPID_STREAM_TYPE", kGroupDeviceProps, false) },
		{ kPropTriggerPulseWidth, std::make_tuple("STRID_PN_PULSE_WIDTH_NS", "STRID_PD_PULSE_WIDTH", kGroupSync, false) },
		{ kPropDelayChannel, std::make_tuple("STRID_DELAY_CHANNEL", "TIPID_DELAY_CHANNEL", kGroupDeviceProps, false) },
		{ kPropDelay, std::make_tuple("STRID_DELAY", "TIPID_DELAY", kGroupDeviceProps, false) },
		{ kGroupChannelDelay, std::make_tuple("STRID_CHANNEL_DELAY", "", kGroupDeviceProps, true) },
		{ kPropChn1Delay, std::make_tuple("STRID_CHN1_DELAY", "TIPID_CHN1_DELAY", kGroupChannelDelay, false) },
		{ kPropChn2Delay, std::make_tuple("STRID_CHN2_DELAY", "TIPID_CHN2_DELAY", kGroupChannelDelay, false) },
		{ kPropChn3Delay, std::make_tuple("STRID_CHN3_DELAY", "TIPID_CHN3_DELAY", kGroupChannelDelay, false) },
		{ kPropChn4Delay, std::make_tuple("STRID_CHN4_DELAY", "TIPID_CHN4_DELAY", kGroupChannelDelay, false) },
		{ kPropChn5Delay, std::make_tuple("STRID_CHN5_DELAY", "TIPID_CHN5_DELAY", kGroupChannelDelay, false) },
		{ kPropChn6Delay, std::make_tuple("STRID_CHN6_DELAY", "TIPID_CHN6_DELAY", kGroupChannelDelay, false) },
		{ kPropChn7Delay, std::make_tuple("STRID_CHN7_DELAY", "TIPID_CHN7_DELAY", kGroupChannelDelay, false) },
		{ kPropChn8Delay, std::make_tuple("STRID_CHN8_DELAY", "TIPID_CHN8_DELAY", kGroupChannelDelay, false) },
		{ kPropFrameRate, std::make_tuple("STRID_PN_FRAME_RATE", "STRID_PD_FRAME_RATE", kGroupDeviceProps, false) },
		{ kGroupExposure, std::make_tuple("STRID_GN_EXPOSURE_TIME", "", kGroupDeviceProps, true) },
		{ kPropExposureTime, std::make_tuple("STRID_PN_EXPOSURE_TIME", "STRID_PD_EXPOSURE_TIME", kGroupExposure, false) },
		{ kPropExposureTimeUnit, std::make_tuple("STRID_PN_EXPOSURE_TIME_UNIT", "STRID_PD_EXPOSURE_TIME_UNIT", kGroupExposure, false) },
		{ kPropExposureDelay, std::make_tuple("STRID_EXPOSURE_DELAY", "TIPID_EXPOSURE_DELAY", kGroupExposure, false) },
		{ kPropOverExposureTip, std::make_tuple("STRID_OVEREXPOSURE_TIP", "TIPID_OVEREXPOSURE_TIP", kGroupExposure, false) },
		{ kPropRecordingOffsetMode, std::make_tuple("STRID_PN_RECORDING_OFFSET_MODE", "STRID_PD_RECORDING_OFFSET_MODE", kGroupRecordingProps, false) },
		{ kPropRecordingOffset, std::make_tuple("STRID_PN_RECORDING_OFFSET", "STRID_PD_RECORDING_OFFSET", kGroupRecordingProps, false) },
		{ kPropRecordingUnit, std::make_tuple("STRID_PN_RECORDING_UNIT", "STRID_PD_RECORDING_UNIT", kGroupRecordingProps, false) },
		{ kPropRecordingLength, std::make_tuple("STRID_PN_RECORDING_LENGTH", "STRID_PD_RECORDING_LENGTH", kGroupRecordingProps, false) },
		{ kPropRealtimeExport, std::make_tuple("STRID_REALTIME_EXPORT", "TIPID_REALTIME_EXPORT", kGroupRecordingProps, false) },
		{ kPropAutoExport, std::make_tuple("STRID_AUTO_EXPORT", "TIPID_AUTO_EXPORT", kGroupRecordingProps, false) },
		{ kPropVideoFormat, std::make_tuple("STRID_PN_VIDEO_FORMAT", "STRID_PD_VIDEO_FORMAT", kGroupRecordingProps, false) },
		{ kPropStreamType, std::make_tuple("STRID_PN_STREAM_TYPE", "STRID_PD_STREAM_TYPE", kGroupAdvancedProps, false) },
		{ kPropImageNamePrefix, std::make_tuple("STRID_IMAGE_NAME_PREFIX", "TIPID_IMAGE_NAME_PREFIX", kGroupRecordingProps, false) },
		{ kPropImageNameSuffix, std::make_tuple("STRID_IMAGE_NAME_SUFFIX", "TIPID_IMAGE_NAME_SUFFIX", kGroupRecordingProps, false) },
		{ kPropWatermark, std::make_tuple("STRID_WATERMARK", "TIPID_WATERMARK", kGroupAdvancedProps, false) },
		{ kPropTriggerSyncEnable, std::make_tuple("STRID_TRIGGER_SYNC", "STRID_TRIGGER_SYNC", kGroupAdvancedProps, false) },
		{ kPropGroupName, std::make_tuple("STRID_PN_GROUP_NAME", "STRID_PD_GROUP_NAME", kGroupBasicProps, false) },
		{ kPropGroupDeviceModel, std::make_tuple("STRID_PN_GROUP_DEVICE_MODEL", "STRID_PD_GROUP_DEVICE_MODEL", kGroupBasicProps, false) },
		{ kPropGroupDeviceCount, std::make_tuple("STRID_PN_GROUP_DEVICE_NUMBER", "STRID_PD_GROUP_DEVICE_NUMBER", kGroupBasicProps, false) },
		{ kPropProductName, std::make_tuple("STRID_PN_PRODUCT_NAME", "STRID_PD_PRODUCT_NAME", kGroupBasicProps, false) },
		{ kPropProductVersion, std::make_tuple("STRID_PN_PRODUCT_VERSION", "STRID_PD_PRODUCT_VERSION", kGroupBasicProps, false) },
		{ kPropPulseMode, std::make_tuple("STRID_PN_PULSE_MODE", "STRID_PD_PULSE_MODE", kGroupDeviceProps, false) },
		{ kPropPulseNumber, std::make_tuple("STRID_PN_PULSE_NUMBER", "STRID_PD_PULSE_NUMBER", kGroupDeviceProps, false) },
		{ kGroupEDR, std::make_tuple("STRID_GN_EDR", "", kGroupDeviceProps, true) },
		{ kPropDoubleExposureTime, std::make_tuple("STRID_PN_DOUBLE_EXPOSURE_TIME", "STRID_PD_DOUBLE_EXPOSURE_TIME", kGroupEDR, false) },
		{ kPropDoubleExposureTimeUnit, std::make_tuple("STRID_PN_DOUBLE_EXPOSURE_TIME_UNIT", "STRID_PD_DOUBLE_EXPOSURE_TIME_UNIT", kGroupEDR, false) },
		{ kPropEDRLowerThreshold, std::make_tuple("STRID_PN_LOWER_THRESHOLD", "STRID_PD_LOWER_THRESHOLD", kGroupEDR, false) },
		{ kPropEDRUpperThreshold, std::make_tuple("STRID_PN_UPPER_THRESHOLD", "STRID_PD_UPPER_THRESHOLD", kGroupEDR, false) },
		{ kPropEDRThreshold, std::make_tuple("STRID_EDR_THRESHOLD", "TIPID_EDR_THRESHOLD", kGroupEDR, false) },
		{ kPropEnableDoubleExposureTime, std::make_tuple("STRID_PN_ENABLE_DOUBLE_EXPOSURE", "STRID_PD_ENABLE_DOUBLE_EXPOSURE", kGroupEDR, false) },
		{ kGroupPIV, std::make_tuple("STRID_PIV_SETTINGS", "", kGroupDeviceProps, true) },
		{ kPropEnablePIV, std::make_tuple("STRID_ENABLE_PIV", "TIPID_ENABLE_PIV", kGroupPIV, false) },
		{ kPropPIVExposureTime, std::make_tuple("STRID_PIV_EXPOSURE_TIME", "TIPID_PIV_EXPOSURE_TIME", kGroupPIV, false) },
		{ kPropPIVAcquisitionFrequency, std::make_tuple("STRID_PIV_ACQUISITION_FREQUENCY", "TIPID_PIV_ACQUISITION_FREQUENCY", kGroupPIV, false) },
		{ kPropPIVAcquisitionFrequencyUnit, std::make_tuple("STRID_PIV_ACQUISITION_FREQUENCY_UNIT", "TIPID_PIV_ACQUISITION_FREQUENCY_UNIT", kGroupPIV, false) },
		{ kPropLaserInterval, std::make_tuple("STRID_LASER_INTERVAL", "TIPID_LASER_INTERVAL", kGroupPIV, false) },
		{ kGroupLaserA, std::make_tuple("STRID_LASER_A_PARAM", "", kGroupPIV, true) },
		{ kPropLaserAReadyOffset, std::make_tuple("STRID_PULSE_WIDTH_1", "TIPID_PULSE_WIDTH_1", kGroupLaserA, false) },
		{ kPropLaserASendOffset, std::make_tuple("STRID_PULSE_WIDTH_2", "TIPID_PULSE_WIDTH_2", kGroupLaserA, false) },
		{ kPropLaserAPulseWidth, std::make_tuple("STRID_PULSE_INTERVAL", "TIPID_PULSE_INTERVAL", kGroupLaserA, false) },
		{ kGroupLaserB, std::make_tuple("STRID_LASER_B_PARAM", "", kGroupPIV, true) },
		{ kPropLaserBReadyOffset, std::make_tuple("STRID_PULSE_WIDTH_1", "TIPID_PULSE_WIDTH_1", kGroupLaserB, false) },
		{ kPropLaserBSendOffset, std::make_tuple("STRID_PULSE_WIDTH_2", "TIPID_PULSE_WIDTH_2", kGroupLaserB, false) },
		{ kPropLaserBPulseWidth, std::make_tuple("STRID_PULSE_INTERVAL", "TIPID_PULSE_INTERVAL", kGroupLaserB, false) },
		{ kPropVfpnCompensation, std::make_tuple("STRID_VFPN_COMPENSATION", "TIPID_VFPN_COMPENSATION", kGroupRecordingProps, false) },
		{ kPropHobCompensation, std::make_tuple("STRID_HOB_COMPENSATION", "TIPID_HOB_COMPENSATION", kGroupRecordingProps, false) },
		{ kPropDisplayHighestBit, std::make_tuple("STRID_DISPLAY_HIGHEST_BIT", "TIPID_DISPLAY_HIGHEST_BIT", kGroupRecordingProps, false) },
		{ kPropSdiFpsResol, std::make_tuple("STRID_SDI_RESOLUTION_FRAME_RATE", "TIPID_SDI_RESOLUTION_FRAME_RATE", kGroupSdiParam, false) },
		{ kPropTimeAvgImage, std::make_tuple("STRID_EXPORT_TIME_AVG_IMAGE", "TIPID_EXPORT_TIME_AVG_IMAGE", kGroupRecordingProps, false) },
		{ kPropTimeStdevImage, std::make_tuple("STRID_EXPORT_TIME_STDEV_IMAGE", "TIPID_EXPORT_TIME_STDEV_IMAGE", kGroupRecordingProps, false) },
		{ kPropStripeTemperatureLevel, std::make_tuple("STRID_ACQUIRE_TEMPERATURE", "TIPID_ACQUIRE_TEMPERATURE", kGroupDeviceProps, false) },
		{ kPropPixelBitDepth, std::make_tuple("STRID_PIXEL_BIT_DEPTH", "TIPID_PIXEL_BIT_DEPTH", kGroupAdvancedProps, false) },
		{ kPropDisplayBitDepth, std::make_tuple("STRID_DISPLAY_BIT_DEPTH", "STRID_DISPLAY_BIT_DEPTH", kGroupAdvancedProps, false) },


		//CF18-control-begin//////////////////////////////////////////////////
		{ kPropCF18ExternalTriggerSignal, std::make_tuple("STRID_CF18_EXTERNAL_TRIGGER_SIGNAL", "", kGroupDeviceProps, true) },
		{ kPropCF18ExternalSynSignal, std::make_tuple("STRID_CF18_EXTERNAL_SYN_SIGNAL", "", kGroupDeviceProps, true) },
		{ kPropCF18BSignal, std::make_tuple("STRID_CF18_B_SIGNAL", "", kGroupDeviceProps, true) },
		{ kPropCF18InternalSynSignal, std::make_tuple("STRID_CF18_INTERNAL_SYN_SIGNAL", "", kGroupDeviceProps, true) },
		{ kPropCF18InternalTriggerSignal, std::make_tuple("STRID_CF18_INTERNAL_TRIGGER_SIGNAL", "", kGroupDeviceProps, true) },

		{ kPropCF18ETJitterEliminationTime, std::make_tuple("STRID_CF18_JITTER_ELIMINATION_TIME", "", kPropCF18ExternalTriggerSignal, false) },
		{ kPropCF18ETPolarityReversal, std::make_tuple("STRID_CF18_POLARITY_REVERSAL", "", kPropCF18ExternalTriggerSignal, false) },
		{ kPropCF18ETRisingCount, std::make_tuple("STRID_CF18_RISING_COUNT", "", kPropCF18ExternalTriggerSignal, false) },
		{ kPropCF18ETFallingCount, std::make_tuple("STRID_CF18_FALLING_COUNT", "", kPropCF18ExternalTriggerSignal, false) },

		{ kPropCF18ENJitterEliminationTime, std::make_tuple("STRID_CF18_JITTER_ELIMINATION_TIME", "", kPropCF18ExternalSynSignal, false) },
		{ kPropCF18ENPolarityReversal, std::make_tuple("STRID_CF18_POLARITY_REVERSAL", "", kPropCF18ExternalSynSignal, false) },
		{ kPropCF18ENRisingCount, std::make_tuple("STRID_CF18_RISING_COUNT", "", kPropCF18ExternalSynSignal, false) },
		{ kPropCF18ENFallingCount, std::make_tuple("STRID_CF18_FALLING_COUNT", "", kPropCF18ExternalSynSignal, false) },

		{ kPropCF18BJitterEliminationTime, std::make_tuple("STRID_CF18_JITTER_ELIMINATION_TIME", "", kPropCF18BSignal, false) },

		{ kPropCF18INChannel, std::make_tuple("STRID_CF18_CHANNEL", "", kPropCF18InternalSynSignal, false) },
		{ kPropCF18Cycle, std::make_tuple("STRID_CF18_CYCLE", "", kPropCF18InternalSynSignal, false) },
		{ kPropCF18Unit, std::make_tuple("STRID_CF18_UNIT", "", kPropCF18InternalSynSignal, false) },
		{ kPropCF18INHighLevelWidth, std::make_tuple("STRID_CF18_HIGH_LEVEL_WIDTH", "", kPropCF18InternalSynSignal, false) },
		{ kPropCF18INRisingDelay, std::make_tuple("STRID_CF18_RISING_DELAY", "", kPropCF18InternalSynSignal, false) },

		{ kPropCF18ITChannel, std::make_tuple("STRID_CF18_CHANNEL", "", kPropCF18InternalTriggerSignal, false) },
		{ kPropCF18ITHighLevelWidth, std::make_tuple("STRID_CF18_HIGH_LEVEL_WIDTH", "", kPropCF18InternalTriggerSignal, false) },
		{ kPropCF18ITRisingDelay, std::make_tuple("STRID_CF18_RISING_DELAY", "", kPropCF18InternalTriggerSignal, false) },
		//CF18-control-end////////////////////////////////////////////////////
	};

	InitUI();
}

CSPropertyViewDevice::~CSPropertyViewDevice()
{
	if (m_property_editor)
	{
		delete m_property_editor;
		m_property_editor = nullptr;
	}

	if (m_property_manager)
	{
		delete m_property_manager;
		m_property_manager = nullptr;
	}

	delete ui;
}

void CSPropertyViewDevice::UpdateCurrentPropertyList()
{
	//直接刷新整个属性表格
	if (!m_currrent_device_ptr.isNull())
	{
		updatePropertyList(ListForCamera);
	}
	else
	{
		updatePropertyList(ListForSystem);
	}
}

void CSPropertyViewDevice::InitUI()
{
	//设置列表样式
	ui->widgetPropertyBrowser->setHeaderVisible(false);//取消表头
	ui->widgetPropertyBrowser->setAlternatingRowColors(false);
	ui->widgetPropertyBrowser->setIndentation(10);

	updatePropertyList(ListForSystem);//初始显示系统属性页

	//连接设备管理器信号
	connect(m_device_magager_ptr.data(), &DeviceManager::currentDeviceChanged, this, &CSPropertyViewDevice::slotCurrentDeviceChanged);

	connect(m_device_magager_ptr.data(), &DeviceManager::signalCurrentGroupChanged, this, &CSPropertyViewDevice::slotCurrentGroupChanged);

	//连接属性值变更信号
	connect(m_property_manager, &QtVariantPropertyManager::valueChanged, this, &CSPropertyViewDevice::slotPropertyValueChanged);
	connect(m_property_manager, &QtVariantPropertyManager::signalEditingFinished, this, &CSPropertyViewDevice::slotPropertyEditingFinished);

	//连接选项选择信号
	connect(ui->widgetPropertyBrowser, &CSTreePropertyBrowser::currentItemChanged, this ,&CSPropertyViewDevice::slotPropertyItemChanged);

	//信息刷新计时器
	m_CF18_info_timer_ptr = new QTimer();
	m_CF18_info_timer_ptr->setInterval(kUpdateCF18InfoInterval);
	QObject::connect(m_CF18_info_timer_ptr, &QTimer::timeout, this, &CSPropertyViewDevice::slotUpdateCF18Info);
	m_CF18_info_timer_ptr->start();
}

void CSPropertyViewDevice::updatePropertyList(PropertyListType list_type)
{
	//获取对应的属性列表
	QList<PropertyType> new_property_types;
	switch (list_type)
	{
		//相机属性和触发器属性同为设备属性
	case CSPropertyViewDevice::ListForCamera:
	case CSPropertyViewDevice::ListForTrigger:
	{
		if (m_currrent_device_ptr && (Unconnected == m_currrent_device_ptr->getState())) {
			updatePropertyList(ListForSystem);
		}
		else {
			getPropertyTypes(m_currrent_device_ptr, new_property_types);
		}
	}
	break;
	case CSPropertyViewDevice::ListForSystem:
	{
		getPropertyTypes(QSharedPointer<Device>(), new_property_types);
	}
	break;
	case CSPropertyViewDevice::ListForGroup: {
		getPropertyTypes(QSharedPointer<Device>(), new_property_types);
		break;
	}
	default:
		break;
	}


	//判断属性表是否相同 
	if (m_old_property_types == new_property_types)
	{
		// 相同则只刷新列表
		updatePropertiesValue(m_currrent_device_ptr, m_old_property_types);
	}
	else
	{
		//清空属性控件对应表
		m_map_edit_type.clear();
		m_map_type_edit.clear();
		//不相同则删除属性表,在刷新列表时新建列表
		for (auto property_to_remove : ui->widgetPropertyBrowser->properties())
		{
			ui->widgetPropertyBrowser->removeProperty(property_to_remove);
		}
		m_old_property_types = new_property_types;
		updatePropertiesValue(m_currrent_device_ptr, new_property_types);

	}
}

void CSPropertyViewDevice::getPropertyTypes(QSharedPointer<Device> device_ptr, QList<PropertyType> & prop_types)
{
	if (m_is_group) {
		prop_types.push_back(kPropGropName);
		prop_types.push_back(kPropGropDeviceNum);
		return;
	}

	if (device_ptr.isNull())//无设备,显示系统信息
	{
		prop_types.push_back(kPropProductName);
		prop_types.push_back(kPropProductVersion);

	}
	else//有设备 TODO:Device类接口不齐全,先实现基础参数配置
	{
		if (device_ptr->IsCamera())//相机参数
		{
			prop_types.push_back(kPropDeviceName);
			prop_types.push_back(kPropModel);
			prop_types.push_back(kPropIP);
			prop_types.push_back(kPropSN);
			
			if (device_ptr->IsDeviceIndexSupported())
			{
				prop_types.push_back(kPropDeviceIndex);
			}
			
			prop_types.push_back(kPropStatus);
			if (device_ptr->IsStandByModeSupported())
			{
				prop_types.push_back(kPropHardwareStandby);
			}

			prop_types.push_back(kPropSyncSource);
			if (device_ptr->IsPulseWidthUnit100Ns())
			{
				prop_types.push_back(kPropPulseWidth100Ns);

			}
			else
			{
				prop_types.push_back(kPropPulseWidth);

			}

			if (FunctionCustomizer::GetInstance().isUsabilityVersion())
			{
				prop_types.push_back(kPropResolution);
				prop_types.push_back(kPropRoiOffsetX);
				prop_types.push_back(kPropRoiOffsetY);
				prop_types.push_back(kPropRoiWidth);
				prop_types.push_back(kPropRoiHeight);
			}

			prop_types.push_back(kPropFrameRate);

			prop_types.push_back(kPropExposureTime);
			prop_types.push_back(kPropExposureTimeUnit);

			if (device_ptr->IsOverExposureTipSupported())
			{
				prop_types.push_back(kPropOverExposureTip);
			}
			if (device_ptr->IsPIVSupported())
			{
				//TODO:后续支持级联拓扑模式
				{
					// 非级联拓扑模式，PIV仅支持开启/关闭
					prop_types.push_back(kPropEnablePIV);
				}
			}
			prop_types.push_back(kPropTriggerMode);
			if (device_ptr->IsExternalTriggerModeSupported())
			{
				prop_types.push_back(kPropExternalTriggerMode);
			}
			if (device_ptr->IsJitterEliminationLengthSupported())
			{
				prop_types.push_back(kPropJitterEliminationLength);
			}

			prop_types.push_back(kPropRecordingOffsetMode);
			prop_types.push_back(kPropRecordingOffset);
			prop_types.push_back(kPropRecordingUnit);
			prop_types.push_back(kPropRecordingLength);
			//prop_types.push_back(kPropVideoFormat);

			if (device_ptr->IsSdiCtrlSupported())
			{
				prop_types.push_back(kPropSdiFpsResol);
			}
			prop_types.push_back(kPropPixelBitDepth);
			prop_types.push_back(kPropDisplayBitDepth);
			prop_types.push_back(kPropStreamType);
			//prop_types.push_back(kPropWatermark);
			prop_types.push_back(kPropTriggerSyncEnable);
		}
		else//触发器参数 
		{
			if (FunctionCustomizer::GetInstance().isCF18ControlSupported()) {
				prop_types.push_back(kPropDeviceName);
				prop_types.push_back(kPropModel);
				prop_types.push_back(kPropIP);
				//prop_types.push_back(kPropSN);
				prop_types.push_back(kPropStatus);

				prop_types.push_back(kPropCF18ETJitterEliminationTime);
				prop_types.push_back(kPropCF18ETPolarityReversal);
				prop_types.push_back(kPropCF18ETRisingCount);
				prop_types.push_back(kPropCF18ETFallingCount);

				prop_types.push_back(kPropCF18ENJitterEliminationTime);
				prop_types.push_back(kPropCF18ENPolarityReversal);
				prop_types.push_back(kPropCF18ENRisingCount);
				prop_types.push_back(kPropCF18ENFallingCount);

				prop_types.push_back(kPropCF18BJitterEliminationTime);

				prop_types.push_back(kPropCF18INChannel);
				prop_types.push_back(kPropCF18Cycle);
				//prop_types.push_back(kPropCF18Unit);
				prop_types.push_back(kPropCF18INHighLevelWidth);
				prop_types.push_back(kPropCF18INRisingDelay);

				prop_types.push_back(kPropCF18ITChannel);
				prop_types.push_back(kPropCF18ITHighLevelWidth);
				prop_types.push_back(kPropCF18ITRisingDelay);
			}else{
				prop_types.push_back(kPropModel);
				prop_types.push_back(kPropIP);
				prop_types.push_back(kPropDeviceName);
				if (device_ptr->IsDeviceIndexSupported())
				{
					prop_types.push_back(kPropDeviceIndex);
				}
				prop_types.push_back(kPropStatus);
				prop_types.push_back(kPropSyncSource);
				prop_types.push_back(kPropTriggerPulseWidth);
				prop_types.push_back(kPropFrameRate);
				prop_types.push_back(kPropTriggerMode);
				prop_types.push_back(kPropSpacebarTrigger);
				prop_types.push_back(kPropExternalTriggerMode);
				if (FunctionCustomizer::GetInstance().isH150Enabled())
				{
					prop_types.push_back(kPropChn1Delay);
					prop_types.push_back(kPropChn2Delay);
					prop_types.push_back(kPropChn3Delay);
					prop_types.push_back(kPropChn4Delay);
					prop_types.push_back(kPropChn5Delay);
					prop_types.push_back(kPropChn6Delay);
					prop_types.push_back(kPropChn7Delay);
					prop_types.push_back(kPropChn8Delay);
				}
				else
				{
					if (device_ptr->IsPIVSupported())
					{
						prop_types.push_back(kPropChn5Delay);
						prop_types.push_back(kPropChn6Delay);
						prop_types.push_back(kPropChn7Delay);
						prop_types.push_back(kPropChn8Delay);
					}
				}

				if (device_ptr->IsPIVSupported())
				{
					prop_types.push_back(kPropEnablePIV);
					//				prop_types.push_back(kPropPIVExposureTime); // PIV曝光时间不再通过上位机设置
					prop_types.push_back(kPropLaserAReadyOffset);
					prop_types.push_back(kPropLaserASendOffset);
					prop_types.push_back(kPropLaserAPulseWidth);
					prop_types.push_back(kPropLaserBReadyOffset);
					prop_types.push_back(kPropLaserBSendOffset);
					prop_types.push_back(kPropLaserBPulseWidth);
				}

				prop_types.push_back(kPropRecordingOffsetMode);
				prop_types.push_back(kPropRecordingOffset);
				prop_types.push_back(kPropRecordingUnit);
				prop_types.push_back(kPropRecordingLength);
			}
			}
	}
}

void CSPropertyViewDevice::updatePropertiesValue(QSharedPointer<Device> device_ptr, const QList<PropertyType> prop_types)
{
	for (auto prop_type : prop_types)
	{
		updatePropertyValue(device_ptr, prop_type);
	}
}

void CSPropertyViewDevice::updateEnumProperty(QSharedPointer<Device> device_ptr, const PropertyType prop_type, int device_prop_type, pf pfunction, bool allow_edit)
{
	//枚举类型属性,先获取全部支持的选项,建立选项序号和枚举值的映射关系,判断当前设备的参数是否在其支持列表中,不在的话则设置为第一个
	//获取支持的选项
	QVariantList var_list;//支持的枚举值
	auto devicePropType = (Device::PropType)device_prop_type;
	device_ptr->getSupportedProperties((Device::PropType)device_prop_type, var_list);
	QStringList str_list;//枚举值对应的字符串,按选项顺序排序
	QMap<int, QVariant> index_map; //<选项序号, 属性值>

	//判断设备属性是否在其支持列表中
	bool b_prop_exist = false;
	int device_property = device_ptr->getProperty((Device::PropType)device_prop_type).toInt();
	int device_property_index = -1;//设备当前属性值对应的序号
	for (int i = 0; i < var_list.count(); i++)
	{
		int enum_value = var_list.at(i).toInt();
		if (enum_value == device_property)//当前支持该属性
		{
			b_prop_exist = true;
			device_property_index = i;
		}
		index_map[i] = enum_value;
		str_list << pfunction(enum_value);
	}
	m_map_enums[prop_type] = index_map;//保存对应的映射关系


	//设备属性当前不支持,设为支持的属性
	if (!b_prop_exist)
	{
		if (device_ptr->allowsAcquire())
		{
			device_ptr->setProperty((Device::PropType)device_prop_type, index_map[0]);
		}
		device_property_index = 0;

	}
	//添加到属性列表中
	addProperty(prop_type, str_list, device_property_index, allow_edit);

}

void CSPropertyViewDevice::updatePropertyValue(QSharedPointer<Device> device_ptr, const PropertyType prop_type)
{
	//刷新属性列表时禁止属性变更操作
	m_enable_value_change = false;

	//判断传入的属性类别是不是在属性关系表中
	if (m_map_props.find(prop_type) == m_map_props.end())
	{
		m_enable_value_change = true;
		return;
	}

	if (m_is_group) {
		switch (prop_type)
		{
		case kPropGropName:
			addProperty(kPropGropName, m_group_name);
			break;
		case kPropGropDeviceNum:
			addProperty(kPropGropDeviceNum, m_group_device_num);
			break;
		default:
			break;
		}
		m_enable_value_change = true;
		return;
	}

	if (device_ptr.isNull())//显示系统信息
	{
		switch (prop_type)
		{
		case kPropProductName:
			addProperty(kPropProductName, UIExplorer::instance().getProductFullName());
			break;
		case kPropProductVersion:
			addProperty(kPropProductVersion, UIExplorer::instance().getProductVersion());
			break;
		default:
			break;
		}
		m_enable_value_change = true;
		return;
	}
	//TODO: 设备属性列表 Device类接口不齐全,先实现基础参数配置

	DeviceState current_state = device_ptr->getState();
	bool allow_edit_stopped = false;//只有停机时可修改
	if (current_state == Connected)
	{
		allow_edit_stopped = true;
	}

	bool allow_edit = false;//预览时可修改
	if (current_state == Connected ||
		current_state == Previewing )
	{
		allow_edit = true;
	}

	bool allow_edit_acquiring = false;//采集时可修改
	if (current_state == Connected ||
		current_state == Previewing ||
		current_state == Acquiring)
	{
		allow_edit_acquiring = true;
	}

	switch (prop_type)
	{
	case kPropModel:
		addProperty(prop_type, device_ptr->getModelName());
		break;
	case kPropIP:
		addProperty(prop_type, device_ptr->getIp());
		break;
	case kPropSN:
		addProperty(prop_type, device_ptr->getSn());
		break;
	case kPropDeviceName:
		addProperty(prop_type, device_ptr->getProperty(Device::PropName).toString(), allow_edit_acquiring);
		break;
	case kPropDeviceIndex:
		addProperty(prop_type, device_ptr->getProperty(Device::PropDeviceIndex).toInt(),0,UINT16_MAX,1, allow_edit_acquiring);
		break;

	case kPropStatus:
		addProperty(prop_type, device_ptr->getStateStr());
		break;
	case kPropHardwareStandby:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropHardwareStandby, &DeviceUtils::getOnOffText, device_ptr->AllowEditStandByMode());
	}
	break;
	case kPropTriggerMode:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropTriggerMode, &DeviceUtils::getTriggerModeText, device_ptr->AllowsEditTriggerMode());
	}
	break;
	case kPropExternalTriggerMode:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropExTriggerMode, &DeviceUtils::getExternalTriggerModeText, device_ptr->AllowsEditExternalTriggerMode());
	}
	break;
	case kPropJitterEliminationLength:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange(Device::PropJitterEliminationLength, min, max, inc);
		//自动修正
		qint64 val = device_ptr->getProperty(Device::PropJitterEliminationLength).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val && (device_ptr->allowsAcquire() || device_ptr->IsSupportAcquireEdit()))
		{
			device_ptr->setProperty(Device::PropJitterEliminationLength, correct_val);
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropJitterEliminationLength).toInt(), min, max, inc, device_ptr->AllowsEditJitterEliminationLength());
	}
	break;
	case kPropFrameRate:
	{
		//特殊选项:提供支持的典型值选项,并且支持编辑
		//获取典型值
		bool allow_edit_fps = device_ptr->AllowsEditFrameRate();
		QVariantList var_list;
		device_ptr->getTypicalProperties(Device::PropFrameRate, var_list, false);//获取典型值,且不需要自定义选项
		QStringList str_list;//属性对应的字符串,按选项顺序排

		//准备帧率列表
		for (int i = 0; i < var_list.count(); i++)
		{
			int val_item = var_list.at(i).toInt();
			str_list << QString("%1").arg(val_item);
		}

		//获取当前数据
		int current_val = device_ptr->getProperty(Device::PropFrameRate).toInt();
		//获取范围	
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange(Device::PropFrameRate, min, max, inc);

		//如果当前帧率不支持,自动修正到最大值并且下发,界面不支持编辑的时候也可以自动修正
		if (current_val > max/* && allow_edit_fps*/ && allow_edit_acquiring)
		{
			current_val = max;
			device_ptr->setProperty(Device::PropFrameRate, max);
		}
		if (!allow_edit_fps)//不可编辑时不限制大小
		{
			min = 0;
			max = VALID_MAX;
		}

		//根据属性名称判断当前属性列表中是否已经存在该属性项了,没有则新建,有则更新
		//查找当前属性列表
		QString prop_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[prop_type]));
		QtVariantProperty * prop_item = dynamic_cast<QtVariantProperty*>(m_map_type_edit.value(prop_type, nullptr));
		if (!prop_item)//没有找到,生成一个
		{

			prop_item = m_property_manager->addProperty(CSVariantPropertyManager::intComboPropertyTypeId(), prop_name);
			prop_item->setAttribute("selections", str_list);//添加枚举属性值
			if (min <= max)//取值范围合法性
			{
				prop_item->setAttribute("minimum", (int)min);//设置取值范围
				prop_item->setAttribute("maximum", (int)max);
			}
			prop_item->setValue(current_val);

			QString prop_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[prop_type]));
			prop_item->setDescriptionToolTip(prop_desc);//写入描述文案

			//添加到自己的父节点中
			addProperty(prop_item, getPropertyParent(prop_type));
			//记录控件对应的属性种类
			m_map_edit_type[prop_item] = prop_type;
			m_map_type_edit[prop_type] = prop_item;

		}
		else//找到了,设置新的属性值
		{
			prop_item->setAttribute("selections", str_list);//设置属性值
			if (min <= max)//取值范围合法性
			{
				prop_item->setAttribute("minimum", (int)min);//设置取值范围
				prop_item->setAttribute("maximum", (int)max);
			}
			prop_item->setValue(current_val);
		}
		prop_item->setEnabled(allow_edit_fps);


	}
	break;
	case kPropExposureTime:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange(Device::PropExposureTime, min, max, inc);
		//自动修正
		qint64 val = device_ptr->getProperty(Device::PropExposureTime).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val&& (device_ptr->allowsAcquire() || device_ptr->IsSupportAcquireEdit()))
		{
			device_ptr->setProperty(Device::PropExposureTime, correct_val);
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropExposureTime).toInt(), min, max, inc, device_ptr->AllowsEditExposureTime());
	}
	break;

	case kPropExposureTimeUnit:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropExposureTimeUnit, &DeviceUtils::getExposureTimeUnitText, device_ptr->AllowsEditExposureTime());

	}
	break;

	case kPropOverExposureTip:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropOverExposureTipEnable, &DeviceUtils::getOnOffText, device_ptr->AllowsEditOverExposureTip());
	}
	break;

	case kPropSyncSource:
	{
		bool bEdit = allow_edit;
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		updateEnumProperty(device_ptr, prop_type, Device::PropSyncSource, &DeviceUtils::getSyncSourceText, bEdit);

	}
	break;

	case kPropPulseWidth:
	{
		qint64 min = 2, max = min, inc = 0;
		device_ptr->getPropertyRange(Device::PropPulseWidth, min, max, inc);
		bool bEdit = allow_edit;
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropPulseWidth).toInt(), min, max, inc, bEdit);
		//device_ptr->setProperty(Device::PropPulseWidth, val);

	}
	break;

	case kPropPulseWidth100Ns:
	{
		qint64 min = 2, max = min, inc = 0;
		device_ptr->getPropertyRange(Device::PropPulseWidth, min, max, inc);
		bool bEdit = allow_edit;
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropPulseWidth).toInt(), min, max, inc, bEdit);
		//device_ptr->setProperty(Device::PropPulseWidth, val);

	}
	break;

	case kPropTriggerPulseWidth:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange(Device::PropPulseWidth, min, max, inc);
		bool bEdit = allow_edit;
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropPulseWidth).toInt(), min, max, inc, bEdit);

	}
	break;
	
	case kPropResolution:
	{
		//特殊选项:提供支持的典型值选项,并且在当前设置不为典型值时显示为手动模式
		//获取典型值
		QVariantList var_list;
		device_ptr->getTypicalProperties(Device::PropRoi, var_list,false);//获取典型值,且不需要自定义选项
		QStringList str_list;//属性对应的字符串,按选项顺序排
		QMap<int, QVariant> index_map;//选项序号对应属性值

		//判断设备当前的属性是否在支持列表中
		bool b_prop_exist = false;
		QRect current_roi = device_ptr->GetRoi();
		int current_roi_index = -1;//当前设备roi对应的选项序号
		for (int i = 0; i < var_list.count(); i++)
		{
			QRect roi_item = var_list.at(i).toRect();
			if (roi_item == current_roi)//当前为该roi
			{
				b_prop_exist = true;
				current_roi_index = i;
			}
			index_map[i] = roi_item;
			str_list << QString("%1 x %2").arg(roi_item.width()).arg(roi_item.height());
		}
		m_map_enums[prop_type] = index_map;//保存当前的对应关系

		//如果当前roi不支持,则选项显示为"手动模式"(采集卡除外)
		if (!b_prop_exist )
		{
			current_roi_index = str_list.size();
			str_list << tr("Manual Mode");
		}

		addProperty(prop_type, str_list,current_roi_index, device_ptr->allowEditRoi());
	}
	break;
	
	case kPropRoiOffsetX:
	{
		HscIntRange int_range{};
		device_ptr->getRoiXRange(int_range);
		bool bEdit = allow_edit;
		if (bEdit)
		{
			if (device_ptr->GetRoiConstraintCondition() == Device::kCenterConstraint || device_ptr->GetRoiConstraintCondition() == Device::kHorCenterConstraint)
			{
				bEdit = false;
			}
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropRoi).toRect().x(), int_range.min, int_range.max, int_range.inc, bEdit);
		
	}
	break;	
	
	case kPropRoiOffsetY:
	{
		HscIntRange int_range{};
		device_ptr->getRoiYRange(int_range);
		bool bEdit = allow_edit;
		if (bEdit)
		{
			if (device_ptr->GetRoiConstraintCondition() == Device::kCenterConstraint || device_ptr->GetRoiConstraintCondition() == Device::kVerCenterConstraint)
			{
				bEdit = false;
			}
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropRoi).toRect().y(), int_range.min, int_range.max, int_range.inc, bEdit);
	}
	break;

	case kPropRoiWidth:
	{
		HscIntRange int_range{};
		if (device_ptr->GetRoiConstraintCondition() == Device::kCenterConstraint || device_ptr->GetRoiConstraintCondition() == Device::kHorCenterConstraint)
		{
			device_ptr->getRoiWidthRange(0, int_range);
		}
		else
		{
			device_ptr->getRoiWidthRange(device_ptr->getProperty(Device::PropRoi).toRect().x(), int_range);
		}
		auto bEdit = allow_edit && int_range.inc != 0;
		addProperty(prop_type, device_ptr->getProperty(Device::PropRoi).toRect().width(), int_range.min, int_range.max, int_range.inc, bEdit);
	}
	break;

	case kPropRoiHeight:
	{
		HscIntRange int_range{};
		if (device_ptr->GetRoiConstraintCondition() == Device::kCenterConstraint || device_ptr->GetRoiConstraintCondition() == Device::kVerCenterConstraint)
		{
			device_ptr->getRoiHeightRange(0, int_range);
		}
		else
		{
			device_ptr->getRoiHeightRange(device_ptr->getProperty(Device::PropRoi).toRect().y(), int_range);
		}
		auto bEdit = allow_edit && int_range.inc != 0;
		addProperty(prop_type, device_ptr->getProperty(Device::PropRoi).toRect().height(), int_range.min, int_range.max, int_range.inc, bEdit);
	}
	break;

	case kPropRecordingOffsetMode:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropRecordingOffsetMode, &DeviceUtils::getRecordingOffsetModeText, device_ptr->AllowsEditRecordingOffsetMode());
	}
	break;

	case kPropRecordingOffset:
	{
		qint64 min = 0, max = 0, inc = 0;
		bool allow_edit_offset = device_ptr->AllowsEditRecordingSettings()/* && !device_ptr->IsPIVEnabled()*/ && !device_ptr->IsRealtimeExportSupported();
		if (!allow_edit_offset)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					RecordMode mode = device_ptr->getProperty(Device::PropRecordingOffsetMode).value<RecordMode>();
					if (mode == RECORD_AFTER_SHUTTER)
					{
						allow_edit_offset = true;
					}
				}
			}
		}
		if (allow_edit_offset)
		{
			device_ptr->getPropertyRange(Device::PropRecordingOffset, min, max, inc);
			//自动修正
			qint64 val = device_ptr->getProperty(Device::PropRecordingOffset).toInt();
			qint64 correct_val = val;
			if (val < min)
			{
				correct_val = min;
			}
			if (val > max)
			{
				correct_val = max;
			}
			if (correct_val != val&& device_ptr->allowsAcquire() )
			{
				device_ptr->setProperty(Device::PropRecordingOffset, correct_val);
			}

		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropRecordingOffset).toInt(), min, max, inc, allow_edit_offset);
	}
	break;

	case kPropRecordingLength:
	{
		qint64 min = 0, max = 0, inc = 0;
		bool bEdit = device_ptr->AllowsEditRecordingSettings();
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				if (device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		if (bEdit)
		{
			device_ptr->getPropertyRange(Device::PropRecordingLength, min, max, inc);
			//自动修正
			qint64 val = device_ptr->getProperty(Device::PropRecordingLength).toInt();
			qint64 correct_val = val;
			if (val < min)
			{
				correct_val = min;
			}
			if (val > max)
			{
				correct_val = max;
			}
			if (correct_val != val&& device_ptr->allowsAcquire() &&
				( m_bRecordUnitChange || val < min/*小于最小值时需要修正*/))
			{
				device_ptr->setProperty(Device::PropRecordingLength, correct_val);
				if (m_bRecordUnitChange)
				{
					if (max < val)  val = max;
				//	m_bRecordUnitChange = false;
				}
			}
			//如果不支持停机内存管理,则在最大范围变更时不做自动限制(最大值等于当前值)
			//if (!device_ptr->IsMemoryManagementSupported())
			{
				if (max < val)
				{
					max = val;
				}
			}
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropRecordingLength).toInt(), min, max, inc, bEdit);
	}
	break;

	case kPropRecordingUnit:
	{
		bool bEdit = device_ptr->AllowsEditRecordingUnit();
		if (!bEdit)
		{
			if (current_state == DeviceState::Acquiring)
			{
				// 高采禁止改保存单位
				//if (device_ptr->IsSupportAcquireEdit())
				//{
				//	bEdit = true;
				//}
			}
		}
		updateEnumProperty(device_ptr, prop_type, Device::PropRecordingUnit, &DeviceUtils::getRecordingUnitText, bEdit);
	}
	break;

	//case kPropVideoFormat:
	//{
	//	updateEnumProperty(device_ptr, prop_type, Device::PropVideoFormat, &DeviceUtils::getVideoFormatText, allow_edit);
	//}
	//break;

	case kPropStreamType:
	{
		bool bEdit = allow_edit;
		if (!bEdit)
		{
			//if (device_ptr->IsSupportAcquireEdit())
			//{
				//bEdit = true;
			//}
		}
		updateEnumProperty(device_ptr, prop_type, Device::PropStreamType, &DeviceUtils::getStreamTypeText, bEdit);
	}
	break;

	case kPropSdiFpsResol:
	{

		updateEnumProperty(device_ptr, prop_type, Device::PropSdiFpsResols, &DeviceUtils::getSDIParamText, allow_edit_stopped);//应固件要求,只有停机时可以修改
	}
	break;
	///case kPropWatermark:
	//{
	//	updateEnumProperty(device_ptr, prop_type, Device::PropWatermarkEnable, &DeviceUtils::getOnOffText, device_ptr->AllowsEditWatermark());
	//}
	//break;

	case kPropTriggerSyncEnable:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropTriggerSyncEnable, &DeviceUtils::getOnOffText, device_ptr->AllowsEditTriggerSyncEnabled());
	}
	break;
	case kPropChn1Delay:
	case kPropChn2Delay:
	case kPropChn3Delay:
	case kPropChn4Delay:
	case kPropChn5Delay:
	case kPropChn6Delay:
	case kPropChn7Delay:
	case kPropChn8Delay: 
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(prop_type - kPropChn1Delay + Device::PropChn1Delay), min, max, inc);
		addProperty(prop_type, 
			device_ptr->getProperty((Device::PropType)(prop_type - kPropChn1Delay + Device::PropChn1Delay)).toInt(),
			min, max, inc, 
			device_ptr->IsRootTrigger() && device_ptr->getState() == Connected);

	}
	break;
	case kPropEnablePIV:
	{
		//枚举类型属性,先获取全部支持的选项,建立选项序号和枚举值的映射关系,判断当前设备的参数是否在其支持列表中,不在的话则设置为第一个
		//获取支持的选项
		QVariantList var_list;//支持的枚举值
		if (device_ptr->IsPIVSupported() && device_ptr->getProperty(Device::PropSyncSource) == SYNC_EXTERNAL)
		{
			var_list << true;
		}
		var_list << false;
		QStringList str_list;//枚举值对应的字符串,按选项顺序排序
		QMap<int, QVariant> index_map; //<选项序号, 属性值>

		//判断设备属性是否在其支持列表中
		bool b_prop_exist = false;
		auto current_param = qvariant_cast<HscPIVParam>(device_ptr->getProperty(Device::PropPivParam));
		int device_property = current_param.enabled;
		int device_property_index = -1;//设备当前属性值对应的序号
		for (int i = 0; i < var_list.count(); i++)
		{
			int enum_value = var_list.at(i).toInt();
			if (enum_value == device_property)//当前支持该属性
			{
				b_prop_exist = true;
				device_property_index = i;
			}
			index_map[i] = enum_value;
			str_list << DeviceUtils::getOnOffText(enum_value);
		}
		m_map_enums[prop_type] = index_map;//保存对应的映射关系

		//设备属性当前不支持,设为支持的属性
		if (!b_prop_exist)
		{
			device_property_index = 0;
			current_param.enabled = index_map[device_property_index].toInt();
			device_ptr->setProperty(Device::PropPivParam, QVariant::fromValue(current_param));
		}
		//添加到属性列表中
		addProperty(prop_type, str_list, device_property_index, device_ptr->allowsEditPIVEnable());
	}
	break;
	case kPropPixelBitDepth:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropPixelBitDepth, &DeviceUtils::getPixelBitDepthText, allow_edit);//应固件要求,改为预览时可编辑
	}
	break;
	case kPropDisplayBitDepth:
	{
		updateEnumProperty(device_ptr, prop_type, Device::PropDisplayBitDepth, &DeviceUtils::getDisplayBitDepthText, allow_edit_acquiring);
	}
	break;
	case kPropCF18ETJitterEliminationTime:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18ETJitterEliminationTime), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18ETJitterEliminationTime).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18ETJitterEliminationTime, correct_val);
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18ETJitterEliminationTime).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18ETPolarityReversal:
	{
		bool bEdit = allow_edit;
		updateEnumProperty(device_ptr, prop_type, Device::PropCF18ETPolarityReversal, &DeviceUtils::getCF18PolarityText, bEdit);
	}
	break;
	case kPropCF18ETRisingCount:
	{
		addUserDefineProperty(prop_type, QString::number(device_ptr->getProperty(Device::PropCF18ETRisingCount).toInt()), allow_edit);
	}
	break;
	case kPropCF18ETFallingCount:
	{
		addUserDefineProperty(prop_type, QString::number(device_ptr->getProperty(Device::PropCF18ETFallingCount).toInt()), allow_edit);
	}
	break;
	case kPropCF18ENJitterEliminationTime:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18ENJitterEliminationTime), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18ENJitterEliminationTime).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18ENJitterEliminationTime, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18ENJitterEliminationTime).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18ENPolarityReversal:
	{
		bool bEdit = allow_edit;
		updateEnumProperty(device_ptr, prop_type, Device::PropCF18ENPolarityReversal, &DeviceUtils::getCF18PolarityText, bEdit);
	}
	break;
	case kPropCF18ENRisingCount:
	{
		addUserDefineProperty(prop_type, QString::number(device_ptr->getProperty(Device::PropCF18ENRisingCount).toInt()), allow_edit);
	}
	break;
	case kPropCF18ENFallingCount:
	{
		addUserDefineProperty(prop_type, QString::number(device_ptr->getProperty(Device::PropCF18ENFallingCount).toInt()), allow_edit);
	}
	break;
	case kPropCF18BJitterEliminationTime:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18BJitterEliminationTime), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18BJitterEliminationTime).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18BJitterEliminationTime, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18BJitterEliminationTime).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18INChannel:
	{
		bool bEdit = allow_edit;
		updateEnumProperty(device_ptr, prop_type, Device::PropCF18INChannel, &DeviceUtils::getCF18Channel, bEdit);
	}
	break;
	case kPropCF18Cycle:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18Cycle), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18Cycle).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18Cycle, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18Cycle).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18Unit:
	{
		bool bEdit = allow_edit;
		updateEnumProperty(device_ptr, prop_type, Device::PropCF18Unit, &DeviceUtils::getCF18SignalUnit, bEdit);
	}
	break;
	case kPropCF18INHighLevelWidth:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18INHighLevelWidth), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18INHighLevelWidth).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18INHighLevelWidth, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18INHighLevelWidth).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18INRisingDelay:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18INRisingDelay), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18INRisingDelay).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18INRisingDelay, correct_val);
		}
		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18INRisingDelay).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18ITChannel:
	{
		bool bEdit = allow_edit;
		updateEnumProperty(device_ptr, prop_type, Device::PropCF18ITChannel, &DeviceUtils::getCF18Channel, bEdit);
	}
	break;
	case kPropCF18ITHighLevelWidth:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18ITHighLevelWidth), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18ITHighLevelWidth).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18ITHighLevelWidth, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18ITHighLevelWidth).toInt(), min, max, inc, allow_edit);
	}
	break;
	case kPropCF18ITRisingDelay:
	{
		qint64 min = 0, max = 0, inc = 0;
		device_ptr->getPropertyRange((Device::PropType)(Device::PropCF18ITRisingDelay), min, max, inc);

		// 自动修正
		qint64 val = device_ptr->getProperty(Device::PropCF18ITRisingDelay).toInt();
		qint64 correct_val = val;
		if (val < min && min > 0)
		{
			correct_val = min;
		}
		if (val > max && max > 0)
		{
			correct_val = max;
		}
		if (correct_val != val)
		{
			device_ptr->setProperty(Device::PropCF18ITRisingDelay, correct_val);
		}

		addProperty(prop_type, device_ptr->getProperty(Device::PropCF18ITRisingDelay).toInt(), min, max, inc, allow_edit);
	}
	break;
	default:
		break;
	}

	//属性刷新完成,允许属性变更
	m_enable_value_change = true;
}

void CSPropertyViewDevice::slotPropertyEditingFinished(QtProperty *property)
{
	//限制属性变更操作
	if (!m_enable_value_change)
	{
		return;
	}
	if (m_currrent_device_ptr.isNull())
	{
		return;
	}

	//获取对应的属性种类
	PropertyType prop_type = m_map_edit_type[property];
	QString value_text;
	//根据不同种类设置参数到设备
	switch (prop_type)
	{
	case kPropRecordingLength:
	{
		qint64 min = 0, max = 0, inc = 0;
		bool bEdit = m_currrent_device_ptr->AllowsEditRecordingSettings();
		if (!bEdit)
		{
			DeviceState current_state = m_currrent_device_ptr->getState();
			if (current_state == DeviceState::Acquiring)
			{
				if (m_currrent_device_ptr->IsSupportAcquireEdit())
				{
					bEdit = true;
				}
			}
		}
		if (bEdit)
		{
			m_currrent_device_ptr->getPropertyRange(Device::PropRecordingLength, min, max, inc);
			//自动修正
			qint64 val = m_currrent_device_ptr->getProperty(Device::PropRecordingLength).toInt();
			qint64 correct_val = val;
			if (val < min)
			{
				correct_val = min;
			}
			if (val > max)
			{
				correct_val = max;
			}
			if (correct_val != val && m_currrent_device_ptr->allowsAcquire())
			{
				m_currrent_device_ptr->setProperty(Device::PropRecordingLength, correct_val);

				addProperty(prop_type, m_currrent_device_ptr->getProperty(Device::PropRecordingLength).toInt(), min, max, inc, bEdit);
			}
		}

	}
		break;
	default:
		break;
	}
}

void CSPropertyViewDevice::slotPropertyValueChanged(QtProperty *property, const QVariant &val)
{
	//限制属性变更操作
	if (!m_enable_value_change)
	{
		return;
	}

	//获取对应的属性种类
	PropertyType prop_type = m_map_edit_type[property];
	QString value_text;
	//根据不同种类设置参数到设备
	switch (prop_type)
	{
	case kPropDeviceName:
		m_currrent_device_ptr->setProperty(Device::PropName, val);
		break;
	case kPropDeviceIndex:
		m_currrent_device_ptr->setProperty(Device::PropDeviceIndex, val);
		break;
	case kPropExposureTime:
		m_currrent_device_ptr->setProperty(Device::PropExposureTime, val);
		break;
	case kPropExposureTimeUnit:
	{
		m_currrent_device_ptr->setProperty(Device::PropExposureTimeUnit, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropOverExposureTip:
	{
		m_currrent_device_ptr->setProperty(Device::PropOverExposureTipEnable, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropFrameRate:
		//获取当前combox对应的属性值
		value_text = property->valueText();
		m_currrent_device_ptr->setProperty(Device::PropFrameRate, value_text.toInt());

		break;
	case kPropTriggerMode:
	{
		m_currrent_device_ptr->setProperty(Device::PropTriggerMode, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropExternalTriggerMode:
	{
		m_currrent_device_ptr->setProperty(Device::PropExTriggerMode, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropJitterEliminationLength:
		m_currrent_device_ptr->setProperty(Device::PropJitterEliminationLength, val);
		break;
	case kPropSyncSource:
	{
		m_currrent_device_ptr->setProperty(Device::PropSyncSource, (m_map_enums[prop_type])[val.toInt()]);

// 		if (m_currrent_device_ptr->IsPIVEnabled())
// 		{
// 			HscPIVParam param;
// 			param = m_currrent_device_ptr->getProperty(Device::PropPivParam).value<HscPIVParam>();
// 			if (param.enabled)
// 			{
// 				RecordMode mode = m_currrent_device_ptr->getProperty(Device::PropRecordingOffsetMode).value<RecordMode>();
// 				if (mode == RECORD_BEFORE_SHUTTER)
// 				{
// 					qint64 val = m_currrent_device_ptr->getProperty(Device::PropRecordingOffset).toInt();
// 					if (0 != val)
// 					{
// 						val = 0;
// 						m_currrent_device_ptr->setProperty(Device::PropRecordingOffset, val);
// 					}
// 					mode = RECORD_AFTER_SHUTTER;
// 					m_currrent_device_ptr->setProperty(Device::PropRecordingOffsetMode, mode);
// 				}
// 			}
// 		}
	}
	break;
	case kPropResolution:
	{
		m_bRecordUnitChange = true;
		m_currrent_device_ptr->setProperty(Device::PropRoi, (m_map_enums[prop_type])[val.toInt()]);
		m_bRecordUnitChange = false;
		{
			QRect newRoi = m_currrent_device_ptr->getProperty(Device::PropRoi).toRect();
			{
				HscAutoExposureParameter param = m_currrent_device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				param.autoExpArea = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			{
				HscIntelligentTriggerParamV2 param = m_currrent_device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				if ((m_currrent_device_ptr->IsSupportCMOSDigitGain() || m_currrent_device_ptr->IsIntelligentTriggerV5Supported()) && !m_currrent_device_ptr->isGrabberDevice())
				{
					QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), newRoi);
				}
				param.roi = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
		//联动:刷新roi() (设置生效自动刷新全部属性) 
// 		updatePropertyValue(m_currrent_device_ptr, kPropRoiOffsetX);
// 		updatePropertyValue(m_currrent_device_ptr, kPropRoiOffsetY);
// 		updatePropertyValue(m_currrent_device_ptr, kPropRoiWidth);
// 		updatePropertyValue(m_currrent_device_ptr, kPropRoiHeight);
	}
	break;

	case kPropPulseWidth:
	{
		m_currrent_device_ptr->setProperty(Device::PropPulseWidth, val);
	}
	break;
	case kPropPulseWidth100Ns:
	{
		m_currrent_device_ptr->setProperty(Device::PropPulseWidth, val);
	}
	break;
	case kPropTriggerPulseWidth:
	{
		m_currrent_device_ptr->setProperty(Device::PropPulseWidth, val);
	}
	break;
	case kPropRoiOffsetX:
	{
		QRect roi =  m_currrent_device_ptr->GetRoi();
		QRect oldroi(roi.left(), roi.top(), roi.width(), roi.height());
		roi.moveLeft(val.toInt());
		m_currrent_device_ptr->setProperty(Device::PropRoi, roi);
		QRect newRoi = m_currrent_device_ptr->getProperty(Device::PropRoi).toRect();
		if (newRoi.left() != oldroi.left() || newRoi.top() != oldroi.top() || newRoi.width() != oldroi.width() || newRoi.height() != oldroi.height())
		{
			{
				HscAutoExposureParameter param = m_currrent_device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				param.autoExpArea = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			{
				HscIntelligentTriggerParamV2 param = m_currrent_device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				if ((m_currrent_device_ptr->IsSupportCMOSDigitGain() || m_currrent_device_ptr->IsIntelligentTriggerV5Supported()) && !m_currrent_device_ptr->isGrabberDevice())
				{
					QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), newRoi);
				}
				param.roi = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
	}
	break;

	case kPropRoiOffsetY:
	{
		QRect roi = m_currrent_device_ptr->GetRoi();
		QRect oldroi(roi.left(), roi.top(), roi.width(), roi.height());
		roi.moveTop(val.toInt());
		m_currrent_device_ptr->setProperty(Device::PropRoi, roi);
		QRect newRoi = m_currrent_device_ptr->getProperty(Device::PropRoi).toRect();
		if (newRoi.left() != oldroi.left() || newRoi.top() != oldroi.top() || newRoi.width() != oldroi.width() || newRoi.height() != oldroi.height())
		{
			{
				HscAutoExposureParameter param = m_currrent_device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				param.autoExpArea = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			{
				HscIntelligentTriggerParamV2 param = m_currrent_device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				if ((m_currrent_device_ptr->IsSupportCMOSDigitGain() || m_currrent_device_ptr->IsIntelligentTriggerV5Supported()) && !m_currrent_device_ptr->isGrabberDevice())
				{
					QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), newRoi);
				}
				param.roi = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
	}
	break;

	case kPropRoiWidth:
	{	
		QRect roi = m_currrent_device_ptr->GetRoi();
		QRect oldroi(roi.left(), roi.top(), roi.width(), roi.height());
		roi.setWidth(val.toInt());
		m_bRecordUnitChange = true;
		m_currrent_device_ptr->setProperty(Device::PropRoi, roi);
		QRect newRoi = m_currrent_device_ptr->getProperty(Device::PropRoi).toRect();
		if (newRoi.left() != oldroi.left() || newRoi.top() != oldroi.top() || newRoi.width() != oldroi.width() || newRoi.height() != oldroi.height())
		{
			
			{
				HscAutoExposureParameter param = m_currrent_device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				param.autoExpArea = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			{
				HscIntelligentTriggerParamV2 param = m_currrent_device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				if ((m_currrent_device_ptr->IsSupportCMOSDigitGain() || m_currrent_device_ptr->IsIntelligentTriggerV5Supported()) && !m_currrent_device_ptr->isGrabberDevice())
				{
					QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), newRoi);
				}
				param.roi = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
		m_bRecordUnitChange = false;
	}
	break;

	case kPropRoiHeight:
	{
		QRect roi = m_currrent_device_ptr->GetRoi();
		QRect oldroi(roi.left(), roi.top(), roi.width(), roi.height());
		roi.setHeight(val.toInt());
		m_bRecordUnitChange = true;
		m_currrent_device_ptr->setProperty(Device::PropRoi, roi);
		
		QRect newRoi = m_currrent_device_ptr->getProperty(Device::PropRoi).toRect();
		if (newRoi.left() != oldroi.left() || newRoi.top() != oldroi.top() || newRoi.width() != oldroi.width() || newRoi.height() != oldroi.height())
		{
			
			{
				HscAutoExposureParameter param = m_currrent_device_ptr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				param.autoExpArea = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			{
				HscIntelligentTriggerParamV2 param = m_currrent_device_ptr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				if ((m_currrent_device_ptr->IsSupportCMOSDigitGain() || m_currrent_device_ptr->IsIntelligentTriggerV5Supported()) && !m_currrent_device_ptr->isGrabberDevice())
				{
					QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), newRoi);
				}
				param.roi = Device::QRectTCameraWindowRect(newRoi);
				m_currrent_device_ptr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
		m_bRecordUnitChange = false;
	}
	break;

	case kPropRecordingOffsetMode:
	{
		m_currrent_device_ptr->setProperty(Device::PropRecordingOffsetMode, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;

	case kPropRecordingUnit:
	{
		m_bRecordUnitChange = true;
		m_currrent_device_ptr->setProperty(Device::PropRecordingUnit, (m_map_enums[prop_type])[val.toInt()]);
		m_bRecordUnitChange = false;
	}
	break;

	case kPropRecordingOffset:
		m_currrent_device_ptr->setProperty(Device::PropRecordingOffset, val);
		break;

	case kPropRecordingLength:
		m_currrent_device_ptr->setProperty(Device::PropRecordingLength, val);
		break;

	//case kPropVideoFormat:
	//{
	//	m_currrent_device_ptr->setProperty(Device::PropVideoFormat, (m_map_enums[prop_type])[val.toInt()]);
	//}
	break;

	case kPropStreamType:
	{
		m_currrent_device_ptr->setProperty(Device::PropStreamType, (m_map_enums[prop_type])[val.toInt()]);
		if (val.toInt() != TYPE_RAW8 && val.toInt() != TYPE_RGB8888)
		{
			QRect roi = m_currrent_device_ptr->GetRoi();
			HscIntRange range;
			m_currrent_device_ptr->getRoiHeightRange(roi.y(), range);
			if (range.max < roi.height() || range.min > roi.height()) {
				m_currrent_device_ptr->setProperty(Device::PropRoi, roi);
			}
		}
		
		//联动: 保存格式刷新(设置生效自动刷新全部属性) 
		//updatePropertyValue(m_currrent_device_ptr, kPropVideoFormat);
	}
	break;

	case kPropSdiFpsResol:
	{
		m_currrent_device_ptr->setProperty(Device::PropSdiFpsResols, (m_map_enums[prop_type])[val.toInt()]);

	}
	break;
	case kPropWatermark:
	{
		m_currrent_device_ptr->setProperty(Device::PropWatermarkEnable, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropTriggerSyncEnable:
	{
		m_currrent_device_ptr->setProperty(Device::PropTriggerSyncEnable, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropChn1Delay:
	case kPropChn2Delay:
	case kPropChn3Delay:
	case kPropChn4Delay:
	case kPropChn5Delay:
	case kPropChn6Delay:
	case kPropChn7Delay:
	case kPropChn8Delay: 
	{
		m_currrent_device_ptr->setProperty((Device::PropType)(prop_type - kPropChn1Delay + Device::PropChn1Delay), val);
	}
	break;
	case kPropHardwareStandby:
	{
	
		if (m_currrent_device_ptr->IsStandByModeSupported())
		{
			bool isStandbyOn = (m_map_enums[prop_type])[val.toInt()].toBool();
			if (isStandbyOn)
			{
				QString msgStrID;
				DeviceState state = m_currrent_device_ptr->getState();
				if (state == Previewing || state == Acquiring || state == Recording)
				{
					msgStrID = "STRID_STANDBY_MSG_BUSY";
				}
				else if (state == Connected)
				{
					msgStrID = "STRID_STANDBY_MSG_IDLE";
				}

				if (!msgStrID.isEmpty())
				{
					if (UIUtils::showQuestionMsgBox(this, UIExplorer::instance().getStringById(msgStrID)) == true)
					{
						m_currrent_device_ptr->setProperty(Device::PropHardwareStandby, true);
					}
					else
					{
						ui->widgetPropertyBrowser->setFocus();
						m_currrent_device_ptr->setProperty(Device::PropHardwareStandby, false);
					}
				}
			}
			else
			{
				m_currrent_device_ptr->setProperty(Device::PropHardwareStandby, false);
			}
		}
	}
	break;
	case kPropEnablePIV:
	{
		bool on = (m_map_enums[prop_type])[val.toInt()].toBool();
		if (m_currrent_device_ptr->IsPIVEnabled() != on)
		{
			HscPIVParam param;
			param = m_currrent_device_ptr->getProperty(Device::PropPivParam).value<HscPIVParam>();
			param.enabled = on;
// 			if (on)
// 			{
// 				RecordMode mode = m_currrent_device_ptr->getProperty(Device::PropRecordingOffsetMode).value<RecordMode>();
// 				if (mode == RECORD_BEFORE_SHUTTER)
// 				{
// 					qint64 val = m_currrent_device_ptr->getProperty(Device::PropRecordingOffset).toInt();
// 					if (0 != val)
// 					{
// 						val = 0;
// 						m_currrent_device_ptr->setProperty(Device::PropRecordingOffset, val);
// 					}
// 					mode = RECORD_AFTER_SHUTTER;
// 					m_currrent_device_ptr->setProperty(Device::PropRecordingOffsetMode, mode);
// 				}
// 			}
			m_currrent_device_ptr->setProperty(Device::PropPivParam,QVariant::fromValue(param));

			//TODO:级联支持

			//TODO:参数修正

		}
	}
	break;
	case kPropPixelBitDepth:
	{
		//判断参数是否和当前一致 一致则跳过
		if ((m_map_enums[prop_type])[val.toInt()] == m_currrent_device_ptr->getProperty(Device::PropPixelBitDepth) )
		{
			break;
		}
		//添加弹窗确认是否需要执行操作 告知需要格式化视频列表
		//询问是否确定
		QTimer::singleShot(100, this, [=] {
			if (UIUtils::showQuestionMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_CHANGE_DEPTH_TIP")))
			{
				m_currrent_device_ptr->setProperty(Device::PropPixelBitDepth, (m_map_enums[prop_type])[val.toInt()]);
				//通知相机开始格式化
				HscResult delete_res = m_currrent_device_ptr->DeleteAllVideoClips();
				if (delete_res != HSC_OK)
				{
					UIUtils::showErrorMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_FORMAT_FAIL"));
					return;
				}
				//显示格式化进度对话框
				CSDlgProgressFormat dlg(m_currrent_device_ptr);
				dlg.exec();
				m_currrent_device_ptr->updateVideoSegmentList();

				UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_VL_MSG_CHANGE_DEPTH_SUCCESS_TIP"));
			}
			else
			{
				//不修改,回退界面设置参数(刷新当前参数)
				setFocus(Qt::FocusReason::OtherFocusReason);
				updateEnumProperty(m_currrent_device_ptr, prop_type, Device::PropPixelBitDepth, &DeviceUtils::getPixelBitDepthText, true);
			}
		});

	}
	break;
	case kPropDisplayBitDepth:
	{
		m_currrent_device_ptr->setProperty(Device::PropDisplayBitDepth, val);
	}
	break;
	case kPropCF18ETJitterEliminationTime:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ETJitterEliminationTime, val);
	}
	break;
	case kPropCF18ETPolarityReversal:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ETPolarityReversal, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropCF18ETRisingCount:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ETRisingCount, val);
	}
	break;
	case kPropCF18ETFallingCount:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ETFallingCount, val);
	}
	break;
	case kPropCF18ENJitterEliminationTime:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ENJitterEliminationTime, val);
	}
	break;
	case kPropCF18ENPolarityReversal:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ENPolarityReversal, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropCF18ENRisingCount:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ENRisingCount, val);
	}
	break;
	case kPropCF18ENFallingCount:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ENFallingCount, val);
	}
	break;
	case kPropCF18BJitterEliminationTime:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18BJitterEliminationTime, val);
	}
	break;
	case kPropCF18INChannel:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18INChannel, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropCF18Cycle:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18Cycle, val);
	}
	break;
	case kPropCF18Unit:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18Unit, (m_map_enums[prop_type])[val.toInt()]);
		if (1 == m_currrent_device_ptr->getProperty(Device::PropCF18Unit).toInt())
		{
			m_currrent_device_ptr->setProperty(Device::PropCF18Cycle, floor(m_currrent_device_ptr->getProperty(Device::PropCF18Cycle).toInt() * 1000));
			m_currrent_device_ptr->setProperty(Device::PropCF18INHighLevelWidth, floor(m_currrent_device_ptr->getProperty(Device::PropCF18INHighLevelWidth).toInt() * 1000));
			m_currrent_device_ptr->setProperty(Device::PropCF18INRisingDelay, floor(m_currrent_device_ptr->getProperty(Device::PropCF18INRisingDelay).toInt() * 1000));
		}
		else {
			m_currrent_device_ptr->setProperty(Device::PropCF18Cycle, floor(m_currrent_device_ptr->getProperty(Device::PropCF18Cycle).toInt() / 1000.0));
			m_currrent_device_ptr->setProperty(Device::PropCF18INHighLevelWidth, floor(m_currrent_device_ptr->getProperty(Device::PropCF18INHighLevelWidth).toInt()) / 1000.0);
			m_currrent_device_ptr->setProperty(Device::PropCF18INRisingDelay, floor(m_currrent_device_ptr->getProperty(Device::PropCF18INRisingDelay).toInt()) / 1000.0);
		}
	}
	break;
	case kPropCF18INHighLevelWidth:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18INHighLevelWidth, val);
	}
	break;
	case kPropCF18INRisingDelay:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18INRisingDelay, val);
	}
	break;
	case kPropCF18ITChannel:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ITChannel, (m_map_enums[prop_type])[val.toInt()]);
	}
	break;
	case kPropCF18ITHighLevelWidth:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ITHighLevelWidth, val);
	}
	break;
	case kPropCF18ITRisingDelay:
	{
		m_currrent_device_ptr->setProperty(Device::PropCF18ITRisingDelay, val);
	}
	break;
	default:
		break;
	}

}


void CSPropertyViewDevice::slotPropertyItemChanged(QtBrowserItem* browser_item)
{
	//获取当前选中项的文案
	if (!browser_item)
	{
		ui->textBrowser_desc->setText("");
		return;
	}
	QString prop_desc = QString("<p><span style=\"font-weight:600; \">%1</span><br> %2</p>")//首行加粗
		.arg(browser_item->property()->propertyName())
		.arg(browser_item->property()->descriptionToolTip());

	ui->textBrowser_desc->setText(prop_desc );//显示到描述面板中
}

void CSPropertyViewDevice::slotUpdateCF18Info()
{
	if (m_currrent_device_ptr && (Connected == m_currrent_device_ptr->getState()) && m_currrent_device_ptr->IsCF18()) {
		QList<PropertyType> prop_types;
		prop_types.push_back(kPropCF18ETRisingCount);
		prop_types.push_back(kPropCF18ETFallingCount);
		prop_types.push_back(kPropCF18ENRisingCount);
		prop_types.push_back(kPropCF18ENFallingCount);
		for (auto prop_type : prop_types)
		{
			updatePropertyValue(m_currrent_device_ptr, prop_type);
		}
	}
}

void CSPropertyViewDevice::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		//获取对应的属性列表
		QList<PropertyType> new_property_types;
		getPropertyTypes(m_currrent_device_ptr, new_property_types);

		//清空属性控件对应表
		m_map_edit_type.clear();
		m_map_type_edit.clear();
		//属性表,在刷新列表时新建列表
		for (auto property_to_remove : ui->widgetPropertyBrowser->properties())
		{
			ui->widgetPropertyBrowser->removeProperty(property_to_remove);
		}
		m_old_property_types = new_property_types;
		updatePropertiesValue(m_currrent_device_ptr, new_property_types);

		
	}

	QWidget::changeEvent(event);
}

void CSPropertyViewDevice::addProperty(const PropertyType prop_type, const QString value_str, bool allowsEdit /*= false */)
{
	//根据属性名称判断当前属性列表中是否已经存在该属性项了,没有则新建,有则更新
	//查找当前属性列表
	QString prop_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[prop_type]));
	QtVariantProperty * prop_item = dynamic_cast<QtVariantProperty*>(m_map_type_edit.value(prop_type,nullptr));
	if (!prop_item)//没有找到,生成一个
	{

		prop_item = m_property_manager->addProperty(QVariant::Type::String, prop_name);
		prop_item->setAttribute("regExp", QRegExp(".{0,31}"));//设置输入限制
		m_property_manager->setValue(prop_item, value_str);//设置属性值

		QString str_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[prop_type]));
		prop_item->setDescriptionToolTip(str_desc);//设置描述文字

		//添加到自己的父节点中
		addProperty(prop_item, getPropertyParent(prop_type));
		//记录控件对应的属性种类
		m_map_edit_type[prop_item] = prop_type;
		m_map_type_edit[prop_type] = prop_item;
	}
	else//找到了,设置新的属性值
	{
		m_property_manager->setValue(prop_item, value_str);
	}
	prop_item->setEnabled(allowsEdit);

}

void CSPropertyViewDevice::addProperty(const PropertyType prop_type, long value, long min /*= 0*/, long max /*= 0*/, long inc /*= 0*/, bool bEnable /*=true*/)
{
	//根据属性名称判断当前属性列表中是否已经存在该属性项了,没有则新建,有则更新
	//查找当前属性列表
	QString prop_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[prop_type]));
	QtVariantProperty * prop_item = dynamic_cast<QtVariantProperty*>(m_map_type_edit.value(prop_type, nullptr));
	if (!bEnable)//不可编辑时不限制大小
	{
		min = 0;
		max = VALID_MAX;
		inc = 1;
	}
	if (!prop_item)//没有找到,生成一个
	{

		prop_item = m_property_manager->addProperty(QVariant::Type::Int, prop_name);
		if (min <= max)//取值范围合法性
		{
			prop_item->setAttribute("minimum", (int)min);//设置取值范围
			prop_item->setAttribute("maximum", (int)max);
		}
		if (inc > 0)//步进值合法性
		{
			prop_item->setAttribute("singleStep", (int)inc);//设置步进值
		}
		prop_item->setValue((int)value);

		QString prop_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[prop_type]));
		prop_item->setDescriptionToolTip(prop_desc);//写入描述文案

		//添加到自己的父节点中
		addProperty(prop_item, getPropertyParent(prop_type));
		//记录控件对应的属性种类
		m_map_edit_type[prop_item] = prop_type;
		m_map_type_edit[prop_type] = prop_item;

	}
	else//找到了,设置新的属性值
	{
		if (min <= max)//取值范围合法性
		{
			prop_item->setAttribute("minimum", (int)min);//设置取值范围
			prop_item->setAttribute("maximum", (int)max);
		}
		if (inc > 0)//步进值合法性
		{
			prop_item->setAttribute("singleStep", (int)inc);//设置步进值
		}
		prop_item->setValue((int)value);
	}
	prop_item->setEnabled(bEnable);

}

void CSPropertyViewDevice::addProperty(QtProperty* prop_item, QtProperty* prop_group)
{
	if (prop_item)
	{
		if (prop_group)//有父节点,添加到父节点下
		{
			prop_group->addSubProperty(prop_item);
			//父节点和子节点的颜色控制
			ui->widgetPropertyBrowser->setBackgroundColor(ui->widgetPropertyBrowser->items(prop_group).first(),QColor(218,218,218));
			ui->widgetPropertyBrowser->setBackgroundColor(ui->widgetPropertyBrowser->items(prop_item).first(), QColor(Qt::white));

		}
		else//顶层节点,添加到界面列表中
		{
			ui->widgetPropertyBrowser->addProperty(prop_item);
		}
	}
}

void CSPropertyViewDevice::addProperty(const PropertyType prop_type, const QStringList option_str_list, int option_index, bool allowsEdit /*= false*/)
{
	//根据属性名称判断当前属性列表中是否已经存在该属性项了,没有则新建,有则更新
	//查找当前属性列表
	QString prop_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[prop_type]));
	QtVariantProperty * prop_item = dynamic_cast<QtVariantProperty*>(m_map_type_edit.value(prop_type, nullptr));
	if (!prop_item)//没有找到,生成一个
	{

		prop_item = m_property_manager->addProperty(QtVariantPropertyManager::enumTypeId(), prop_name);
		prop_item->setAttribute("enumNames", option_str_list);//添加枚举属性值
		prop_item->setValue(option_index);

		QString prop_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[prop_type]));
		prop_item->setDescriptionToolTip(prop_desc);//写入描述文案

		//添加到自己的父节点中
		addProperty(prop_item, getPropertyParent(prop_type));
		//记录控件对应的属性种类
		m_map_edit_type[prop_item] = prop_type;
		m_map_type_edit[prop_type] = prop_item;

	}
	else//找到了,设置新的属性值
	{
		prop_item->setAttribute("enumNames", option_str_list);//设置属性值
		prop_item->setValue(option_index);
	}
	prop_item->setEnabled(allowsEdit);

}

void CSPropertyViewDevice::addUserDefineProperty(const PropertyType prop_type, const QString value_str, bool allowsEdit /*= false*/)
{
	//根据属性名称判断当前属性列表中是否已经存在该属性项了,没有则新建,有则更新
	//查找当前属性列表
	QString prop_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[prop_type]));
	QtVariantProperty * prop_item = dynamic_cast<QtVariantProperty*>(m_map_type_edit.value(prop_type, nullptr));
	if (!prop_item)//没有找到,生成一个
	{

		prop_item = m_property_manager->addProperty(QVariant::Type::UserType, prop_name);
		prop_item->setAttribute("regExp", QRegExp(".{0,31}"));//设置输入限制
		prop_item->setValue(value_str);
		//m_property_manager->setValue(prop_item, value_str);//设置属性值

		QString str_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[prop_type]));
		prop_item->setDescriptionToolTip(str_desc);//设置描述文字

		//添加到自己的父节点中
		addProperty(prop_item, getPropertyParent(prop_type));
		//记录控件对应的属性种类
		m_map_edit_type[prop_item] = prop_type;
		m_map_type_edit[prop_type] = prop_item;
	}
	else//找到了,设置新的属性值
	{
		m_property_manager->setValue(prop_item, value_str);
	}
	prop_item->setEnabled(allowsEdit);
}

QtProperty * CSPropertyViewDevice::getPropertyParent(PropertyType property_type)
{
	//判断传入的属性类型是否在属性关系表中
	if (m_map_props.find(property_type) != m_map_props.end())
	{
		//判断父属性是否在属性关系表中,如果在表中判断有没有子节点
		PropertyType parent_type = std::get<2>(m_map_props[property_type]);
		if (m_map_props.find(parent_type) != m_map_props.end() && std::get<3>(m_map_props[parent_type]))
		{
			//查找这个父节点 
			QString parent_name = UIExplorer::instance().getStringById(std::get<0>(m_map_props[parent_type]));
			QtProperty* parent_item = m_map_type_edit.value(parent_type, nullptr);
			if (!parent_item)//没找到,说明没有创建出来,创建父节点
			{
				parent_item = m_property_manager->addProperty(QtVariantPropertyManager::groupTypeId(), parent_name);
				m_map_type_edit[parent_type] = parent_item;

				QString prop_desc = UIExplorer::instance().getStringById(std::get<1>(m_map_props[parent_type]));
				parent_item->setDescriptionToolTip(prop_desc);//写入描述文字
				
				addProperty(parent_item, getPropertyParent(parent_type));
			}
			return parent_item;
		}
	}
	//条件不符合说明该节点是顶层节点,返回空指针
	return nullptr;
}

void CSPropertyViewDevice::slotCurrentDeviceChanged(const QString current_ip)
{
	if (current_ip.isEmpty()) {
		if (!m_currrent_device_ptr.isNull())
		{
			disconnect(m_currrent_device_ptr.data(), 0, this, 0);//断开原来设备与该界面的所有连接
			m_currrent_device_ptr.reset();
		}
		updatePropertyList(ListForSystem);
		return;
	}
	Q_UNUSED(current_ip);
	//判断是不是之前的设备
	if (m_currrent_device_ptr == m_device_magager_ptr->getCurrentDevice())
	{
		if (m_currrent_device_ptr && (Unconnected == m_currrent_device_ptr->getState())) {
			updatePropertyList(ListForSystem);
		}
		return;
	}
	if (!m_currrent_device_ptr.isNull())
	{
		disconnect(m_currrent_device_ptr.data(), 0, this, 0);//断开原来设备与该界面的所有连接
	}
	m_currrent_device_ptr = m_device_magager_ptr->getCurrentDevice();
	//TODO:判断设备类型
	if (m_currrent_device_ptr.isNull())
	{
		updatePropertyList(ListForSystem);
	}
	else
	{
		//创建连接
		auto connect_type = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);//限制单次连接,避免多次连接导致多次弹窗
		auto connect_type_queue = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);
		if (m_currrent_device_ptr->IsCF18()) {
			connect(m_currrent_device_ptr.data(), &Device::cf18StateChanged, this, &CSPropertyViewDevice::slotDeviceStateChanged, connect_type_queue);
		}
		else {
			connect(m_currrent_device_ptr.data(), &Device::stateChanged, this, &CSPropertyViewDevice::slotDeviceStateChanged, connect_type_queue);
		}
		connect(m_currrent_device_ptr.data(), &Device::propertyChanged, this, &CSPropertyViewDevice::slotDevicePropertyChanged, connect_type);
		
		m_currrent_device_ptr->IsCF18() ? updatePropertyList(ListForTrigger) : updatePropertyList(ListForCamera);
	}
}

void CSPropertyViewDevice::slotCurrentGroupChanged(const QString group_name, const QString device_num)
{
	m_group_name = group_name;
	m_group_device_num = device_num;
	m_is_group = true;
	updatePropertyList(ListForGroup);
	m_is_group = false;
}

void CSPropertyViewDevice::slotDeviceStateChanged()
{
	if (!m_currrent_device_ptr.isNull())
	{
		m_currrent_device_ptr->IsCF18() ? updatePropertyList(ListForTrigger) : updatePropertyList(ListForCamera);
	}
}

void CSPropertyViewDevice::slotDevicePropertyChanged()
{
	//直接刷新整个属性表格
	if (!m_currrent_device_ptr.isNull())
	{
		m_currrent_device_ptr->IsCF18() ? updatePropertyList(ListForTrigger) : updatePropertyList(ListForCamera);
	}
}


