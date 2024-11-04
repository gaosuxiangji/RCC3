#include "csrccapptranslator.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>
#include <QStringList>

static const char* kLanguageKey = "System/Language";

void CSRccAppTranslator::setLanguage(QLocale::Language language)
{
	QLocale locale(language);


	if (QLocale::Language::English == language)
	{
		{
			if (nullptr == main_translator_ptr_)
			{
				main_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (main_translator_ptr_->load("csrccapp-zh_CN", ":/translations"))
			{
				QApplication::removeTranslator(main_translator_ptr_.data());
			}
		}

		{
			if (nullptr == visualization_translator_ptr_)
			{
				visualization_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (visualization_translator_ptr_->load("qt_zh_CN", ":/translations"))
			{
				QApplication::removeTranslator(visualization_translator_ptr_.data());
			}
		}

		{
			if (nullptr == qt_translator_ptr_)
			{
				qt_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (qt_translator_ptr_->load("refaapp_en", ":/translations"))
			{
				QApplication::installTranslator(qt_translator_ptr_.data());
			}
		}	
	}
	else
	{
		{
			if (nullptr == qt_translator_ptr_)
			{
				qt_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (qt_translator_ptr_->load("refaapp_en", ":/translations"))
			{
				QApplication::removeTranslator(qt_translator_ptr_.data());
			}
		}

		{
			if (nullptr == main_translator_ptr_)
			{
				main_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (main_translator_ptr_->load("csrccapp-zh_CN", ":/translations"))
			{
				QApplication::installTranslator(main_translator_ptr_.data());
			}
		}
		
		{
			if (nullptr == visualization_translator_ptr_)
			{
				visualization_translator_ptr_ = QSharedPointer<QTranslator>(new QTranslator);
			}

			if (visualization_translator_ptr_->load("qt_zh_CN", ":/translations"))
			{
				QApplication::installTranslator(visualization_translator_ptr_.data());
			}
		}
	}

	QSettings settings;
	settings.setValue(kLanguageKey, (int)language);
}

QLocale::Language CSRccAppTranslator::getLanguage() const
{
	QSettings settings;
	return QLocale::Language(settings.value(kLanguageKey, (int)QLocale::Language::Chinese).toInt());
}

CSRccAppTranslator::CSRccAppTranslator()
{

}


CSRccAppTranslator::~CSRccAppTranslator()
{
}
