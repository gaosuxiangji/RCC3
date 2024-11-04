#include "paramspinbox.h"
#include "Common/KeyBoardControl/keyboardcontrol.h"

#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>
#include <QEvent>

#define ENABLE_KEY_BOAED 0

ParamSpinBox::ParamSpinBox(QWidget *parent /*= Q_NULLPTR*/)
	:QSpinBox(parent)
{
	installEventFilter(this);
}

ParamSpinBox::~ParamSpinBox()
{

}

QValidator::State ParamSpinBox::validate(QString &s, int &i) const
{
	QValidator::State res = QSpinBox::validate(s, i);
	switch (res)
	{
	case QValidator::Invalid:
	{
		bool isok = false;
		s.toInt(&isok);
		if (isok)
		{
			fixup(s);
			return QSpinBox::validate(s, i);
		}
	}
	break;
	case QValidator::Intermediate:
		break;
	case QValidator::Acceptable:
		break;
	default:
		break;
	}
	return res;
}

void ParamSpinBox::fixup(QString &input) const
{
	//判断是否是数字,判断最大最小值
	bool isok = false;
	int param = input.toInt(&isok);
	if (isok)
	{
		if (param > maximum())
		{
			param = maximum();
		}
		else if (param < minimum())
		{
			param = minimum();
		}
		input = QString().setNum(param);
	}

}

bool ParamSpinBox::eventFilter(QObject *obj, QEvent *event)
{
#if ENABLE_KEY_BOAED
	if (obj == this)
	{
		if (event->type() == QEvent::FocusIn)
		{
			KeyBoardControl::OpenKeyBoard();

			//TODO：移动键盘，根据实际需求调整
			//static const QRect screen_rc = qApp->desktop()->screenGeometry();
			//if (screen_rc.width() * screen_rc.height() > 0)
			//{
			//	KeyBoardControl::MoveKeyBoard(QRect(screen_rc.width() / 4 + screen_rc.x(), screen_rc.height() / 8 * 3 + screen_rc.y(), screen_rc.width() / 2, screen_rc.height() / 4));
			//}
		}
		else if (event->type() == QEvent::FocusOut)
		{
			KeyBoardControl::CloseKeyBoard();
		}
	}
#endif

	return QSpinBox::eventFilter(obj, event);
}
