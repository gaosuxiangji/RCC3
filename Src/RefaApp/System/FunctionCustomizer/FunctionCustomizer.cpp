#include "FunctionCustomizer.h"
#include <QStringList>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSettings>

FunctionCustomizer::FunctionCustomizer()
{
	QCommandLineParser parser;
	parser.setApplicationDescription("command line helper");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

	QCommandLineOption consoleOption("d",QString("open console"));//设置输入“-d”时打开控制台
	parser.addOption(consoleOption);

	QCommandLineOption TargetScoringOption("ipm", QString("Target Scoring"));//设置输入“-ipm”时打开报靶功能
	parser.addOption(TargetScoringOption);

	QCommandLineOption usabilityOption("usability", QString("usability"));//设置输入“-usability”时启用易用性版本
	parser.addOption(usabilityOption);

	QCommandLineOption xiguangsuoOption("xiguangsuo", QString("xiguangsuo Version"));//设置输入“-xiguangsuo”时打开西光所版本
	parser.addOption(xiguangsuoOption);

	QCommandLineOption integratingSphereOption("integrating_sphere", QString("integrating sphere"));//设置输入“-integrating_sphere”时打开积分球版本
	parser.addOption(integratingSphereOption);

	QCommandLineOption h150_Option("h150", QString("H150 Version"));//设置输入“-h150”时打开控制台
	parser.addOption(h150_Option);

	QCommandLineOption cf18_Option("cf18", QString("CF18 Control Version"));//设置输入“-cf18”时，打开CF18控制版本
	parser.addOption(cf18_Option);

	parser.process(*qApp);

	bconsole_eanble_ = parser.isSet(consoleOption);//获取是否打开控制台

	if (!btarget_scoring_support_)
	{
		btarget_scoring_support_ = parser.isSet(TargetScoringOption);//获取是否打开报靶功能
	}
	if (!busability_enable_)
	{
		busability_enable_ = parser.isSet(usabilityOption);//获取是否是易用性版本
	}
	bxiguangsuo_version_ = parser.isSet(xiguangsuoOption);//获取是否是西光所版本
	bintegrating_sphere_version_ = parser.isSet(integratingSphereOption);//获取是否是积分球版本
	bh150_enable_ = parser.isSet(h150_Option);//获取是否是H150版本

	if (!m_bcf18_version)
	{
		m_bcf18_version = parser.isSet(cf18_Option);//获取是否CF18版本
	}

}

FunctionCustomizer::~FunctionCustomizer()
{

}

bool FunctionCustomizer::isConsoleEnabled() const
{
	return bconsole_eanble_;
}

bool FunctionCustomizer::isTargetScoringSupported() const
{
	return btarget_scoring_support_;
}

bool FunctionCustomizer::isUsabilityVersion() const
{
	return busability_enable_;
}

bool FunctionCustomizer::isXiguangsuoVersion() const
{
	return bxiguangsuo_version_;
}

bool FunctionCustomizer::isIntegratingSphereVersion() const
{
	return bintegrating_sphere_version_;
}

bool FunctionCustomizer::isH150Enabled() const
{
	return bh150_enable_;
}

//也有使用“HKEY_LOCAL_MACHINE”的，这种方法如果不是管理员权限会写入失败
static const QString RegRun("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\\");
void FunctionCustomizer::setAutoRun(bool bauto_run) const
{
	QSettings reg_settings(RegRun, QSettings::NativeFormat);
	QString app_name = qApp->applicationName();
	if (bauto_run)
	{
		QString app_path = qApp->applicationFilePath();
		reg_settings.setValue(app_name, app_path.replace("/", "\\"));
	}
	else
	{
		reg_settings.remove(app_name);
	}
}

bool FunctionCustomizer::isAutoRun() const
{
	QSettings reg_settings(RegRun, QSettings::NativeFormat);
	QString app_name = qApp->applicationName();
	QString app_path = qApp->applicationFilePath().replace("/", "\\");
	QString read_reg_path = reg_settings.value(qApp->applicationName()).toString();
	return read_reg_path == app_path;
}

bool FunctionCustomizer::isCF18ControlSupported() const
{
	return m_bcf18_version;
}
