#include "device.h"

#include <QSettings>
#include <QMutexLocker>
#include <QHostAddress>
#include <QDateTime>
#include <QMetaEnum>
#include <QtMath>
#include <QTimer>
#include <thread>
#include <QElapsedTimer>
#include <QImage>
#include <QCoreApplication>
#include <QMessageBox>
#include <iostream>
#include "HscAPI.h"
#include "HscAPIHeader.h"
#include "AgCapability/acquisition_period_range_capability.h"
#include "AgCapability/exposure_time_range_capability.h"
#include "AgCapability/roi_range_capability.h"
#include "deviceutils.h"
#include "Main/rccapp/render/PlayerViewBase.h"
#include <HscCommandAPI.h>
#include <HscDeviceAPI.h>
#include <Device/csframeinfo.h>
#ifdef CSRCCAPP

#else

#include "realtimeplayercontroller.h"
#include "RMAImage.h"
#endif //CSRCCAPP

#include "Video/VideoItemManager/videoitemmanager.h"
#include "Video/VideoUtils/videoutils.h"
#include "Common/LogUtils/logutils.h"
#include "Common/PathUtils/pathutils.h"
#include "Common/UIExplorer/uiexplorer.h"
//#include "FallPointMeasure/FallPointMeasure.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "System/SystemSettings/systemsettingsmanager.h"

#include "imageprocessor.h"
#include "previewthread.h"
#include "acquirethread.h"
#include "msglistener.h"
#include "Common/LogUtils/logutils.h"
#include <fstream>
#include <iostream>
using namespace std;
//向上对齐
#define ALIGNUP(size,alignSize)    ((size)%(alignSize)==0?(size):(size+alignSize-(size)%(alignSize)))
//向下对齐
#define ALIGNDOWN(size,alignSize)  ((size)%(alignSize)==0?(size):(size-(size)%(alignSize)))


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

float changeU2F(uint16_t fix)
{
	float f = (float)u16ToDouble(1, 7, 8, fix);
	return f;
}

