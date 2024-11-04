#include "csrccapp.h"
#include "ui_csrccapp.h"
#include "csviewmanager.h"
#include <QGridLayout>
#include <QSettings>
#include <QStorageInfo>
#include <QDir>
#include <QFile>
#include <QLayout>
#include <qmetaobject.h>
#include "Device/devicemanager.h"
#include "Device/device.h"
#include "Device/imageprocessor.h"
#include "render/PlayerViewBase.h"
#include "Common/UIUtils/uiutils.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "Common/AgErrorCode/agerrorcode.h"
#include "Common/ErrorCodeUtils/errorcodeutils.h"
#include "Common/LogUtils/logutils.h"
#include "System/csdlgimagetrainning.h"
#include "System/SystemSettings/csdlgarmmanualwhitebalance.h"
#include "System/SystemSettings/csdlgmanualwhitebalance.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "System/SystemSettings/csdlggainsetting.h"
#include "System/SystemSettings/csdlggainsettingv2.h"
#include "System/SystemSettings/csdlggainsettingv3.h"
#include "System/SystemSettings/csdlggainsettingv4.h"
#include "System/SystemSettings/csdlgluminanceandcontrastsetting.h"
#include "System/SystemSettings/csdlgsetoption.h"
#include "System/Experiment/csdlgnewexp.h"
#include "System/Experiment/csdlgopenexp.h"
//#include "System/csdlgfallpointmeasuresetup.h"
#include "System/csdlgselfcheck.h"
#include "System/cshealthmanager.h"
#include "Device/csframeinfo.h"
#include "csmenulocalvideo/csmenulocalvideo.h"
#include "cslocalvideoplayer/cslocalvideoplayer.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "System/SystemSettings/csdlghealthmanagersetting.h"
#include <QDesktopServices>
#include "System/SystemSettings/csdlgdeviceauth.h"
#include "cssoftwareversion/cssoftwareversion.h"
#include "recoveringdata/recoveringdatawidget.h"
#include "ManualWhiteBalance/CSManualWhiteBalanceDlg.h"
#ifndef Q_OS_WIN32
#include <X11/XKBlib.h>
#include <stdio.h>
#endif
#include <QMessageBox>
#ifdef Q_OS_WIN32
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
#endif // Q_OS_WIN32

#include "csexportpreview/csexportpreview.h"
#include "cslowlightmode/cslowlightmode.h"

CSRccApp::CSRccApp(QWidget *parent) 
	: QMainWindow(parent)
	, ui(new Ui::CSRccApp)
	, m_iViewNum(16)
	, m_iWindowIndex(-1)
	, m_nGrayOrRGBAtQueryPoint(0)
	, m_pViewManage(new CSViewManager(this))
	, m_winCrossLineSet(new CSWindowCrossLineSetting(this))
{
    ui->setupUi(this);
	installEventFilter(this);
    InitUI();
	ConnectSignalSlot();
	Init();
	qRegisterMetaType<Device::RoiTypes>("RoiTypes");
	qRegisterMetaType<QRect>("QRect");
	m_winCrossLineSet->setModal(true);
}

CSRccApp::~CSRccApp()
{
	removeEventFilter(this);
    delete ui;
}

void CSRccApp::onMainWindowShown()
{
	m_device_search_list_widget->refreshSearch();
}

void CSRccApp::UpdateDevicePropertyList()
{
	m_property_view_device_widget->UpdateCurrentPropertyList();
}

void CSRccApp::EnableRoiRelatedWidgets(bool enable)
{
	ui->menuBar->setEnabled(enable);
	ui->statusBar->setEnabled(enable);
	ui->cameraAcqToolBar->setEnabled(enable);
	ui->acqWindowToolBar->setEnabled(enable);
	ui->runAlarmToolBar->setEnabled(enable);
	ui->userManualToolBar->setEnabled(enable);
	ui->dockWidgetDevicelist->setEnabled(enable);
	ui->dockWidgetPropertyList->setEnabled(enable);
	ui->dockWidgetVideoList->setEnabled(enable);
	ui->dockWidgetTemperature->setEnabled(enable);
	ui->cf18_control_dockWidget->setEnabled(enable);
}

void CSRccApp::AddDeviceToView(const QString device_ip)
{
	if (!device_ip.isEmpty())
	{
		GetViewManagerPtr()->AddToDisplayList(device_ip);
	}
}

void CSRccApp::Init()
{
	//初始化m_iViewNum个播放控件
	for (int i = 0; i < m_iViewNum; i++)
	{
		InitVideoWidget(i);
	}



	std::function<void(int, const QString&)> associate_callback = [this](int view_id, const QString& device_name) 
	{
		CSLOG_INFO("Associate view:{} to device:{}", view_id, device_name.toStdString());
		auto device = DeviceManager::instance().getDevice(device_name);
		auto player = GetViewManagerPtr()->getView(view_id);
		if (device)
		{
			if (player)
			{
				connect(device.data(), &Device::updateFrame, player, &CPlayerViewBase::SlotUpdateImage);
			}
			connect(device.data(), &Device::SignalUpdateFrameIntelligentAvgBright, this, \
				&CSRccApp::SlotUpdateFrameIntelligentAvgBright, Qt::QueuedConnection);
			connect(device.data(), &Device::SignalUpdateFrameAutoExposureAvgGray, this, \
				&CSRccApp::SlotUpdateFrameAutoExposureAvgGray, Qt::QueuedConnection);
			//TODO:device status signals
			device->updateViewOSD();//manual refresh osd
		}
		if (player)
		{
			GetViewManagerPtr()->setViewDeviceName(player->getId(), device_name);
		}
	};

	std::function<void(int, const QString&)> un_associate_callback = [this](int view_id, const QString& device_name) 
	{
		CSLOG_INFO("Unassociate view:{} with device:{}", view_id, device_name.toStdString());
        auto device = DeviceManager::instance().getDevice(device_name);
		auto player = GetViewManagerPtr()->getView(view_id);
		if (device)
		{
			if (player)
			{
				disconnect(device.data(), &Device::updateFrame, player, &CPlayerViewBase::SlotUpdateImage);
			}
			disconnect(device.data(), &Device::SignalUpdateFrameIntelligentAvgBright, this, \
				&CSRccApp::SlotUpdateFrameIntelligentAvgBright);
			disconnect(device.data(), &Device::SignalUpdateFrameAutoExposureAvgGray, this, \
				&CSRccApp::SlotUpdateFrameAutoExposureAvgGray);
			//TODO:device status signals
		}
		if (player)
		{
			GetViewManagerPtr()->setViewDeviceName(player->getId(), "");
			player->SlotUpdateImage(makeOsdInfo());
		}	
	};

	GetViewManagerPtr()->registerCbEx(associate_callback, un_associate_callback);

	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		CSHealthManager::instance().start();
	}


    //加载已经记录的标定文件
	SystemSettingsManager::instance().loadCalibrationParamFiles();

	(QLocale::Language::Chinese == CSRccAppTranslator::instance().getLanguage()) ? ui->actionChinese->setChecked(true)\
		: ui->actionEnglish->setChecked(true);
}

void CSRccApp::InitUI()
{
	this->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->dockWidgetDevicelist->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->dockWidgetPropertyList->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->dockWidgetVideoList->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->dockWidgetTemperature->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->dockWidgetPointMeasure->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->cf18_control_dockWidget->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());
	ui->menuBar->setStyleSheet(SystemSettingsManager::instance().get12PxStyle());

	QVBoxLayout* dock_widget_vlayout = new QVBoxLayout(ui->dockWidgetContentsDevicelist);
	dock_widget_vlayout->setContentsMargins(0, 0, 0, 0);
	dock_widget_vlayout->setSpacing(6);
	dock_widget_vlayout->setStretch(3, 1);
	m_device_list_widget = new CSDeviceListWidget(ui->dockWidgetContentsDevicelist);
	dock_widget_vlayout->addWidget(m_device_list_widget);
	m_device_search_list_widget = new CSDeviceSearchListWidget(ui->dockWidgetContentsDevicelist);
	dock_widget_vlayout->addWidget(m_device_search_list_widget);

	QVBoxLayout* dock_widget_vlayout_Property = new QVBoxLayout(ui->dockWidgetContentsPropertyList);
	dock_widget_vlayout_Property->setContentsMargins(0, 0, 0, 0);
	m_property_view_device_widget = new CSPropertyViewDevice(ui->dockWidgetContentsPropertyList);
	dock_widget_vlayout_Property->addWidget(m_property_view_device_widget);

	QVBoxLayout* dock_widget_vlayout_list_view_video = new QVBoxLayout(ui->dockWidgetContentsVideoList);
	dock_widget_vlayout_list_view_video->setContentsMargins(0, 0, 0, 0);
	m_list_view_video_widget = new CSListViewVideo(ui->dockWidgetContentsVideoList);
	dock_widget_vlayout_list_view_video->addWidget(m_list_view_video_widget);

	QVBoxLayout* dock_widget_vlayout_ContentsTemperature = new QVBoxLayout(ui->dockWidgetContentsTemperature);
	dock_widget_vlayout_ContentsTemperature->setContentsMargins(0, 0, 0, 0);
	m_devicetemperaturepanel_widget = new CSDeviceTemperaturePanel(ui->dockWidgetContentsTemperature);
	dock_widget_vlayout_ContentsTemperature->addWidget(m_devicetemperaturepanel_widget);

	QVBoxLayout* dock_widget_vlayout_measure_page = new QVBoxLayout(ui->dockWidgetContentsMeasure);
	dock_widget_vlayout_measure_page->setContentsMargins(0, 0, 0, 0);
	m_measure_page_dlg_widget = new CSMeasurePageDlg(ui->dockWidgetContentsMeasure);
	dock_widget_vlayout_measure_page->addWidget(m_measure_page_dlg_widget);

	QVBoxLayout* dock_widget_vlayout_cf18_control_pane = new QVBoxLayout(ui->dockWidgetContentsCF18);
	dock_widget_vlayout_cf18_control_pane->setContentsMargins(0, 0, 0, 0);
	m_cf18_control_panel_widget = new CSCF18ControlPanel(ui->dockWidgetContentsCF18);
	dock_widget_vlayout_cf18_control_pane->addWidget(m_cf18_control_panel_widget);

	setAutoFillBackground(false);
	m_mainLayout = new QGridLayout(ui->widget);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

    //去掉状态栏右下角的三角
    ui->statusBar->setSizeGripEnabled(false);

    m_pFrameRateLabel = new QLabel(this);
    m_pFrameRateLabel->setObjectName("numLabel");
    ui->statusBar->addPermanentWidget(m_pFrameRateLabel);

    m_pXLabel = new QLabel(this);
    m_pXLabel->setObjectName("numLabel");
    m_pXLabel->setMinimumWidth(40);
    ui->statusBar->addPermanentWidget(m_pXLabel);    //显示永久信息

    m_pYLabel = new QLabel(this);
    m_pYLabel->setObjectName("numLabel");
    m_pYLabel->setMinimumWidth(40);
    ui->statusBar->addPermanentWidget(m_pYLabel);

	m_pGrayOrRGBLabel = new QLabel(this);
    m_pGrayOrRGBLabel->setObjectName("numLabel");
	m_pGrayOrRGBLabel->setMinimumWidth(70);
    ui->statusBar->addPermanentWidget(m_pGrayOrRGBLabel);

	m_pAvgLumLabel = new QLabel(this);
	m_pAvgLumLabel->setObjectName("numLabel");
	m_pAvgLumLabel->setMinimumWidth(70);
	m_pAvgLumLabel->setText(tr("Average Luminance: ") + QString::number(0));
    ui->statusBar->addPermanentWidget(m_pAvgLumLabel);

	m_pZoomCoefficientLabel = new QLabel(this);
	m_pZoomCoefficientLabel->setObjectName("numLabel");
    m_pZoomCoefficientLabel->setMinimumWidth(50);
    ui->statusBar->addPermanentWidget(m_pZoomCoefficientLabel);

	m_pRotateLabel = new QLabel(this);
	m_pRotateLabel->setObjectName("numLabel");
	m_pRotateLabel->setMinimumWidth(40);
    ui->statusBar->addPermanentWidget(m_pRotateLabel);

	m_pBcode = new QLabel(this);
	m_pBcode->setObjectName("numLabel");
	m_pBcode->setMinimumWidth(80);
	ui->statusBar->addPermanentWidget(m_pBcode);
	SetBcode(m_BcodeOn);

	m_pCapLabel = new QLabel(this);
    bool caps_state = false;
    bool num_state = false;
    bool scrl_state = false;
#ifdef Q_OS_WIN32
    caps_state = LOBYTE(GetKeyState(VK_CAPITAL));
    num_state = LOBYTE(GetKeyState(VK_NUMLOCK));
    scrl_state = LOBYTE(GetKeyState(VK_SCROLL));
#else
    Display* d = XOpenDisplay((char*)0);
    if(d)
    {
        unsigned n;
        XkbGetIndicatorState(d,XkbUseCoreKbd,&n);
        caps_state = (n == 1);
        num_state = (n == 2);
        if(n==3)
        {
            caps_state = true;
            num_state = true;
        }
    }
#endif
    UpdateCapLabelStatus(caps_state);
    m_pCapLabel->setText("CAP");
	UpdateCapLabelStatus(caps_state);
    ui->statusBar->addPermanentWidget(m_pCapLabel);

	m_pNumLabel = new QLabel(this);
    UpdateNumLabelStatus(num_state);
	m_pNumLabel->setText("NUM");
    ui->statusBar->addPermanentWidget(m_pNumLabel);

	m_pScrlLabel = new QLabel(this);
    UpdateScrlLabelStatus(scrl_state);
	m_pScrlLabel->setText("SCRL");
    ui->statusBar->addPermanentWidget(m_pScrlLabel);


	//合并视频列表,属性列表,温控列表
	tabifyDockWidget(ui->dockWidgetPropertyList, ui->dockWidgetVideoList);
	tabifyDockWidget(ui->dockWidgetPropertyList, ui->dockWidgetTemperature);
	tabifyDockWidget(ui->dockWidgetPropertyList, ui->dockWidgetPointMeasure);
	tabifyDockWidget(ui->dockWidgetPropertyList, ui->cf18_control_dockWidget);
	ui->dockWidgetPropertyList->raise();//属性列表置顶
	//隐藏温控列表
	ui->actionTemperature_Panel->setEnabled(false);
	ui->actionTemperature_Panel->setChecked(false);
	ui->dockWidgetTemperature->setVisible(false);

	//隐藏CF18控制列表
	ui->actionControl_Panel->setEnabled(false);
	ui->actionControl_Panel->setChecked(false);
	ui->cf18_control_dockWidget->setVisible(false);

	//白平衡菜单项单选控制
	m_actionGroup_WhiteBalance = new QActionGroup(ui->menuImage_I);
	m_actionGroup_WhiteBalance->addAction(ui->actionNone);
	m_actionGroup_WhiteBalance->addAction(ui->actionAuto);
	m_actionGroup_WhiteBalance->addAction(ui->actionManual);
	m_actionGroup_WhiteBalance->addAction(ui->actionSettingManual);

	//色彩模式单选控制
	m_actionGroup_ColorMode = new QActionGroup(ui->menuImage_I);
	m_actionGroup_ColorMode->addAction(ui->actionMonochrome);
	m_actionGroup_ColorMode->addAction(ui->actionColor);

	//拍摄环境单选控制
	m_actionGroup_WbEnv = new QActionGroup(ui->menuImage_I);
	m_actionGroup_WbEnv->addAction(ui->actionEnvironmentNone);
	m_actionGroup_WbEnv->addAction(ui->actionEnvironmentAuto);
	m_actionGroup_WbEnv->addAction(ui->actionSunny);
	m_actionGroup_WbEnv->addAction(ui->actionSunset);	
	m_actionGroup_WbEnv->addAction(ui->actionCloudy_Day);
	m_actionGroup_WbEnv->addAction(ui->actionFill_Light);

	//风扇控制单选控制
	m_actionGroup_FanControl = new QActionGroup(ui->menuImage_I);
	m_actionGroup_FanControl->addAction(ui->actionFan_ControlOpen);
	m_actionGroup_FanControl->addAction(ui->actionFan_ControlClose);
	m_actionGroup_FanControl->addAction(ui->actionFan_ControlAuto);

	//自动曝光单选控制
	m_actionGroup_AutoExposure = new QActionGroup(ui->menuImage_I);
    m_actionGroup_AutoExposure->addAction(ui->AutoExposureOpen);
    m_actionGroup_AutoExposure->addAction(ui->AutoExposureClose);

	//智能触发单选控制
	m_actionGroup_IntelligentTrigger = new QActionGroup(ui->menuImage_I);
	m_actionGroup_IntelligentTrigger->addAction(ui->IntelligentTriggerOpen);
	m_actionGroup_IntelligentTrigger->addAction(ui->IntelligentTriggerClose);

	//二次曝光单选控制
	m_actionGroup_EdrExposure = new QActionGroup(ui->menuImage_I);
	m_actionGroup_EdrExposure->addAction(ui->EdrExposureOpen);
	m_actionGroup_EdrExposure->addAction(ui->EdrExposureClose);

	//镜像,倒像,翻转控制(非互斥)
	m_actionGroup_ImageCtrl = new QActionGroup(ui->mainToolBar);
	m_actionGroup_ImageCtrl->addAction(ui->actionMirror);
	m_actionGroup_ImageCtrl->addAction(ui->actionInverted_Image);
	m_actionGroup_ImageCtrl->addAction(ui->actionFlip_The_Image);
	m_actionGroup_ImageCtrl->setExclusive(false);	

	//翻译单选控制
	m_actionGroup_Translate = new QActionGroup(ui->menuLanguage);
	m_actionGroup_Translate->addAction(ui->actionChinese);
	m_actionGroup_Translate->addAction(ui->actionEnglish);

	//非调试模式下隐藏滤波按钮
	if (!FunctionCustomizer::GetInstance().isConsoleEnabled())
	{
		ui->actionFiler1->setVisible(false);
		ui->actionFilter->setVisible(false);
	}

	//西光所版本采集相关工具栏布局变更
	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
 		//添加一个空白工具栏,添加空白布局. 
		QToolBar* toolbar_spacer = new QToolBar(this);
		addToolBar(toolbar_spacer);
		toolbar_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		QWidget* widget_spacer = new QWidget(this);
		widget_spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		toolbar_spacer->addWidget(widget_spacer);
		QHBoxLayout* layout_spacer = new QHBoxLayout(widget_spacer);
		widget_spacer->setLayout(layout_spacer);
		layout_spacer->addSpacerItem(new QSpacerItem(m_ui_max_size, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

		//添加西光所触发按钮工具栏
		m_trigger_toolbar = new QToolBar(this);
		m_trigger_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
		m_trigger_toolbar->addAction(ui->actionTrigger_T);
		addToolBar(Qt::TopToolBarArea, m_trigger_toolbar);
		//去除原来的触发按钮显示
		ui->cameraAcqToolBar->removeAction(ui->actionTrigger_T);
	}

	ui->mainToolBar->setIconSize(QSize(36, 36));
	ui->cameraAcqToolBar->setIconSize(QSize(36, 36));
	ui->acqWindowToolBar->setIconSize(QSize(36, 36));
	ui->runAlarmToolBar->setIconSize(QSize(36, 36));
	ui->userManualToolBar->setIconSize(QSize(36, 36));
	if (m_trigger_toolbar)
	{
		m_trigger_toolbar->setIconSize(QSize(36,36));
	}

	ui->action_W->setVisible(false);
	ui->actionBright_Field_Correction->setVisible(false);

	{
		ui->menuShooting_Mode->menuAction()->setVisible(false);
		ui->actionChroma_Adjustment->setVisible(false);
		ui->actionData_Processing_Analysis->setVisible(false);
		ui->actionData_Storage_Management->setVisible(false);

		ui->actionUAV_Calibration->setVisible(false);
		ui->actionStitching_Preview->setVisible(false);

		ui->actionRemote_Switch->setVisible(false);
		ui->actionDelay_Correction->setVisible(false);
		ui->actionDrop_Measurement_Settings->setVisible(false);
		ui->action_C->setVisible(false);
	}	
}
#include <qthread.h>
void CSRccApp::InitVideoWidget(int index)
{
	auto video = new CPlayerViewBase(index, 1, ui->widget);
	bool ok = connect(video, &CPlayerViewBase::sigCurrentCursorInVideo, this, &CSRccApp::SlotCurrentCursorInVideo);
	ok = connect(video, &CPlayerViewBase::sigZoomCoefficientChanged, this, &CSRccApp::SlotZoomCoefficientChanged/*, Qt::QueuedConnection*/);
	ok = connect(video, &CPlayerViewBase::SignalUpdateGrayOrRGBValue, this, &CSRccApp::SlotUpdateGrayOrRGBValue);
	ok = connect(video, &CPlayerViewBase::SignalUpdateCustomCrossLineCenterPoint, this, &CSRccApp::SlotUpdateCustomCrossLineCenterPoint);
	ok = connect(video, &CPlayerViewBase::SignalCustomCrossLineCenterMovePoint, this, &CSRccApp::SlotCustomCrossLineCenterMovePoint, Qt::DirectConnection);
	ok = connect(video, &CPlayerViewBase::updateRoiInfoSignal, this, &CSRccApp::updateRoiInfoSlot, Qt::DirectConnection);// 同线程信号槽不要乱用异步绑定
	ok = connect(GetViewManagerPtr(), &CSViewManager::signalSplitWidget, video, &CPlayerViewBase::SlotDisplayWidgetAdjust);
	video->hide();
	GetViewManagerPtr()->AddView(video);
	int row = index / 4;
	int col = index % 4;
	m_mainLayout->addWidget(video, row, col);
}

