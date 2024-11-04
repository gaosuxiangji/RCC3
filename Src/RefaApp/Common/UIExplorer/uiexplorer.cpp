#include "uiexplorer.h"
#include "version/version.h"
#include <QObject>
#include <QSettings>
#include "Main/rccapp/csglobaldefine.h"

UIExplorer &UIExplorer::instance()
{
    static UIExplorer inst;
    return inst;
}

UIExplorer::~UIExplorer()
{
	if (m_setting_ptr)
	{
		delete m_setting_ptr;
		m_setting_ptr = nullptr;
	}
}

QString UIExplorer::getProductName() const
{
	return QObject::tr("RccApp");
}

QString UIExplorer::getProductFullName() const
{
	return QObject::tr("Revealer Camera Control");
}

QString UIExplorer::getProductVersion() const
{
    return VER_FILE_VERSION_STR;
}

QString UIExplorer::getCompanyName() const
{
    return QObject::tr("FuHuang AgileDevice");
}

void UIExplorer::reset(int lang)
{
	//重置字符表
	if (m_setting_ptr)
	{
		delete m_setting_ptr;
		m_setting_ptr = nullptr;
	}
	m_setting_ptr = new QSettings(QString("://strings/%1/gdstrings.ini").arg(lang == 1 ? "en_US" : "zh_CN"), QSettings::IniFormat);

}

QString UIExplorer::getStringById(const QString id) const
{
	//获取(ini文件中遇到英文逗号时,qsetting会自动分割为list,这里需要用逗号组合起来)
	QStringList str_segments = m_setting_ptr->value(id, QString()).toStringList();
	return str_segments.join(',');
}

UIExplorer::UIExplorer()
{

}
