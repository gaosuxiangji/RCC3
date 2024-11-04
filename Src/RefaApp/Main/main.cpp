#include "refamainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QThread>
#include <QElapsedTimer>
#include <QFile>
#include <QLocale>
#include <QSharedMemory>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QStandardPaths>
#include <QScreen>
#include <QSettings>
#include <QRegExp>
#include <QDebug>
#include <QStyleFactory>
#include "systemlauncher.h"
#include "System/SystemSettings/systemsettingsmanager.h"
#include "Common/LogUtils/logutils.h"
#include "System/FunctionCustomizer/FunctionCustomizer.h"
#include "csrccapptranslator.h"

#ifdef _WIN32
#include "CrashDump/CrashDumpInfo.h"
#include <qt_windows.h>
#endif


#include "Main/rccapp/csglobaldefine.h"


int main(int argc, char *argv[])
{
	//qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));//qt铏氭嫙閿洏锛屽鏄撳湪绋嬪簭鍚姩鏃跺紩璧风▼搴忓穿婧?
// 	QSurfaceFormat format = QSurfaceFormat::defaultFormat();
// 	QPair<int, int> version = format.version();
// 	format.setVersion(1,0);
// 	QSurfaceFormat::setDefaultFormat(format);

	// QT5.7瀵归珮dpi鐨勬敮鎸佷笉濂斤紝鐩存帴浣跨敤浼氬嚭闂锛屾墍浠ヤ笉鍚敤楂榙pi锛屽湪qss涓皟鏁村ぇ灏?
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

    QApplication app(argc, argv);
	// 璁剧疆缁勭粐鍚嶅拰绋嬪簭鍚?
	app.setOrganizationName("AgileDevice");
	app.setApplicationName("Revealer_Camera_Control");

	//閫氳繃鍏变韩鍐呭瓨瀹炵幇绋嬪簭浜掓枼
	const char * __application_id = "_CSRCCAPP_RUNNING_FLAG";
	QSharedMemory singleApp(__application_id);
	if (singleApp.attach())
	{
		//灏濊瘯閲婃斁鍐呭瓨,閬垮厤鍐呭瓨娉勬紡瀵艰嚧鐨勫彉閲忔湭閲婃斁
		singleApp.detach();
		if (singleApp.attach())//鍏变韩鍐呭瓨瀛樺湪,浠ｈ〃绋嬪簭姝ｅ湪杩愯
		{
			CSRccAppTranslator::instance().setLanguage(CSRccAppTranslator::instance().getLanguage());
			QMessageBox::warning(nullptr, QObject::tr("Tips"), QObject::tr("App Running." ));
			return 0;
		}
    }
	//閰嶇疆鏃ュ織
	LogUtils::initLog();
	CSLOG_INFO("\n Revealer_Camera_Control start.");
	//鍒涘缓鍏变韩鍐呭瓨
	if (!singleApp.create(1))
	{
		if (singleApp.error() == QSharedMemory::AlreadyExists)
		{
			CSRccAppTranslator::instance().setLanguage(CSRccAppTranslator::instance().getLanguage());
			QMessageBox::warning(nullptr, QObject::tr("Tips"), QObject::tr("App Running."));
		}
		CSLOG_ERROR("Create shared memory {}",singleApp.errorString().toStdString());
		return 0;
	}

#ifdef _WIN32
	bool bopend_console = FunctionCustomizer::GetInstance().isConsoleEnabled();
	if (bopend_console)
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	}
#endif

	app.setAutoSipEnabled(true);

#ifndef Q_OS_WIN32
    app.setStyle(QStyleFactory::create("Fusion"));
#endif

	//娉ㄥ唽琛ㄤ娇鐢ㄩ粯璁ゆ牸寮?浠ヨ幏鍙栨洿蹇殑璇诲啓閫熷害
	//QSettings::setDefaultFormat(QSettings::IniFormat);//閰嶇疆鏍煎紡榛樿涓篿ni鏂囦欢 璺緞涓簑in:%appdata% linux:$HOME/Settings



#ifdef CSRCCAPP
	//鍔犺浇缈昏瘧
	CSRccAppTranslator::instance().setLanguage(CSRccAppTranslator::instance().getLanguage());

	// 杞藉叆鏍峰紡琛?
	QFile csqssFile(":/qss/csrccapp.qss");
	if (csqssFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString qss = QString::fromUtf8(csqssFile.readAll());
		app.setStyleSheet(qss);
		csqssFile.close();
	}

#else
	// 杞藉叆鏍峰紡琛?
	auto screen_ptr = app.primaryScreen();
	qreal dpi = screen_ptr->logicalDotsPerInch();
	int scale = qRound(dpi / 96);
#if _DEBUG
	scale = 2;
#endif

	QFile qssFile;
	if (scale <= 1)
	{
		qssFile.setFileName(":/qss/default.qss");
	}
	else
	{
		qssFile.setFileName(":/qss/scale_150per.qss");
    }

	if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QString qss = QString::fromUtf8(qssFile.readAll());
		app.setStyleSheet(qss);
		qssFile.close();
	}
#endif //CSRCCAPP

#ifdef WIN32
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);//璁剧疆宕╂簝寮傚父澶勭悊
	//windows璁剧疆璇ョ嚎绋嬩繚鎸佸敜閱?
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
#endif


    // 绯荤粺鍚姩
    SystemLauncher launcher;

    int res = app.exec();


#ifdef _WIN32
	if (bopend_console)
	{
		FreeConsole();
	}
#endif
	CSLOG_INFO("Revealer_Camera_Control end.\n");
	//LogUtils::releaseLog();
	return res;
}