void CSRccApp::ConnectSignalSlot()
{
	bool ok = connect(GetViewManagerPtr(), &CSViewManager::signalSplitWidget, this, &CSRccApp::SlotDisplayWidgetAdjust);
	ok = connect(&DeviceManager::instance(), &DeviceManager::deviceConnected, this, &CSRccApp::SlotDeviceConnected, Qt::DirectConnection);//获取设备
	ok = connect(&DeviceManager::instance(), &DeviceManager::deviceDisconnected, this, &CSRccApp::SlotDeviceDisconnected);//断开设备
	ok = connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, this, &CSRccApp::SlotCurrentDeviceChanged);//当前设备切换
	ok = connect(GetViewManagerPtr(), &CSViewManager::SignalDisplayWidgetChanged, this, &CSRccApp::SlotDisplayWidgetClicked);
	ok = connect(GetViewManagerPtr(), &CSViewManager::signalStopDevices, this, &CSRccApp::slotStopDevices);

	ok = connect(m_actionGroup_WhiteBalance, &QActionGroup::triggered, this, &CSRccApp::SlotWhiteBalanceActionGroupTriggered);//白平衡操作
	ok = connect(m_actionGroup_ColorMode, &QActionGroup::triggered, this, &CSRccApp::SlotColorModeActionGroupTriggered);//色彩模式操作
	ok = connect(m_actionGroup_WbEnv, &QActionGroup::triggered, this, &CSRccApp::SlotWbEnvActionGroupTriggered);//拍摄环境操作
	ok = connect(m_actionGroup_ImageCtrl, &QActionGroup::triggered, this, &CSRccApp::SlotImageCtrlActionGroupTriggered);
	ok = connect(m_actionGroup_FanControl, &QActionGroup::triggered, this, &CSRccApp::SlotFanControlActionGroupTriggered);//风扇控制操作
	ok = connect(m_actionGroup_AutoExposure, &QActionGroup::triggered, this, &CSRccApp::SlotAutoExposureActionGroupTriggered);//自动曝光操作
	ok = connect(m_actionGroup_IntelligentTrigger, &QActionGroup::triggered, this, &CSRccApp::SlotIntelligentTriggerActionGroupTriggered);//智能触发操作
	ok = connect(m_actionGroup_EdrExposure, &QActionGroup::triggered, this, &CSRccApp::SlotEdrExposureTriggerActionGroupTriggered);//二次曝光操作
	ok = connect(m_actionGroup_Translate, &QActionGroup::triggered, this, &CSRccApp::SlotTranslateTriggerActionGroupTriggered);//翻译操作
	ok = connect(m_winCrossLineSet, &CSWindowCrossLineSetting::SignalGetPictureCenterPoint, this, &CSRccApp::SlotGetPictureCenterPoint);//获取图像中心点坐标
	ok = connect(this, &CSRccApp::SignalLocalVideoDispaly, this, &CSRccApp::SlotLocalVideoDispaly, Qt::QueuedConnection);
	ok = connect(ui->dockWidgetDevicelist, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionDevice_List->setChecked(false);
	});
	ok = connect(ui->dockWidgetPropertyList, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionProperty_List->setChecked(false);
	});
	ok = connect(ui->dockWidgetVideoList, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionVideo_List->setChecked(false);
	});
	ok = connect(ui->dockWidgetPointMeasure, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionMeasure->setChecked(false);
	});
	ok = connect(ui->dockWidgetTemperature, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionTemperature_Panel->setChecked(false);
		auto device_ptr = DeviceManager::instance().getCurrentDevice();
		if (device_ptr)
		{
			device_ptr->setProperty(Device::PropTemperaturePanelEnable, false);
		}
	});
	ok = connect(ui->cf18_control_dockWidget, &CSDockWidget::SignalDocWidgetClose, this, [=]() {
		ui->actionControl_Panel->setChecked(false);
		auto device_ptr = DeviceManager::instance().getCurrentDevice();
		if (device_ptr)
		{
			device_ptr->setProperty(Device::PropCF18ControlPanelEnable, false);
		}
	});
	ok = connect(m_device_list_widget, &CSDeviceListWidget::signalUpdateDeviceRotate, this, &CSRccApp::SlotUpdateDeviceRotate);
	ok = connect(m_device_list_widget, &CSDeviceListWidget::signalRemoveCurrentDevice, this, &CSRccApp::slotRemoveCurrentDevice);
	ok = connect(m_device_list_widget, &CSDeviceListWidget::signalItemClicked, m_device_search_list_widget, &CSDeviceSearchListWidget::slotItemClicked);
	ok = connect(m_device_search_list_widget, &CSDeviceSearchListWidget::signalUpdateDeviceRotate, this, &CSRccApp::SlotUpdateDeviceRotate);
	ok = connect(m_device_search_list_widget, &CSDeviceSearchListWidget::signalSearchDeviceFinished, m_device_list_widget, &CSDeviceListWidget::slotSearchDeviceFinished);
	ok = connect(m_device_search_list_widget, &CSDeviceSearchListWidget::signalItemClicked, m_device_list_widget, &CSDeviceListWidget::slotCurrentItemClicked);
	ok = connect(ui->actionAcquisition_Window_Settings, &QAction::triggered, this, [=] {
		if (!ui->actionAcquisition_Window_Settings->isEnabled()) {
			CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
			if (pVideoPlayer) {
				pVideoPlayer->setRoiVisible(Device::RoiTypes::kDeviceRoi, false);
				pVideoPlayer->setFeaturePointVisible(Device::RoiTypes::kDeviceRoi, false);
				pVideoPlayer->setRoiVisible(Device::kAutoExposureRoi, false);
				pVideoPlayer->setRoiVisible(Device::kIntelligentTriggerRoi, false);
			}
		}
	});	//帧率默认为0
	SetFrameRate(0);
	ok = connect(ui->action_low_light_mode, &QAction::triggered, this, &CSRccApp::slotLowLightModeTrigger);
	connect(m_list_view_video_widget, &CSListViewVideo::signalExportpreviewShowMain, this, &CSRccApp::slotExportpreviewShowMain);
}

void CSRccApp::updateMainWindowUI()
{
	//更新全部菜单栏中的操作使能和选中,工具栏操作自动同步显示
	updateFileActionsUI();//项目菜单(文件)

	updateAcquisitionActionsUI();//采集

	updateSettingActionsUI();//设置

	updateImageActionsUI();//图像

	updatToolsActionsUI();//工具

	updatViewsActionsUI();//视图

	updatHelpsActionsUI();//帮助

	UpdatAlarmActionsUI();//运行报警

	updateOtherUI();//其他
}

void CSRccApp::updateFileActionsUI()
{
	//有相机高采时禁用
	ui->actionOpen_File_O->setEnabled(!DeviceManager::instance().isAnyCameraHighMode());
}

void CSRccApp::updateImageActionsUI()
{
	//根据设备状态切换图像相关操作使能
	updateColorModeActionsUI();//色彩模式

	updateWhiteBalanceActionsUI();//白平衡
	
	updateWbEnvActionsUI();//拍摄环境

	//TODO:暂时禁用没有实现的选项
	ui->action_W->setEnabled(false);
	ui->actionBright_Field_Correction->setEnabled(false);
	ui->actionDay_Mode->setEnabled(false);
	ui->actionNight_Mode->setEnabled(false);
	ui->actionChroma_Adjustment->setEnabled(false);
	ui->actionData_Processing_Analysis->setEnabled(false);
	ui->actionData_Storage_Management->setEnabled(false);

	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	//没有设备则禁用相关操作
	if (device_ptr.isNull()||
		device_ptr->getState() == Unconnected||
		device_ptr->getState() == DeviceState::Disconnected ||
		device_ptr->getState() == DeviceState::StandBy ||
		device_ptr->getState() == DeviceState::ToWakeup ||
		device_ptr->IsTrigger())
	{
		updateLowLightActionStatus(false);
		ui->actionImage_Training_E->setEnabled(false);
		ui->actionSet_Up_Darkfield->setEnabled(false);
		ui->action_DarkFieldCorrection->setEnabled(false);
		ui->actionGain_Setting->setEnabled(false);
		ui->actionBrightness_Contrast->setEnabled(false);
		ui->actionBinning_Mode->setVisible(false);
		return;
	}

	//预览并且自动曝光关闭时低照度模式可开启
 	if (device_ptr && (DeviceState::Previewing == device_ptr->getState()) && !m_bAutoExposureOpened) {
		updateLowLightActionStatus(true);
	}
	else {
		updateLowLightActionStatus(false);
	}

	//像素融合
	ui->actionBinning_Mode->setVisible(device_ptr->IsBinningModeSupported());
	if (device_ptr->IsBinningModeSupported())
	{
		HscDisplayMode displayMode = device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>();
		if (displayMode != HSC_DISPLAY_MONO)
		{
			ui->actionBinning_Mode->blockSignals(true);
			ui->actionBinning_Mode->setChecked(false);
			ui->actionBinning_Mode->setEnabled(false);
			ui->actionBinning_Mode->blockSignals(false);
		}
		else
		{
			ui->actionBinning_Mode->setEnabled(device_ptr->AllowsSetBinningMode());
			bool binning_enable = false;
			device_ptr->GetBinningMode(binning_enable);
			ui->actionBinning_Mode->setChecked(binning_enable);
		}
	}

	//图像训练
	ui->actionImage_Training_E->setEnabled(device_ptr->allowsImageTraining());

	//暗场矫正使能
	if (device_ptr->AllowsEnableBlackField())
	{
		ui->action_DarkFieldCorrection->setEnabled(true);
		if (device_ptr->getProperty(Device::PropBlackFieldEnable).toBool())
		{
			ui->action_DarkFieldCorrection->setChecked(true);
		}
		else
		{
			ui->action_DarkFieldCorrection->setChecked(false);
		}
	}
	else
	{
		ui->action_DarkFieldCorrection->setEnabled(false);
	}

	//暗场矫正
	ui->actionSet_Up_Darkfield->setEnabled(device_ptr->AllowsBlackFieldCalibration());

	//增益设置
	if (FunctionCustomizer::GetInstance().isUsabilityVersion())
	{
		ui->actionGain_Setting->setEnabled(device_ptr->AllowsSetGain());
	}
	else
	{
		ui->actionGain_Setting->setEnabled(false);
	}

	//亮度对比度
	if (FunctionCustomizer::GetInstance().isH150Enabled())
	{
		ui->actionBrightness_Contrast->setEnabled(true);
	}
	else
	{	
		ui->actionBrightness_Contrast->setEnabled(device_ptr->AllowsEditLuminanceAndContrast());
	}
	
}

void CSRccApp::updateWhiteBalanceActionsUI()
{
	//没有设备则全禁用,
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull()|| 
		device_ptr->getState() == Unconnected||
		device_ptr->getState() == DeviceState::Disconnected ||
		device_ptr->getState() == DeviceState::StandBy ||
		device_ptr->getState() == DeviceState::ToWakeup ||
		device_ptr->IsTrigger())
	{
		ui->actionNone->setEnabled(false);
		ui->actionAuto->setEnabled(false);
		ui->actionManual->setEnabled(false);
		ui->actionSettingManual->setEnabled(false);
		return;
	}
	if (device_ptr->isSupportManualWhiteBalanceMode())
	{
		if (device_ptr->getState() != Previewing)
		{
			ui->actionNone->setEnabled(false);
			ui->actionAuto->setEnabled(false);
			ui->actionManual->setEnabled(false);
			ui->actionSettingManual->setEnabled(false);
			return;
		}
		HscDisplayMode displayMode = device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>();
		if (displayMode != HSC_DISPLAY_COLOR)
		{
			ui->actionNone->setEnabled(false);
			ui->actionAuto->setEnabled(false);
			ui->actionManual->setEnabled(false);
			ui->actionSettingManual->setEnabled(false);
			return;
		}
		HscHisiManualAwb param;
		param.mode = AWB_MODE_NONE;
		HscResult res = device_ptr->GetManualWhiteBalance(param);
		if (res == HSC_OK)
		{
			ui->actionNone->setEnabled(true);
			if (device_ptr->isSupportManualWhiteBalanceAutoMode()) {
				ui->actionAuto->setEnabled(true);
			}
			else {
				ui->actionAuto->setEnabled(false);
			}
			ui->actionManual->setEnabled(true);
			ui->actionSettingManual->setEnabled(false);
			switch (param.mode)
			{
			case AWB_MODE_NONE:
				ui->actionNone->setChecked(true);
				break;
			case AWB_MODE_AUTO:
				ui->actionAuto->setChecked(true);
				break;
			case AWB_MODE_MANUAL:
			case AWB_UPDATE_MANUAL_PARAM:
				ui->actionManual->setChecked(true);
				ui->actionSettingManual->setEnabled(true);
				break;
			default:
				break;
			}
		}
		else
		{
			ui->actionNone->setEnabled(false);
			ui->actionAuto->setEnabled(false);
			ui->actionManual->setEnabled(false);
		}
		return;
	}
	ui->actionSettingManual->setEnabled(false);
	
	//判断使能
	if (device_ptr->allowsEditArmWbMode())//arm白平衡模式
	{
		ui->actionNone->setEnabled(false);
		ui->actionAuto->setEnabled(true);
		ui->actionManual->setEnabled(true);
	} 
	else if (device_ptr->allowsEditWbMode())//普通白平衡
	{
		ui->actionNone->setEnabled(true);
		ui->actionAuto->setEnabled(true);
		ui->actionManual->setEnabled(true);
	}
	else
	{
		ui->actionNone->setEnabled(false);
		ui->actionAuto->setEnabled(false);
		ui->actionManual->setEnabled(false);
		return;
	}

	//判断启用
	HscWhiteBalanceMode wb_mode = device_ptr->GetWbMode();
	switch (wb_mode)
	{
	case HSC_WB_NONE:
		ui->actionNone->setChecked(true);
		break;
	case HSC_WB_MANUAL_GAIN:
		ui->actionManual->setChecked(true);
		break;
	case HSC_WB_AUTO_GAIN_FROM_SOFTWARE:
		ui->actionAuto->setChecked(true);
		break;
	default:
		break;
	}

}

void CSRccApp::updateColorModeActionsUI()
{
	//没有设备则全禁用,
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull() || 
		device_ptr->getState() == Unconnected||
		device_ptr->getState() == DeviceState::Disconnected ||
		device_ptr->getState() == DeviceState::StandBy ||
		device_ptr->getState() == DeviceState::ToWakeup ||
		device_ptr->IsTrigger())
	{
		ui->actionMonochrome->setEnabled(false);
		ui->actionColor->setEnabled(false);
		ui->actionMonochrome->setChecked(true);
		return;
	}

	//判断使能
	if (device_ptr->AllowsEditDisplayMode())
	{
		ui->actionMonochrome->setEnabled(true);
		ui->actionColor->setEnabled(true);
	}
	else
	{
		//色彩模式禁止编辑时，状态栏根据真实的色彩模式显示RGB或灰度值
		m_bColor = device_ptr->isColorSupported() ? true : false;
		if (m_bColor)
		{
			ui->actionColor->setChecked(true);
		}
		else
		{
			ui->actionMonochrome->setChecked(true);
		}

		ui->actionMonochrome->setEnabled(false);
		ui->actionColor->setEnabled(false);
		return;
	}

	//判断启用
	HscDisplayMode display_mode = device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>();

	switch (display_mode)
	{
	case HSC_DISPLAY_MONO:
		ui->actionMonochrome->setChecked(true);
		m_bColor = false;
		break;
	case HSC_DISPLAY_COLOR:
		ui->actionColor->setChecked(true);
		m_bColor = true;
		break;
	default:
		break;
	}

	//自动修正
	QList<HscDisplayMode> display_mode_list;
	device_ptr->GetSupportedDisplayModes(device_ptr->getProperty(Device::PropStreamType).value<StreamType>(), display_mode_list);
	if (!display_mode_list.contains(display_mode))
	{
		device_ptr->setProperty(Device::PropDisplayMode, display_mode_list.first());
	}
	
}

void CSRccApp::updateWbEnvActionsUI()
{
	//没有设备则全禁用,
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull() ||
		device_ptr->getState() == Unconnected ||
		device_ptr->getState() == DeviceState::Disconnected ||
		device_ptr->getState() == DeviceState::StandBy ||
		device_ptr->getState() == DeviceState::ToWakeup ||
		device_ptr->IsTrigger())
	{
		ui->actionEnvironmentNone->setEnabled(false);
		ui->actionEnvironmentAuto->setEnabled(false);
		ui->actionSunny->setEnabled(false);
		ui->actionSunset->setEnabled(false);
		ui->actionCloudy_Day->setEnabled(false);
		ui->actionFill_Light->setEnabled(false);
		return;
	}
	//判断使能
	bool enable = false;
	do 
	{
		if (!device_ptr->isColorSupported() || device_ptr->getState() != Previewing)
		{
			enable = false;
			break;
		}
		if (device_ptr->getProperty(Device::PropStreamType).value<StreamType>() != TYPE_RAW8 && device_ptr->getProperty(Device::PropStreamType).value<StreamType>() != TYPE_RGB8888)
		{
			enable = false;
		}
		else
		{
			enable = (device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>() == HSC_DISPLAY_COLOR);
		}

	} while (0);

	ui->actionEnvironmentNone->setEnabled(enable);
	ui->actionEnvironmentAuto->setEnabled(enable);
	ui->actionSunny->setEnabled(enable);
	ui->actionSunset->setEnabled(enable);
	ui->actionCloudy_Day->setEnabled(enable);
	ui->actionFill_Light->setEnabled(enable);

	//判断选中
	HScWbEnv env_mode= device_ptr->getProperty(Device::PropWbEnv).value<HScWbEnv>();
	switch (env_mode)
	{
	case HSC_WB_ENV_NONE:
		ui->actionEnvironmentNone->setChecked(true);
		break;
	case HSC_WB_ENV_AUTO_FROM_ISP:
		ui->actionEnvironmentAuto->setChecked(true);
		break;
	case HSC_WB_ENV_SUNNY:
		ui->actionSunny->setChecked(true);
		break;
	case HSC_WB_ENV_CLOUDY:
		ui->actionCloudy_Day->setChecked(true);
		break;
	case HSC_WB_ENV_LIGHT:
		ui->actionFill_Light->setChecked(true);
		break;
	case HSC_WB_ENV_SUNSET:
		ui->actionSunset->setChecked(true);
		break;
	default:
		break;
	}

}

