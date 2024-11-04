#ifndef PARAMSPINBOX_H
#define PARAMSPINBOX_H

#include <QObject>
#include <QSpinBox>

class QEvent;

//�Զ����������Сֵ�������ݵ�spinbox
//ʹ�÷���:		��������SpinBoxʹ��,
//				���������ݲ���SetRange���õķ�Χ��ʱ���Զ�����Ϊ���ֵ������Сֵ
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