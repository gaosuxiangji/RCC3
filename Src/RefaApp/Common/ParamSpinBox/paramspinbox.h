#ifndef PARAMSPINBOX_H
#define PARAMSPINBOX_H

#include <QObject>
#include <QSpinBox>

class QEvent;

//自动根据最大最小值修正数据的spinbox
//使用方法:		当做正常SpinBox使用,
//				在输入数据不在SetRange设置的范围内时会自动调整为最大值或者最小值
class ParamSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	explicit ParamSpinBox(QWidget *parent = Q_NULLPTR);
	~ParamSpinBox();

protected:

	QValidator::State validate(QString &, int &) const override;


	void fixup(QString &input) const override;

	virtual bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // PARAMSPINBOX_H