void CSRccApp::updatToolsActionsUI()
{
	//TODO:暂时禁用未实现选项
	ui->actionStitching_Preview->setEnabled(false);//拼接预览
	ui->actionUAV_Calibration->setEnabled(false);//无人机标定

	updateImageCtrlUI();//图像工具控制

	//没有设备则全禁用,
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull() || 
		device_ptr->getState() == Unconnected ||
		device_ptr->getState() == DeviceState::Disconnected ||
		device_ptr->getState() == DeviceState::StandBy ||
		device_ptr->getState() == DeviceState::ToWakeup ||
		device_ptr->IsTrigger())
	{
		ui->actionEnlarge->setEnabled(false);
		ui->actionZoom_Out_Z->setEnabled(false);
		ui->action1_1Show->setEnabled(false);
		ui->actionFit_Window->setEnabled(false);
		ui->actionCrossHair->setEnabled(false);
		ui->actionWindow_Centerline->setEnabled(false);
		ui->actionWindow_Centerline_Settings->setEnabled(false);
		ui->actionRotate_The_Image_R->setEnabled(false);
		return;
	}

	//判断使能
	bool enable = false;
	DeviceState cur_state = device_ptr->getState();
	if (cur_state == Previewing || cur_state == Acquiring || cur_state == Recording)
	{
		enable = true;
	}
	ui->actionEnlarge->setEnabled(enable);
	ui->actionZoom_Out_Z->setEnabled(enable);
	ui->action1_1Show->setEnabled(enable);
	ui->actionFit_Window->setEnabled(enable);
	ui->actionCrossHair->setEnabled(enable);
	ui->actionWindow_Centerline->setEnabled(enable);
	ui->actionWindow_Centerline_Settings->setEnabled(enable);
	ui->actionRotate_The_Image_R->setEnabled(enable);


	//TODO: 判断选中
	if (enable)
	{
		ui->actionWindow_Centerline->setChecked(SystemSettingsManager::instance().isWindow_Centerline());
	}
}

void CSRccApp::updateOtherUI()
{
	//没有设备则置灰和设备相关的操作
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	if (nullptr == pDevice ||
		pDevice->IsTrigger())
	{
		ui->actionAcquisition_Window_Settings->setEnabled(false);	//设置roi
		ui->actionAcquisition_Window_Settings->setChecked(false);
		ui->actionMaximize_The_Acquisition_Window->setEnabled(false);//最大roi
		ui->actionCenter_The_Acquisition_Window->setEnabled(false);//roi居中
		ui->actionDisplay_The_Acquisition_Window->setEnabled(false);//显示roi
		ui->actionDisplay_The_Acquisition_Window->setChecked(false);

		return;
	}

	//使能控制
	bool enable = (pDevice->getState() == DeviceState::Previewing);//只允许在预览时使用
	if (!pDevice->isGrabberDevice()) {
		ui->actionAcquisition_Window_Settings->setEnabled(enable);
		ui->actionMaximize_The_Acquisition_Window->setEnabled(enable);
		ui->actionCenter_The_Acquisition_Window->setEnabled(enable);
		ui->actionDisplay_The_Acquisition_Window->setEnabled(enable);
	}
	

	//选中控制
	if (ui->actionAcquisition_Window_Settings->isChecked())
	{
		ui->actionAcquisition_Window_Settings->setChecked(false);
	}

	ui->actionDisplay_The_Acquisition_Window->setChecked(pDevice->getProperty(Device::PropRoiVisible).toBool());

}

void CSRccApp::updatViewsActionsUI()
{
	//判断是否需要显示温控列表
	UpdateTemperaturePanelDisplay(DeviceManager::instance().getCurrentDevice());

	//判断是否显示CF18控制列表
	UpdateCF18ControlPanelDisplay();

	//停靠窗口状态更新
	ui->actionDevice_List->setChecked(ui->dockWidgetDevicelist->isVisible());
	ui->actionProperty_List->setChecked(ui->dockWidgetPropertyList->isVisible());
	ui->actionVideo_List->setChecked(ui->dockWidgetVideoList->isVisible());
	ui->actionMeasure->setChecked(ui->dockWidgetPointMeasure->isVisible());

	//工具栏窗口状态更新
	ui->actionWindow_Tool->setChecked(ui->mainToolBar->isVisible());
	ui->actionCamera_Acquisition->setChecked(ui->cameraAcqToolBar->isVisible());
	ui->actionAcquisition_Window->setChecked(ui->acqWindowToolBar->isVisible());
	ui->actionRun_Alarm->setChecked(ui->runAlarmToolBar->isVisible());
	ui->actionUser_Manual->setChecked(ui->userManualToolBar->isVisible());

	//状态栏显示状态更新
	ui->actionStatus_Bar_S->setChecked(ui->statusBar->isVisible());


}

void CSRccApp::updatHelpsActionsUI()
{
	//恢复出厂设置
	auto device = DeviceManager::instance().getCurrentDevice();
	bool bResetOpen = false;
	if (device && device->IsFactoryResetSupported())
	{
		bResetOpen = true;
	}
	ui->actionReset->setEnabled(bResetOpen);

	//网络配置
	bool bNetconfigOpen = false;
	if (device && device->IsNetConfigSupported())
	{
		bNetconfigOpen = true;
	}
	ui->actionNetwork_Configuration->setEnabled(bNetconfigOpen);
	
	//未连接设备-跨网段设备
	m_is_gateway_device = false;
	if (device && (Unconnected == device->getState()) && device->isGatewayDevice()) {
		m_is_gateway_device = true;
		ui->actionNetwork_Configuration->setEnabled(device->isGatewayDevice());
	}
	//设备授权
	bool bAllowAuth = false;
	if (device && device->IsAuthEnabled())
	{
		bAllowAuth = true;
	}
	ui->actionDevice_Authorization->setEnabled(bAllowAuth);

	//在线升级
	bool bOnlineUpdate = false;
#ifdef Q_OS_WIN32
	if (device && device->IsOnlineUpdate())
	{
		bOnlineUpdate = true;
	}
#endif // Q_OS_WIN32
	ui->actionOnline_Upgrade->setEnabled(bOnlineUpdate);
}

void CSRccApp::updateAcquisitionActionsUI()
{

	//触发按钮
	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		bool enable_trigger = false;
		if (m_current_device_ptr)
		{
			if (m_current_device_ptr->allowsTrigger())
			{
				if (m_current_device_ptr->getProperty(Device::PropTriggerMode) != TRIGGER_EXTERNAL)
				{
					enable_trigger = true;
				}
			}
			else if(m_current_device_ptr->getState() == Recording)//正在录制时也可以点击停止录制
			{
				enable_trigger = true;
			}
		}
		ui->actionTrigger_T->setEnabled(enable_trigger);
		QIcon icon;
		QString text;

		if (m_current_device_ptr && m_current_device_ptr->getState() == Recording)//使用西光所定制触发图像
		{
			icon.addFile(QStringLiteral(":/image/image/xgs_stop_recording.png"));
			text = tr("Stop Recording");
		}
		else
		{
			icon.addFile(QStringLiteral(":/image/image/xgs_trigger.png"));
			text = tr("Trigger");
		}

		ui->actionTrigger_T->setIcon(icon);
		ui->actionTrigger_T->setText(text);
		ui->actionTrigger_T->setToolTip(text);
	}
	else
	{
		ui->actionTrigger_T->setEnabled(DeviceManager::instance().AllowSoftTriggerAllDevice());
	}

	//抓拍动作
	ui->actionTake_photo->setEnabled(GetViewManagerPtr()->IsSnapEnabled());

	//没有设备则全部禁用
	if (m_current_device_ptr.isNull()||
		m_current_device_ptr->getState() == Unconnected ||
		m_current_device_ptr->getState() == DeviceState::Disconnected ||
		m_current_device_ptr->getState() == DeviceState::StandBy ||
		m_current_device_ptr->IsTrigger())
	{
		ui->actionPreview_P->setEnabled(false);
		ui->actionHigh_speed_Acquisition->setEnabled(false);
		ui->actionStop_Camera_S->setEnabled(false);
		return;
	}

	//根据当前设备状态切换采集界面使能
	ui->actionPreview_P->setEnabled(m_current_device_ptr->allowsPreview());
	ui->actionHigh_speed_Acquisition->setEnabled(m_current_device_ptr->allowsAcquire());
	ui->actionStop_Camera_S->setEnabled(m_current_device_ptr->allowsStop());


}

void CSRccApp::updateSettingActionsUI()
{
	//设置菜单栏选项控制

	//TODO:暂时禁用没有实现的选项
	ui->actionRemote_Switch->setEnabled(false);//远程开关机
	ui->actionDelay_Correction->setEnabled(false);//延时校正
	ui->action_C->setEnabled(false);//转台控制



	ui->actionDrop_Measurement_Settings->setEnabled(FunctionCustomizer::GetInstance().isTargetScoringSupported());//落点测量设置
	ui->actionMachine_Self_check->setEnabled(DeviceManager::instance().hasDevices());//整机自检
	ui->actionHealth_Management_Settings->setVisible(FunctionCustomizer::GetInstance().isXiguangsuoVersion());//健康管理设置

	//没有设备则置灰和设备相关的操作
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	if (nullptr == pDevice || pDevice->IsTrigger())
	{
		ui->menuIntelligent_Trigger->setEnabled(false);	//智能触发使能
		ui->menuAuto_Exposure->setEnabled(false);//自动曝光
		ui->menuEdr_Exposure->setEnabled(false);//二次曝光
		ui->menuFan_Control_F->setEnabled(false);//风扇控制
		return;
	}

	//使能控制
	ui->menuIntelligent_Trigger->setEnabled(pDevice->AllowsSetIntelligentTrigger());
	ui->menuAuto_Exposure->setEnabled(pDevice->AllowSetAutoExposure());
	ui->menuEdr_Exposure->setEnabled(pDevice->AllowSetEdrExposure());
	ui->menuFan_Control_F->setEnabled(pDevice->AllowEditFanCtrl());
	ui->action_O->setEnabled(!DeviceManager::instance().isAnyCameraHighMode());//程序运行选项

	if (m_ExType == Device::kAutoExposureRoi)
	{
		emit SignalForbid(pDevice->AllowSetAutoExposure());
	}
	else if (m_ExType == Device::kIntelligentTriggerRoi)
	{
		emit SignalForbid(pDevice->AllowsSetIntelligentTrigger());
	}
	emit SignalEdrForbid(pDevice->AllowSetEdrExposure());
	//选中控制
	//风扇控制
	if (pDevice->isGrabberDevice()) {	//GR系列没有自动风扇，置灰
		ui->actionFan_ControlAuto->setEnabled(false);
	}
	else {
		ui->actionFan_ControlAuto->setEnabled(true);
	}
	uint8_t fan_state = 0;
	pDevice->GetFanState(fan_state);
	if (fan_state == 1)
	{
		ui->actionFan_ControlOpen->setChecked(true);
	}
	else if (fan_state == 0)
	{
		ui->actionFan_ControlClose->setChecked(true);
	}
	else if (fan_state == 101)
	{
		ui->actionFan_ControlAuto->setChecked(true);
	}
	


	//非调试模式下隐藏滤波按钮
	if (!FunctionCustomizer::GetInstance().isConsoleEnabled())
	{
		ui->actionFiler1->setVisible(false);
		ui->actionFilter->setVisible(false);
	}

}

void CSRccApp::UpdatAlarmActionsUI()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (pVideoPlayer && pVideoPlayer->GetTrackRoiEnabled())
	{
		ui->actionRun_Alarm_W->setEnabled(false);
	}

	auto device = DeviceManager::instance().getCurrentDevice();
	if (device && device->GetVecWarnings().size() > 0)
	{
		ui->actionRun_Alarm_W->setEnabled(true);
	}
	else
	{
		ui->actionRun_Alarm_W->setEnabled(false);
	}
}

void CSRccApp::UpdateTemperaturePanelDisplay(QSharedPointer<Device> device_ptr)
{
	bool enable = false;
	bool check = false;
	if (device_ptr )
	{
		if (device_ptr->IsTemperaturePanelSupported())
		{
			enable = true;

			if (device_ptr->getProperty(Device::PropTemperaturePanelEnable).toBool())
			{
				check = true;
			}
		}
	}
	ui->actionTemperature_Panel->setEnabled(enable);

	ui->actionTemperature_Panel->setChecked(check);
	ui->dockWidgetTemperature->setVisible(check);

}

void CSRccApp::UpdateCF18ControlPanelDisplay()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr)
	{
		ui->cf18_control_dockWidget->setVisible(device_ptr->IsCF18());
	}
	else
	{
		ui->cf18_control_dockWidget->setVisible(false);
	}

	bool enable = false;
	bool check = false;
	if (device_ptr)
	{
		if (device_ptr->IsCF18())
		{
			enable = true;

			if (device_ptr->getProperty(Device::PropCF18ControlPanelEnable).toBool())
			{
				check = true;
			}
		}
	}
	ui->actionControl_Panel->setEnabled(enable);

	ui->actionControl_Panel->setChecked(check);
	ui->cf18_control_dockWidget->setVisible(check);

}

void CSRccApp::SetXStatusValue(int value)
{
	QString xText = "X: " + QString::number(value);
	m_pXLabel->setText(xText);
}

void CSRccApp::SetYStatusValue(int value)
{
	QString yText = "Y: " + QString::number(value);
	m_pYLabel->setText(yText);
}

void CSRccApp::RefreshQLabelStyle(QLabel* label, const QString& name)
{
	style()->unpolish(label);
	label->setObjectName(name);
	style()->polish(label);
}

void CSRccApp::SetZoomCoefficient(const float coef)
{
	if (nullptr != m_pZoomCoefficientLabel)
	{
		m_fZoomCoefficient = coef;
		m_pZoomCoefficientLabel->setText(tr("ratio:") + QString::number(coef,'f',2));
	}
}

void CSRccApp::SetFrameRate(const int rate)
{
	m_iFrameRate = rate;
	QString text = tr("Frame Rate:") + QString::number(rate);
	m_pFrameRateLabel->setText(text);
}

void CSRccApp::SetStatusBarRotateValue(int angle)
{
	m_iBarRotate = angle;
	m_pRotateLabel->setText(tr("rotate:") + QString::number(angle * 90));
}

void CSRccApp::SetBcode(const bool isOn)
{
	m_BcodeOn = isOn;
	QString test;
	if (isOn) test = tr("Connect");
	else test = tr("Disconnect");
	QString text = tr("B Code:") + test; 
	m_pBcode->setText(text);
}

void CSRccApp::SetAvgLum(const uint32_t avgLum)
{
	m_uAvgLum = avgLum;
	m_pAvgLumLabel->setText(tr("Average Luminance: ") + QString::number(avgLum));
}

void CSRccApp::SetGrayOrRGB(bool bshow)
{
	m_bGrayOrRGB = bshow;
	if (bshow)
	{
		QString str;
		if (m_bColor)
		{
			str = QString("RGB:(%1,%2,%3)").arg(bgr_values[2]).arg(bgr_values[1]).arg(bgr_values[0]);
		}
		else
		{
			str = tr("Gray:") + QString::number(m_nGrayOrRGBAtQueryPoint);
		}

		m_pGrayOrRGBLabel->setText(str);
	}
	else
	{
		m_pGrayOrRGBLabel->setText("");
	}
}

void CSRccApp::UpdateAppTitle(QString exp_name, QString exp_code)
{
	m_project_info = QString("%1[%2]").arg(exp_name).arg(exp_code);
	QString str = m_project_info + UIExplorer::instance().getProductName();
	setWindowTitle(str);
}

void CSRccApp::UpdateDeviceRoiStatus(QString device_ip)
{
	auto pDevice = DeviceManager::instance().getDevice(device_ip);
	QString device_name;
	if (pDevice)
	{
		device_name = pDevice->getIpOrSn();
	}
	auto pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_name);
	if (nullptr != pVideoPlayer && nullptr != pDevice)
	{
		{
			RoiInfo roi_info;
			roi_info.roi_type = Device::RoiTypes::kDeviceRoi;
			roi_info.roi_rect = GetRoiTypeRect(Device::kDeviceRoi, pDevice);
			pVideoPlayer->drawRoiRect(roi_info);
		}
		if (pDevice->getProperty(Device::PropRoiVisible).toBool())
		{
			on_actionDisplay_The_Acquisition_Window_triggered(true);
		}
	}
}

void CSRccApp::UpdateAutoExposureRoiStatus(QString device_ip)
{
	auto pDevice = DeviceManager::instance().getDevice(device_ip);
	QString device_name;
	if (pDevice)
	{
		device_name = pDevice->getIpOrSn();
	}
	auto pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_name);
	if (nullptr != pVideoPlayer && nullptr != pDevice)
	{
		if (pDevice->IsAutoExposureSupported())
		{
			HscAutoExposureParameter param = pDevice->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			m_bAutoExposureOpened = param.bEnable;
			RoiInfo roi_info;
			roi_info.roi_type = Device::RoiTypes::kAutoExposureRoi;
			roi_info.roi_rect = GetRoiTypeRect(Device::kAutoExposureRoi, pDevice);
			roi_info.roi_color = param.roi_color;
			pVideoPlayer->drawRoiRect(roi_info);
			pDevice->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			if (param.bEnable)
			{
				ui->AutoExposureOpen->setChecked(true);
			}
			else
			{
				ui->AutoExposureClose->setChecked(true);
			}
		}
		else
		{
			pVideoPlayer->setRoiVisible(Device::kAutoExposureRoi, false);
		}
	}
}


QRect CSRccApp::GetRoiTypeRect(Device::RoiTypes type, QSharedPointer<Device> pDevice)
{
	QRect rect;
	if (pDevice == nullptr)
	{
		return rect;
	}
	switch (type)
	{
	case Device::kUnknownRoi:
		break;
	case Device::kDeviceRoi:
	{
		DeviceState status = pDevice->getState();
		bool b_change_preview = pDevice->IsChangePreviewResoultionSupport();
		rect = pDevice->GetRoi(Device::RoiTypes::kDeviceRoi);
		if (status == ToAcquire || status == Acquiring || status == Recording || (b_change_preview && status == Previewing))
		{
			int nWidth = rect.width();
			int nHeight = rect.height();
			rect.setLeft(0);
			rect.setRight(rect.left() + nWidth - 1);
			rect.setTop(0);
			rect.setBottom(rect.top() + nHeight - 1);
		}
		break;
	}
	case Device::kAutoExposureRoi:
	{
		HscAutoExposureParameter param = pDevice->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		DeviceState status = pDevice->getState();
		rect = Device::CameraWindowRect2QRect(param.autoExpArea);
		bool b_change_preview = pDevice->IsChangePreviewResoultionSupport();
		if (status == ToAcquire || status == Acquiring || status == Recording || (b_change_preview && status == Previewing))
		{
			QRect device_roi = pDevice->GetRoi(Device::RoiTypes::kDeviceRoi);
			int nWidth = rect.width();
			int nHeight = rect.height();
			rect.setLeft(rect.left() - device_roi.left());
			rect.setRight(rect.left() + nWidth - 1);
			rect.setTop(rect.top() + pDevice->GetExtraHeight() - device_roi.top());
			rect.setBottom(rect.top() + nHeight - 1);
		}
	}
	break;
	case Device::kIntelligentTriggerRoi:
	{
		HscIntelligentTriggerParamV2 param = pDevice->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		DeviceState status = pDevice->getState();
		rect = Device::CameraWindowRect2QRect(param.roi);
		bool b_change_preview = pDevice->IsChangePreviewResoultionSupport();
		if (status == ToAcquire || status == Acquiring || status == Recording || (b_change_preview && status == Previewing))
		{
			QRect device_roi = pDevice->GetRoi(Device::RoiTypes::kDeviceRoi);
			int nWidth = rect.width();
			int nHeight = rect.height();
			rect.setLeft(rect.left() - device_roi.left());
			rect.setRight(rect.left() + nWidth - 1);
			rect.setTop(rect.top() + pDevice->GetExtraHeight() - device_roi.top());
			rect.setBottom(rect.top() + nHeight - 1);
		}
	}
	break;
	default:
		break;
	}
	return rect;
}

QPoint CSRccApp::GetRoiOffsetPoint(QSharedPointer<Device> pDevice)
{
	QPoint ptOffset(0, 0);
	if (pDevice)
	{
		DeviceState status = pDevice->getState();
		if (status == ToAcquire || status == Acquiring || status == Recording)
		{
			auto rect = pDevice->GetRoi(Device::RoiTypes::kDeviceRoi);
			ptOffset.setX(rect.left());
			ptOffset.setY(rect.top());
		}
	}
	return ptOffset;
}

