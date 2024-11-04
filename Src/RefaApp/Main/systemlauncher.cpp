#include "systemlauncher.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

#include "Splash/splashwidget.h"
#include "framelesswindow/framelesswindow.h"
#include "refamainwindow.h"
#include "maininitthread.h"
#include "Common/UIExplorer/uiexplorer.h"
#include "rccapp/csrccapp.h"
#include "Common/LogUtils/logutils.h"
#include "Common/AgErrorCode/agerrorcode.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include <QMessageBox>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#ifdef _DEBUG
#define SPLASH_SHOW 0
#else
#define SPLASH_SHOW 1
#endif // _DEBUG


SystemLauncher::SystemLauncher()
{
	//rgq
#ifdef _WINDOWS
	auto supportOpenGL = HscIsSupportOpenGL(3, 3);
	if (supportOpenGL == false) {
		QMessageBox msgBox;
		QString strMsg = tr("openGL version not support.");
		msgBox.setWindowTitle(QObject::tr("RCC"));
		msgBox.setText(strMsg);
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.addButton(tr("OK"), QMessageBox::YesRole);
		msgBox.exec();
		exit(-1);
	}
#endif

	// 启动页初始化和显示，跳过/超时时，通知主界面显示
	splash_widget_ptr_.reset(new SplashWidget);
	splash_widget_ptr_->setProductName(UIExplorer::instance().getProductFullName());
	splash_widget_ptr_->setProductVersion(UIExplorer::instance().getProductVersion());
	splash_widget_ptr_->setCompanyName(UIExplorer::instance().getCompanyName());
	connect(splash_widget_ptr_.data(), &SplashWidget::skipped, this, &SystemLauncher::showMainWindow);
#if SPLASH_SHOW
	splash_widget_ptr_->show();
#endif 
	// splash_widget_ptr_->show();

	// add by lwzh:
	// 初始化SDK并没有长耗时,考虑到需要在同一个线程进行SDK的初始化及反初始化,这里就不再使用线程来操作了.
	initSDK();
	SystemLauncher::initMainWindow();
}

SystemLauncher::~SystemLauncher()
{
	HscSdkRelease();
}

void SystemLauncher::initSDK()
{
	// 初始化系统配置
	SystemSettingsManager::instance().initialize();

	// 初始化错误码
	AgErrorCode::instance().reset(SystemSettingsManager::instance().getLanguage() == QLocale::English ? 1 : 0);

	// 初始化描述文案表
	UIExplorer::instance().reset(SystemSettingsManager::instance().getLanguage() == QLocale::English ? 1 : 0);

	QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	QString appTempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/AgileDevice/Revealer_Camera_Control";
	QString appPath = QCoreApplication::applicationDirPath();

	//创建路径
	QDir dir;
	dir.mkpath(appDataPath);
	dir.mkpath(appTempPath);

	//初始化RccSDK
	HscResult res = HscSdkInit(appDataPath.toLocal8Bit().data(), appTempPath.toLocal8Bit().data(), appPath.toLocal8Bit().data());
	qDebug() << "HscSdkInit: " << res;
}

void SystemLauncher::initMainWindow()
{
	CSLOG_INFO("initMainWindow begin");

#ifdef CSRCCAPP
	main_window_ptr_ = new CSRccApp();
	QCoreApplication::instance()->installNativeEventFilter(main_window_ptr_);
	main_window_ptr_->onMainWindowShown();

#else
	// 主界面初始化
	frameless_main_window_ptr_.reset(new FramelessWindow);
	main_window_ptr_ = new RefaMainWindow(frameless_main_window_ptr_.data());
	frameless_main_window_ptr_->setContent(main_window_ptr_);
#endif // CSRCCAPP
	// 启动页跳过使能
	splash_widget_ptr_->setSkipEnabled(true);
	CSLOG_INFO("initMainWindow end. Skip enabled.");

	//已经启动动画已跳过（倒计时结束跳过或手动点击跳过按钮都算）且界面未显示则显示主界面
#if SPLASH_SHOW
	if (!is_main_window_shown && splash_widget_ptr_->isSkipped())
#endif
	{
		showMainWindow();
	}
}

void SystemLauncher::showMainWindow()
{
	CSLOG_INFO("Show main window.");
#ifdef CSRCCAPP
	if (!main_window_ptr_)//只有初始化完成后才会创建主窗口，所以这里只需要判断是否完成主窗口创建
	{
		qDebug() << "main_window_ptr_ is empty, and quit showing main window.";
		return;
	}
#else
	if (!main_window_ptr_ || !frameless_main_window_ptr_)//只有初始化完成后才会创建主窗口，所以这里只需要判断是否完成主窗口创建
	{
		qDebug() << "main_window_ptr_ is empty, and quit showing main window.");
		return;
	}
#endif // CSRCCAPP


	if (is_main_window_shown)
	{
		qDebug() << "Main window is shown.";
		return;
	}
	is_main_window_shown = true;

#ifdef CSRCCAPP
	main_window_ptr_->showMaximized();
#else
	// 主界面显示
	frameless_main_window_ptr_->showMaximized();
#endif // CSRCCAPP

	// 等待主界面显示完成
	int timeout = 10 * 1000;
	enum { TimeOutMs = 10 };
	QElapsedTimer timer;
	timer.start();
	while (!main_window_ptr_->isVisible())
	{
		const int remaining = timeout - int(timer.elapsed());
		if (remaining <= 0)
			break;
		QCoreApplication::processEvents(QEventLoop::AllEvents, remaining);
		QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
		QThread::msleep(TimeOutMs);
	}

#ifndef CSRCCAPP
	main_window_ptr_->setInitStackedWidget();
#endif // !CSRCCAPP

	// 启动页隐藏
	splash_widget_ptr_->hide();

#ifndef CSRCCAPP
	//主界面显示之后开始自动搜索等操作
	main_window_ptr_->onMainWindowShown();
#else
	//主界面显示之后开始自动搜索等操作
	//main_window_ptr_->onMainWindowShown();

	//主界面显示之后刷新页面状态
	main_window_ptr_->updateMainWindowUI();
#endif // !CSRCCAPP	
}
