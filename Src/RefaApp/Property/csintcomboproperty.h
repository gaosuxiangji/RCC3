#ifndef CSINTCOMBOPROPERTY_H
#define CSINTCOMBOPROPERTY_H

/************************************************************************/
/* 
该文件中拓展了qt官方的属性列表控件框架,框架的拓展基于官方文档的说明:
doc.qt.io/archives/qq/qq18-propertybrowser.html
自定义了对于QComboBox的动作行为,使其可以在一定范围内编辑数字,并且提供了范围修正功能,
*/
/************************************************************************/

#include <QObject>
#include <QIntValidator>
#include <qtpropertymanager.h>
#include <qteditorfactory.h>
#include <qtvariantproperty.h>
#include <QList>
#include <QMap>
#include <limits.h>
#include <QComboBox>
#include <QFocusEvent>
//可编辑下拉框类型
class CSIntComboPropertyType
{
};
Q_DECLARE_METATYPE(CSIntComboPropertyType)


//可编辑下拉框属性管理器(继承qt的属性管理,可以自定义属性类型,在对应的CSVariantEditorFactory里可以自定义控件)
class CSVariantPropertyManager : public QtVariantPropertyManager
{
	Q_OBJECT
public:
	CSVariantPropertyManager(QObject * parent = 0);
	~CSVariantPropertyManager();

	//判断属性类型是否支持,添加自定义的属性支持
	virtual bool isPropertyTypeSupported(int propertyType) const;

	//获取属性的QVariant属性类型
	int valueType(const QtProperty *property) const;

	//获取属性类型的全部特征种类
	virtual QStringList attributes(int propertyType) const;

	//获取属性类型对应特征的QVariant属性类型
	virtual int attributeType(int propertyType, const QString &attribute) const;

	//可编辑下拉框属性类型id
	static int intComboPropertyTypeId();

	//获取属性值
	virtual QVariant value(const QtProperty *property) const;

	//获取属性特征值
	virtual QVariant attributeValue(const QtProperty *property, const QString &attribute) const;

public slots:
	//设置属性值
	virtual void setValue(QtProperty *property, const QVariant &val);

	//设置属性特征值
	virtual void setAttribute(QtProperty *property,
		const QString &attribute, const QVariant &value);

protected:
	//获取属性值的显示文本,框架调用
	QString valueText(const QtProperty *property) const;

	//初始化属性,框架调用
	virtual void initializeProperty(QtProperty *property);

	//移除属性,框架调用
	virtual void uninitializeProperty(QtProperty *property);

private:

	//设置属性的最大值特征
	void setMax(QtProperty *property, int max);
	
	//设置属性的最小值特征
	void setMin(QtProperty *property, int min);
	
	//设置属性的选项列表特征
	void setSelections(QtProperty *property, QStringList list);
	struct IntComboSelections
	{
		int current_value{ -1 };//当前数据
		QStringList selection_list;//全部选项
		int min{ INT_MIN };
		int max{ INT_MAX };
	};

	//属性项对应的数据项
	QMap<const QtProperty*, IntComboSelections> theValues;

	const QString m_selectionsAttribute;//下拉框选项属性名称
	const QString m_maximumAttribute;//最大值属性名称
	const QString m_minimumAttribute;//最小值属性名称
};

//可编辑下拉框编辑器工厂
class CSVariantEditorFactory : public QtVariantEditorFactory
{
	Q_OBJECT
public:
	CSVariantEditorFactory(QObject * parent = 0);
	~CSVariantEditorFactory();

protected:
	//连接属性管理器,框架调用
	void connectPropertyManager(QtVariantPropertyManager *manager);

	//创建编辑器,框架调用
	QWidget *createEditor(QtVariantPropertyManager *manager, QtProperty *property,
		QWidget *parent);

	//断开属性管理器.框架调用
	void disconnectPropertyManager(QtVariantPropertyManager *manager);

private slots:
void slotPropertyChanged(QtProperty * property, const QVariant &val);
void slotPropertyAttributeChanged(QtProperty *property,const QString &attribute, const QVariant &val);
void slotSetValue(const QString & value);
void slotEditorDestroyed(QObject * object);
private:


	QMap<QtProperty*, QList<QComboBox*>> createdEditors;
	QMap<QComboBox*, QtProperty*> editorToProperty;
};


//自动根据最大最小值修正数据的intvalidator
//使用方法:		当做正常intvalidator使用,
//				在输入数据不在SetRange设置的范围内时会自动调整为最大值或者最小值
class QFixIntValidator : public QIntValidator
{
	Q_OBJECT
public:
	explicit QFixIntValidator(QObject *parent = Q_NULLPTR) :QIntValidator(parent) {

	}
	~QFixIntValidator() {

	}

protected:

	QValidator::State validate(QString &s, int &i) const override {
		QValidator::State res = QIntValidator::validate(s, i);
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


	void fixup(QString &input) const override {
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

};


class CSComboBox : public QComboBox
{
	Q_OBJECT
public:
	explicit CSComboBox(QWidget *parent = Q_NULLPTR) :QComboBox(parent) {

	}
	~CSComboBox() {

	}

protected:


	void focusOutEvent(QFocusEvent *e) override
	{
		emit editTextFinished(QComboBox::currentText());
		return QComboBox::focusOutEvent(e);
	}


Q_SIGNALS:
	void editTextFinished(const QString &);
	

};


#endif // CSINTCOMBOPROPERTY_H