void CSRccApp::setDeviceRoiEditDisabled()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		if (ui->actionAcquisition_Window_Settings->isChecked()) {
			ui->actionAcquisition_Window_Settings->setChecked(false);
			pVideoPlayer->resetParam4DeviceRoiForbid();
			on_actionAcquisition_Window_Settings_triggered(ui->actionAcquisition_Window_Settings->isChecked());
		}
	}
}

void CSRccApp::cf18ControlProcess(bool connected)
{
	m_cf18_control_panel_widget->updateSynControlPanelStatus(connected);
	if (connected) {
		for (int i = 0; i < m_const_cf18_channel_num; ++i) {
			CmdChnStatus current_chn_status{};
			if (GetChnStatus(i, current_chn_status)) {
				m_cf18_control_panel_widget->updateChnStatus(i, current_chn_status);
			}
		}
	}
}

void CSRccApp::UpdateIntelligentTriggerRoiStatus(QString device_ip)
{
	auto pDevice = DeviceManager::instance().getDevice(device_ip);
	QString device_name;
	if (pDevice)
	{
		device_name = pDevice->getIpOrSn();
	}
	auto pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_name);
	if (nullptr != pVideoPlayer && nullptr != pDevice)
	{
		if (pDevice->IsIntelligentTriggerSupported())
		{
			HscIntelligentTriggerParamV2 param = pDevice->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			m_bIntelligentOpened = param.enable;
			RoiInfo roi_info;
			roi_info.roi_type = Device::RoiTypes::kIntelligentTriggerRoi;
			roi_info.roi_rect = GetRoiTypeRect(Device::kIntelligentTriggerRoi, pDevice);
			roi_info.roi_color = param.roi_color;
			pVideoPlayer->drawRoiRect(roi_info);
			pDevice->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			if (param.enable)
			{
				ui->IntelligentTriggerOpen->setChecked(true);
			}
			else
			{
				ui->IntelligentTriggerClose->setChecked(true);
			}
		}
		else
		{
			pVideoPlayer->setRoiVisible(Device::kIntelligentTriggerRoi, false);
		}
	}
}

void CSRccApp::UpdateEdrExposureStatus()
{
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	if (nullptr != pDevice)
	{
		if (pDevice->IsEdrExposureSupported())
		{
			bool bEnable = false;
			if (pDevice->IsIntelligentTriggerV4Supported()|| pDevice->isSupportHighBitParam()) {
				HscEDRParamV2 param = pDevice->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
				bEnable = param.enabled;
			}
			else {
				HscEDRParam param = pDevice->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
				bEnable = param.enabled;
			}

			if (bEnable)
			{
				ui->EdrExposureOpen->setChecked(true);
			}
			else
			{
				ui->EdrExposureClose->setChecked(true);
			}
		}
	}
}

void CSRccApp::UpdatePropFocusPointStatus()
{
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	QString device_name;
	if (pDevice)
	{
		device_name = pDevice->getIpOrSn();
	}
	auto pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_name);
	if (nullptr != pVideoPlayer && nullptr != pDevice)
	{
		auto ptCenterPoint = pDevice->getProperty(Device::PropType::PropFocusPoint).toPointF();
		auto ptOffset = GetRoiOffsetPoint(pDevice);
		if (ptOffset.x() != 0 || ptOffset.y() != 0)
		{
			ptCenterPoint.setX(ptCenterPoint.x() - ptOffset.x());
			ptCenterPoint.setY(ptCenterPoint.y() - ptOffset.y());
		}
		pVideoPlayer->SetCustomCrossLineCenterPoint(ptCenterPoint);
		if (pDevice->getProperty(Device::PropFocusPointVisible).toBool())
		{
			on_actionWindow_Centerline_triggered(pDevice->getProperty(Device::PropFocusPointVisible).toBool());
		}
	}
}

void CSRccApp::UpdateOverExposureStatus()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (pVideoPlayer) {
		auto device_name = pVideoPlayer->GetDeviceName();
		auto device = DeviceManager::instance().getDevice(device_name);
		if (device) {
			bool bOver = device->getProperty(Device::PropType::PropOverExposureTipEnable).toBool();
			pVideoPlayer->SetOverExposureStatus(bOver);
		}
	}
}

void CSRccApp::UpadateExploreTimeValue()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (devicePtr)
	{
		//写入真实曝光时间
		uint32_t exposure_cur_unit = devicePtr->getProperty(Device::PropType::PropExposureTime).toInt();
		agile_device::capability::Units unit = devicePtr->getProperty(Device::PropType::PropExposureTimeUnit).value<agile_device::capability::Units>();
		agile_device::capability::Units real_unit = devicePtr->GetRealExposureTimeUnit();
		m_iEdrExploreTimeValue = devicePtr->convertTime(exposure_cur_unit, unit, real_unit);
	}
}

QPointF CSRccApp::GetPictureCenterPoint() const
{
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	if (nullptr != pDevice)
	{
		if (pDevice->getState() == Acquiring || pDevice->isGrabberDevice()) {
			auto _roi = pDevice->getProperty(Device::PropRoi).toRectF();
			return _roi.center();
		}
	    //return pDevice->getProperty(Device::PropRoi).toRectF().center();
		QVariant value;
		value.setValue(pDevice->GetMaxRoi(Device::RoiTypes::kDeviceRoi));
		return value.toRectF().center();
	}
	return QPointF{};
}

void CSRccApp::QString2UCharArray(const QStringList &strList, BYTE *arr)
{
	for (int i = 0; i < strList.size(); ++i)
	{
		bool ok;
		arr[i] = (strList[i].toLatin1()).toUInt(&ok, 10);
	}
}

QString CSRccApp::FormatWarningMessage(QSharedPointer<Device> pDevice, const Device::Warning warning)
{
	QString str;
	switch (warning.msg_type)
	{
	case MSG_UPS_ELECTRICITY_LOW:
	{
		str = UIExplorer::instance().getStringById("STRID_ALERT_UPS_LOW_FORMAT") + GetDesc();
	}
	break;
	default:
		break;
	}

	return str;
}

QString CSRccApp::GetDesc() const
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device)
	{
		QString deviceType = device->IsTrigger() ? "STRID_DT_TRIGGER" : "STRID_DT_CAMERA";
		QString deviceName = device->getProperty(Device::PropType::PropName).toString();
		QString ipOrSn = device->getIpOrSn();
		if (deviceName.isEmpty())
		{
			return deviceType + "[" + ipOrSn + "]";
		}
		else
		{
			return deviceType + "[" + deviceName + " " + ipOrSn + "]";
		}
	}
	return "";
}

void CSRccApp::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		auto videoList = GetViewManagerPtr()->GetDisplayList();
		if (videoList.empty())
		{
			CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->GetFullViewBase();
			if (pVideoPlayer)
			{
				QString strCurrentStatus{};
				QSharedPointer<Device> new_device_ptr = DeviceManager::instance().getCurrentDevice();
				if (new_device_ptr) {
					strCurrentStatus = new_device_ptr->getFormattedStateStr();
				}
				else {
					strCurrentStatus = tr("no device associate");
				}
				pVideoPlayer->SetViewShowLabelInfo(CPlayerViewBase::ViewConnectStatusLabel, strCurrentStatus);
			}
		}
		else
		{
			for (auto e : videoList)
			{
				auto strName = e->GetDeviceName();
				auto pDevice = DeviceManager::instance().getDevice(strName);
				if (pDevice)
				{
					pDevice->updateViewOSD();
				}
				else
				{
					QString strCurrentStatus = tr("no device associate");
					e->SetViewShowLabelInfo(CPlayerViewBase::ViewConnectStatusLabel, strCurrentStatus);
				}
			}
		}
		UpdateStatusBarTranslate();
		setWindowTitle(m_project_info + UIExplorer::instance().getProductName());
	}

	QMainWindow::changeEvent(event);
}

void CSRccApp::hideEvent(QHideEvent *event)
{
	//保存窗口当前状态
	SystemSettingsManager::instance().saveWindowState(saveState());
	event->accept();
}

void CSRccApp::showEvent(QShowEvent *event)
{
	try
	{
		restoreState(SystemSettingsManager::instance().getWindowState());//恢复以往记忆的窗口状态
	}
	catch (const std::exception&e)
	{
		CSLOG_INFO("restoreState crush:{} ",e.what());
	}
	SlotCurrentDeviceStateChanged("", Unconnected);//初始化界面控件使能状态

	QMainWindow::showEvent(event);
}

void CSRccApp::SlotDeviceConnected(const QString& devicename)
{
	auto devicePtr = DeviceManager::instance().getDevice(devicename);
	if (devicePtr)
	{
		auto connect_type_auto = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);
		auto ok = connect(devicePtr.data(), &Device::updateFrameRate, this, &CSRccApp::SlotUpdateFrameRate, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::propertyChanged, this, &CSRccApp::SlotPropertyChanged, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::SignalAvgLum, this, &CSRccApp::SlotAvgLum, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::SignalAlarmMsg, this, &CSRccApp::SlotAlarmMsg, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::updateBcode, this, &CSRccApp::SlotBcode, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::ErrMsg, this, &CSRccApp::SlotErrMsg, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::SignalRecoveringData, this, &CSRccApp::SlotRecoveringData, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::SignalRecoveringDataProgress, this, &CSRccApp::SlotRecoveringDataProgress, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::signalETSnap, this, &CSRccApp::slotETSnap, connect_type_auto);
		ok = connect(devicePtr.data(), &Device::signalTemperatureAbnormal, this, &CSRccApp::slotTemperatureAbnormal, connect_type_auto);
		Q_UNUSED(ok);
	}
}

void CSRccApp::SlotDeviceDisconnected(const QString& devicename)
{
	auto devicePtr = DeviceManager::instance().getDevice(devicename);
	if (devicePtr && !devicePtr->IsCF18())
	{
		auto ok = disconnect(devicePtr.data(), &Device::updateFrameRate, this, &CSRccApp::SlotUpdateFrameRate);
		ok = disconnect(devicePtr.data(), &Device::propertyChanged, this, &CSRccApp::SlotPropertyChanged);
		ok = disconnect(devicePtr.data(), &Device::SignalAvgLum, this, &CSRccApp::SlotAvgLum);
		ok = disconnect(devicePtr.data(), &Device::SignalAlarmMsg, this, &CSRccApp::SlotAlarmMsg);
		ok = disconnect(devicePtr.data(), &Device::updateBcode, this, &CSRccApp::SlotBcode);
		ok = disconnect(devicePtr.data(), &Device::ErrMsg, this, &CSRccApp::SlotErrMsg);
		ok = disconnect(devicePtr.data(), &Device::SignalRecoveringData, this, &CSRccApp::SlotRecoveringData);
		ok = disconnect(devicePtr.data(), &Device::SignalRecoveringDataProgress, this, &CSRccApp::SlotRecoveringDataProgress);
		ok = disconnect(devicePtr.data(), &Device::signalETSnap, this, &CSRccApp::slotETSnap);
		ok = disconnect(devicePtr.data(), &Device::signalTemperatureAbnormal, this, &CSRccApp::slotTemperatureAbnormal);
		Q_UNUSED(ok);
	}
	GetViewManagerPtr()->RemoveFromDisplayList(devicename);//设备断开时将其从视图中移除

}

void CSRccApp::SlotCurrentDeviceChanged(const QString& device_ip)
{
	Q_UNUSED(device_ip);

	CPlayerViewBase* pPlayerView = GetViewManagerPtr()->getSelectView();
	if (pPlayerView)
	{
		pPlayerView->EnableDrawCrossLine(ui->actionCrossHair->isChecked());
		pPlayerView->setFeaturePointVisible(Device::kDeviceRoi, false);
	}

	//当前设备切换,判断是否和之前的设备相同,不同则断开之前的信号绑定,对新的设备进行绑定
	QSharedPointer<Device> new_device_ptr = DeviceManager::instance().getCurrentDevice();
	if (new_device_ptr.isNull())
	{
		SlotCurrentDeviceStateChanged(device_ip,Unconnected);
		GetViewManagerPtr()->on_device_clicked(QString(""));//设置获取当前设备的默认roi
	}
	//判断是否与当前设备相同
	if (m_current_device_ptr == new_device_ptr)
	{
		return;
	}
	
	//不相同,断开原来设备的相关信号
	if (!m_current_device_ptr.isNull())
	{
		if (m_current_device_ptr->IsCF18()) {
			disconnect(m_current_device_ptr.data(), &Device::cf18StateChanged, this, &CSRccApp::SlotCurrentDeviceStateChanged);
		}
		else {
			disconnect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSRccApp::SlotCurrentDeviceStateChanged);
		}
		disconnect(m_current_device_ptr.data(), &Device::updateTemperatureInfo, this, &CSRccApp::SlotTemperatureInfoUpdated);
		disconnect(m_current_device_ptr.data(), &Device::signalUpdateDrawStatusInfo, this, &CSRccApp::slotUpdateDrawStatusInfo);
		CPlayerViewBase* pPlayerView_ = GetViewManagerPtr()->getDeviceView(m_current_device_ptr->getIpOrSn());
		if (pPlayerView_)
		{
			pPlayerView_->EnableDrawCrossLine(false);
		}
	}
	m_current_device_ptr = new_device_ptr;

	if (m_current_device_ptr && m_current_device_ptr->IsCF18()) {
		if (!m_current_device_ptr.isNull()) {
			connect(m_current_device_ptr.data(), &Device::cf18StateChanged, this, &CSRccApp::SlotCurrentDeviceStateChanged, Qt::QueuedConnection);
			SlotCurrentDeviceStateChanged(device_ip, m_current_device_ptr->getState());//根据当前状态执行操作
			connect(m_current_device_ptr.data(), &Device::SignalUpdateUIInfo, this, &CSRccApp::SlotUpdateUIInfo);
			SlotCurrentDeviceStateChanged(device_ip, m_current_device_ptr->getState());//根据当前状态执行操作
			GetViewManagerPtr()->on_device_clicked(new_device_ptr->getIpOrSn());//设置获取当前设备的默认roi
			SystemSettingsManager::instance().setCurrentDeviceIP(m_current_device_ptr->getIpOrSn());
		}
		return;
	}

	if (!m_current_device_ptr.isNull())
	{
		//连接新的设备信号
		connect(m_current_device_ptr.data(), &Device::stateChanged, this, &CSRccApp::SlotCurrentDeviceStateChanged,Qt::QueuedConnection);
		connect(m_current_device_ptr.data(), &Device::updateTemperatureInfo, this, &CSRccApp::SlotTemperatureInfoUpdated);
		connect(m_current_device_ptr.data(), &Device::SignalUpdateUIInfo, this, &CSRccApp::SlotUpdateUIInfo);
		connect(m_current_device_ptr.data(), &Device::signalUpdateDrawStatusInfo, this, &CSRccApp::slotUpdateDrawStatusInfo);
		SlotCurrentDeviceStateChanged(device_ip,m_current_device_ptr->getState());//根据当前状态执行操作
		GetViewManagerPtr()->on_device_clicked(new_device_ptr->getIpOrSn());//设置获取当前设备的默认roi
		SystemSettingsManager::instance().setCurrentDeviceIP(m_current_device_ptr->getIpOrSn());
	}
	else
	{
		SlotCurrentDeviceStateChanged(device_ip,Unconnected);
		GetViewManagerPtr()->on_device_clicked(QString(""));//设置获取当前设备的默认roi
	}
	
	auto pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		ui->actionAnti_aliasing->setChecked(pVideoPlayer->GetAntiAliasingStatus());
		pVideoPlayer->SetAntiAliasing(pVideoPlayer->GetAntiAliasingStatus());
		if (!pVideoPlayer->GetDeviceName().isEmpty())
			pVideoPlayer->EnableDrawCrossLine(this->ui->actionCrossHair->isChecked());// 修复通过列表框切换设备时 不显示十字线的问题
	}
}

void CSRccApp::SlotTemperatureInfoUpdated(QString temperature_info)
{
	m_devicetemperaturepanel_widget->setTemperatureInfo(temperature_info);
}

void CSRccApp::SlotCurrentDeviceStateChanged(const QString &current_ip, DeviceState device_state)
{
	//更新整个界面的UI使能和启用
	updateMainWindowUI();
	
	//设备状态切换时取消对主界面控件的禁用
	EnableRoiRelatedWidgets(true);
	
	//更新旋转
	bool bUpdateRotate = false;
	bool bNeedUpdateRotate = false;

	//发送状态的设备是选中设备时，更新状态
	if (DeviceManager::instance().getCurrentDevice() == qobject_cast<Device*>(sender()))
	{
		auto pDevice = DeviceManager::instance().getCurrentDevice();

		if (pDevice && (DeviceModel::DEVICE_TRIGGER_CF18 == pDevice->getModel())) {
			bool bEnable = false;
			bEnable = (DeviceState::Connected == device_state) ? true : false;
			cf18ControlProcess(bEnable);
		}

		if (pDevice && pDevice->IsStandByModeSupported() && (StandBy == pDevice->getState())) {
			bool bopen = pDevice->getProperty(Device::PropHardwareStandby).toBool();
			updateProjectItem(bopen);
		}
		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
		if (pVideoPlayer && pDevice && pDevice->IsCF18())
		{
			pVideoPlayer->SetViewShowLabelInfo(CPlayerViewBase::ViewConnectStatusLabel, pDevice->getFormattedStateStr());
		}
		if (pDevice && (!pDevice->IsCF18()) &&((Unconnected != pDevice->getState()) && (Disconnecting != pDevice->getState())&&
			(Disconnected != pDevice->getState()) && (StandBy != pDevice->getState()) && 
			(Reconnecting != pDevice->getState())))
		{
			emit signalDisconnect2GainSet();
			m_bBarRotateVisible = true;
			int iRotationType = pDevice->getProperty(Device::PropType::PropRotateType).toInt();
			SetStatusBarRotateValue(iRotationType);
			bUpdateRotate = true;
			pDevice->setProperty(Device::PropType::PropRotateType, QVariant::fromValue(iRotationType));
			if (pVideoPlayer)
			{
				pVideoPlayer->SetViewShowLabelInfo(CPlayerViewBase::ViewConnectStatusLabel, pDevice->getFormattedStateStr());

				UpdatAlarmActionsUI();
				updateAcquisitionActionsUI();

				//pVideoPlayer->RefreshSize();//更新状态时刷新图像界面显示,用于暂时修复部分电脑状态更新后显示异常的问题

				pVideoPlayer->SetRotateType(ROTATE_TYPE(iRotationType));
			}
			bool bEnbale = pDevice->getProperty(Device::PropFocusPointVisible).toBool();
			if (SystemSettingsManager::instance().isWindow_Centerline() != bEnbale)
			{
				pDevice->setProperty(Device::PropFocusPointVisible, !bEnbale);
			}

			if (pVideoPlayer) {
// 				if (pDevice && (Acquiring == pDevice->getState()|| ToAcquire == pDevice->getState())) {
// 					pVideoPlayer->setDeviceInAcquiring(true);
// 				}
				if (pVideoPlayer->GetTrackRoiEnabled())
				{
					pVideoPlayer->resetParam4DeviceRoiForbid();
				}
				if (pDevice && Previewing != pDevice->getState() && Acquiring != pDevice->getState() && Recording != pDevice->getState())
				{
					pVideoPlayer->showBackgroundImage();
					pVideoPlayer->FitView(true);    //背景图时，不响应旋转
				}

				if (pDevice && Previewing == pDevice->getState() || Acquiring == pDevice->getState() || Recording == pDevice->getState())
					pVideoPlayer->turn2Display(true);
			}

			if (pDevice && pVideoPlayer) {
				if (ToAcquire == pDevice->getState() || Acquiring == pDevice->getState() || Recording == pDevice->getState())
					pVideoPlayer->setDisplayStatus(true, pDevice->getProperty(Device::PropFocusPoint).toPointF());
				else
					pVideoPlayer->setDisplayStatus(false, pDevice->getProperty(Device::PropFocusPoint).toPointF());
			}
		}
	}

	if (DeviceState::Acquiring == device_state ||
		DeviceState::ToAcquire == device_state ||
		DeviceState::Recording == device_state ||
		DeviceState::ToPreview == device_state ||
		DeviceState::Previewing == device_state)
	{
		UpdateDeviceRoiStatus(current_ip);
		UpdateAutoExposureRoiStatus(current_ip);
		UpdateIntelligentTriggerRoiStatus(current_ip);
		UpdateOverExposureStatus();
	}


	if (DeviceState::Previewing == device_state)
	{
		UpdateEdrExposureStatus();
		UpdatePropFocusPointStatus();
		UpadateExploreTimeValue();
		bNeedUpdateRotate = true;
	}
	else if(DeviceState::Acquiring == device_state || DeviceState::Recording == device_state)
	{
		UpdatePropFocusPointStatus();
		bNeedUpdateRotate = true;
	}
	if (!bUpdateRotate && bNeedUpdateRotate)
	{
		auto pDevice = DeviceManager::instance().getCurrentDevice();
		if (pDevice && (Previewing == pDevice->getState() || Acquiring == pDevice->getState() || Recording == pDevice->getState()))
		{
			int iRotationType = pDevice->getProperty(Device::PropType::PropRotateType).toInt();
			SetStatusBarRotateValue(iRotationType);
		}
	}
}

