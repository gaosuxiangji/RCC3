#include "refamainwindow.h"
#include "ui_refamainwindow.h"

#include <QDesktopServices>
#include <QCoreApplication>
#include <QLocale>
#include <QUrl>
#include <QCloseEvent>
#include <QTimer>
#include <QThread>

#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/UIUtils/uiutils.h"
#include "Device/devicemanager.h"
#include "Device/switchtoplaybackthread.h"
#include "Device/switchtorealtimethread.h"
#include "LogRunner.h"

using namespace FHJD_LOG;

RefaMainWindow::RefaMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RefaMainWindow)
{
	LOG_INFO("Main window construction");
    ui->setupUi(this);

	initUi();
}

RefaMainWindow::~RefaMainWindow()
{
    delete ui;
}

void RefaMainWindow::setInitStackedWidget()
{
	ui->stackedWidget->blockSignals(true);
	for (int i = ui->stackedWidget->count() - 1; i >= 0; --i)
	{
		ui->stackedWidget->setCurrentIndex(i);
	}
	ui->stackedWidget->blockSignals(false);
}

void RefaMainWindow::onMainWindowShown()
{
	ui->pageSettings->onMainWindowShown();
}

void RefaMainWindow::on_toolButtonHelp_clicked()
{
	//TODO：暂时没有html版本的使用说明，替换为pdf
#if 0
    QString filepath = QString("%1/help/%2/index.html").arg(QCoreApplication::applicationDirPath()).arg(QLocale::languageToString(SystemSettingsManager::instance().getLanguage()).toLower());
#else
	QString filepath;
	if (QLocale::Chinese == SystemSettingsManager::instance().getLanguage())
	{
		filepath = QStringLiteral("%1/help/%2/便携式摄像仪用户使用手册.pdf").arg(QCoreApplication::applicationDirPath()).arg(QLocale::languageToString(SystemSettingsManager::instance().getLanguage()).toLower());
	}
#endif
	bool ok = QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
    if (!ok)
    {
        UIUtils::showInfoMsgBox(this, tr("Help Documents Not Found."));
    }
}

void RefaMainWindow::setFullScreen(bool benabled)
{
	ui->widgetMainToolbar->setVisible(!benabled);
	SystemSettingsManager::instance().setFullScreenEnabled(benabled);

	setVisible(!benabled);
	if (ui->pageRealtime == ui->stackedWidget->currentWidget())
		ui->pageRealtime->setFullScreen(benabled);
	else if (ui->pageRemote == ui->stackedWidget->currentWidget())
		ui->pageRemote->setFullScreen(benabled);
	else if (ui->pageLocal == ui->stackedWidget->currentWidget())
		ui->pageLocal->setFullScreen(benabled);
}

void RefaMainWindow::onAutoPlaybackNeeded(const QVariant & video_id)
{
	// 切换到录制回放界面
	ui->toolButtonRemote->setChecked(true);
	ui->stackedWidget->setCurrentWidget(ui->pageRemote);

	ui->pageRemote->setCurrentVideo(video_id);
}

void RefaMainWindow::onCurrentPageChanged(int index)
{
	ui->pageRemote->pause();

    if (index == 1) // 切换到实时图像页面
	{
        // 鼠标等待
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        // 界面禁用
        this->setEnabled(false);

        SwitchToRealtimeThread *thread_ptr = new SwitchToRealtimeThread(ui->pageRemote->getPlaybackPlayerController(), this);
        connect(thread_ptr, &QThread::finished, this, &RefaMainWindow::onSwitchToRealtimeThreadFinished);
        connect(thread_ptr, &QThread::finished, thread_ptr, &QObject::deleteLater);
		thread_ptr->start();
	}
    else if (index == 2) // 切换到录制回放页面
	{
		// 鼠标等待
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		// 界面禁用
		this->setEnabled(false);

        SwitchToPlaybackThread *thread_ptr = new SwitchToPlaybackThread(ui->pageRemote->getPlaybackPlayerController(), this);
        connect(thread_ptr, &QThread::finished, this, &RefaMainWindow::onSwitchToPlaybackThreadFinished);
		connect(thread_ptr, &QThread::finished, thread_ptr, &QObject::deleteLater);
		thread_ptr->start();
	}

    previous_page_index_ = index;
}

void RefaMainWindow::onSwitchToRealtimeThreadFinished()
{
    // 界面使能
    this->setEnabled(true);

    // 鼠标恢复
    QApplication::restoreOverrideCursor();
}

void RefaMainWindow::onSwitchToPlaybackThreadFinished()
{
	// 更新录制回放设备列表
	ui->pageRemote->updateVideoList();


    // 界面使能
    this->setEnabled(true);

    // 鼠标恢复
    QApplication::restoreOverrideCursor();
}

void RefaMainWindow::onDeviceConnected(const QString & device_ip)
{
	qApp->processEvents();//防止界面阻塞

	auto device_ptr = DeviceManager::instance().getDevice(device_ip);
	if (!device_ptr)
	{
		return;
	}
	if (device_ptr->getState() != DeviceState::Connected)
	{
		device_ptr->stop();
	}
	device_ptr->preview(); // 连接后自动进入预览模式
}

void RefaMainWindow::onAutoConnectDevicesSuccess()
{
	//自动连接成功后切换至实时预览界面
	if (ui->stackedWidget->currentIndex() != 1)
	{
		ui->stackedWidget->setCurrentIndex(1);
	}
}

void RefaMainWindow::initUi()
{
	LOG_INFO("Init main window start.");
	ui->buttonGroupMainToolbar->setId(ui->toolButtonSettings, 0);
	ui->buttonGroupMainToolbar->setId(ui->toolButtonRealtime, 1);
	ui->buttonGroupMainToolbar->setId(ui->toolButtonRemote, 2);
	ui->buttonGroupMainToolbar->setId(ui->toolButtonLocal, 3);
	ui->buttonGroupMainToolbar->button(0)->setChecked(true);
	ui->stackedWidget->setCurrentIndex(0);

	connect(ui->buttonGroupMainToolbar, QOverload<int>::of(&QButtonGroup::buttonClicked), ui->stackedWidget, &QStackedWidget::setCurrentIndex);
	connect(ui->pageRealtime, &RealtimeMainWidget::fullScreen, this, &RefaMainWindow::setFullScreen);
	connect(ui->pageRemote, &RemoteMainWidget::fullScreen, this, &RefaMainWindow::setFullScreen);
	connect(ui->pageLocal, &LocalMainWidget::fullScreen, this, &RefaMainWindow::setFullScreen);

	// 自动回放关联
	DeviceManager& device_manager = DeviceManager::instance();
	connect(&device_manager, &DeviceManager::autoPlaybackNeeded, this, &RefaMainWindow::onAutoPlaybackNeeded);

	// 设备连接后自动切换至实时图像
	connect(&device_manager, &DeviceManager::deviceConnected, this, &RefaMainWindow::onDeviceConnected);

	// 页面切换关联
	connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, &RefaMainWindow::onCurrentPageChanged);

	// 自动连接成功
	connect(ui->pageSettings, &DeviceSystemWidget::sigAutoConnectDeviceSuccess, this, &RefaMainWindow::onAutoConnectDevicesSuccess);

	LOG_INFO("Init main window end.");
}
