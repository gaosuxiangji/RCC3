#ifndef PARAMINTVALIDATOR_H
#define PARAMINTVALIDATOR_H

#include <QObject>
#include <QIntValidator>

//参数合法性检查修正类
//使用方法:		当做正常QIntValidator使用,用setRange输入最大最小值
//				在使用QLineEdit输入整形参数时可以调用setValidator来绑定该修正类
//				在输入数据不在范围内时可以自动修正为最大值或者最小值
class ParamIntValidator : public QIntValidator
{
    Q_OBJECT
public:
	explicit ParamIntValidator(QObject * parent = Q_NULLPTR);
	ParamIntValidator(int bottom, int top, QObject *parent = Q_NULLPTR);
	~ParamIntValidator();

	QValidator::State validate(QString &, int &) const override;

    void fixup(QString &input) const override ;
};

#endif // PARAMINTVALIDATOR_H