void CSRccApp::SlotUpdateUIInfo()
{
	//更新整个界面的UI使能和启用
	updateMainWindowUI();

	//设备状态切换时取消对主界面控件的禁用
	EnableRoiRelatedWidgets(true);

	//发送状态的设备是选中设备时，更新状态
	if (DeviceManager::instance().getCurrentDevice() == qobject_cast<Device*>(sender()))
	{
		auto pDevice = DeviceManager::instance().getCurrentDevice();
		if (pDevice && ((Unconnected != pDevice->getState()) || (Disconnecting != pDevice->getState())||
			(Disconnected != pDevice->getState()) || (StandBy != pDevice->getState()) || 
			(Reconnecting != pDevice->getState())))
		{
			m_bBarRotateVisible = true;
			int iRotationType = pDevice->getProperty(Device::PropType::PropRotateType).toInt();
			SetStatusBarRotateValue(iRotationType);
			pDevice->setProperty(Device::PropType::PropRotateType, QVariant::fromValue(iRotationType));
			CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
			if (pVideoPlayer)
			{
				pVideoPlayer->SetRotateType(ROTATE_TYPE(iRotationType));
			}
			bool bEnbale = pDevice->getProperty(Device::PropFocusPointVisible).toBool();
			if (SystemSettingsManager::instance().isWindow_Centerline() != bEnbale)
			{
				pDevice->setProperty(Device::PropFocusPointVisible, !bEnbale);
			}
		}
	}
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	if (!pDevice)
	{
		return;
	}
	DeviceState device_state = pDevice->getState();
	
	QString current_ip = pDevice->getIpOrSn();
	if (DeviceState::Previewing == device_state)
	{
		UpdateDeviceRoiStatus(current_ip);
		UpdateAutoExposureRoiStatus(current_ip);
		UpdateIntelligentTriggerRoiStatus(current_ip);
		UpdateEdrExposureStatus();
		UpdatePropFocusPointStatus();
		UpdateOverExposureStatus();
		UpadateExploreTimeValue();
	}
	else if(DeviceState::Acquiring == device_state || DeviceState::Recording == device_state)
	{
		UpdateDeviceRoiStatus(current_ip);
		UpdateAutoExposureRoiStatus(current_ip);
		UpdateIntelligentTriggerRoiStatus(current_ip);
		UpdatePropFocusPointStatus();
	}
}

void CSRccApp::slotUpdateDrawStatusInfo(Device::DrawTypeStatusInfo info)
{
	if (DeviceManager::instance().getCurrentDevice() == qobject_cast<Device*>(sender()))
	{
		//没有设备则置灰和设备相关的操作
		auto pDevice = DeviceManager::instance().getCurrentDevice();
		if (nullptr == pDevice ||
			pDevice->IsTrigger())
		{
			ui->actionAcquisition_Window_Settings->setEnabled(false);	//设置roi
			ui->actionAcquisition_Window_Settings->setChecked(false);
			return;
		}

		//使能控制
		bool enable = (pDevice->getState() == DeviceState::Previewing);//只允许在预览时使用
		if (pDevice->isGrabberDevice()) {
			ui->actionAcquisition_Window_Settings->setEnabled(false);
		}
		else {
			Device::DrawTypeStatusInfo drawStatus = pDevice->getDrawTypeStatusInfo();
			ui->actionAcquisition_Window_Settings->blockSignals(true);
			if (drawStatus == Device::DTSI_Noraml || drawStatus == Device::DTSI_DrawROI)
			{
				ui->actionAcquisition_Window_Settings->setEnabled(enable);
			}
			else
			{
				ui->actionAcquisition_Window_Settings->setEnabled(false);
			}
			ui->actionAcquisition_Window_Settings->blockSignals(false);
		}
	}
}

void CSRccApp::slotETSnap()
{
	auto sender_device = qobject_cast<Device*>(sender());
	if (sender_device && (DeviceState::Previewing == sender_device->getState())) {
		CPlayerViewBase *sender_view = GetViewManagerPtr()->getDeviceView(sender_device->getIpOrSn());
		if (sender_view) {
			sender_view->Snapshot();
		}
	}
}

void CSRccApp::slotTemperatureAbnormal(QString strName)
{
	UIUtils::showWarnMsgBox(this, tr("The temperature of %1 device is abnormal. Please check the device status!").arg(strName));
}

void CSRccApp::slotRemoveCurrentDevice()
{
	if (!m_current_device_ptr.isNull())
	{
		//if ((DeviceState::Connected == m_current_device_ptr->getState()) || (DeviceState::Previewing == m_current_device_ptr->getState())) 
		{
			m_current_device_ptr->disconnect(true);
		}
		DeviceManager::instance().removeAddedDevice(m_current_device_ptr);
	}
}

void CSRccApp::SlotDisplayWidgetClicked(int index)
{
	//更新窗口索引
	m_iWindowIndex = index;

	//设备选择改变，主动获取离职缩放系数，并更新在状态栏
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		SetZoomCoefficient(pVideoPlayer->GetZoomCoeff());
	}
	else {
		DeviceManager::instance().SetCurrentDevice(QSharedPointer<Device>{});
		return;
	}
	auto device_name = pVideoPlayer->GetDeviceName();
	auto device =DeviceManager::instance().getDevice(device_name);
	//切换设备时，默认帧率为0
	SetFrameRate(0);

	DeviceManager::instance().SetCurrentDevice(device);
}

void CSRccApp::SlotDisplayWidgetAdjust(int type)
{
	auto videoList = GetViewManagerPtr()->GetDisplayList();
	if (videoList.empty())
		return;
	CSViewManager::SplitType split_type = static_cast<CSViewManager::SplitType>(type);
	int row{}, col{};
	CSViewManager::getSplitRowCol(split_type, row, col);
	Q_ASSERT(videoList.size() == type);
	for (int i = 0; i < row; ++i)
	{
		for (int j = 0; j < col; ++j)
		{
			m_mainLayout->addWidget(videoList.at(i*row + j), i, j);
		}
	}

	for (auto e : videoList)
	{
		e->show();
	}
}

void CSRccApp::SlotCurrentCursorInVideo(const QPoint& pt)
{
	if (pt == QPointF{-1,-1})
	{
		RefreshQLabelStyle(m_pXLabel, "numLabelforbid");
		RefreshQLabelStyle(m_pYLabel, "numLabelforbid");
	}
	else
	{
		RefreshQLabelStyle(m_pXLabel, "numLabel");
		RefreshQLabelStyle(m_pYLabel, "numLabel");

		SetXStatusValue(pt.x());
		SetYStatusValue(pt.y());
	}
}

void CSRccApp::SlotZoomCoefficientChanged(float coef)
{
	SetZoomCoefficient(coef);
}

void CSRccApp::SlotUpdateFrameRate(qreal fps)
{
	Device* pSenderDevice = qobject_cast<Device*>(sender());
	if (DeviceManager::instance().getCurrentDevice() && pSenderDevice)
	{
		if (pSenderDevice->getIpOrSn() == DeviceManager::instance().getCurrentDevice()->getIpOrSn())
		{
			SetFrameRate(fps);
		}
	}
}

void CSRccApp::SlotBcode(bool bcode)
{
	Device* pSenderDevice = qobject_cast<Device*>(sender());
	if (DeviceManager::instance().getCurrentDevice() && pSenderDevice)
	{
		if (pSenderDevice->getIpOrSn() == DeviceManager::instance().getCurrentDevice()->getIpOrSn())
		{
			SetBcode(bcode);
		}
	}
}

void CSRccApp::SlotRecoveringData()
{
	qDebug() << "SlotRecoveringData.";
	auto device_ptr = qobject_cast<Device*>(sender());
	if (device_ptr == nullptr)
	{
		return;
	}
	bool bRet = UIUtils::showQuestionMsgBox(this, tr("The system has detected that the device[%1] need power-off data recovery. Continue?").arg(device_ptr->getIpOrSn()));
	if (bRet)
	{
		HscResult res = device_ptr->StartDataRecovery();
		if (res == HSC_OK)
		{
			//开始恢复断电数据
			recoveringdatawidget widget(this);	 // 数据恢复对话框
			connect(this, &CSRccApp::SignalRecoveringDataProgress, &widget, &recoveringdatawidget::onProgressChanged);
			widget.ShowProgressDlg();
			if (device_ptr == nullptr)
			{
				return;
			}
			bool b_success = widget.result();
			device_ptr->FinishDataRecovery(!b_success);
			if (b_success)
			{
				UIUtils::showInfoMsgBox(this, tr("Data recovery was finished."));
			}
			/*if (nProgress >= 100)
			{
				UIUtils::showErrorMsgBox(this, tr("Data recovery was successful."));
			}
			else
			{
				UIUtils::showErrorMsgBox(this, tr("Data recovery was Field."));
			}*/
		}
		else
		{
			UIUtils::showErrorMsgBox(this, tr("Data recovery was Field."));
		}
	}
}

void CSRccApp::SlotRecoveringDataProgress(int nProgress)
{
	qDebug() << "RecoveringDataProgress:" << nProgress;
	emit SignalRecoveringDataProgress(nProgress);
}

void CSRccApp::RecoveringDataFail()
{
	qDebug() << "RecoveringDataFail";
	emit SignalRecoveringDataProgress(-1);
	return;
}

void CSRccApp::slotStopDevices(const QStringList& devices)
{
	for (const auto &e : devices)
	{
		auto device_ptr = DeviceManager::instance().getDevice(e);
		if (device_ptr && device_ptr->allowsStop())
			device_ptr->stop();
	}
}

void CSRccApp::SlotColorModeActionGroupTriggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//根据选中状态进行操作
	if (ui->actionMonochrome->isChecked())
	{
		device_ptr->setProperty(Device::PropDisplayMode, HscDisplayMode::HSC_DISPLAY_MONO);
		m_bColor = false;
	}
	else if (ui->actionColor->isChecked())
	{
		device_ptr->setProperty(Device::PropDisplayMode, HscDisplayMode::HSC_DISPLAY_COLOR);
		m_bColor = true;
	}


}

void CSRccApp::SlotWbEnvActionGroupTriggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//根据选中状态进行操作
	if (ui->actionEnvironmentNone->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_NONE);
	}
	else if (ui->actionEnvironmentAuto->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_AUTO_FROM_ISP);
	}
	else if (ui->actionSunny->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_SUNNY);
	}
	else if (ui->actionSunset->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_SUNSET);
	}
	else if (ui->actionCloudy_Day->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_CLOUDY);
	}
	else if (ui->actionFill_Light->isChecked())
	{
		device_ptr->setProperty(Device::PropWbEnv, HSC_WB_ENV_LIGHT);
	}

}

void CSRccApp::SlotFanControlActionGroupTriggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}

	//根据选中状态进行操作
	if (ui->actionFan_ControlOpen->isChecked())
	{
		device_ptr->SetFanState(1);
	}
	else if (ui->actionFan_ControlClose->isChecked())
	{
		device_ptr->SetFanState(0);
	}
	else if (ui->actionFan_ControlAuto->isChecked()) {
		device_ptr->SetFanState(101);
	}
}

void CSRccApp::SlotAutoExposureActionGroupTriggered()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (ui->AutoExposureOpen->isChecked())
	{
		m_bAutoExposureOpened = true;
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.bEnable = true;
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		if (devicePtr->getState() == Previewing)
		{
			updateLowLightActionStatus(!m_bAutoExposureOpened);
		}
	}
	else if (ui->AutoExposureClose->isChecked())
	{
		m_bAutoExposureOpened = false;
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		param.bEnable = false;
		devicePtr->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		if (devicePtr->getState() == Previewing)
		{
			updateLowLightActionStatus(!m_bAutoExposureOpened);
		}
	}
}

void CSRccApp::SlotImageCtrlActionGroupTriggered(QAction* act)
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (!device)
		return;
	auto image_ctrl_param = device->getProperty(Device::PropImageCtrl).value<HscImageCtrlParam>();
	if (act == ui->actionFlip_The_Image)
		image_ctrl_param.enableFlipImage = act->isChecked();
	else if (act == ui->actionMirror) 
			image_ctrl_param.enableMirrorImage = act->isChecked();
	else if (act == ui->actionInverted_Image)
			image_ctrl_param.enableInvertImage = act->isChecked();
	device->setProperty(Device::PropImageCtrl, QVariant::fromValue(image_ctrl_param));
}

void CSRccApp::SlotIntelligentTriggerActionGroupTriggered()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr == devicePtr)
	{
		return;
	}

	if (ui->IntelligentTriggerOpen->isChecked())
	{
		m_bIntelligentOpened = true;
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		param.enable = true;
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
	}
	else if (ui->IntelligentTriggerClose->isChecked())
	{
		m_bIntelligentOpened = false;
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		param.enable = false;
		devicePtr->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
	}
}

void CSRccApp::SlotEdrExposureTriggerActionGroupTriggered()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != devicePtr)
	{
		bool bSetting = false;
		bool bEnable = false;
		if (ui->EdrExposureOpen->isChecked())
		{
			bEnable = true;
			bSetting = true;
		}
		else if (ui->EdrExposureClose->isChecked())
		{
			bEnable = false;
			bSetting = true;
		}
		if (bSetting)
		{
			if (devicePtr->IsIntelligentTriggerV4Supported() || devicePtr->isSupportHighBitParam()) {
				HscEDRParamV2 param = devicePtr->getProperty(Device::PropEdrExposureV2).value<HscEDRParamV2>();
				param.enabled = bEnable;
				devicePtr->setProperty(Device::PropEdrExposureV2, QVariant::fromValue(param));
			}
			else {
				HscEDRParam param = devicePtr->getProperty(Device::PropEdrExposure).value<HscEDRParam>();
				param.enabled = bEnable;
				devicePtr->setProperty(Device::PropEdrExposure, QVariant::fromValue(param));
			}
		}
	}
}

void CSRccApp::SlotTranslateTriggerActionGroupTriggered()
{
	if (ui->actionChinese->isChecked())
	{
		CSRccAppTranslator::instance().setLanguage(QLocale::Language::Chinese);
		SystemSettingsManager::instance().setLanguage(QLocale::Chinese);
	}
	else if (ui->actionEnglish->isChecked())
	{
		CSRccAppTranslator::instance().setLanguage(QLocale::Language::English);
		SystemSettingsManager::instance().setLanguage(QLocale::English);
	}
	m_winCrossLineSet->UpdateTranslate();
	// 初始化错误码
	AgErrorCode::instance().reset(SystemSettingsManager::instance().getLanguage() == QLocale::English ? 1 : 0);

	// 初始化描述文案表
	UIExplorer::instance().reset(SystemSettingsManager::instance().getLanguage() == QLocale::English ? 1 : 0);
}

void CSRccApp::SlotGetPictureCenterPoint()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != GetViewManagerPtr())
	{
		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
		if ((nullptr != pVideoPlayer) && (nullptr != devicePtr))
		{
			QPointF point = GetPictureCenterPoint();
			if (!point.isNull())
			{
				m_winCrossLineSet->SetCustomCrossLineCenterPointLineEditText(point);
				devicePtr->setProperty(Device::PropFocusPoint, QVariant::fromValue(point));
			}
		}
	}
}

void CSRccApp::SlotPropertyChanged(Device::PropType type, const QVariant & value)
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	QString device_name;
	if (devicePtr)
	{
		device_name = devicePtr->getIpOrSn();
	}
	auto pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_name);
	if ((nullptr == devicePtr) || (nullptr == pVideoPlayer))
	{
		return;
	}

	if (Device::PropType::PropIntelligentTrigger == type)
	{
		HscIntelligentTriggerParamV2 param = devicePtr->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
		RoiInfo roi_info;
		roi_info.roi_type = Device::RoiTypes::kIntelligentTriggerRoi;
		roi_info.roi_rect = GetRoiTypeRect(Device::kIntelligentTriggerRoi, devicePtr);
		roi_info.roi_color = param.roi_color;
		pVideoPlayer->drawRoiRect(roi_info, m_mid_btn_clicked);
		pVideoPlayer->setRoiVisible(Device::RoiTypes::kIntelligentTriggerRoi, param.roi_visible/*&m_bIntelligentOpened*/);
		//pVideoPlayer->updateItemLayer(Device::RoiTypes::kIntelligentTriggerRoi);
	}

	if (Device::PropType::PropAutoExposure == type)
	{
		HscAutoExposureParameter param = devicePtr->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
		RoiInfo roi_info;
		roi_info.roi_type = Device::RoiTypes::kAutoExposureRoi;
		roi_info.roi_rect = GetRoiTypeRect(Device::kAutoExposureRoi, devicePtr);
		roi_info.roi_color = param.roi_color;
		pVideoPlayer->drawRoiRect(roi_info, m_mid_btn_clicked);
		pVideoPlayer->setRoiVisible(Device::RoiTypes::kAutoExposureRoi, param.roi_visible/*&m_bAutoExposureOpened*/);
		//pVideoPlayer->updateItemLayer(Device::RoiTypes::kAutoExposureRoi);
	}

	if (Device::PropType::PropRoi == type)
	{
		RoiInfo roi_info;
		roi_info.roi_type = Device::RoiTypes::kDeviceRoi;
		roi_info.roi_rect = GetRoiTypeRect(Device::kDeviceRoi, devicePtr);
		pVideoPlayer->drawRoiRect(roi_info, m_mid_btn_clicked);
		m_mid_btn_clicked = false;
		//pVideoPlayer->updateItemLayer(Device::RoiTypes::kDeviceRoi);
	}

	if (Device::PropType::PropRoiVisible == type)
	{
		pVideoPlayer->setRoiVisible(Device::RoiTypes::kDeviceRoi, devicePtr->getProperty(Device::PropRoiVisible).toBool());

	}

	if (Device::PropType::PropRotateType == type)
	{
		pVideoPlayer->SetRotateType(ROTATE_TYPE(devicePtr->getProperty(Device::PropRotateType).toInt()));
	}

	if (Device::PropType::PropFocusPointVisible == type)
	{
		bool bvisible = devicePtr->getProperty(Device::PropFocusPointVisible).toBool();
		pVideoPlayer->EnableDrawCustomCrossLine(bvisible);
	}

	if (Device::PropType::PropFocusPoint == type)
	{
		auto ptCenterPoint = devicePtr->getProperty(Device::PropType::PropFocusPoint).toPointF();
		auto ptOffset = GetRoiOffsetPoint(devicePtr);
		if (ptOffset.x() != 0 || ptOffset.y() != 0)
		{
			ptCenterPoint.setX(ptCenterPoint.x() - ptOffset.x());
			ptCenterPoint.setY(ptCenterPoint.y() - ptOffset.y());
		}
		pVideoPlayer->SetCustomCrossLineCenterPoint(ptCenterPoint);
	}

	if (Device::PropType::PropOverExposureTipEnable == type)
	{
		UpdateOverExposureStatus();
	}

	if (Device::PropType::PropDisplayMode == type || Device::PropType::PropStreamType == type)
	{
		updateImageActionsUI();
	}

	if (Device::PropType::PropExposureTime == type)
	{
		UpadateExploreTimeValue();
		if (devicePtr)
		{
			devicePtr->correctEdrExposure();
		}
	}

	if (Device::PropType::PropPivParam == type || Device::PropType::PropSyncSource == type)
	{
		updateSettingActionsUI();
	}

	if (Device::PropType::PropFrameRate == type)
	{
		handleFrameRateChange();
	}

	if (Device::PropType::PropHardwareStandby == type) {
		bool bopen = devicePtr->getProperty(Device::PropHardwareStandby).toBool();
		updateProjectItem(bopen);
	}
}

