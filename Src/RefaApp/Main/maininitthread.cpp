#include "maininitthread.h"

#include <QFile>
#include <QDir>
#include <QApplication>
#include <QStandardPaths>
#include "HscAPI.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/AgErrorCode/agerrorcode.h"
#include "Common/UIExplorer/uiexplorer.h"
// #include "LogRunner.h"

// using namespace FHJD_LOG;
#include <QDebug>
MainInitThread::MainInitThread()
{

}

void MainInitThread::run()
{
	qDebug()<<"MainInitThread start."<<endl;

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
	qDebug()<<"HscSdkInit: " << res;

	qDebug()<<"MainInitThread end.";
}