void Device::initFunctions()
{
    // 设置父设备IP
    map_prop_set_[PropType::PropParentIp] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取父设备IP
    map_prop_get_[PropType::PropParentIp] = [this](QVariant & value ,bool from_local){
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropParentIp), "");

        return HSC_OK;
    };

    // 设置设备名称
    map_prop_set_[PropType::PropName] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取设备名称
    map_prop_get_[PropType::PropName] = [this](QVariant & value, bool from_local){
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropName), "");

        return HSC_OK;
    };

    // 设置温控列表使能
    map_prop_set_[PropType::PropTemperaturePanelEnable] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        if (IsTemperaturePanelSupported())
        {
            settings.setValue(this->settingsKey(type), value);
        }

        return HSC_OK;
    };

    // 获取温控列表使能
    map_prop_get_[PropType::PropTemperaturePanelEnable] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropTemperaturePanelEnable), false);
        if (!IsTemperaturePanelSupported())
        {
            value = false;
        }

        return HSC_OK;
    };




    // 设置设备序号
    map_prop_set_[PropType::PropDeviceIndex] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取设备序号
    map_prop_get_[PropType::PropDeviceIndex] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropDeviceIndex), 0);

        return HSC_OK;
    };

    // 设置OSD可见
    map_prop_set_[PropType::PropOsdVisible] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取OSD可见
    map_prop_get_[PropType::PropOsdVisible] = [this](QVariant & value, bool from_local){
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropOsdVisible), true).toBool();

        return HSC_OK;
    };

    // 设置叠加水印使能
    map_prop_set_[PropType::PropWatermarkEnable] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取叠加水印使能
    map_prop_get_[PropType::PropWatermarkEnable] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropWatermarkEnable), true).toBool();

        return HSC_OK;
    };

    // 设置角度数据矫正
    map_prop_set_[PropType::PropAngleDataCorrection] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取角度数据矫正
    map_prop_get_[PropType::PropAngleDataCorrection] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropAngleDataCorrection), 0).toInt();

        return HSC_OK;
    };


    // 设置焦点
    map_prop_set_[PropType::PropFocusPoint] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取焦点
    map_prop_get_[PropType::PropFocusPoint] = [this](QVariant & value, bool from_local){
        QSettings settings;
        QRect max_roi{};
        getMaxRoi(max_roi);
        value = settings.value(this->settingsKey(PropType::PropFocusPoint), QPoint((max_roi.x() + max_roi.width()) / 2, (max_roi.y() + max_roi.height()) / 2));

        return HSC_OK;
    };

    // 设置焦点可见
    map_prop_set_[PropType::PropFocusPointVisible] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取焦点可见
    map_prop_get_[PropType::PropFocusPointVisible] = [this](QVariant & value, bool from_local){
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropFocusPointVisible), false).toBool();

        return HSC_OK;
    };

    // 设置ROI
    map_prop_set_[PropType::PropRoi] = [this](PropType type, const QVariant & value){
        QRect rect = value.toRect();
        CameraWindowRect roi;
        correctRoi(rect, kDeviceRoi);
        roi.x = rect.x();
        roi.y = rect.y();
        roi.width = rect.width();
        roi.height = rect.height();
        HscResult res = HscSetImageWindow(device_handle_, roi);
        if (res != HSC_OK){
            return res;
        }
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value);
        }
        return res;
    };

    // 获取ROI
    map_prop_get_[PropType::PropRoi] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;
        if (getState()== Unconnected || getState() == DeviceState::Disconnected)
        {
            value = QRect(0, 0, 1280, 800);
            return res;
        }
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            CameraWindowRect roi{};
            res = HscGetCameraWinRect(device_handle_, &roi);
            if (res == HSC_OK)
            {
                value.setValue(QRect(roi.x, roi.y, roi.width, roi.height));
            }
        }
        else
        {
            QSettings settings;
            QRect max_roi{};
            getMaxRoi(max_roi);
            value = settings.value(this->settingsKey(PropType::PropRoi), max_roi);
        }

        return res;
    };

    // 设置ROI可见
    map_prop_set_[PropType::PropRoiVisible] = [this](PropType type, const QVariant & value){
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);

        return HSC_OK;
    };

    // 获取ROI可见
    map_prop_get_[PropType::PropRoiVisible] = [this](QVariant & value, bool from_local){
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropRoiVisible), true).toBool();

        return HSC_OK;
    };

    // 设置暗场校正使能
    map_prop_set_[PropType::PropBlackFieldEnable] = [this](PropType type, const QVariant & value) {
        QSettings settings;

        HscResult res = HscEnableBlackField(device_handle_, value.toBool());
        if (res == HSC_OK)
        {
            settings.setValue(this->settingsKey(type), value.toBool());
        }

        return HSC_OK;
    };

    // 获取暗场校正使能
    map_prop_get_[PropType::PropBlackFieldEnable] = [this](QVariant & value, bool from_local) {
        //QSettings settings;
        //value = settings.value(this->settingsKey(PropType::PropBlackFieldEnable), true).toBool();
		bool blackFieldEnable = false;
		HscGetEnableBlackCorrect(device_handle_, &blackFieldEnable);
		value = blackFieldEnable;
        return HSC_OK;
    };

    // 设置Lut
    map_prop_set_[PropType::PropLut] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        QList<QVariant> point_list = value.toList();
        unsigned short lut_x[4] = { 0 };
        unsigned short lut_y[4] = { 0 };

        for (int i = 0; i < 4; i++)
        {
            lut_x[i] = point_list.at(i).toPoint().x();
            lut_y[i] = point_list.at(i).toPoint().y();
        }
        HscResult res = HscSetLut(device_handle_, lut_x, lut_y);
        if (res == HSC_OK)
        {
            settings.setValue(this->settingsKey(type), value.toList());
        }

        return HSC_OK;
    };

    // 获取Lut
    map_prop_get_[PropType::PropLut] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        QList<QVariant> default_lut;
        for (int i = 0; i < 4; i++)
        {
            default_lut.append(QPoint(0, 0));
        }
        unsigned short lut_x[4] = { 0 };
        unsigned short lut_y[4] = { 0 };
        if (settingsNeedsLoadFromDevice() && !from_local)
        {

            HscResult res = HscGetLut(device_handle_, lut_x, lut_y);
            if (res == HSC_OK)
            {
                for (int i = 0; i < 4; i++)
                {
                    default_lut[i] = QPoint(lut_x[i], lut_y[i]);
                }
            }
            value = default_lut;
        }
        else
        {
            value = settings.value(this->settingsKey(PropType::PropLut), default_lut);
        }
        return HSC_OK;
    };

    // 设置数字增益
    map_prop_set_[PropType::PropDigitalGain] = [this](PropType type, const QVariant & value) {
        if (!value.isValid())
        {
            return HSC_CANCEL;
        }
        HscResult res = HSC_OK;
        QList<QVariant> lut_points;
        lut_points = getProperty(Device::PropLut).toList();
        lut_points[0] = QPoint(lut_points.at(0).toPoint().x(), value.toInt());
        res = setProperty2Device(PropLut, lut_points);
        return res;
    };

    // 获取数字增益
    map_prop_get_[PropType::PropDigitalGain] = [this](QVariant & value, bool from_local) {
        QList<QVariant> lut_points;
        lut_points = getProperty(Device::PropLut, from_local).toList();
		if (!lut_points.isEmpty() && lut_points.at(0).isValid())
		{
			value = lut_points.at(0).toPoint().y();
		}
        return HSC_OK;
    };


    // 设置显示模式
    map_prop_set_[PropType::PropDisplayMode] = [this](PropType type, const QVariant & value) {
        HscDisplayMode mode = value.value<HscDisplayMode>();
        HscResult res = HscDevSet(device_handle_,HSC_CFG_DISPLAY_MODE,&mode ,sizeof(HscDisplayMode));
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };

    // 获取显示模式
    map_prop_get_[PropType::PropDisplayMode] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscDisplayMode display_mode = HscDisplayMode(0);
            HscResult res = HscDevGet(device_handle_, HSC_CFG_DISPLAY_MODE, &display_mode, sizeof(display_mode));
            if (res == HSC_OK)
            {
                value.setValue((int)display_mode);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropDisplayMode), (int)HscDisplayMode(0));
        }

        return res;
    };

    // 设置拍摄环境
    map_prop_set_[PropType::PropWbEnv] = [this](PropType type, const QVariant & value) {
        HScWbEnv env = value.value<HScWbEnv>();
		QSettings settings;
		settings.setValue(this->settingsKey(type), value.toInt());

        if (settingsNeedsLoadFromDevice())
        {
            HscColorCorrectInfo info;
            getColorCorrectInfo(info);
            info.awb_env_ = env;
            setColorCorrectInfo(info);
        }
        else
        {
            auto processor = getProcessor();
            if (processor)
            {
                processor->setWbEnv(env);
            }
        }
        return HSC_OK;
    };

    // 获取拍摄环境
    map_prop_get_[PropType::PropWbEnv] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        value = HScWbEnv::HSC_WB_ENV_LIGHT;
		if (from_local)
		{
			QSettings settings;
			value = settings.value(this->settingsKey(PropType::PropWbEnv), (int)HScWbEnv::HSC_WB_ENV_LIGHT);
			return res;
		}
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscColorCorrectInfo info;
            getColorCorrectInfo(info);
            value = (int)info.awb_env_;
        }
        else
        {
            auto processor = getProcessor();
            if (processor)
            {
                value = processor->getWbEnv();
            }
        }


        return res;
    };

	// 设置白平衡参数
	map_prop_set_[PropType::PropWhiteBalance] = [this](PropType type, const QVariant & value) {

		if (!isSupportManualWhiteBalanceMode())//手动白平衡参数支持保存
		{
			return HSC_OK;
		}
		if (!checkVariantType<HscHisiManualAwb>(value))
		{
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);

		HscHisiManualAwb param = qvariant_cast<HscHisiManualAwb>(value);
		HscResult res = SetManualWhiteBalance(param);
		if (res == HSC_OK)
		{
			if (param.mode == AWB_MODE_MANUAL)
			{
				HscColorCorrectInfo info;
				if (getColorCorrectInfo(info) == HSC_OK)
				{
					info.r_gain_ = changeU2F(param.r_gain);
					info.g_gain_ = changeU2F(param.gr_gain);
					info.b_gain_ = changeU2F(param.b_gain);
					setColorCorrectInfo(info);
				}
			}

		}
		return res;
	};

	// 获取白平衡参数
	map_prop_get_[PropType::PropWhiteBalance] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (!isSupportManualWhiteBalanceMode())//手动白平衡参数支持保存
		{
			return HSC_OK;
		}
		if (from_local)
		{
			HscHisiManualAwb default_value{};
			QSettings settings;
			value = settings.value(this->settingsKey(PropType::PropWhiteBalance),QVariant::fromValue(default_value));
			return res;
		}
		HscHisiManualAwb temp_value{};
		res = GetManualWhiteBalance(temp_value);
		if (res == HSC_OK)
		{
			value = QVariant::fromValue(temp_value);
		}
		return res;
	};

    // 设置亮度
    map_prop_set_[PropType::PropLuminance] = [this](PropType type, const QVariant & value) {

        HscResult res = HscSetLuminance(device_handle_, value.toInt());
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };

    // 获取亮度
    map_prop_get_[PropType::PropLuminance] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            int luminance = 50;
            res = HscGetLuminance(device_handle_,&luminance);
            if (res == HSC_OK)
            {
                value.setValue(luminance);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropLuminance), 50).toInt();
        }

        return res;
    };

    // 设置对比度
    map_prop_set_[PropType::PropContrast] = [this](PropType type, const QVariant & value) {

        HscResult res = HscSetContrast(device_handle_, value.toInt());
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };

    // 获取对比度
    map_prop_get_[PropType::PropContrast] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            int contrast = 50;
            res = HscGetContrast(device_handle_, &contrast);
            if (res == HSC_OK)
            {
                value.setValue(contrast);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropContrast), 50).toInt();
        }

        return res;
    };

    // 设置帧率
    map_prop_set_[PropType::PropFrameRate] = [this](PropType type, const QVariant & value){

        qint64 min_acquisition_period = 1; // TODO

        HscResult res = HscSetExposurePeriod(device_handle_, toAcquisitionPeriod(getCalPeriodDivisor(), value.toLongLong(), min_acquisition_period));
        if (res == HSC_OK)
        {
           // if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value);
            }
            m_fps_backup = value.toUInt();

        }

        return res;
    };

    // 获取帧率
    map_prop_get_[PropType::PropFrameRate] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            DWORD period = 10000;
            res = HscGetExposurePeriod(device_handle_, &period);
            if (res == HSC_OK)
            {
                value.setValue(toFrameRate(getCalPeriodDivisor(), period));
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropFrameRate), 100);
        }
        m_fps_backup = value.toUInt();
        return res;
    };

    // 设置曝光时间
    map_prop_set_[PropType::PropExposureTime] = [this](PropType type, const QVariant & value){
        //将界面中的曝光时间转为设备中的曝光时间
        DWORD exposure_time = ConvertExposureTime(value.toLongLong(),getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(),
            GetRealExposureTimeUnit(),1) ;
        HscResult res = HscSetExposureTime(device_handle_, exposure_time);
        if (res == HSC_OK)
        {
          //  if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), QVariant::fromValue(exposure_time));
            }
        }

        return res;
    };

    // 获取曝光时间
    map_prop_get_[PropType::PropExposureTime] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            DWORD exposure_time = 1;
            res = HscGetExposureTime(device_handle_, &exposure_time);
            //将设备的曝光时间转换为界面的曝光时间
            exposure_time = ConvertExposureTime(exposure_time, GetRealExposureTimeUnit(),
                getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(),
                getProperty(PropFrameRate).toInt(),false);
            if (res == HSC_OK)
            {
                value.setValue(exposure_time);
            }
        }
        else
        {
            QSettings settings;
            auto exposure_time =settings.value(this->settingsKey(PropType::PropExposureTime), 1).toLongLong();
            //将设备的曝光时间转换为界面的曝光时间
            exposure_time = ConvertExposureTime(exposure_time, GetRealExposureTimeUnit(),
                getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(),
                getProperty(PropFrameRate).toInt(),false);
            value  = QVariant::fromValue(exposure_time);
        }

        return res;
    };

	// 设置低照度模式-曝光时间
	map_prop_set_[PropType::PropLowLightMode] = [this](PropType type, const QVariant & value) {
		//将界面中的曝光时间转为设备中的曝光时间
		DWORD exposure_time = ConvertExposureTime(value.toLongLong(), agile_device::capability::Units::kUnitUs,
			GetRealExposureTimeUnit(), 1);
		HscResult res = HscSetExposureTime(device_handle_, exposure_time);
		if (res == HSC_OK)
		{
			{
				QSettings settings;
				settings.setValue(this->settingsKey(type), QVariant::fromValue(exposure_time));
			}
		}

		return res;
	};

	// 获取设置低照度模式-曝光时间
	map_prop_get_[PropType::PropLowLightMode] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		QSettings settings;
		auto exposure_time = settings.value(this->settingsKey(PropType::PropLowLightMode), 0).toLongLong();
		//将设备的曝光时间转换为界面的曝光时间
		exposure_time = ConvertExposureTime(exposure_time, GetRealExposureTimeUnit(),
			agile_device::capability::Units::kUnitUs,
			1, false);
		value = QVariant::fromValue(exposure_time);

		return res;
	};

    // 设置曝光时间单位
    map_prop_set_[PropType::PropExposureTimeUnit] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value.toInt());

        return HSC_OK;
    };

    // 获取曝光时间单位
    map_prop_get_[PropType::PropExposureTimeUnit] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropExposureTimeUnit), agile_device::capability::Units::kUnitUs);

        return HSC_OK;
    };

    // 设置过曝提示使能
    map_prop_set_[PropType::PropOverExposureTipEnable] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value.toBool());

        return HSC_OK;
    };

    // 获取过曝提示使能
    map_prop_get_[PropType::PropOverExposureTipEnable] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropOverExposureTipEnable),false).toBool();

        return HSC_OK;
    };

    // 设置协议格式
    map_prop_set_[PropType::PropStreamType] = [this](PropType type, const QVariant & value) {
        HscResult res = HscSetStreamType(device_handle_, value.value<StreamType>());
        if (res == HSC_OK)
        {
           // if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }

            QList<HscDisplayMode> display_modes;
            GetSupportedDisplayModes(value.value<StreamType>(), display_modes);
            HscDisplayMode display_mode = getProperty(Device::PropDisplayMode).value<HscDisplayMode>();
            if (!display_modes.empty() && std::find(display_modes.begin(), display_modes.end(), display_mode) == display_modes.end())
            {
                if (display_modes.size() > 0)
                {
                    setProperty2Device(Device::PropDisplayMode, display_modes[0]);
                }
            }
        }

        return res;
    };

    // 获取协议格式
    map_prop_get_[PropType::PropStreamType] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            StreamType stream_type = TYPE_RAW8;
            res = HscGetStreamType(device_handle_, &stream_type);
            if (res == HSC_OK)
            {
                value.setValue((int)stream_type);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropStreamType), (int)TYPE_RAW8);
        }

        return res;
    };

    // 设置模拟增益
    map_prop_set_[PropType::PropAnalogGain] = [this](PropType type, const QVariant & value){
        HscResult res = HscSetCameraGain(device_handle_, value.value<ANALOG_GAIN_TYPE>());
        if (res == HSC_OK)
        {
           // if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取模拟增益
    map_prop_get_[PropType::PropAnalogGain] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            ANALOG_GAIN_TYPE gain = AAG_1;
            res = HscGetCameraGain(device_handle_, &gain);
            if (res == HSC_OK)
            {
                value.setValue((int)gain);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropAnalogGain), (int)AAG_1);
        }

        return res;
    };

    // 设置同步方式
    map_prop_set_[PropType::PropSyncSource] = [this](PropType type, const QVariant & value){
        HscResult res = HscSyncSource(device_handle_, value.value<SyncSource>());
        if (res == HSC_OK)
        {
          //  if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取同步方式
    map_prop_get_[PropType::PropSyncSource] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            SyncSource sync_source = SYNC_INTERNAL;
            res = HscGetSyncMode(device_handle_, &sync_source);
            if (res == HSC_OK)
            {
                value.setValue((int)sync_source);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropSyncSource), (int)SYNC_INTERNAL);
        }

        return res;
    };

    // 设置触发方式
    map_prop_set_[PropType::PropTriggerMode] = [this](PropType type, const QVariant & value){
        HscResult res = HscSetTriggerMode(device_handle_, value.value<TriggerMode>());
        if (res == HSC_OK)
        {
          //  if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取触发方式
    map_prop_get_[PropType::PropTriggerMode] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            TriggerMode mode = TRIGGER_INTERNAL;
            res = HscGetTrigMode(device_handle_, &mode);
            if (res == HSC_OK)
            {
                value.setValue((int)mode);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropTriggerMode), (int)TRIGGER_INTERNAL);
        }

        return res;
    };

    // 设置外触发方式
    map_prop_set_[PropType::PropExTriggerMode] = [this](PropType type, const QVariant & value) {
        HscResult res = HscSetExternalTriggerMode(device_handle_, value.value<ExternalTriggerMode>());
        if (res == HSC_OK)
        {
            //if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取外触发方式
    map_prop_get_[PropType::PropExTriggerMode] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            ExternalTriggerMode mode = ExternalTriggerMode::TRIGGER_RAISING;
            res = HscGetExternalTrigMode(device_handle_, &mode);
            if (res == HSC_OK)
            {
                value.setValue((int)mode);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropExTriggerMode), (int)TRIGGER_RAISING);
        }

        return res;
    };

    //设置消抖长度
    map_prop_set_[PropType::PropJitterEliminationLength] = [this](PropType type, const QVariant & value) {
        uint32_t length = value.toInt();
        HscResult res = HscDevSet(device_handle_, HSC_CFG_JITTER_ELIMINATION_LENGTH, &length, sizeof(length));
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };

    // 获取消抖长度
    map_prop_get_[PropType::PropJitterEliminationLength] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            uint32_t length = 0;
            res = HscDevGet(device_handle_, HSC_CFG_JITTER_ELIMINATION_LENGTH, &length, sizeof(length));
            if (res == HSC_OK)
            {
                value.setValue(length);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropJitterEliminationLength), 1).toInt();
        }

        return res;
    };

    //设置通道延时
    map_prop_set_[PropType::PropChn1Delay] = [this](PropType type, const QVariant & value) {
        HscResult res = HscSetTriggerRouteDelay(device_handle_,type - PropChn1Delay,value.toInt());
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };
    map_prop_set_[PropType::PropChn2Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn3Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn4Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn5Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn6Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn7Delay] = map_prop_set_[PropType::PropChn1Delay];
    map_prop_set_[PropType::PropChn8Delay] = map_prop_set_[PropType::PropChn1Delay];

    // 获取通道延时
    map_prop_get_[PropType::PropChn1Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn1Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn2Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn2Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn3Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn3Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn4Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn4Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn5Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn5Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn6Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn6Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn7Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn7Delay), 0).toInt();
        return res;
    };
    map_prop_get_[PropType::PropChn8Delay] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropChn8Delay), 0).toInt();
        return res;
    };

    // 设置脉冲宽度
    map_prop_set_[PropType::PropPulseWidth] = [this](PropType type, const QVariant & value) {

        HscResult res = HscSetPulseWidth(device_handle_, value.toInt());
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }

        return res;
    };

    // 获取脉冲宽度
    map_prop_get_[PropType::PropPulseWidth] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            DWORD  pulseWidth = 0;
            res = HscGetPulseWidth(device_handle_, &pulseWidth);
            if (res == HSC_OK)
            {
                value.setValue(pulseWidth);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropPulseWidth), IsCamera()? 1 : 500).toInt();
        }

        return res;
    };

    // 设置起点方式
    map_prop_set_[PropType::PropRecordingOffsetMode] = [this](PropType type, const QVariant & value){
        RecordMode mode = RECORD_AFTER_SHUTTER;
        RecordType unit = RECORD_BY_FRAME;
        DWORD offset = 0;
        DWORD length = 1000;
        HscResult res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
        if (res != HSC_OK)
        {
            return res;
        }

        mode = value.value<RecordMode>();

        res = HscSetVideoClipParams(device_handle_, mode, unit, offset, length);
        if (res == HSC_OK)
        {
           // if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取保存起点方式
    map_prop_get_[PropType::PropRecordingOffsetMode] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            RecordMode mode = RECORD_AFTER_SHUTTER;
            RecordType unit = RECORD_BY_FRAME;
            DWORD offset = 0;
            DWORD length = 1000;
            res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
            if (res == HSC_OK)
            {
                value.setValue((int)mode);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropRecordingOffsetMode), (int)RECORD_AFTER_SHUTTER);
        }

        return res;
    };

    // 设置保存起点
    map_prop_set_[PropType::PropRecordingOffset] = [this](PropType type, const QVariant & value){
        RecordMode mode = RECORD_AFTER_SHUTTER;
        RecordType unit = RECORD_BY_FRAME;
        DWORD offset = 0;
        DWORD length = 1000;
        HscResult res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
        if (res != HSC_OK)
        {
            return res;
        }

        offset = value.toLongLong();

		qint64 min = 0, max = 0, inc = 0;

		{
			getPropertyRange(Device::PropRecordingOffset, min, max, inc);
			//自动修正
			qint64 val = offset;
			qint64 correct_val = val;
			if (val < min)
			{
				correct_val = min;
			}
			if (val > max)
			{
				correct_val = max;
			}
			if (correct_val != val)
			{
				offset = correct_val;
			}
		}

        res = HscSetVideoClipParams(device_handle_, mode, unit, offset, length);
        if (res == HSC_OK)
        {
            //if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value);
            }
        }

        return res;
    };

    // 获取保存起点
    map_prop_get_[PropType::PropRecordingOffset] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            RecordMode mode = RECORD_AFTER_SHUTTER;
            RecordType unit = RECORD_BY_FRAME;
            DWORD offset = 0;
            DWORD length = 1000;
            res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
            if (res == HSC_OK)
            {
                value.setValue(offset);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropRecordingOffset), 0);
        }

        return res;
    };

    // 设置保存长度
    map_prop_set_[PropType::PropRecordingLength] = [this](PropType type, const QVariant & value){
        RecordMode mode = RECORD_AFTER_SHUTTER;
        RecordType unit = RECORD_BY_FRAME;
        DWORD offset = 0;
        DWORD length = 1000;
        HscResult res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
        if (res != HSC_OK)
        {
            return res;
        }

        length = value.toLongLong();

        //检查保存长度是否正确
        qint64 cur_length = 0;
        qint64 length_min = 0;
        qint64 length_max = 1000;
        qint64 length_inc = 1;
        getPropertyRange(Device::PropRecordingLength, length_min, length_max, length_inc);
        if (IsCamera() &&( length > length_max)) //|| length < length_min))
        {
            return HSC_NO_ENOUGH_MEMORY;
        }

        res = HscSetVideoClipParams(device_handle_, mode, unit, offset, length);
        if (res == HSC_OK)
        {
          //  if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(PropType::PropRecordingLength), value);
            }
        }

        return res;
    };

    // 获取保存长度
    map_prop_get_[PropType::PropRecordingLength] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            RecordMode mode = RECORD_AFTER_SHUTTER;
            RecordType unit = RECORD_BY_FRAME;
            DWORD offset = 0;
            DWORD length = 1000;
            res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
            if (res == HSC_OK)
            {
                value.setValue(length);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropRecordingLength), 1000);
        }

        return res;
    };

    // 设置保存单位
    map_prop_set_[PropType::PropRecordingUnit] = [this](PropType type, const QVariant & value){
        RecordMode mode = RECORD_AFTER_SHUTTER;
        RecordType unit = RECORD_BY_FRAME;
        DWORD offset = 0;
        DWORD length = 1000;
        HscResult res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
        if (res != HSC_OK)
        {
            return res;
        }

		unit = value.value<RecordType>();

        res = HscSetVideoClipParams(device_handle_, mode, unit, offset, length);
        if (res == HSC_OK)
        {
           // if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取保存单位
    map_prop_get_[PropType::PropRecordingUnit] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            RecordMode mode = RECORD_AFTER_SHUTTER;
            RecordType unit = RECORD_BY_FRAME;
            DWORD offset = 0;
            DWORD length = 1000;
            res = HscGetVideoClipParams(device_handle_, &mode, &unit, &offset, &length);
            if (res == HSC_OK)
            {
                value.setValue((int)unit);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropRecordingUnit), (int)RECORD_BY_FRAME);
        }

        return res;
    };

    // 设置视频格式
    map_prop_set_[PropType::PropVideoFormat] = [this](PropType type, const QVariant & value){
        HscResult res = HscSetVideoFormat(device_handle_, value.value<VideoFormat>());
        if (res == HSC_OK)
        {
            //xj1310无法保存视频格式,尝试同时保存在本地
//             if (!settingsNeedsLoadFromDevice())
//             {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
//             }
        }

        return res;
    };

    // 获取视频格式
    map_prop_get_[PropType::PropVideoFormat] = [this](QVariant & value, bool from_local){
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            //xj1310无法保存视频格式,尝试从本地读取默认值,
            QSettings settings;
            ::VideoFormat format = settings.value(this->settingsKey(PropType::PropVideoFormat), VIDEO_AVI).value<VideoFormat>();
            res = HscGetVideoFormat(device_handle_, &format);
            if (res == HSC_OK)
            {
                value.setValue((int)format);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropVideoFormat), (int)VIDEO_AVI);
        }

        return res;
    };


    // 设置SDI参数
    map_prop_set_[PropType::PropSdiFpsResols] = [this](PropType type, const QVariant & value) {
        HscResult res = HscSetSDIFpsResol(device_handle_, value.value<SDIFpsResol>());
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), value.toInt());
        }
        return res;
    };

    // 获取SDI参数
    map_prop_get_[PropType::PropSdiFpsResols] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            SDIFpsResol sdi_fps_resol = SdiFpsResol_1080P_50FPS;
            res = HscGetSDIFpsResol(device_handle_, &sdi_fps_resol);
            if (res == HSC_OK)
            {
                value.setValue((int)sdi_fps_resol);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropSdiFpsResols), (int)SdiFpsResol_1080P_50FPS);
        }

        return res;
    };

    // 设置触发同步使能
    map_prop_set_[PropType::PropTriggerSyncEnable] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value);
        bool enabled = value.toBool();
		uint8_t val = 0;
		if (enabled) val = TrigSyncMode::HSC_ENABLE_ON;
		else val = TrigSyncMode::HSC_DISABLE;
        HscResult res = HscDevSet(device_handle_, HSC_CFG_TRIGGER_SYNC_ENABLE, &val, sizeof(val));
        return res;
    };

    // 获取触发同步使能
    map_prop_get_[PropType::PropTriggerSyncEnable] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropTriggerSyncEnable), true).toBool();
        bool enabled = value.toBool();
		if (from_local)
		{
			return HSC_OK;
		}
		uint8_t val = 0;
        HscResult res = HscDevGet(device_handle_, HSC_CFG_TRIGGER_SYNC_ENABLE, &val, sizeof(val));
		if (res != HSC_OK)
		{
			value = enabled;
		}
		else
		{
			if (val == TrigSyncMode::HSC_ENABLE_ON) value = true;
			else value = false;
		}
        return res;
    };
    // 设置待机状态
    map_prop_set_[PropType::PropHardwareStandby] = [this](PropType type, const QVariant & value) {
        HscResult res = HSC_OK;
        if (IsStandByModeSupported())
        {
            bool isStandbyOn = value.toBool();
            if (isStandbyOn != getProperty(PropHardwareStandby).toBool())
            {
                if (isStandbyOn)
                {
                    res = SetHardwareStandby();
                }
                else
                {
                    res = StartHardwareAwake();
                }
            }
        }
        return res;
    };

    // 获取待机状态
    map_prop_get_[PropType::PropHardwareStandby] = [this](QVariant & value, bool from_local) {

        bool enabled = false;
        if (getState() == DeviceState::StandBy)
        {
            enabled = true;
        }
        value = enabled;
        return HSC_OK;
    };

    // 设置旋转类型
    map_prop_set_[PropType::PropRotateType] = [this](PropType type, const QVariant & value) {
        HscResult res = HscSetRotationType(device_handle_, value.value<HscRotationType>());
        if (res == HSC_OK)
        {
        //	if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), value.toInt());
            }
        }

        return res;
    };

    // 获取旋转类型
    map_prop_get_[PropType::PropRotateType] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;

        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscRotationType rotate_type = HSC_ROTATION_NONE;
            res = HscGetRotationType(device_handle_, &rotate_type);
            if (res == HSC_OK)
            {
                value.setValue((int)rotate_type);
            }
        }
        else
        {
            QSettings settings;
            value = settings.value(this->settingsKey(PropType::PropRotateType), (int)HSC_ROTATION_NONE);
        }

        return res;
    };

    // 获取支持的协议格式
    map_prop_get_supported_[PropType::PropStreamType] = [this](QVariantList & values) {
		int nBits = getProperty(PropPixelBitDepth).toInt();
		if ( nBits > 8) 
		{
			values << TYPE_RAW8;
		}
		else if (device_capability_.streamNum > 0)
		{
			for (int i = 0; i < device_capability_.streamNum; i++)
			{

				values << (StreamType)device_capability_.streamTypes[i];
			}
		}
         else
        {
            values << TYPE_RAW8;
        }
    };

    // 获取支持的模拟增益
    map_prop_get_supported_[PropType::PropAnalogGain] = [this](QVariantList & values) {
        if (device_capability_.supportAnalogGain == 1)
        {
            values << AAG_1;
            values << AAG_2;
            values << AAG_4;
            values << AAG_8;
            if (getModel() == DEVICE_HT160A||
                getModel() == DEVICE_HT120A ||
                getModel() == DEVICE_HT50A ||
                getModel() == DEVICE_HT40A)
            {
                values << AAG_16;
            }
        }
        else
        {
            values << AAG_1;
        }
    };

    // 获取支持的触发方式
    map_prop_get_supported_[PropType::PropTriggerMode] = [this](QVariantList & values) {
      values << TRIGGER_INTERNAL;
      values << TRIGGER_EXTERNAL;
    };

    // 获取支持的外触发方式
    map_prop_get_supported_[PropType::PropExTriggerMode] = [this](QVariantList & values) {
        HscEnumRange range{};
        HscResult res = HscDevGet(device_handle_, HscRangeSelector(HSC_CFG_EXTERNAL_TRIGGER_MODE), &range, sizeof(range));
        if (res == HSC_OK)
        {
            for (int i = 0; i < range.count; i++)
            {
                values.push_back(ExternalTriggerMode(range.entries[i]));
            }
        }
        else
        {
            values << TRIGGER_RAISING; // 上升沿
            values << TRIGGER_FALLING; // 下降沿
        }

    };

    // 获取支持的同步方式
    map_prop_get_supported_[PropType::PropSyncSource] = [this](QVariantList & values) {
        values << SYNC_INTERNAL;
        values << SYNC_EXTERNAL;
    };

    // 获取支持的保存起点方式
    map_prop_get_supported_[PropType::PropRecordingOffsetMode] = [this](QVariantList & values) {
        values << RECORD_BEFORE_SHUTTER;
        values << RECORD_AFTER_SHUTTER;
    };

    // 获取支持的保存单位
    map_prop_get_supported_[PropType::PropRecordingUnit]  = [this](QVariantList & values) {
        values << RECORD_BY_FRAME;
        values << RECORD_BY_TIME;
		values << RECORD_BY_TIME_S;
    };

    // 获取支持的保存格式
    map_prop_get_supported_[PropType::PropVideoFormat]  = [this](QVariantList & values) {
        StreamType stream_type = getProperty(PropType::PropStreamType).value<StreamType>();
        if (stream_type == TYPE_RAW8)
        {
            values << VIDEO_RHVD;
        }

        values << VIDEO_AVI;
        if (stream_type != TYPE_H264)
        {
            values << VIDEO_MP4;
            values << VIDEO_BMP;
            values << VIDEO_JPG;
            values << VIDEO_TIF;
			values << VIDEO_PNG;
        }
    };

    // 获取支持的SDI参数
    map_prop_get_supported_[PropType::PropSdiFpsResols] = [this](QVariantList & values) {
        values << SdiFpsResol_720P_25FPS;
		if (m_pulsewidth_mode != 1)//G系列定制,不支持SDI切换
		{
			values << SdiFpsResol_720P_50FPS;
			values << SdiFpsResol_1080P_25FPS;
			values << SdiFpsResol_1080P_50FPS;
		}
    };

    // 获取支持的叠加水印使能
    map_prop_get_supported_[PropType::PropWatermarkEnable] = [this](QVariantList & values) {
        if (IsWatermarkSupported())
        {
            values << true;
        }
        values << false;
    };

    // 获取支持的停机功能使能
    map_prop_get_supported_[PropType::PropHardwareStandby] = [this](QVariantList & values) {
        if (IsStandByModeSupported())
        {
            values << true;
        }
        values << false;
    };


    // 获取支持的过曝提示使能
    map_prop_get_supported_[PropType::PropOverExposureTipEnable] = [this](QVariantList & values) {
        if (IsOverExposureTipSupported())
        {
            values << true;
        }
        values << false;
    };

    // 获取支持的触发同步使能
    map_prop_get_supported_[PropType::PropTriggerSyncEnable] = [this](QVariantList & values) {
		values.push_back(false);
		//外同步时不支持设置触发同步
		if (getProperty(Device::PropSyncSource).value<SyncSource>() == SYNC_EXTERNAL)
		{
			return ;
		}

		//内触发不支持触发同步
		if (getProperty(Device::PropTriggerMode).value<TriggerMode>() == TRIGGER_INTERNAL)
		{
			return ;
		}
		
		values.push_back(true);
    };

    // 获取支持的自动曝光灰度模式
    map_prop_get_supported_[PropType::PropAutoExposure] = [this](QVariantList & values) {
        if (IsAutoExposureSupported())
        {
            // X213/X113/G系列/S系列 ISP不支持峰值灰度
			if (m_exposure_value_type == 0)
			{
				values << AutoExposureGrayMode_Avg;
				DeviceModel model = getModel();
				if (DEVICE_X213_ISPSeries != model && DEVICE_X113_ISPSeries != model && !isGType() && m_frame_head_type_!= HEAD_TYPE::eS1315)
				{
					values << AutoExposureGrayMode_Peak;
				}
			}
			else if (m_exposure_value_type == 1)
			{
				values << AutoExposureGrayMode_Avg;
			}
			else if (m_exposure_value_type == 2)
			{
				values << AutoExposureGrayMode_Peak;
			}
        }
    };

    // 获取支持曝光单位
    map_prop_get_supported_[PropType::PropExposureTimeUnit] = [this](QVariantList & values) {
        HscIntRange range;
        int frame_rate = getProperty(PropFrameRate).toInt();
        if (GetExposureTimeRangeByFrameRate(frame_rate, agile_device::capability::kUnitUs, range) && range.max > 0)
        {
            values.push_back(agile_device::capability::kUnitUs);
        }

        if (device_capability_.exposureTimeUnit >= 1)
        {
            if (GetExposureTimeRangeByFrameRate(frame_rate, agile_device::capability::kUnit100ns, range) && range.max > 0)
            {
                values.push_back(agile_device::capability::kUnit100ns);
            }

            if (device_capability_.exposureTimeUnit >= 2)
            {
                if (GetExposureTimeRangeByFrameRate(frame_rate, agile_device::capability::kUnit10ns, range) && range.max > 0)
                {
                    values.push_back(agile_device::capability::kUnit10ns);
                }

                if (device_capability_.exposureTimeUnit >= 3)
                {
                    if (GetExposureTimeRangeByFrameRate(frame_rate, agile_device::capability::kUnitNs, range) && range.max > 0)
                    {
                        values.push_back(agile_device::capability::kUnitNs);
                    }
                }
            }
        }

        if (values.empty())
        {
            // 无有效单位时，添加支持的最小单位
            if (device_capability_.exposureTimeUnit == 3)
            {
                values.push_back(agile_device::capability::kUnitNs);
            }
            else if (device_capability_.exposureTimeUnit == 2)
            {
                values.push_back(agile_device::capability::kUnit10ns);
            }
            else if (device_capability_.exposureTimeUnit == 1)
            {
                values.push_back(agile_device::capability::kUnit100ns);
            }
            else
            {
                values.push_back(agile_device::capability::kUnitUs);
            }
        }

    };

    // 获取支持二次曝光单位
    map_prop_get_supported_[PropType::PropEdrExposureUnit] = [this](QVariantList & variant_list) {
        auto exposure_time = getProperty(PropType::PropExposureTime).toLongLong();
        agile_device::capability::Units exposure_time_unit = getProperty(PropType::PropExposureTimeUnit).value<agile_device::capability::Units>();
        auto value = convertTime(exposure_time, exposure_time_unit, agile_device::capability::kUnitUs);
        if (value >= 1)
            variant_list << agile_device::capability::kUnitUs;
        if (device_capability_.exposureTimeUnit >= 1) {
            double value = convertTime(exposure_time, exposure_time_unit, agile_device::capability::kUnit100ns);
            if (value >= 1)
                variant_list << agile_device::capability::kUnit100ns;
            if (device_capability_.exposureTimeUnit >= 2) {
                double value = convertTime(exposure_time, exposure_time_unit, agile_device::capability::kUnit10ns);
                if (value >= 1)
                    variant_list << agile_device::capability::kUnit10ns;
                if (device_capability_.exposureTimeUnit >= 3) {
                    double value = convertTime(exposure_time, exposure_time_unit, agile_device::capability::kUnitNs);
                    if (value >= 1)
                        variant_list << agile_device::capability::kUnitNs;
                }
            }
        }
        if (variant_list.isEmpty()) {
            // 无有效单位时，添加支持的最小单位
            if (device_capability_.exposureTimeUnit == 3)
                variant_list << agile_device::capability::kUnitNs;
            else if (device_capability_.exposureTimeUnit == 2)
                variant_list.push_back(agile_device::capability::kUnit10ns);
            else if (device_capability_.exposureTimeUnit == 1)
                variant_list.push_back(agile_device::capability::kUnit100ns);
            else
                variant_list.push_back(agile_device::capability::kUnitUs);
        }

    };

	// 获取支持的像素位数
	map_prop_get_supported_[PropType::PropPixelBitDepth] = [this](QVariantList & values) {
		QString strText;

		StreamType stream_type = getProperty(PropType::PropStreamType).value<StreamType>();
		if (m_bits && stream_type == TYPE_RAW8)//只有raw格式下可以支持高bit位数切换
		{
			strText = tr("%1").arg(m_bits);
			if (!strText.isEmpty())
			{
				QStringList vctList = strText.split(',',QString::SkipEmptyParts);
				for (auto text : vctList)
				{
					int nValue = text.toInt();
					values << nValue;
				}
			}
		}
		if (values.size() == 0)
		{
			values << 8;
		}
	};

	// 获取支持的显示位深
	map_prop_get_supported_[PropType::PropDisplayBitDepth] = [this](QVariantList & values) {
		QString strText;

		StreamType stream_type = getProperty(PropType::PropStreamType).value<StreamType>();
		int pixel_depth = getProperty(PropType::PropPixelBitDepth).value<int>();

		if (m_bits && stream_type == TYPE_RAW8)//只有raw格式下可以支持高bit位数切换
		{
			if (pixel_depth > 0)
			{
				values << DBD_0_7;
			}
			if (pixel_depth > 8)
			{
				values << DBD_1_8;
				values << DBD_2_9;
			}
			if (pixel_depth > 10)
			{
				values << DBD_3_10;
				values << DBD_4_11;
			}
		}
		if (values.size() == 0)
		{
			values << DBD_0_7;
		}
	};

	// 获取支持的像素数量触发模式
	map_prop_get_supported_[PropType::PropPixelTriggerMode] = [this](QVariantList & values) {
		values << 0;  // 高于
		values << 1;  // 低于
	};

	// 获取CF18-外触发信号-极性反转状态
	map_prop_get_supported_[PropType::PropCF18ETPolarityReversal] = [this](QVariantList & values) {
		values << 0;  // 关闭
		values << 1;  // 开启
	};

	// 获取CF18-外同步信号-极性反转状态
	map_prop_get_supported_[PropType::PropCF18ENPolarityReversal] = [this](QVariantList & values) {
		values << 0;  // 关闭
		values << 1;  // 开启
	};

	// 获取CF18-内同步信号-通道
	map_prop_get_supported_[PropType::PropCF18INChannel] = [this](QVariantList & values) {
		values << CF18Channel::CF18_CHANNEL_1;
		values << CF18Channel::CF18_CHANNEL_2;
		values << CF18Channel::CF18_CHANNEL_3;
		values << CF18Channel::CF18_CHANNEL_4;
		values << CF18Channel::CF18_CHANNEL_5;
		values << CF18Channel::CF18_CHANNEL_6;
		values << CF18Channel::CF18_CHANNEL_7;
		values << CF18Channel::CF18_CHANNEL_8;
	};

	// 获取CF18-内同步信号-单位
	map_prop_get_supported_[PropType::PropCF18Unit] = [this](QVariantList & values) {
		values << 0;  // Hz
		values << 1;  // us
	};

	// 获取CF18-内触发信号-通道
	map_prop_get_supported_[PropType::PropCF18ITChannel] = [this](QVariantList & values) {
		values << CF18Channel::CF18_CHANNEL_1;
		values << CF18Channel::CF18_CHANNEL_2;
		values << CF18Channel::CF18_CHANNEL_3;
		values << CF18Channel::CF18_CHANNEL_4;
		values << CF18Channel::CF18_CHANNEL_5;
		values << CF18Channel::CF18_CHANNEL_6;
		values << CF18Channel::CF18_CHANNEL_7;
		values << CF18Channel::CF18_CHANNEL_8;
	};

    // 获取帧率范围
    map_prop_get_range_[PropType::PropFrameRate] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        HscIntRange range{};
        if (getAcquisitionPeriodRange(getProperty(PropType::PropRoi).toRect(), range))
        {
            min = toFrameRate(getCalPeriodDivisor(), range.max);
            max = toFrameRate(getCalPeriodDivisor(), range.min);
            inc = 1;
        }
    };

    // 获取曝光时间范围
    map_prop_get_range_[PropType::PropExposureTime] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        HscIntRange range{};
        qint64 period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
        if (getExposureTimeRange(period,getProperty(PropExposureTimeUnit).value<agile_device::capability::Units>(), range))
        {
            min = range.min;
            max = range.max;
            inc = range.inc;
        }
    };

    // 获取数字增益范围
    map_prop_get_range_[PropType::PropDigitalGain] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        min = 0;
        max = 240;
        inc = 1;
    };

    // 获取保存起点范围
    map_prop_get_range_[PropType::PropRecordingOffset] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        DeviceState state = getState();
        if (state == DeviceState::Unconnected || state == DeviceState::Disconnected)
        {
            min = 0;
            max = 0;
            inc = 1;
            return;
        }
        if (IsTrigger())
        {
            min = 0;
            max = VALID_MAX;
            inc = 1;
            return;
        }
        QRect roi = getProperty(PropType::PropRoi).toRect();
        uint64_t max_frame_count = HscGetMaxAvalibleRecordFrames(device_handle_, roi.x(), roi.y(), roi.width(), roi.height());
        double dmin = 0;
        double dmax = max_frame_count - 1;
        RecordMode offset_mode = getProperty(PropType::PropRecordingOffsetMode).value<RecordMode>();
        if (offset_mode == RecordMode::RECORD_AFTER_SHUTTER)
        {
			dmax = VALID_MAX;
        }
		if (isGrabberDevice() && offset_mode == RecordMode::RECORD_BEFORE_SHUTTER) {//2023,11,1GR系列相机触发前最多保存8G内存的视频
			max_frame_count = HscGetMaxAvalibleRecordFramesBeforeTrigger(device_handle_, roi.x(), roi.y(), roi.width(), roi.height());
			dmax = max_frame_count - 1;
		}
        RecordType unit = getProperty(PropType::PropRecordingUnit).value<RecordType>();
        if (unit == RECORD_BY_TIME)
        {
            qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
            dmin = dmin * acquisition_period / (getCalPeriodDivisor() / 1000);
            dmax = dmax * acquisition_period / (getCalPeriodDivisor() / 1000);
        }
		if (unit == RECORD_BY_TIME_S)
		{
			qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
			dmin = dmin / getCalPeriodDivisor() * acquisition_period;
			dmax = dmax / getCalPeriodDivisor() * acquisition_period;
		}

        min = (qint64)std::ceil(dmin);
        max = (qint64)std::floor(dmax);
        if (max < min)
        {
            max = min;
        }
        inc = 1;
    };

    // 获取保存长度范围
    map_prop_get_range_[PropType::PropRecordingLength] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        DeviceState state = getState();
        if (state == DeviceState::Unconnected || state == DeviceState::Disconnected)
        {
            min = 0;
            max = 0;
            inc = 1;
            return;
        }
        if (IsTrigger())
        {
            min = 1;
            max = VALID_MAX;
            inc = 1;
            return;
        }
        QRect roi = getProperty(PropType::PropRoi).toRect();
        uint64_t max_frame_count = HscGetMaxAvalibleRecordFrames(device_handle_, roi.x(), roi.y(), roi.width(), roi.height());
        double dmin = 1;//保存长度最小为1
        double dmax = max_frame_count;

        RecordType unit = getProperty(PropType::PropRecordingUnit).value<RecordType>();
        if ( unit == RECORD_BY_TIME)
        {
            qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
            dmin = dmin * acquisition_period / (getCalPeriodDivisor()/1000);
            dmax = dmax * acquisition_period / (getCalPeriodDivisor()/1000);
        }
		if (unit == RECORD_BY_TIME_S)
		{
			qint64 acquisition_period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());
			dmin = dmin / getCalPeriodDivisor() * acquisition_period;
			dmax = dmax / getCalPeriodDivisor() * acquisition_period;
		}
		//保存起点为触发前时,保存长度不可小于保存起点
		if (IsMemoryManagementSupported())
		{
			RecordMode offset_mode = getProperty(PropType::PropRecordingOffsetMode).value<RecordMode>();
			if (offset_mode == RecordMode::RECORD_BEFORE_SHUTTER)
			{
				uint64_t offset = getProperty(PropType::PropRecordingOffset).toInt();
				if (dmin < offset)
				{
					dmin = offset;
				}
			}
		}

        min = ceil(dmin);
        max = floor(dmax);
        if (max < min)
        {
			max = min = 1;
        }
        inc = 1;
    };

	// 获取保存帧长度范围
	map_prop_get_range_[PropType::PropRecordingFrameLength] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		DeviceState state = getState();
		if (state == DeviceState::Unconnected || state == DeviceState::Disconnected)
		{
			min = 0;
			max = 0;
			inc = 1;
			return;
		}
		if (IsTrigger())
		{
			min = 1;
			max = VALID_MAX;
			inc = 1;
			return;
		}
		QRect roi = getProperty(PropType::PropRoi).toRect();
		uint64_t max_frame_count = HscGetMaxAvalibleRecordFrames(device_handle_, roi.x(), roi.y(), roi.width(), roi.height());
		max = max_frame_count;

		double dmin = 1;//保存长度最小为1	
		//保存起点为触发前时,保存长度不可小于保存起点
		if (IsMemoryManagementSupported())
		{
			RecordMode offset_mode = getProperty(PropType::PropRecordingOffsetMode).value<RecordMode>();
			if (offset_mode == RecordMode::RECORD_BEFORE_SHUTTER)
			{
				uint64_t offset = getProperty(PropType::PropRecordingOffset).toInt();
				if (dmin < offset)
				{
					dmin = offset;
				}
			}
		}
		min = qCeil(dmin);

		if (max < min)
		{
			max = min = 0;
		}
		inc = 1;
	};

    //获取消抖长度范围
    map_prop_get_range_[PropType::PropJitterEliminationLength] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        DeviceState state = getState();
        if (state == DeviceState::Unconnected || state == DeviceState::Connecting || state == DeviceState::Disconnected)
        {
            min = 0;
            max = 0;
            inc = 1;
            return;
        }
        HscIntRange range;
        HscResult res = HscDevGet(device_handle_, HscRangeSelector(HSC_CFG_JITTER_ELIMINATION_LENGTH), &range, sizeof(range));
        if (res != HSC_OK)
        {
            min = 0;
            max = 0;
            inc = 1;
            return ;
        }
        else
        {
            min = range.min;
            max = range.max;
            inc = range.inc;
            return;
        }
    };

    //获取脉冲宽度范围
    map_prop_get_range_[PropType::PropPulseWidth] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        DeviceState state = getState();
        if (state == DeviceState::Unconnected || state == DeviceState::Connecting || state == DeviceState::Disconnected)
        {
            min = 0;
            max = 0;
            inc = 1;
            return;
        }

        qint64 period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());

        if (IsTrigger())
        {
            min = 500;
            max = period * 1000 - 1;
        }
        else
        {
            getPulseWidthRange(period, min, max);
        }
        inc = 1;

    };

    //获取通道延时范围
    map_prop_get_range_[PropType::PropChn1Delay] = [this](qint64 & min, qint64 & max, qint64 & inc) {
        DeviceState state = getState();
        if (state == DeviceState::Unconnected || state == DeviceState::Connecting || state == DeviceState::Disconnected)
        {
            min = 0;
            max = 0;
            inc = 1;
            return;
        }

        qint64 period = toAcquisitionPeriod(getProperty(PropType::PropFrameRate).toLongLong());

		if (IsPIVEnabled())
		{
			min = 0;
			max = (period - 1) * 1000 ;//TODO:PIV支持
		}
		else
		{
			if (IsRootTrigger())
			{
				min = 0;
				max = (period - 1) * 1000;
			}
			else
			{
				//TODO:级联支持
			}

		}
        inc = 5;

    };
    map_prop_get_range_[PropType::PropChn2Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn3Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn4Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn5Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn6Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn7Delay] = map_prop_get_range_[PropType::PropChn1Delay];
    map_prop_get_range_[PropType::PropChn8Delay] = map_prop_get_range_[PropType::PropChn1Delay];


    // 设置智能触发
    map_prop_set_[PropType::PropIntelligentTrigger] = [this](PropType type, const QVariant & value) {
		if (!checkVariantType<HscIntelligentTriggerParamV2>(value)) {
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}	
        HscIntelligentTriggerParamV2 IntelligentTriggerParamV2 = qvariant_cast<HscIntelligentTriggerParamV2>(value);
        QRect rect = CameraWindowRect2QRect(IntelligentTriggerParamV2.roi);
        correctRoi(rect, kIntelligentTriggerRoi);
        CameraWindowRect correct_rect = QRectTCameraWindowRect(rect);
        memcpy(&(IntelligentTriggerParamV2.roi), &correct_rect, sizeof(correct_rect));
		if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam())
		{
			IntelligentTriggerParamV2.bit_width = (uint8_t)getProperty(PropPixelBitDepth).toInt();
		}
        HscResult res = HscSetIntelligentTriggerParamV2(device_handle_,&IntelligentTriggerParamV2);
        if (res == HSC_OK)
        {
        //	if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), QVariant::fromValue(IntelligentTriggerParamV2));
            }
        }
        return res;
    };

    // 获取智能触发
    map_prop_get_[PropType::PropIntelligentTrigger] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        if (settingsNeedsLoadFromDevice() && !from_local)
        {

            HscIntelligentTriggerParamV2 IntelligentTriggerParamV2{};
            IntelligentTriggerParamV2.roi_color = qRgb(0, 255, 0);

            res = HscGetIntelligentTriggerParamV2(device_handle_, &IntelligentTriggerParamV2);
            if (res == HSC_OK)
            {
                value.setValue(IntelligentTriggerParamV2);
            }
        }
        else {
            QSettings settings;
            HscIntelligentTriggerParamV2 default_value{};
            CameraWindowRect max_roi = QRectTCameraWindowRect(GetMaxRoi(kIntelligentTriggerRoi));
            memcpy(&default_value.roi, &max_roi,sizeof(CameraWindowRect));
            value = settings.value(settingsKey(PropIntelligentTrigger), QVariant::fromValue(default_value));
        }
        return res;
    };


    // 设置自动曝光
    map_prop_set_[PropType::PropAutoExposure] = [this](PropType type, const QVariant & value) {

		if (!checkVariantType<HscAutoExposureParameter>(value)) {
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
        HscAutoExposureParameter param = qvariant_cast<HscAutoExposureParameter>(value);

        QRect rect = CameraWindowRect2QRect(param.autoExpArea);
        correctRoi(rect, kAutoExposureRoi);
        CameraWindowRect correct_rect = QRectTCameraWindowRect(rect);
        memcpy(&(param.autoExpArea), &correct_rect, sizeof(correct_rect));
        HscResult res = HscSetAutoExposureParameter(device_handle_, &param);
        if (res == HSC_OK)
        {
            //if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
            }
        }
        return res;
    };

    // 获取自动曝光
    map_prop_get_[PropType::PropAutoExposure] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
			HscAutoExposureParameter param{};
			param.roi_color = qRgb(0, 0, 255);
            res = HscGetAutoExposureParameter(device_handle_, &param);
            if (res == HSC_OK)
            {
                value.setValue(param);
            }
        }
        else {
            QSettings settings;
            HscAutoExposureParameter default_value{};
            CameraWindowRect max_roi = QRectTCameraWindowRect(GetMaxRoi(kAutoExposureRoi));
            memcpy(&default_value.autoExpArea, &max_roi, sizeof(CameraWindowRect));
            value = settings.value(settingsKey(PropAutoExposure), QVariant::fromValue(default_value));
        }
        return res;
    };

    //设置二次曝光
    map_prop_set_[PropType::PropEdrExposure] = [this](PropType type, const QVariant & value) {

		if (!checkVariantType<HscEDRParam>(value)) {
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
        HscEDRParam param = qvariant_cast<HscEDRParam>(value);
        //将界面中的二次曝光时间转为设备中的二次曝光时间
        param.doubleExposureTime = ConvertExposureTime(param.doubleExposureTime,\
            getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), GetRealExposureTimeUnit(), 1);
        HscResult res = HscSetEDRParam(device_handle_, param);
        if (res == HSC_OK)
        {
            //if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
            }
        }
        return res;
    };

    // 获取二次曝光
    map_prop_get_[PropType::PropEdrExposure] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscEDRParam param{};
            res = HscGetEDRParam(device_handle_, &param);
            //将设备的二次曝光时间转换为界面的二次曝光时间
            param.doubleExposureTime = ConvertExposureTime(param.doubleExposureTime, GetRealExposureTimeUnit(),\
                getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(),\
                getProperty(PropFrameRate).toInt());
            if (res == HSC_OK)
            {
                value.setValue(param);
            }
        }
        else {
            QSettings settings;
            HscEDRParam default_value{};
            value = settings.value(settingsKey(PropEdrExposure), QVariant::fromValue(default_value));

            ////将设备的二次曝光时间转换为界面的二次曝光时间
            HscEDRParam defaultParam = qvariant_cast<HscEDRParam>(value);
            defaultParam.doubleExposureTime = ConvertExposureTime(defaultParam.doubleExposureTime, GetRealExposureTimeUnit(), \
                getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), \
                getProperty(PropFrameRate).toInt());
            value = QVariant::fromValue(defaultParam);
        }
        return res;
    };

	//设置二次曝光V2
	map_prop_set_[PropType::PropEdrExposureV2] = [this](PropType type, const QVariant & value) {

		if (!checkVariantType<HscEDRParamV2>(value))
			return HSC_INVALID_PARAMETER;
		HscEDRParamV2 param = qvariant_cast<HscEDRParamV2>(value);
		//将界面中的二次曝光时间转为设备中的二次曝光时间
		param.doubleExposureTime = ConvertExposureTime(param.doubleExposureTime, \
			getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), GetRealExposureTimeUnit(), 1);
		HscResult res = HscSetEDRParamV2(device_handle_, param);
		if (res == HSC_OK)
		{
			//if (!settingsNeedsLoadFromDevice())
			{
				QSettings settings;
				settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
			}
		}
		return res;
	};

	// 获取二次曝光V2
	map_prop_get_[PropType::PropEdrExposureV2] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (settingsNeedsLoadFromDevice() && !from_local)
		{
			HscEDRParamV2 param{};
			res = HscGetEDRParamV2(device_handle_, &param);
			//将设备的二次曝光时间转换为界面的二次曝光时间
			param.doubleExposureTime = ConvertExposureTime(param.doubleExposureTime, GetRealExposureTimeUnit(), \
				getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), \
				getProperty(PropFrameRate).toInt());
			if (res == HSC_OK)
			{
				value.setValue(param);
			}
		}
		else {
			QSettings settings;
			HscEDRParamV2 default_value{};
			value = settings.value(settingsKey(PropEdrExposureV2), QVariant::fromValue(default_value));

			////将设备的二次曝光时间转换为界面的二次曝光时间
			HscEDRParamV2 defaultParam = qvariant_cast<HscEDRParamV2>(value);
			defaultParam.doubleExposureTime = ConvertExposureTime(defaultParam.doubleExposureTime, GetRealExposureTimeUnit(), \
				getProperty(PropEdrExposureUnit).value<agile_device::capability::Units>(), \
				getProperty(PropFrameRate).toInt());
			value = QVariant::fromValue(defaultParam);
		}
		return res;
	};

    // 设置二次曝光时间单位
    map_prop_set_[PropType::PropEdrExposureUnit] = [this](PropType type, const QVariant & value) {
        QSettings settings;
        settings.setValue(this->settingsKey(type), value.toInt());

        return HSC_OK;
    };

    // 获取二次曝光时间单位
    map_prop_get_[PropType::PropEdrExposureUnit] = [this](QVariant & value, bool from_local) {
        QSettings settings;
        value = settings.value(this->settingsKey(PropType::PropEdrExposureUnit), agile_device::capability::Units::kUnitUs);

        return HSC_OK;
    };

    // 设置图像控制
    map_prop_set_[PropType::PropImageCtrl] = [this](PropType type, const QVariant & value) {
		if (!checkVariantType<HscImageCtrlParam>(value))
		{
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
        HscImageCtrlParam param = qvariant_cast<HscImageCtrlParam>(value);
        HscResult res = HscSetImageCtrlParam(device_handle_, &param);
        if (res == HSC_OK)
        {
            //if (!settingsNeedsLoadFromDevice())
            {
                QSettings settings;
                settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
            }
        }
        return res;
    };

    // 获取图像控制参数
    map_prop_get_[PropType::PropImageCtrl] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscImageCtrlParam param{};
            res = HscGetImageCtrlParam(device_handle_, &param);
            if (res == HSC_OK)
            {
                value.setValue(param);
            }
        }
        else {
            QSettings settings;
            HscImageCtrlParam default_value{};
            value = settings.value(settingsKey(PropImageCtrl), QVariant::fromValue(default_value));
        }
        return res;
    };
    // 设置piv参数
    map_prop_set_[PropType::PropPivParam] = [this](PropType type, const QVariant & value) {
        if (!checkVariantType<HscPIVParam>(value)){
			CSLOG_ERROR("Illegal parameters, device_type:{}", type);
			return HSC_INVALID_PARAMETER;
		}
        HscPIVParam param = qvariant_cast<HscPIVParam>(value);
        HscResult res = HscSetPIVParam(device_handle_, &param);
        if (res == HSC_OK)
        {
            QSettings settings;
            settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
        }
        return res;
    };

    // 获取piv参数
    map_prop_get_[PropType::PropPivParam] = [this](QVariant & value, bool from_local) {
        HscResult res = HSC_OK;
        if (settingsNeedsLoadFromDevice() && !from_local)
        {
            HscPIVParam param{};
            res = HscGetPIVParam(device_handle_, &param);
            if (res == HSC_OK)
            {
                value.setValue(param);
            }
        }
        else {
            QSettings settings;
            HscPIVParam default_value{};
            value = settings.value(settingsKey(PropPivParam), QVariant::fromValue(default_value));
        }
        return res;
    };

	// 设置像素位深
	map_prop_set_[PropType::PropPixelBitDepth] = [this](PropType type, const QVariant & value) {
		int32_t param = value.toInt();
		HscResult res = HSC_OK;//HscDevSet(device_handle_, HSC_CFG_BIT_VAL, &param, sizeof(param));
		if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam())
		{
			res = HscDevSet(device_handle_, HSC_CFG_BIT_VAL, &param, sizeof(param));
		}
		if (res == HSC_OK)
		{
			QSettings settings;
			settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
		}
		if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam())
		{
			current_out_bpp_ = value.toInt();
			if (image_processor_ptr_)
			{
				image_processor_ptr_->SetSignificantBitsPerPixel(current_out_bpp_);
				image_processor_ptr_->SetBitsPerPixel(current_out_bpp_ <= 8 ? 8 : 16);
			}
		}
		return res;
	};

	// 获取像素位深
	map_prop_get_[PropType::PropPixelBitDepth] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (settingsNeedsLoadFromDevice() && !from_local)
		{
			int32_t param{8};
			HscResult res = HSC_OK;
			if (IsIntelligentTriggerV4Supported() || isSupportHighBitParam())
			{
				res = HscDevGet(device_handle_, HSC_CFG_BIT_VAL, &param, sizeof(param));
			}
			if (res == HSC_OK)
			{
				value.setValue(param);
				current_out_bpp_ = value.toInt();
			}
		}
		else {
			QSettings settings;
			int32_t default_value{8};
			value = settings.value(settingsKey(PropPixelBitDepth), QVariant::fromValue(default_value));			
			current_out_bpp_ = value.toInt();
		}
		return res;
	};

	// 设置显示像素位深
	map_prop_set_[PropType::PropDisplayBitDepth] = [this](PropType type, const QVariant & value) {
		int32_t param = value.toInt();
		current_display_bpp_ = (DisplayBitDepth)param;
		{
			QSettings settings;
			settings.setValue(this->settingsKey(PropDisplayBitDepth), QVariant::fromValue(param));
		}

		return HSC_OK;
	};

	// 获取显示像素位深
	map_prop_get_[PropType::PropDisplayBitDepth] = [this](QVariant & value, bool from_local) {
		if (current_display_bpp_ != DBD_NULL)
		{
			value = QVariant::fromValue((int32_t)current_display_bpp_);
		}
		else
		{
			QSettings settings;
			int32_t default_value{ 0 };
			value = settings.value(settingsKey(PropDisplayBitDepth), QVariant::fromValue(default_value));
			current_display_bpp_ = (DisplayBitDepth)value.toInt();
		}
		return HSC_OK;
	};

	// 设置激活像素触发模式
	map_prop_set_[PropType::PropPixelTriggerMode] = [this](PropType type, const QVariant & value) {
		int32_t param = value.toInt();
		HscResult res = HSC_OK;//HscDevSet(device_handle_, HSC_CFG_PIXEL_TRIGGER, &param, sizeof(param));
		if (res == HSC_OK)
		{
			QSettings settings;
			settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
		}
		return res;
	};

	// 获取激活像素触发模式
	map_prop_get_[PropType::PropPixelTriggerMode] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (settingsNeedsLoadFromDevice() && !from_local)
		{
			int32_t param{ 0 };
			res = HSC_OK;//HscDevGet(device_handle_, HSC_CFG_PIXEL_TRIGGER, &param, sizeof(param));
			if (res == HSC_OK)
			{
				value.setValue(param);
			}
		}
		else {
			QSettings settings;
			int32_t default_value{0};
			value = settings.value(settingsKey(PropPixelTriggerMode), QVariant::fromValue(default_value));
		}
		return res;
	};

	// 设置激活像素触发数量
	map_prop_set_[PropType::PropPixelTriggerNumber] = [this](PropType type, const QVariant & value) {
		uint64_t param = value.toULongLong();
		HscResult res = HSC_OK;//HscDevSet(device_handle_, HSC_CFG_PIXEL_TRIGGER_VAL, &param, sizeof(param));
		if (res == HSC_OK)
		{
			QSettings settings;
			settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
		}
		return res;
	};

	// 获取激活像素触发数量
	map_prop_get_[PropType::PropPixelTriggerNumber] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (settingsNeedsLoadFromDevice() && !from_local)
		{
			uint64_t param{ 1 };
			res = HSC_OK;//HscDevGet(device_handle_, HSC_CFG_PIXEL_TRIGGER_VAL, &param, sizeof(param));
			if (res == HSC_OK)
			{
				value.setValue(param);
			}
		}
		else {
			QSettings settings;
			uint64_t default_value{1};
			value = settings.value(settingsKey(PropPixelTriggerNumber), QVariant::fromValue(default_value));
		}
		return res;
	};

	// 设置CMos数字增益
	map_prop_set_[PropType::PropCmosDigitalVal] = [this](PropType type, const QVariant & value) {
		uint32_t param = value.toInt();
		HscResult res = HscDevSet(device_handle_, HSC_CFG_CMOS_DIGITAL_VAL, &param, sizeof(param));
		if (res == HSC_OK)
		{
			QSettings settings;
			settings.setValue(this->settingsKey(type), QVariant::fromValue(param));
		}
		return res;
	};

	// 获取CMos数字增益
	map_prop_get_[PropType::PropCmosDigitalVal] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		if (settingsNeedsLoadFromDevice() && !from_local)
		{
			uint32_t param{ 0 };
			res = HscDevGet(device_handle_, HSC_CFG_CMOS_DIGITAL_VAL, &param, sizeof(param));
			if (res == HSC_OK)
			{
				value.setValue(param);
			}
		}
		else {
			QSettings settings;
			uint32_t default_value{ 0 };
			value = settings.value(settingsKey(PropCmosDigitalVal), QVariant::fromValue(default_value));
		}
		return res;
	};

	// 获取-外触发信号-消抖时间-范围
	map_cf18_prop_get_range_[PropType::PropCF18ETJitterEliminationTime] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-外同步信号-消抖时间-范围
	map_cf18_prop_get_range_[PropType::PropCF18ENJitterEliminationTime] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-B码信号-消抖时间-范围
	map_cf18_prop_get_range_[PropType::PropCF18BJitterEliminationTime] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-周期-范围
	map_cf18_prop_get_range_[PropType::PropCF18Cycle] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-内同步信号-高电平宽度-范围
	map_cf18_prop_get_range_[PropType::PropCF18INHighLevelWidth] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-内同步信号-上升沿延迟-范围
	map_cf18_prop_get_range_[PropType::PropCF18INRisingDelay] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;

	};

	// 获取-内触发信号-高电平宽度-范围
	map_cf18_prop_get_range_[PropType::PropCF18ITHighLevelWidth] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};

	// 获取-内触发信号-上升沿延迟-范围
	map_cf18_prop_get_range_[PropType::PropCF18ITRisingDelay] = [this](qint64 & min, qint64 & max, qint64 & inc) {
		min = 0;
		max = 65535;
		inc = 1;
	};


	// 设置设备名称
	map_cf18_prop_set_[PropType::PropName] = [this](PropType type, const QVariant & value) {
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);

		return HSC_OK;
	};

	// 获取设备名称
	map_cf18_prop_get_[PropType::PropName] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropName), "");

		return HSC_OK;
	};

	// 设置设备序号
	map_cf18_prop_set_[PropType::PropDeviceIndex] = [this](PropType type, const QVariant & value) {
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);

		return HSC_OK;
	};

	// 获取设备序号
	map_cf18_prop_get_[PropType::PropDeviceIndex] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropDeviceIndex), 0);

		return HSC_OK;
	};

	// 设置-外触发信号-极性反转
	map_cf18_prop_set_[PropType::PropCF18ETPolarityReversal] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			et_param.polarityReversal = value.toInt();
			SetInputTrigger(et_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外触发信号-极性反转
	map_cf18_prop_get_[PropType::PropCF18ETPolarityReversal] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			value.setValue(et_param.polarityReversal);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外同步信号-极性反转
	map_cf18_prop_set_[PropType::PropCF18ENPolarityReversal] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			en_param.polarityReversal = value.toInt();
			SetInputSync(en_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外同步信号-极性反转
	map_cf18_prop_get_[PropType::PropCF18ENPolarityReversal] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			value.setValue(en_param.polarityReversal);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-内同步信号-通道
	map_cf18_prop_set_[PropType::PropCF18INChannel] = [this](PropType type, const QVariant & value) {
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);
		return HSC_OK;
	};

	// 获取-内同步信号-通道
	map_cf18_prop_get_[PropType::PropCF18INChannel] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropCF18INChannel), 0);
		return HSC_OK;
	};

	// 设置-内触发信号-通道
	map_cf18_prop_set_[PropType::PropCF18ITChannel] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);
		return res;
	};

	// 获取-内触发信号-通道
	map_cf18_prop_get_[PropType::PropCF18ITChannel] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropCF18ITChannel), 0);

		return HSC_OK;
	};

	// 设置-单位
	map_cf18_prop_set_[PropType::PropCF18Unit] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		QSettings settings;
		settings.setValue(this->settingsKey(type), value);
		return res;
	};

	// 获取-单位
	map_cf18_prop_get_[PropType::PropCF18Unit] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropCF18Unit), 0);

		return HSC_OK;
	};

	// 设置-外触发信号-消抖时间
	map_cf18_prop_set_[PropType::PropCF18ETJitterEliminationTime] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			et_param.ditheringTime = value.toInt();
			SetInputTrigger(et_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外触发信号-消抖时间
	map_cf18_prop_get_[PropType::PropCF18ETJitterEliminationTime] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			value.setValue(et_param.ditheringTime);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外触发信号-上升沿计数
	map_cf18_prop_set_[PropType::PropCF18ETRisingCount] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			et_param.risingEdgeCnt = value.toInt();
			SetInputTrigger(et_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外触发信号-上升沿计数
	map_cf18_prop_get_[PropType::PropCF18ETRisingCount] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			value.setValue(et_param.risingEdgeCnt);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外触发信号-下降沿计数
	map_cf18_prop_set_[PropType::PropCF18ETFallingCount] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			et_param.fallingEdgeCnt = value.toInt();
			SetInputTrigger(et_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外触发信号-下降沿计数
	map_cf18_prop_get_[PropType::PropCF18ETFallingCount] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputTrigger et_param{};
		if (GetInputTrigger(et_param)) {
			value.setValue(et_param.fallingEdgeCnt);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外同步信号-消抖时间
	map_cf18_prop_set_[PropType::PropCF18ENJitterEliminationTime] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			en_param.ditheringTime = value.toInt();
			SetInputSync(en_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外同步信号-消抖时间
	map_cf18_prop_get_[PropType::PropCF18ENJitterEliminationTime] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			value.setValue(en_param.ditheringTime);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外同步信号-上升沿计数
	map_cf18_prop_set_[PropType::PropCF18ENRisingCount] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			en_param.risingEdgeCnt = value.toInt();
			SetInputSync(en_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外同步信号-上升沿计数
	map_cf18_prop_get_[PropType::PropCF18ENRisingCount] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			value.setValue(en_param.risingEdgeCnt);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-外同步信号-下降沿计数
	map_cf18_prop_set_[PropType::PropCF18ENFallingCount] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			en_param.fallingEdgeCnt = value.toInt();
			SetInputSync(en_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-外同步信号-下降沿计数
	map_cf18_prop_get_[PropType::PropCF18ENFallingCount] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInputSync en_param{};
		if (GetInputSync(en_param)) {
			value.setValue(en_param.fallingEdgeCnt);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-B码信号-消抖时间
	map_cf18_prop_set_[PropType::PropCF18BJitterEliminationTime] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		if (!SetInputBcode(value.toInt())) {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-B码信号-消抖时间
	map_cf18_prop_get_[PropType::PropCF18BJitterEliminationTime] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		uint32_t b_time{};
		if (!GetInputBcode(b_time)) {
			res = HSC_ERROR;
		}
		value.setValue(b_time);
		return res;
	};

	// 设置-周期
	map_cf18_prop_set_[PropType::PropCF18Cycle] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			in_param.channel = getProperty(PropCF18INChannel).toInt();
			in_param.cycle = value.toInt();
			SetInternalSync(in_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-周期
	map_cf18_prop_get_[PropType::PropCF18Cycle] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			value.setValue(in_param.cycle);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-内同步信号-高电平宽度
	map_cf18_prop_set_[PropType::PropCF18INHighLevelWidth] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			in_param.channel = getProperty(PropCF18INChannel).toInt();
			in_param.highLevelWidth = value.toInt();
			SetInternalSync(in_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-内同步信号-高电平宽度
	map_cf18_prop_get_[PropType::PropCF18INHighLevelWidth] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			value.setValue(in_param.highLevelWidth);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-内同步信号-上升沿延迟
	map_cf18_prop_set_[PropType::PropCF18INRisingDelay] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			in_param.channel = getProperty(PropCF18INChannel).toInt();
			in_param.risingDelay = value.toInt();
			SetInternalSync(in_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-内同步信号-上升沿延迟
	map_cf18_prop_get_[PropType::PropCF18INRisingDelay] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInternalSync in_param{};
		if (GetInternalSync(getProperty(PropCF18INChannel).toInt(), in_param)) {
			value.setValue(in_param.risingDelay);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-内触发信号-高电平宽度
	map_cf18_prop_set_[PropType::PropCF18ITHighLevelWidth] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInternalTrigger it_param{};
		if (GetInternalTrigger(getProperty(PropCF18ITChannel).toInt(), it_param)) {
			it_param.channel = getProperty(PropCF18ITChannel).toInt();
			it_param.highLevelWidth = value.toInt();
			SetInternalTrigger(it_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-内触发信号-高电平宽度
	map_cf18_prop_get_[PropType::PropCF18ITHighLevelWidth] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInternalTrigger it_param{};
		if (GetInternalTrigger(getProperty(PropCF18ITChannel).toInt(), it_param)) {
			value.setValue(it_param.highLevelWidth);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 设置-内触发信号-上升沿延迟
	map_cf18_prop_set_[PropType::PropCF18ITRisingDelay] = [this](PropType type, const QVariant & value) {
		HscResult res = HSC_OK;
		CmdInternalTrigger it_param{};
		if (GetInternalTrigger(getProperty(PropCF18ITChannel).toInt(), it_param)) {
			it_param.channel = getProperty(PropCF18ITChannel).toInt();
			it_param.risingDelay = value.toInt();
			SetInternalTrigger(it_param);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};

	// 获取-内触发信号-上升沿延迟
	map_cf18_prop_get_[PropType::PropCF18ITRisingDelay] = [this](QVariant & value, bool from_local) {
		HscResult res = HSC_OK;
		CmdInternalTrigger it_param{};
		if (GetInternalTrigger(getProperty(PropCF18ITChannel).toInt(), it_param)) {
			value.setValue(it_param.risingDelay);
		}
		else {
			res = HSC_ERROR;
		}
		return res;
	};


	// 设置CF18控制面板使能
	map_cf18_prop_set_[PropType::PropCF18ControlPanelEnable] = [this](PropType type, const QVariant & value) {
		QSettings settings;
		if (IsCF18())
		{
			settings.setValue(this->settingsKey(type), value);
		}

		return HSC_OK;
	};

	// 获取CF18控制面板使能
	map_cf18_prop_get_[PropType::PropCF18ControlPanelEnable] = [this](QVariant & value, bool from_local) {
		QSettings settings;
		value = settings.value(this->settingsKey(PropType::PropCF18ControlPanelEnable), false);
		if (!IsCF18())
		{
			value = false;
		}

		return HSC_OK;
	};
}


void Device::getPulseWidthRange(qint64 period, qint64 &min, qint64 &max)
{
	auto framerate = getProperty(PropType::PropFrameRate).toLongLong();
	min = 1;
	if (m_pulsewidth_mode == 1)
	{
		max = 1e6 / framerate - 5;
	}
	else
	{//to do:更改依据：张万林给的公式
		max = 1e6 / framerate - 2;
		if (m_pulsewidth_mode == 2)//单位百纳秒
		{
			max *= 10;
		}
		if (max <=0) {//to do:X系列根据上述公式会出异常值，根据钱昊修改意见，最小值给1
			max = min;
		}
	}
	return;
}
