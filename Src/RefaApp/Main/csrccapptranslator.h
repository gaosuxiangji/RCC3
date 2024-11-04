#ifndef CSRCCAPPTRANSLATOR_H
#define CSRCCAPPTRANSLATOR_H

#include <QLocale>
#include <QTranslator>
#include <QSharedPointer>

/**
 * @brief DIC翻译类
 */
class CSRccAppTranslator
{
public:
	static CSRccAppTranslator & instance() {
		static CSRccAppTranslator inst;
		return inst;
	}

	/**
	 * @brief 设置语言
	 * @param [in] language 语言
	 */
	void setLanguage(QLocale::Language language);

	/**
	 * @brief 获取语言
	 * @return 语言
	 */
	QLocale::Language getLanguage() const;

private:
	CSRccAppTranslator();
	~CSRccAppTranslator();

private:
	QSharedPointer<QTranslator> main_translator_ptr_;
	QSharedPointer<QTranslator> visualization_translator_ptr_;
	QSharedPointer<QTranslator> qt_translator_ptr_;
};

#endif // CSRCCAPPTRANSLATOR_H

