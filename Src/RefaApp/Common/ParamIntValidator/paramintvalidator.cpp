#include "paramintvalidator.h"



ParamIntValidator::ParamIntValidator(QObject * parent /*= Q_NULLPTR*/)
	:QIntValidator(parent)
{

}

ParamIntValidator::ParamIntValidator(int bottom, int top, QObject *parent /*= Q_NULLPTR*/)
	:QIntValidator(bottom,top,parent)
{

}

ParamIntValidator::~ParamIntValidator()
{

}

QValidator::State ParamIntValidator::validate(QString &s, int &i) const
{
	QValidator::State res=  QIntValidator::validate(	s,i);
	switch (res)
	{
	case QValidator::Invalid:
	{
		bool isok = false;
		s.toInt(&isok);
		if (isok)
		{
			fixup(s);
			return QIntValidator::validate(s, i);
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


void ParamIntValidator::fixup(QString &input) const
{
	//判断是否是数字,判断最大最小值
	bool isok = false;
	int param = input.toInt(&isok);
	if (isok)
	{
		if (param > top())
		{
			param = top();
		}
		else if (param < bottom())
		{
			param = bottom();
		}
		input = QString().setNum(param);
	}


}