void CSRccApp::SlotUpdateCustomCrossLineCenterPoint(const QPointF& centerpoint)
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device) {
		QPointF ptCenter(centerpoint);
		auto ptOffset = GetRoiOffsetPoint(device);
		ptCenter.setX(ptCenter.x() + ptOffset.x());
		ptCenter.setY(ptCenter.y() + ptOffset.y());
		device->setProperty(Device::PropFocusPoint, QVariant::fromValue(ptCenter));
	}
}

void CSRccApp::SlotCustomCrossLineCenterMovePoint(const QPoint& centerpoint)
{
	m_winCrossLineSet->SetCustomCrossLineCenterPointLineEditText(centerpoint);
}

void CSRccApp::updateRoiInfoSlot(const Device::RoiTypes& type, const QRect& rect)
{
	auto pDevice = DeviceManager::instance().getCurrentDevice();
	switch (type)
	{
	case Device::RoiTypes::kDeviceRoi:
	{
		if (pDevice) {
			pDevice->setProperty(Device::PropRoi, rect);

			// [2022/8/31 rgq]: 修改ROI时同步修改自动曝光区域、智能触发区域
			QRect roiRect = pDevice->getProperty(Device::PropRoi).toRect();
			if (pDevice->IsAutoExposureSupported())
			{
				HscAutoExposureParameter param = pDevice->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
				auto target_roi = roiRect.intersected(Device::CameraWindowRect2QRect(param.autoExpArea));
				if (target_roi.isNull()) target_roi = roiRect;// 目标区域交集不存在 则重定义目标区域为设备区域
				param.autoExpArea = Device::QRectTCameraWindowRect(roiRect.intersected(target_roi));
				pDevice->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
			}
			if (pDevice->IsIntelligentTriggerSupported())
			{
				HscIntelligentTriggerParamV2 param = pDevice->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
				auto old_roi = Device::CameraWindowRect2QRect(param.roi);
				auto target_roi = roiRect.intersected(old_roi);
				if (target_roi.isNull()) target_roi = roiRect;// 目标区域交集不存在 则重定义目标区域为设备区域
				auto new_roi = roiRect.intersected(target_roi);
				if ((pDevice->IsSupportCMOSDigitGain() || pDevice->IsIntelligentTriggerV5Supported()) && !pDevice->isGrabberDevice())
				{
					param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), new_roi);
				}
				param.roi = Device::QRectTCameraWindowRect(new_roi);
				pDevice->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
			}
		}
		break;
	}
	case Device::kIntelligentTriggerRoi:
	{
		if (pDevice) {
			HscIntelligentTriggerParamV2 param = pDevice->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			QRect roiRect = pDevice->getProperty(Device::PropRoi).toRect();
			auto target_roi = roiRect.intersected(rect);
			if (target_roi.isNull()) target_roi = roiRect;
			if ((pDevice->IsSupportCMOSDigitGain() || pDevice->IsIntelligentTriggerV5Supported()) && !pDevice->isGrabberDevice())
			{
				QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
				param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), target_roi);
			}
			param.roi = Device::QRectTCameraWindowRect(target_roi);
			pDevice->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
		}
		break;
	}
	case Device::kAutoExposureRoi:
	{
		if (pDevice) {
			HscAutoExposureParameter param = pDevice->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			QRect roiRect = pDevice->getProperty(Device::PropRoi).toRect();
			auto target_roi = roiRect.intersected(rect);
			if (target_roi.isNull()) target_roi = roiRect;
			param.autoExpArea = Device::QRectTCameraWindowRect(target_roi);
			pDevice->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		}
		break;
	}
	default:
		break;
	}

	if ((Device::kIntelligentTriggerRoi == type) || (Device::kAutoExposureRoi == type)) {
		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
		if (pVideoPlayer) {
			//pVideoPlayer->setFeaturePointVisible(type, false);
			pDevice->setDrawTypeStatusInfo(Device::DTSI_Noraml);
		}
	}
}

void CSRccApp::SlotLocalVideoDispaly(const QString& path)
{
	if (path.isEmpty())
	{
		//TODO:弹框提示
	} 
	else
	{
		CSLocalVideoPlayer dlg(this);
		if (dlg.LoadFile(path))
		{
			dlg.setWindowState(Qt::WindowMaximized);
			dlg.exec();
		}
		else
		{
			SlotErrMsg(HSC_INVALID_VIDEO);
		}
	}
}

void CSRccApp::SlotUpdateFrameIntelligentAvgBright(const QString &ip, const int value)
{
	emit SignalIntelligentAvgBright(ip, value);
}

void CSRccApp::SlotWhiteBalanceActionGroupTriggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	if (device_ptr->isSupportManualWhiteBalanceMode())
	{
		bool bSetting = false;
		HscResult res = HSC_OK;
		HisiAwbMode mode = AWB_MODE_NONE;
		HscWhiteBalanceMode processMode = HSC_WB_NONE;
		if (ui->actionNone->isChecked())
		{
			bSetting = true;
			mode = AWB_MODE_NONE;
			processMode = HSC_WB_NONE;
		}
		else if (ui->actionAuto->isChecked())
		{
			bSetting = true;
			mode = AWB_MODE_AUTO;
			processMode = HSC_WB_AUTO_GAIN_FROM_SOFTWARE;
		}
		else if (ui->actionManual->isChecked())
		{
			bSetting = true;
			mode = AWB_MODE_MANUAL;
			processMode = HSC_WB_MANUAL_GAIN;
		}
		else if (ui->actionSettingManual->isChecked())
		{
			ui->actionManual->setChecked(true);
			StreamType nType = device_ptr->getProperty(Device::PropStreamType).value<StreamType>();
			bool bChanged = false;
			if (nType != TYPE_RAW8)
			{
				bChanged = true;
				device_ptr->setProperty(Device::PropStreamType, TYPE_RAW8);
			}
			CSManualWhiteBalanceDlg dlg(device_ptr, this);
			dlg.exec();
			if (bChanged)
			{
				if (device_ptr->connected())
				{
					device_ptr->setProperty(Device::PropStreamType, nType);
				}
			}
		}
		if (bSetting)
		{
			device_ptr->ApplyWbMode(processMode);
			HscHisiManualAwb param;
			param.mode = AWB_MODE_NONE;
			HscResult res = device_ptr->GetManualWhiteBalance(param);
			if (res == HSC_OK)
			{
				param.mode = mode;
				res = device_ptr->SetManualWhiteBalance(param);
				if (res == HSC_OK)
				{
					if (param.mode == AWB_MODE_MANUAL)
					{
						HscColorCorrectInfo info;
						if (device_ptr->getColorCorrectInfo(info) == HSC_OK)
						{
							info.r_gain_ = CSManualWhiteBalanceDlg::changeU2F(param.r_gain);
							info.g_gain_ = CSManualWhiteBalanceDlg::changeU2F(param.gr_gain);
							info.b_gain_ = CSManualWhiteBalanceDlg::changeU2F(param.b_gain);
							if (device_ptr->setColorCorrectInfo(info) != HSC_OK)
							{
							}
						}
						ui->actionSettingManual->setEnabled(true);
					}
					else
					{
						ui->actionSettingManual->setEnabled(false);
					}
				}
			}
		}
		return;
	}
	//根据选中状态进行操作
	if (ui->actionNone->isChecked())
	{
		device_ptr->ApplyWbMode(HSC_WB_NONE);
	}
	else if (ui->actionAuto->isChecked())
	{
		device_ptr->ApplyWbMode(HSC_WB_AUTO_GAIN_FROM_SOFTWARE);
	}
	else if (ui->actionManual->isChecked())
	{
		device_ptr->ApplyWbMode(HSC_WB_MANUAL_GAIN);
		if (device_ptr->allowsEditArmWbMode())
		{
			//arm白平衡对话框
			CSDlgArmManualWhiteBalance dlg(device_ptr, this);
			dlg.exec();
		}
		else
		{
			//手动白平衡对话框
			CSDlgManualWhiteBalance dlg(device_ptr, this);
			dlg.exec();
		}
	}
}

void CSRccApp::SlotUpdateGrayOrRGBValue(const QPoint& pt, const cv::Mat& matImage)
{
	if (!matImage.empty()
		&& (0 <= pt.x() && pt.x() < matImage.cols && 0 <= pt.y()))
	{
		if (pt.y() < matImage.rows)
		{
			if (matImage.depth() == CV_16U)
			{
				int channels = matImage.channels();
				if (channels == 1) // gray
				{
					m_nGrayOrRGBAtQueryPoint = matImage.at<uint16_t>(pt.y(), pt.x());
				}
				else if (channels == 3) // BGR
				{
					cv::Vec3w vec = matImage.at<cv::Vec3w>(pt.y(), pt.x());
					bgr_values[0] = vec[0];
					bgr_values[1] = vec[1];
					bgr_values[2] = vec[2];
				}
				else if (channels == 4) // BGR
				{
					cv::Vec4w vec = matImage.at<cv::Vec4w>(pt.y(), pt.x());
					bgr_values[0] = vec[0];
					bgr_values[1] = vec[1];
					bgr_values[2] = vec[2];
				}
			}
			else
			{
				int channels = matImage.channels();
				if (channels == 1) // gray
				{
					m_nGrayOrRGBAtQueryPoint = matImage.data[pt.y()*matImage.cols + pt.x()];
				}
				else if (channels == 3 || channels == 4) // BGR/BGRA
				{
					int pos = (pt.y()*matImage.cols + pt.x()) * channels;
					bgr_values[0] = matImage.data[pos];
					bgr_values[1] = matImage.data[pos + 1];
					bgr_values[2] = matImage.data[pos + 2];
				}
			}
		}
		SetGrayOrRGB();
	}
	else
	{
		m_nGrayOrRGBAtQueryPoint = 0;
		SetGrayOrRGB(false);
	}
	
}

void CSRccApp::SlotUpdateFrameAutoExposureAvgGray(const QString &ip, const int value)
{
	emit SignalAutoExposureAvgGray(ip, value);
}

void CSRccApp::SlotAvgLum(const uint32_t avgLum)
{
	if (DeviceManager::instance().getCurrentDevice() == qobject_cast<Device*>(sender()))
	{
		SetAvgLum(avgLum);
	}
}

void CSRccApp::SlotAlarmMsg()
{
	UpdatAlarmActionsUI();
}

void CSRccApp::SlotErrMsg(int error_code)
{
	if (error_code == HSC_RECOVER_DATA_FAILED)
	{
		RecoveringDataFail();
		return;
	}
	QString caption = QObject::tr("Error Code");
	QString value = QString("0x%1").arg(error_code, 8, 16, QChar('0'));

	QString error_desc;
	int level = 0;
	AgErrorCodeInfo error_code_info{};
	if (AgErrorCode::instance().get(error_code, error_code_info))
	{
		error_desc = error_code_info.desc;
		level = error_code_info.level;
	}

	QString msg_text = QString("%1(%2): %3").arg(caption).arg(value).arg(error_desc);
	msg_text = msg_text.trimmed();
	if (msg_text.endsWith(":"))
	{
		msg_text.chop(1);
	}
	UIUtils::showInfoMsgBox(this, msg_text);
}

void CSRccApp::on_actionStatus_Bar_S_triggered(bool checked)
{
	ui->statusBar->setVisible(checked);
}

void CSRccApp::on_actionDevice_List_triggered(bool checked)
{
	ui->dockWidgetDevicelist->setVisible(checked);
}

void CSRccApp::on_actionProperty_List_triggered(bool checked)
{
	ui->dockWidgetPropertyList->setVisible(checked);
}

void CSRccApp::on_actionVideo_List_triggered(bool checked)
{
	ui->dockWidgetVideoList->setVisible(checked);
}

void CSRccApp::on_actionMeasure_triggered(bool checked)
{
	ui->dockWidgetPointMeasure->setVisible(checked);
}

void CSRccApp::on_actionTemperature_Panel_triggered(bool checked)
{
	ui->dockWidgetTemperature->setVisible(checked);
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropTemperaturePanelEnable, checked);
	}
}

void CSRccApp::on_actionControl_Panel_triggered(bool checked)
{
	ui->cf18_control_dockWidget->setVisible(checked);
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr)
	{
		device_ptr->setProperty(Device::PropCF18ControlPanelEnable, checked);
	}
}

void CSRccApp::slotLowLightModeTrigger()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (devicePtr)
	{
		uint32_t user_fps = devicePtr->getProperty(Device::PropType::PropFrameRate, true).toInt();
		uint32_t user_exposure_value = devicePtr->getProperty(Device::PropType::PropExposureTime, true).toInt();
		devicePtr->setProperty(Device::PropFrameRate, QVariant::fromValue(100));//低照度模式默认切换帧率到100

		CSLowLightMode lowLightMode;
		connect(&lowLightMode, &CSLowLightMode::signalLowLightValue, this, [=](int value) {
			devicePtr->setProperty(Device::PropType::PropLowLightMode, QVariant::fromValue(value));
		});
		uint32_t low_light_exposure_value = 0;

		low_light_exposure_value = devicePtr->getProperty(Device::PropType::PropLowLightMode).toInt();

		if (0 == low_light_exposure_value) {
			low_light_exposure_value = 1000;
		}
		devicePtr->setProperty(Device::PropType::PropLowLightMode, QVariant::fromValue(low_light_exposure_value));
		lowLightMode.setlightValue(low_light_exposure_value);
		lowLightMode.setFixedSize(lowLightMode.size());
		lowLightMode.exec();

		//回退参数
		devicePtr->setProperty(Device::PropFrameRate, QVariant::fromValue(user_fps));
		devicePtr->setProperty(Device::PropExposureTime, QVariant::fromValue(user_exposure_value));
	}

}

void CSRccApp::slotExportpreviewShowMain(bool bShow)
{
	if (bShow)
	{
		if (isMaximized())
		{
			showMaximized();
		}
		else 
		{
			showNormal();
		}
	}
	else
	{
		showMinimized();
	}
}

void CSRccApp::on_actionWindow_Tool_triggered(bool checked)
{
	ui->mainToolBar->setVisible(checked);
}

void CSRccApp::on_actionCamera_Acquisition_triggered(bool checked)
{
	ui->cameraAcqToolBar->setVisible(checked);
	if (m_trigger_toolbar)
	{
		m_trigger_toolbar->setVisible(checked);
	}
}

void CSRccApp::on_actionAcquisition_Window_triggered(bool checked)
{
	ui->acqWindowToolBar->setVisible(checked);
}

void CSRccApp::on_actionRun_Alarm_triggered(bool checked)
{
	ui->runAlarmToolBar->setVisible(checked);
}

void CSRccApp::on_actionUser_Manual_triggered(bool checked)
{
	ui->userManualToolBar->setVisible(checked);
}

void CSRccApp::on_actionEnlarge_triggered()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->ZoomIn();
	}	
}

void CSRccApp::on_actionZoom_Out_Z_triggered()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->ZoomOut();
	}
}

void CSRccApp::on_action1_1Show_triggered()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->FullView();
	}
}

void CSRccApp::on_actionFit_Window_triggered()
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->FitView();
	}
}

void CSRccApp::on_actionCrossHair_triggered(bool checked)
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->EnableDrawCrossLine(checked);
	}
}

void CSRccApp::on_actionWindow_Centerline_triggered(bool checked)
{
	if (checked != SystemSettingsManager::instance().isWindow_Centerline())
	{
		SystemSettingsManager::instance().setWindow_Centerline(checked);
	}
	auto videoList = GetViewManagerPtr()->GetDisplayList();
	if (videoList.empty())
		return;
	for (auto e : videoList)
	{
		e->EnableDrawCustomCrossLine(checked);
		QString deviceName = e->GetDeviceName();
		auto pDevice = DeviceManager::instance().getDevice(deviceName);
		if (pDevice)
		{
			pDevice->setProperty(Device::PropFocusPointVisible, checked);
		}
	}
}

void CSRccApp::on_actionWindow_Centerline_Settings_triggered()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (devicePtr) {
		m_winCrossLineSet->SetCustomCrossLineCenterPointLineEditText(devicePtr->getProperty(Device::PropFocusPoint).toPointF());
	}
	emit m_winCrossLineSet->SignalWindowCrossLineSettingOpened();
	m_winCrossLineSet->show();
}

void CSRccApp::on_actionPreview_P_triggered()
{
	if (DeviceManager::instance().getCurrentDevice())
	{
		auto device = DeviceManager::instance().getCurrentDevice();
		if (!device)
			return;
		GetViewManagerPtr()->AddToDisplayList(device->getIpOrSn());
		device->setShowTip(true);
		device->preview();
		if (device->isGrabberDevice()) {
			ui->actionAcquisition_Window_Settings->setEnabled(false);
			ui->actionMaximize_The_Acquisition_Window->setEnabled(false);
			ui->actionCenter_The_Acquisition_Window->setEnabled(false);
			ui->actionDisplay_The_Acquisition_Window->setEnabled(false);
		}
	}
}

void CSRccApp::on_actionTake_photo_triggered()
{
	GetViewManagerPtr()->Snap(this);
}

void CSRccApp::SlotUpdateDeviceRotate(const QString strIP)
{
	if (strIP.isEmpty())
	{
		return;
	}
	auto pDevice = DeviceManager::instance().getDevice(strIP);
	if (pDevice)
	{
		int iRotationType = pDevice->getProperty(Device::PropType::PropRotateType).toInt();

		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getDeviceView(strIP);
		if (pVideoPlayer)
		{
			pVideoPlayer->SetRotateType(ROTATE_TYPE(iRotationType));
		}
		if (pDevice == DeviceManager::instance().getCurrentDevice())
		{
			SetStatusBarRotateValue(iRotationType);
		}
	}
}

void CSRccApp::on_actionHigh_speed_Acquisition_triggered()
{
	if (m_current_device_ptr)
	{
		if (ui->actionAcquisition_Window_Settings->isChecked()) {
			ui->actionAcquisition_Window_Settings->trigger();
		}
		GetViewManagerPtr()->AddToDisplayList(m_current_device_ptr->getIpOrSn());
		m_current_device_ptr->setShowTip(true);
		m_current_device_ptr->acquire();
	}

}

void CSRccApp::on_actionTrigger_T_triggered()
{
	if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
	{
		if (m_current_device_ptr)
		{
			if (m_current_device_ptr->getState() == Acquiring)
			{
				m_current_device_ptr->trigger();
			}
			else if(m_current_device_ptr->getState() == Recording)//正在录制时点击触发,停机进入下一次高采
			{
				m_current_device_ptr->stopCapture(true, true);
			}
		}
	}
	else
	{
		DeviceManager::instance().SoftTriggerAllDevice();
	}
}

void CSRccApp::on_actionStop_Camera_S_triggered()
{
	if (DeviceManager::instance().getCurrentDevice())
	{
		if (DeviceManager::instance().getCurrentDevice()->getState() == Recording &&
			SystemSettingsManager::instance().isRecordingStopEnabled())
		{
			if (!DeviceManager::instance().getCurrentDevice()->isSupportRecingStop())
			{
				if (UIUtils::showQuestionMsgBox(this,tr("After stopping, the camera does not save data recorded before the stop time. Confirm to stop recording ?"),1))
				{
					DeviceManager::instance().getCurrentDevice()->stop(0);
				}
				return;
			}
			DeviceManager::instance().getCurrentDevice()->setShowTip(true);
			DeviceManager::instance().getCurrentDevice()->stopCapture(true);//正在录制时停止录制并保存视频
		}
		else
		{
			DeviceManager::instance().getCurrentDevice()->stop(0);
		}
	}
}

void CSRccApp::on_actionImage_Training_E_triggered()
{
	if (m_current_device_ptr.isNull())
	{
		return;
	}
	if (m_dlg_imageTraining.isNull())
	{
		m_dlg_imageTraining = new CSDlgImageTrainning();
	}
	//设置当前设备,并且使能操作
	m_dlg_imageTraining->setDevice(m_current_device_ptr);
	m_dlg_imageTraining->show();
}

