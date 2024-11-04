#pragma once

#include <QDoubleValidator>

class QEvent;

class DoubleValidator : public QDoubleValidator
{
	Q_OBJECT

public:
	explicit DoubleValidator(QObject * parent = Q_NULLPTR);
	DoubleValidator(double bottom, double top, int decimals, QObject *parent = Q_NULLPTR);
	~DoubleValidator();

	virtual State validate(QString &, int &) const override;
	//virtual void fixup(QString &) const override;

protected:
	virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
	bool CheckOutOfRange(double val) const;//检查是否超出范围
	bool CheckEndsWithDot(const QString& str) const;//检查最后一个字符是否为'.'
	bool CheckStartsWithSpecialCharacter(const QString& str, const QStringList& characterSet) const;

private:

private:
	const QStringList specialCharacterSet;
};
