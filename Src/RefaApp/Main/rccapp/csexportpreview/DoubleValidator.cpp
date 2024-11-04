#include "DoubleValidator.h"
#include <QEvent>
#include <QLineEdit>

DoubleValidator::DoubleValidator(QObject * parent)
	:specialCharacterSet({ ".","0","+","-." })
{
	if (parent != nullptr && parent->isWidgetType())
	{
		parent->installEventFilter(this);
	}
}

DoubleValidator::DoubleValidator(double bottom, double top, int decimals, QObject *parent)
	:specialCharacterSet({ ".","0","+","-." })
{
	QDoubleValidator::setBottom(bottom);
	QDoubleValidator::setTop(top);
	QDoubleValidator::setDecimals(decimals);

	if (parent != nullptr && parent->isWidgetType())
	{
		parent->installEventFilter(this);
	}
}

DoubleValidator::~DoubleValidator()
{
	if (parent() != nullptr  && parent()->isWidgetType())
	{
		parent()->removeEventFilter(this);
	}
}

QValidator::State DoubleValidator::validate(QString &str, int & pos) const
{
	auto state = QDoubleValidator::validate(str, pos);

	do
	{
		if (Invalid != state)
		{
			if (CheckOutOfRange(str.toDouble()) ||
				CheckStartsWithSpecialCharacter(str, specialCharacterSet))
			{
				state = Invalid;
				break;
			}
		}
	} while (false);

	return state;
}

#if 0
void DoubleValidator::fixup(QString & str) const
{
	QDoubleValidator::fixup(str);
}
#endif

bool DoubleValidator::CheckOutOfRange(double val) const
{
	if (val < bottom() || val > top())
	{
		return true;
	}

	return false;
}

bool DoubleValidator::CheckStartsWithSpecialCharacter(const QString& str, const QStringList& characterSet) const
{
	for (auto& character : characterSet)
	{
		if (str.startsWith(character) &&
			!("0" == str || str.startsWith("0.")))
		{
			return true;
		}
	}

	return false;
}

bool DoubleValidator::CheckEndsWithDot(const QString& str) const
{
	return str.endsWith('.');
}

bool DoubleValidator::eventFilter(QObject *watched, QEvent *event)
{
	if (QEvent::Type::FocusOut == event->type())
	{
		if (watched->isWidgetType())
		{
			auto lineEdit = qobject_cast<QLineEdit*>(watched);
			if (lineEdit != nullptr)
			{
				QString text = lineEdit->text();
				if (CheckEndsWithDot(text))
				{
					text.append("000");
					lineEdit->setText(text);
				}
				else if (text == QString('.') || text == QString('-'))
				{
					lineEdit->clear();
				}
			}
		}
	}

	return QObject::eventFilter(watched, event);
}