void CSRccApp::on_actionSet_Up_Darkfield_triggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//判断状态是否支持矫正
	if (device_ptr->getState() != Connected || device_ptr->getProperty(Device::PropBlackFieldEnable).toBool())
	{
		UIUtils::showErrorMsgBox(this, tr("Dark field correction failed.\
 Please check whether the device is stopped, whether the dark field correction enable is turned off."));
		return;
	}
	//确认是否矫正
	if (UIUtils::showQuestionMsgBox(this, tr("The operation will start the dark field correction of the device[%1]. Continue?").arg(device_ptr->getIpOrSn())))
	{
		//开始矫正
		HscResult res = device_ptr->BlackFieldCalibration();
		if (res == HSC_OK)
		{
			UIUtils::showInfoMsgBox(this, tr("Dark field correction was successful."));
		}
		else
		{
			UIUtils::showErrorMsgBox(this, tr("Dark field correction failed."));
		}
	}
}

void CSRccApp::on_action_DarkFieldCorrection_triggered(bool checked)
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	device_ptr->setProperty(Device::PropBlackFieldEnable, checked);

}

void CSRccApp::on_actionGain_Setting_triggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//进入设置对话框
	if (device_ptr->IsSupportIsDigitalGainSupportedV3()&& device_ptr->isGrabberDevice())
	{
		CSDlgGainSettingV4 dlg(device_ptr, this);
		dlg.exec();
	}
	else if (device_ptr->IsSupportIsDigitalGainSupportedV3())
	{
		CSDlgGainSettingV3 dlg(device_ptr, this);
		dlg.exec();
	}
	else if (device_ptr->IsDigitalGainSupported())
	{
		CSDlgGainSettingV2 dlg(device_ptr,this);
		dlg.exec();
	}
	else
	{
		CSDlgGainSetting dlg(device_ptr,this);
		connect(this, &CSRccApp::signalDisconnect2GainSet, &dlg, &CSDlgGainSetting::slotDisconnect2GainSet);
		dlg.exec();
	}
}

void CSRccApp::on_actionBrightness_Contrast_triggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//进入设置对话框
	CSDlgLuminanceAndContrastSetting dlg(device_ptr,this);
	dlg.exec();
}



void CSRccApp::on_actionExit_X_triggered()
{
	close();
}


void CSRccApp::on_action_O_triggered()
{
	//打开程序运行选项
	CSDlgSetOption dlg(this);
	dlg.exec();
}

void CSRccApp::on_actionMachine_Self_check_triggered()
{
	//打开整机自检对话框
	CSDlgSelfCheck dlg(this);
	dlg.exec();
}

void CSRccApp::on_actionHealth_Management_Settings_triggered()
{
    //打开健康管理设置对话框
    CSDlgHealthManagerSetting dlg(this);
    dlg.exec();

}

void CSRccApp::on_actionNew_File_N_triggered()
{
	//新建实验项目
	CSDlgNewExp dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		//设置当前实验信息,刷新界面标题
		SystemSettingsManager::instance().setCurrentExperiment(dlg.getExperimentInfo());
		UpdateAppTitle(dlg.getExperimentInfo().name, dlg.getExperimentInfo().code);
	}
}

void CSRccApp::on_actionOpen_File_O_triggered()
{

	//打开对话框 在工作路径下寻找全部项目,判断选中
	//将实验项目的ini文件复制到默认路径中
	CSDlgOpenExp dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		//设置当前实验信息,刷新界面标题
		SystemSettingsManager::instance().loadCurrentExperimentInfo();
		UpdateAppTitle(SystemSettingsManager::instance().getCurrentExperiment().name, 
			SystemSettingsManager::instance().getCurrentExperiment().code);
		//已连接设备重新加载ini文件中的参数
		DeviceManager::instance().ReloadAllDeviceConfigFromLocal();
		SlotUpdateUIInfo();
		
	}

}

void CSRccApp::on_actionSave_File_S_triggered()
{
	//判断当前是否有实验项目信息
	if (SystemSettingsManager::instance().getCurrentExperiment().code.isEmpty())
	{
		UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_STD_MSG_NO_CODE"));
		return;
	}


	//有实验信息,将信息写入设置中
	QFile::remove(SystemSettingsManager::instance().getCurrentExperimentDir());
	SystemSettingsManager::instance().saveCurrentExperimentInfo();


	//创建实验对应路径,复制ini文件
	QDir work_path = QDir(SystemSettingsManager::instance().getWorkingDirectory());

	if (work_path.exists())
	{
		QString ini_dir;
		QString ini_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QStorageInfo storage_info = QStorageInfo(ini_path);
		auto disk_size = storage_info.bytesAvailable();
		if (disk_size <= 1024 * 1024) 
		{
			UIUtils::showInfoMsgBox(this, tr("Out Of Disk Space!"));
			return;
		}
		ini_path += '/';
		ini_path += SystemSettingsManager::instance().getCurrentExperiment().code;

		ini_dir = ini_path + '/';
		ini_path += "/Revealer_Camera_Control.ini";
		if (work_path.mkpath(ini_dir))//创建实验路径
		{
			if (QFile::exists(ini_path))
			{
				QFile::remove(ini_path);
			}
			if (QFile::copy(SystemSettingsManager::instance().getCurrentExperimentDir(), ini_path))
			{
				UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_STD_MSG_SAVE_OK"));
				return;
			}
		}
	}

	UIUtils::showErrorMsgBox(this, UIExplorer::instance().getStringById("STRID_STD_MSG_SAVE_FAIL"));
	return;

}

void CSRccApp::on_actionDevice_Authorization_triggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (device_ptr.isNull())
	{
		return;
	}
	//进入对话框
	CSDlgDeviceAuth dlg(device_ptr, this);
	if (dlg.exec() == QDialog::Accepted)
	{
		//选择了确定,开始授权
		HscResult res = device_ptr->setLicensePath(dlg.GetAuthFileName());
		if (res == HSC_OK)
		{
			UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_AUTH_MSG_OK"));
		}
		else
		{
			ErrorCodeUtils::handle(this, res);//授权报错
		}
	}
}

void CSRccApp::closeEvent(QCloseEvent *event)
{
	QString line1 = tr("Confirm close?");
	QString line2 = tr("After clicking OK, the system will disconnect all devices and close the software.");
	bool ok = UIUtils::showQuestionMsgBox(this, QString("%1\r\n%2").arg(line1).arg(line2), 1);
	if (ok)
	{

		// 关闭时断开所有设备信号连接，防止软件关闭时程序崩溃
		QList<QSharedPointer<Device>> devices;
		DeviceManager::instance().getDevices(devices);
		for  (auto var : devices)
		{
			var->QObject::disconnect();
		}
		//断开全部设备连接
		DeviceManager::instance().disconnectAll();

		// 图像训练窗口关闭
		if (!m_dlg_imageTraining.isNull())
		{
			m_dlg_imageTraining->close();
			m_dlg_imageTraining.clear();
		}


		//保存窗口当前状态
		SystemSettingsManager::instance().saveWindowState(saveState());
		if (ui->dockWidgetDevicelist->isFloating())
		{
			m_device_list_widget->close();
			m_device_search_list_widget->close();
		}
		if (ui->dockWidgetPropertyList->isFloating())
		{
			ui->dockWidgetPropertyList->close();
		}
		if (ui->dockWidgetVideoList->isFloating())
		{
			ui->dockWidgetVideoList->close();
		}
		if (ui->dockWidgetPointMeasure->isFloating())
		{
			ui->dockWidgetPointMeasure->close();
		}
		if (ui->dockWidgetTemperature->isFloating())
		{
			ui->dockWidgetTemperature->close();
		}
		if (ui->cf18_control_dockWidget->isFloating()) {
			ui->cf18_control_dockWidget->close();
		}
		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void CSRccApp::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)//空格触发
	{
		if (FunctionCustomizer::GetInstance().isXiguangsuoVersion())
		{
			//西光所版本不支持空格触发
		}
		else
		{
			DeviceManager::instance().SoftTriggerAllDevice();
		}
	}
	else if (event->key() == Qt::Key_Escape)
	{
		if (ui->actionAcquisition_Window_Settings->isEnabled() && ui->actionAcquisition_Window_Settings->isChecked())
		{
			ui->actionAcquisition_Window_Settings->setChecked(false);
		}
		auto device_ptr = DeviceManager::instance().getCurrentDevice();
		if (device_ptr)
		{
			auto status = device_ptr->getDrawTypeStatusInfo();
			if (status != Device::DTSI_Noraml)
			{
				device_ptr->setDrawTypeStatusInfo(Device::DTSI_Noraml);
				CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getDeviceView(device_ptr->getIpOrSn());
				if (pVideoPlayer)
				{
					if (status == Device::DTSI_DrawROI)
					{
						pVideoPlayer->setFeaturePointVisible(Device::RoiTypes::kDeviceRoi, false);
					}
				}
			}
			else {
				if (paramSettingPtr) 
				{
					emit SignalCancelEdit();
				}
			}
		}
		m_measure_page_dlg_widget->EscKeyPress();
	}
	QMainWindow::keyPressEvent(event);
}

void CSRccApp::keyReleaseEvent(QKeyEvent * event)
{
    bool caps_state = false;
    bool num_state = false;
    bool scrl_state = false;
#ifdef Q_OS_WIN32
    caps_state = LOBYTE(GetKeyState(VK_CAPITAL));
    num_state = LOBYTE(GetKeyState(VK_NUMLOCK));
    scrl_state = LOBYTE(GetKeyState(VK_SCROLL));
#else
    Display* d = XOpenDisplay((char*)0);
    if(d)
    {
        unsigned n;
        XkbGetIndicatorState(d,XkbUseCoreKbd,&n);
        caps_state = (n == 1);
        num_state = (n == 2);
        if(n==3)
        {
            caps_state = true;
            num_state = true;
        }
    }
#endif
	if (event->key() == Qt::Key_CapsLock)
	{
        UpdateCapLabelStatus(caps_state);
	}

	if (event->key() == Qt::Key_NumLock)
	{
        UpdateNumLabelStatus(num_state);
	}

	if (event->key() == Qt::Key_ScrollLock)
	{
        UpdateScrlLabelStatus(scrl_state);
	}
	QMainWindow::keyReleaseEvent(event);
}

bool CSRccApp::eventFilter(QObject *obj, QEvent *event)
{
	if (isMaximized()) {
		return QMainWindow::eventFilter(obj, event);
	}

	if (event->type() == QEvent::NonClientAreaMouseMove)
	{
		update();
	}
	return QMainWindow::eventFilter(obj, event);
}

bool CSRccApp::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
	//判断win系统下有没有进入休眠状态
#ifdef Q_OS_WIN32
	if (eventType == "windows_generic_MSG"|| eventType == "windows_dispatcher_MSG")
	{
		MSG *pMsg = reinterpret_cast<MSG*>(message);
		if (pMsg)
		{
			if (pMsg->message == WM_POWERBROADCAST)
			{
				if (pMsg->wParam == PBT_APMSUSPEND)//睡眠
				{
					CSLOG_INFO("system sleep");
					SystemSettingsManager::instance().setSystemSleepState(true);
				}
				if (pMsg->wParam == PBT_APMRESUMEAUTOMATIC)
				{
					CSLOG_INFO("system awake");
					SystemSettingsManager::instance().setSystemSleepState(false);

				}
			}
		}
	}
#else

#endif // Q_OS_WIN32
	return false;
}

void CSRccApp::UpdateStatusBarTranslate()
{
	SetZoomCoefficient(m_fZoomCoefficient);
	SetFrameRate(m_iFrameRate);
	//if (m_bBarRotateVisible)
	{
		SetStatusBarRotateValue(m_iBarRotate);
	}
	SetGrayOrRGB(m_bGrayOrRGB);
	SetAvgLum(m_uAvgLum);
	SetBcode(m_BcodeOn);
}

void CSRccApp::UpdateCapLabelStatus(bool bLight)
{
	style()->unpolish(m_pCapLabel);
	bLight ? m_pCapLabel->setObjectName("numLabel") : m_pCapLabel->setObjectName("capLabel");
	style()->polish(m_pCapLabel);
}

void CSRccApp::UpdateNumLabelStatus(bool bLight)
{
	style()->unpolish(m_pNumLabel);
	bLight ? m_pNumLabel->setObjectName("numLabel") : m_pNumLabel->setObjectName("capLabel");
	style()->polish(m_pNumLabel);
}

void CSRccApp::UpdateScrlLabelStatus(bool bLight)
{
	style()->unpolish(m_pScrlLabel);
	bLight ? m_pScrlLabel->setObjectName("numLabel") : m_pScrlLabel->setObjectName("capLabel");
	style()->polish(m_pScrlLabel);
}

void CSRccApp::updateImageCtrlUI()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (!device)
	{
		m_actionGroup_ImageCtrl->setEnabled(false);
		return;
	}
	if (!device->IsImageCtrlSupported() || !device->AllowsImageCtrl()) {
		m_actionGroup_ImageCtrl->setEnabled(false);
		auto checked_act = m_actionGroup_ImageCtrl->checkedAction();
		if (checked_act)
			checked_act->setChecked(false);
		return;
	}
	if (device->AllowsImageCtrl())
	{
		m_actionGroup_ImageCtrl->setEnabled(true);
	}
	auto image_ctrl_param = device->getProperty(Device::PropImageCtrl).value<HscImageCtrlParam>();
	if (image_ctrl_param.enableFlipImage) {//镜像、倒像全选就是翻转
		ui->actionFlip_The_Image->setChecked(true);
	}
	else if (image_ctrl_param.enableMirrorImage){
		ui->actionMirror->setChecked(true);
	}
	else if (image_ctrl_param.enableInvertImage) {
		ui->actionInverted_Image->setChecked(true);
	}
}

void CSRccApp::on_actionDrop_Measurement_Settings_triggered()
{
    //rgq
	//落点测量设置
    //CSDlgFallPointMeasureSetup dlg(this);
    //dlg.exec();

}


void CSRccApp::on_actionDisplay_The_Acquisition_Window_triggered(bool checked)
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device) {
		device->setProperty(Device::PropRoiVisible, QVariant::fromValue(checked));
		CPlayerViewBase *pPlayerView = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
		if (pPlayerView) {
			pPlayerView->setRoiVisible(Device::RoiTypes::kDeviceRoi, checked);
		}
	}
}

void CSRccApp::on_actionMaximize_The_Acquisition_Window_triggered()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device && device->allowEditRoi())
	{
		device->setProperty(Device::PropRoi, QVariant::fromValue(device->GetMaxRoi(Device::kDeviceRoi)));
		// [2022/8/31 rgq]: 修改ROI时同步修改自动曝光区域、智能触发区域
		QRect roiRect = device->getProperty(Device::PropRoi).toRect();
		if (device->IsAutoExposureSupported())
		{
			HscAutoExposureParameter param = device->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			param.autoExpArea = Device::QRectTCameraWindowRect(roiRect);
			device->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		}
		if (device->IsIntelligentTriggerSupported())
		{
			HscIntelligentTriggerParamV2 param = device->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			if ((device->IsSupportCMOSDigitGain() || device->IsIntelligentTriggerV5Supported()) && !device->isGrabberDevice())
			{
				QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
				param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), roiRect);
			}
			param.roi = Device::QRectTCameraWindowRect(roiRect);
			device->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
		}
		m_property_view_device_widget->UpdateRecordingLengthEnable();
	}
}

void CSRccApp::on_actionCenter_The_Acquisition_Window_triggered()
{
	m_mid_btn_clicked = true;
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device  && device->allowEditRoi()) {
		device->setProperty(Device::PropRoi, QVariant::fromValue(device->getCenterRoi(Device::kDeviceRoi)));
		// [2022/8/31 rgq]: 修改ROI时同步修改自动曝光区域、智能触发区域
		QRect roiRect = device->getProperty(Device::PropRoi).toRect();
		if (device->IsAutoExposureSupported())
		{
			HscAutoExposureParameter param = device->getProperty(Device::PropAutoExposure).value<HscAutoExposureParameter>();
			param.autoExpArea = Device::QRectTCameraWindowRect(roiRect);
			device->setProperty(Device::PropAutoExposure, QVariant::fromValue(param));
		}
		if (device->IsIntelligentTriggerSupported())
		{
			HscIntelligentTriggerParamV2 param = device->getProperty(Device::PropIntelligentTrigger).value<HscIntelligentTriggerParamV2>();
			if ((device->IsSupportCMOSDigitGain() || device->IsIntelligentTriggerV5Supported()) && !device->isGrabberDevice())
			{
				QRect old_roi = Device::CameraWindowRect2QRect(param.roi);
				param.pixel_number_thres = Device::PercentageToCount(Device::CountToPercentage(param.pixel_number_thres, old_roi), roiRect);
			}
			param.roi = Device::QRectTCameraWindowRect(roiRect);
			device->setProperty(Device::PropIntelligentTrigger, QVariant::fromValue(param));
		}
		m_property_view_device_widget->UpdateRecordingLengthEnable();

		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
		if (pVideoPlayer)
		{
			RoiInfo roi_info;
			roi_info.roi_type = Device::RoiTypes::kDeviceRoi;
			roi_info.roi_rect = GetRoiTypeRect(Device::kDeviceRoi, device);
			pVideoPlayer->drawRoiRect(roi_info);
		}
	}
}

void CSRccApp::on_actionAcquisition_Window_Settings_triggered(bool checked)
{
	CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		if (checked && !ui->actionDisplay_The_Acquisition_Window->isChecked()) {
			ui->actionDisplay_The_Acquisition_Window->setChecked(checked);
			pVideoPlayer->setRoiVisible(Device::kDeviceRoi, checked);
		}	
		pVideoPlayer->setFeaturePointVisible(Device::RoiTypes::kDeviceRoi, checked);
		auto pDevice = DeviceManager::instance().getCurrentDevice();
		if (pDevice)
		{
			if (checked)
			{
				pVideoPlayer->setRoiConstraintCondition((int)pDevice->GetRoiConstraintCondition());
			}
			auto drawStatus = Device::DTSI_Noraml;
			if (checked)
			{
				drawStatus = Device::DTSI_DrawROI;
			}
			pDevice->setDrawTypeStatusInfo(drawStatus);
		}
		UpdatAlarmActionsUI();
		updateAcquisitionActionsUI();
	}
}

void CSRccApp::on_action_Intelligent_Param_Setting_triggered()
{
	if (paramSettingPtr) {
		paramSettingPtr->show();
		paramSettingPtr->close();
		this->metaObject()->invokeMethod(this, "on_action_Intelligent_Param_Setting_triggered", Qt::QueuedConnection);
		return;
		//QApplication::processEvents(QEventLoop::AllEvents);
	}
    auto device = DeviceManager::instance().getCurrentDevice();
    if (device && device->AllowsSetIntelligentTrigger())
    {
		setDeviceRoiEditDisabled();
        CSParamSetting setting(Device::PropIntelligentTrigger,device);
		paramSettingPtr = &setting;
		m_ExType = Device::kIntelligentTriggerRoi;
		connect(this, &CSRccApp::SignalIntelligentAvgBright, &setting, &CSParamSetting::SlotIntelligentAvgBright/*, Qt::QueuedConnection*/);
		connect(this, &CSRccApp::SignalForbid, &setting, &CSParamSetting::SlotForbid/*, Qt::QueuedConnection*/);
#if 1
		QEventLoop loop;
		connect(&setting, &CSParamSetting::SignalManualDrawRoi, [&setting, &loop, this, device]() {// 进入绘制 隐藏窗口 只允许绘制
			auto view = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
			if (view) {
				view->setFeaturePointVisible(Device::kIntelligentTriggerRoi, true);
				UpdatAlarmActionsUI();
				updateAcquisitionActionsUI();
				device->setDrawTypeStatusInfo(Device::DTSI_DrawIntelligentTrigger);
			}
			setting.hide();
		});
		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
		if (pVideoPlayer) {
			connect(pVideoPlayer, &CPlayerViewBase::SignalExitDrawRoi, &setting, [&setting]() {// 绘制完成 重新显示窗口
				setting.setModal(true);
				setting.show();
			});
			pVideoPlayer->updateItemLayer(Device::kIntelligentTriggerRoi);
		}
		connect(this->ui->actionAcquisition_Window_Settings, &QAction::triggered, &loop, [&loop]() {// 采集窗口绘制时 退出设置
			loop.exit();
		});
		connect(device.data(), &Device::stateChanged, &loop, [&loop](const QString &, DeviceState) {// 状态变化时 退出设置
			loop.exit();
		});
		connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, &loop, [&loop](const QString &) {// 切换设备 退出绘制
			loop.exit();
		},Qt::QueuedConnection);
		connect(&setting, &CSParamSetting::finished, [&loop]() {// 关闭窗口时 退出设置
			loop.exit();
		});
		connect(this, &CSRccApp::SignalCancelEdit, [&loop]() {// ESC 退出设置
			loop.exit();
		});
		setting.setModal(true);
		setting.show();
		loop.exec();
		if (pVideoPlayer){
			pVideoPlayer->setFeaturePointVisible(Device::kIntelligentTriggerRoi, false);
		}
		if (device->getDrawTypeStatusInfo() == Device::DTSI_DrawIntelligentTrigger)
			device->setDrawTypeStatusInfo(Device::DTSI_Noraml);
		paramSettingPtr = nullptr;
#endif // 1

#if 0
        if (setting.exec() == QDialog::Accepted)//代表手动绘制ROI
        {
            auto view = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
            if (view) {
				view->setFeaturePointVisible(Device::kIntelligentTriggerRoi, true);
				UpdatAlarmActionsUI();
				updateAcquisitionActionsUI();
				device->setDrawTypeStatusInfo(Device::DTSI_DrawIntelligentTrigger);
            }
        }
#endif // 0

    }
}

void CSRccApp::on_action_AutoExposure_Param_Setting_triggered()
{
	if (paramSettingPtr) {
		paramSettingPtr->show();
		paramSettingPtr->close();
		this->metaObject()->invokeMethod(this, "on_action_AutoExposure_Param_Setting_triggered", Qt::QueuedConnection);
		return;
		//QApplication::processEvents(QEventLoop::AllEvents);
	}
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device && device->AllowsSetIntelligentTrigger())
	{
		setDeviceRoiEditDisabled();
		CSParamSetting setting(Device::PropAutoExposure, device);
		paramSettingPtr = &setting;
		m_ExType = Device::kAutoExposureRoi;
		connect(this, &CSRccApp::SignalAutoExposureAvgGray, &setting, &CSParamSetting::SlotAutoExposureAvgGray/*, Qt::QueuedConnection*/);
		connect(this, &CSRccApp::SignalForbid, &setting, &CSParamSetting::SlotForbid/*, Qt::QueuedConnection*/);

#if 1
		QEventLoop loop;
		connect(&setting, &CSParamSetting::SignalManualDrawRoi, [&setting, this, device]() {// 进入绘制 隐藏窗口 只允许绘制
			auto view = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
			if (view) {
				view->setFeaturePointVisible(Device::kAutoExposureRoi, true);
				UpdatAlarmActionsUI();
				updateAcquisitionActionsUI();
				device->setDrawTypeStatusInfo(Device::DTSI_DrawAutoExposure);
			}
			setting.hide();
		});
		CPlayerViewBase *pVideoPlayer = GetViewManagerPtr()->getSelectView();
		if (pVideoPlayer) {
			connect(pVideoPlayer, &CPlayerViewBase::SignalExitDrawRoi, &setting, [&setting]() {// 绘制完成 重新显示窗口
				setting.setModal(true);
				setting.show();
			});
			pVideoPlayer->updateItemLayer(Device::kAutoExposureRoi);
		}
		connect(this->ui->actionAcquisition_Window_Settings, &QAction::triggered, &loop,[&loop]() {// 采集窗口绘制时 退出设置
			loop.exit();
		});
		connect(device.data(), &Device::stateChanged, &loop, [&loop](const QString &, DeviceState) {// 状态变化时 退出设置
			loop.exit();
		});
		connect(&DeviceManager::instance(), &DeviceManager::currentDeviceChanged, &loop, [&loop](const QString &) {// 切换设备 退出绘制
			loop.exit();
		}, Qt::QueuedConnection);
		connect(&setting, &CSParamSetting::finished, [&loop]() {// 关闭窗口时 退出设置
			loop.exit();
		});
		connect(this, &CSRccApp::SignalCancelEdit, [&loop]() {// ESC 退出设置
			loop.exit();
		});
		setting.setModal(true);
		setting.show();
		loop.exec();
		if (pVideoPlayer) {
			pVideoPlayer->setFeaturePointVisible(Device::kAutoExposureRoi, false);
		}
		if(device->getDrawTypeStatusInfo() == Device::DTSI_DrawAutoExposure)
			device->setDrawTypeStatusInfo(Device::DTSI_Noraml);
		paramSettingPtr = nullptr;
#endif // 1

#if 0
		if (setting.exec() == QDialog::Accepted)
		{
			auto view = GetViewManagerPtr()->getDeviceView(device->getIpOrSn());
			if (view) {
				view->setFeaturePointVisible(Device::kAutoExposureRoi, true);
				UpdatAlarmActionsUI();
				updateAcquisitionActionsUI();
				device->setDrawTypeStatusInfo(Device::DTSI_DrawAutoExposure);
			}
		}
#endif // 0

	}
}

void CSRccApp::on_action_EdrExposure_Param_Setting_triggered()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (nullptr != device)
	{
		CSEdrexposureParamSetting setting(device, m_iEdrExploreTimeValue);
		connect(this, &CSRccApp::SignalEdrForbid, &setting, &CSEdrexposureParamSetting::SlotEdrForbid);
		setting.exec();
	}
}

void CSRccApp::on_actionRotate_The_Image_R_triggered()
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (!device_ptr.isNull())
	{
		HscRotationType rotationType = HscRotationType(device_ptr->getProperty(Device::PropType::PropRotateType).toInt());
		if (rotationType + 1 > HSC_ROTATION_CW270)
		{
			rotationType = HSC_ROTATION_NONE;
		}
		else
		{
			rotationType = HscRotationType(rotationType + 1);
		}
		device_ptr->setProperty(Device::PropType::PropRotateType,QVariant::fromValue(rotationType));
		SetStatusBarRotateValue(device_ptr->getProperty(Device::PropType::PropRotateType).toInt());
	}
}


void CSRccApp::on_action_Menu_LocalVideo_triggered()
{
#if 1
#ifdef Q_OS_WIN32
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	wchar_t szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, MAX_PATH);
	PathRemoveFileSpec(szCurrentDir);
	static const wchar_t szAppName[] = L"AGReplayer.exe";
	QString strFileName = SystemSettingsManager::instance().getWorkingDirectory();
	wchar_t szCmd[MAX_PATH] = { 0 };
	auto language = CSRccAppTranslator::instance().instance().getLanguage();
	std::wstring strLanguage = L"zh_CN";
	if (QLocale::Language::English == language)
		strLanguage = L"en_US";
	else if (QLocale::Language::Japanese == language)
		strLanguage = L"ja_JA";
	swprintf_s(szCmd, L"\"%ws\\%ws\" %ws Language=%ws", szCurrentDir, szAppName, strFileName.toStdWString().c_str(), strLanguage.c_str());

	//call UpdateGradeTool.exe application
	if (FALSE == CreateProcess(nullptr, szCmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
		QMessageBox msgBox(this);
		QString strMsg = tr("%1 start record player failed, error code:%2").arg(GetLastError());
		msgBox.setWindowTitle(QObject::tr("RCC"));
		msgBox.setText(strMsg);
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.addButton(tr("OK"), QMessageBox::YesRole);
		msgBox.exec();
	}
#endif // Q_OS_WIN32
#else
	CSMenuLocalVideo dlg(this);

	dlg.SetVideoDir(SystemSettingsManager::instance().getWorkingDirectory());
	
	if (dlg.exec() == QDialog::Accepted)
	{
		emit SignalLocalVideoDispaly(dlg.GetDisplayVideoPath());
	}
#endif
}




void CSRccApp::on_actionAnti_aliasing_triggered(bool checked)
{
	auto pVideoPlayer = GetViewManagerPtr()->getSelectView();
	if (nullptr != pVideoPlayer)
	{
		pVideoPlayer->SetAntiAliasing(checked);
	}
}

void CSRccApp::on_actionBinning_Mode_triggered(bool checked)
{
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (!device_ptr.isNull())
	{
		HscDisplayMode displayMode = device_ptr->getProperty(Device::PropDisplayMode).value<HscDisplayMode>();
		if (displayMode == HSC_DISPLAY_MONO)
		{
			device_ptr->SetBinningMode(checked);
		}
	}
}
void CSRccApp::on_actionCompany_Website_W_triggered()
{
	QString filepath;
	if (ui->actionEnglish->isChecked())
	{
		filepath = "http://en.gaosuxiangji.com/";
	}
	else
	{
		filepath = "http://www.gaosuxiangji.com";
	}

	bool ok = QDesktopServices::openUrl(QUrl(filepath));
	if (!ok)
	{
		UIUtils::showInfoMsgBox(this, tr("Company Website Url Not Found."));
	}
}

void CSRccApp::on_actionSoftware_Version_S_triggered()
{
	CSSoftwareVersion softVersion(this);
	softVersion.exec();
}

void CSRccApp::on_actionUser_Manual_D_triggered()
{

#ifdef Q_OS_WIN32
    QString filepath = QApplication::applicationDirPath() + QString("/") + tr("User Manual") + QString(".pdf");
#else
    QString filepath = QApplication::applicationDirPath() + QString("/../doc/") + tr("User Manual") + QString(".pdf");
#endif

	bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
	if (!ok)
	{
		UIUtils::showInfoMsgBox(this, tr("User Manual Url Not Found."));
	}
}

void CSRccApp::on_actionNetwork_Configuration_triggered()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (nullptr != device)
	{
		if (m_is_gateway_device) {
			QSet<QString> all_ip_set{};
			QList<QSharedPointer<Device>> devices_{};
			DeviceManager::instance().getDevices(devices_);
			for (auto item : devices_) {
				all_ip_set.insert(item->getIpOrSn());
			}
			CSNetConfig::NetParam param;
			param.strIp = device->getDeviceInfo().ip;

			param.strMask = device->getDeviceInfo().mask_addr;
			param.strGate = device->getDeviceInfo().gateway_addr;
			param.trMac = (QString("%1-%2-%3-%4-%5-%6").arg(device->getDeviceInfo().mac_addr[0], 0, 16).arg(device->getDeviceInfo().mac_addr[1], 0, 16).\
				arg(device->getDeviceInfo().mac_addr[2], 0, 16).arg(device->getDeviceInfo().mac_addr[3], 0, 16).\
				arg(device->getDeviceInfo().mac_addr[4], 0, 16).arg(device->getDeviceInfo().mac_addr[5], 0, 16)).toUpper();
			CSNetConfig netConfig(param, this);
			netConfig.setGatewayDeviceTip(true);
			netConfig.setAllDeviceIp(all_ip_set);
			int code = netConfig.exec();
			if (QDialog::Accepted == code) {
				CSNetConfig::NetParam netConfigParam = netConfig.GetNetConfigParam();
				HscResult err = device->SetNetConfigByGateway(device->getDeviceInfo().mac_addr, netConfigParam.strIp.toStdString().c_str(),
					netConfigParam.strMask.toStdString().c_str(), netConfigParam.strGate.toStdString().c_str());
				UIUtils::showInfoMsgBox(this, tr("The network configuration of the device has been modified successfully.\
				Please refresh the device list."));
				/*if (HscResult::HSC_OK == err) {
					UIUtils::showInfoMsgBox(this, tr("The network configuration of the device has been modified successfully.\
				Please refresh the device list."));
				}
				else {
					UIUtils::showInfoMsgBox(this, tr("The network configuration of the device has been modified failed."));
				}*/
			}
		}
		else {
			HscDeviceNetParam netParam;
			device->GetNetConfig(netParam);

			QSet<QString> all_ip_set{};
			QList<QSharedPointer<Device>> devices_{};
			DeviceManager::instance().getDevices(devices_);
			for (auto item : devices_) {
				all_ip_set.insert(item->getIpOrSn());
			}

			CSNetConfig::NetParam param;
			param.strIp = QString("%1.%2.%3.%4").arg(netParam.ipAddr[0]).arg(netParam.ipAddr[1]).arg(netParam.ipAddr[2]).\
				arg(netParam.ipAddr[3]);
			param.strMask = QString("%1.%2.%3.%4").arg(netParam.subnetMask[0]).arg(netParam.subnetMask[1]).\
				arg(netParam.subnetMask[2]).arg(netParam.subnetMask[3]);
			param.strGate = QString("%1.%2.%3.%4").arg(netParam.gateway[0]).arg(netParam.gateway[1]).arg(netParam.gateway[2]).\
				arg(netParam.gateway[3]);
			param.trMac = (QString("%1-%2-%3-%4-%5-%6").arg(netParam.macAddr[0], 0, 16).arg(netParam.macAddr[1], 0, 16).arg(netParam.macAddr[2], 0, 16).\
				arg(netParam.macAddr[3], 0, 16).arg(netParam.macAddr[4], 0, 16).arg(netParam.macAddr[5], 0, 16)).toUpper();

			CSNetConfig netConfig(param, this);
			netConfig.setAllDeviceIp(all_ip_set);
			int code = netConfig.exec();
			if (QDialog::Accepted == code)
			{
				CSNetConfig::NetParam netConfigParam = netConfig.GetNetConfigParam();
				QString2UCharArray(netConfigParam.strIp.split("."), netParam.ipAddr);
				QString2UCharArray(netConfigParam.strMask.split("."), netParam.subnetMask);
				QString2UCharArray(netConfigParam.strGate.split("."), netParam.gateway);

				HscResult res = device->SetNetConfig(netParam);
				if (HSC_OK == res)
				{
					UIUtils::showInfoMsgBox(this, tr("The network configuration of the device has been modified successfully.\
Please restart the device and refresh the device list."));
				}
			}
		}
	}
}

void CSRccApp::on_actionReset_triggered()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device)
	{
		QMessageBox infoBox(this);
		infoBox.setWindowTitle(QObject::tr("RCC"));
		infoBox.setIcon(QMessageBox::Information);
		infoBox.addButton(tr("OK"), QMessageBox::YesRole);
		infoBox.addButton(tr("NO"), QMessageBox::NoRole);
		infoBox.setText(tr("After %1 restores the factory settings, the device parameters will be restored to the factory settings, and the video list will be cleared. After this operation, the device needs to cut off power and reboot. Are you sure to continue?").arg(device->getIpOrSn()));
		int iRet = infoBox.exec();
		if (1 == iRet)
		{
			return;
		}

		if (Previewing == device->getState() || Acquiring == device->getState() || Recording == device->getState())
		{
			device->stop();
		}
	
		HscResult res = device->RestoreFactorySetting();
		if (res != HSC_OK)
		{
			UIUtils::showErrorMsgBox(this, UIExplorer::instance().getStringById("STRID_RFS_MSG_FAILED"));
		}
		else
		{
			SlotUpdateUIInfo();
			UIUtils::showInfoMsgBox(this, UIExplorer::instance().getStringById("STRID_RFS_MSG_OK"));
		}
	}
}

void CSDockWidget::closeEvent(QCloseEvent * event)
{
	emit SignalDocWidgetClose();
}

void CSRccApp::on_actionOnline_Upgrade_triggered()
{
#ifdef Q_OS_WIN32
	do
	{
		auto device = DeviceManager::instance().getCurrentDevice();
		if (nullptr == device)
		{
			break;
		}

		std::wstring ip_or_sn = device->getIpOrSn().toStdWString();
		
		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));
		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		wchar_t szCurrentDir[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szCurrentDir, MAX_PATH);
		PathRemoveFileSpec(szCurrentDir);
		// static const wchar_t szUpgradeToolName[] = L"UpdateGradeTool.exe";
		static const wchar_t szUpgradeToolName[] = L"AGConfigTool.exe";
		wchar_t szCmd[MAX_PATH] = { 0 };
		auto language = CSRccAppTranslator::instance().instance().getLanguage();
		std::wstring strLanguage = L"zh_CN";
		if (QLocale::Language::English == language)
			strLanguage = L"en_US";
		else if(QLocale::Language::Japanese == language)
			strLanguage = L"ja_JA";
		swprintf_s(szCmd, L"\"%ws\\%ws\" ip=%ws,port=21,Language=%ws", szCurrentDir, szUpgradeToolName, ip_or_sn.c_str(), strLanguage.c_str());

		QMessageBox infoBox(this);
		infoBox.setWindowTitle(QObject::tr("RCC"));
		infoBox.setIcon(QMessageBox::Information);
		infoBox.addButton(tr("OK"), QMessageBox::YesRole);
		infoBox.addButton(tr("NO"), QMessageBox::NoRole);
		infoBox.setText(tr("The device needs to be disconnected before online upgrade.\nDo you want to disconnect it?"));
		int iRet = infoBox.exec();
		if (1 == iRet)
		{
			break;
		}

		//step1:stop camera
		if (device->allowsStop())
		{
			device->stop();
		}

		//step2:tell camera jump bootloader
		UpgradeInfo upgradeInfo{};
		HscResult res = device->StartOnlineUpgrade(upgradeInfo);
		if (res != HSC_OK)
		{
			ErrorCodeUtils::handle(this, res);
			break;
		}
		//step3:disconnect camera
		device->disconnect(true);
		DeviceManager::instance().removeAddedDevice(device);

		//call UpdateGradeTool.exe application
		if (FALSE == CreateProcess(nullptr, szCmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {

			QMessageBox msgBox(this);
			QString strMsg = tr("%1 start online upgrade failed, error code:%2").arg(device->getIpOrSn(), GetLastError());
			msgBox.setWindowTitle(QObject::tr("RCC"));
			msgBox.setText(strMsg);
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.addButton(tr("OK"), QMessageBox::YesRole);
			msgBox.exec();
			break;
		}
	} while (false);
#endif // Q_OS_WIN32
}


void CSRccApp::on_actionRun_Alarm_W_triggered()
{
	auto device = DeviceManager::instance().getCurrentDevice();
	if (device)
	{
		//根据报警消息容器的内容，显示报警信息
		QString strWarnings, str;
		//生成报警信息文字内容
		for (int i = 0; i < device->GetVecWarnings().size(); i++)
		{
			Device::Warning warning = device->GetVecWarnings()[i];
			//生成报警信息文字
			str = FormatWarningMessage(device, warning);
			if (strWarnings.length() > 0)
			{
				strWarnings += "\n";
				strWarnings += str;
			}
			else
			{
				strWarnings = str;
			}
		}
		//清空容器
		device->ClearWaringVector();
		//运行报警
		UpdatAlarmActionsUI();

		if (strWarnings.length() > 0)
		{
			//显示报警信息
			UIUtils::showInfoMsgBox(this, strWarnings);
		}
	}
}

void CSRccApp::handleFrameRateChange()
{
	auto devicePtr = DeviceManager::instance().getCurrentDevice();
	if (!devicePtr) return;
	qint64 min1 = 2, max1 = 0, inc1 = 0;
	devicePtr->getPropertyRange(Device::PropPulseWidth, min1, max1, inc1);
	auto val = devicePtr->getProperty(Device::PropPulseWidth).toInt();
	qint64 correct_val = val;
	if (val < min1)
	{
		correct_val = min1;
	}
	if (val > max1)
	{
		correct_val = max1;
	}
	if (correct_val != val)
	{
		devicePtr->setProperty(Device::PropPulseWidth, correct_val);
	}
}

void CSRccApp::updateProjectItem(bool bdiable)
{
	ui->actionNew_File_N->setDisabled(bdiable);
	ui->actionOpen_File_O->setDisabled(bdiable);
	ui->actionSave_File_S->setDisabled(bdiable);
}

void CSRccApp::updateLowLightActionStatus(bool enable)
{
	ui->action_low_light_mode->setEnabled(enable);
}



void CSRccApp::on_actionFilter_triggered(bool checked)
{
	uint8_t enable_value = 0;
	if (checked) {
		ui->actionFiler1->setChecked(false);
		enable_value = 1;
	}
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != device_ptr)
	{
		device_ptr->setFilterEnable(enable_value);
	}
}

void CSRccApp::on_actionFiler1_triggered(bool checked)
{
	uint8_t enable_value = 0;
	if (checked) {
		enable_value = 2;
		ui->actionFilter->setChecked(false);
	}
	
	auto device_ptr = DeviceManager::instance().getCurrentDevice();
	if (nullptr != device_ptr)
	{
		device_ptr->setFilterEnable(enable_value);
	}
}
