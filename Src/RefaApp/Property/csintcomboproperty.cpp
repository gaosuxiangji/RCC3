#include "csintcomboproperty.h"
#include <QAbstractItemView>
CSVariantPropertyManager::CSVariantPropertyManager(QObject * parent)
	:QtVariantPropertyManager(parent),
	m_selectionsAttribute(QLatin1String("selections")),
	m_maximumAttribute(QLatin1String("maximum")),
	m_minimumAttribute(QLatin1String("minimum"))
{

}

CSVariantPropertyManager::~CSVariantPropertyManager()
{
	clear();
}

bool CSVariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
	if (propertyType == intComboPropertyTypeId())
	{
		return true;
	}
	return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int CSVariantPropertyManager::valueType(const QtProperty *property) const
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		return QVariant::Int;
	}
	return QtVariantPropertyManager::valueType(property);
}

QStringList CSVariantPropertyManager::attributes(int propertyType) const
{
	if (propertyType == intComboPropertyTypeId())
	{
		return QStringList{m_minimumAttribute,m_maximumAttribute,m_selectionsAttribute};
	}
	return QtVariantPropertyManager::attributes(propertyType);
}

int CSVariantPropertyManager::attributeType(int propertyType, const QString &attribute) const
{
	if (propertyType == intComboPropertyTypeId())
	{
		if (attribute == m_minimumAttribute || attribute == m_maximumAttribute)
		{
			return QVariant::Int;
		}
		else if(attribute == m_selectionsAttribute)
		{
			return QVariant::List;
		}
	}
	return QtVariantPropertyManager::attributeType(propertyType,attribute);
}

int CSVariantPropertyManager::intComboPropertyTypeId()
{
	return qMetaTypeId<CSIntComboPropertyType>();
}

QVariant CSVariantPropertyManager::value(const QtProperty *property) const
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		const auto it = theValues.constFind(property);
		if (it == theValues.constEnd())
		{
			return 0;
		}
		return it.value().current_value;
	}
	return QtVariantPropertyManager::value(property);
}

QVariant CSVariantPropertyManager::attributeValue(const QtProperty *property, const QString &attribute) const
{
	int propType = propertyType(property);
	if (!propType)
		return QVariant();

	if (propertyType(property) == intComboPropertyTypeId())
	{
		auto it = theValues.find(property);
		if (it == theValues.end())
		{
			return QVariant();
		}
		//根据种类获取属性参数
		if (attribute == m_maximumAttribute)
		{
			return it.value().max;
		}
		else if(attribute == m_minimumAttribute)
		{
			return it.value().min;
		}
		else if (attribute == m_selectionsAttribute)
		{
			return it.value().selection_list;
		}
		return QVariant();
	}

	return QtVariantPropertyManager::attributeValue(property, attribute);
}

void CSVariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
	//判断参数类型是否一致
	int propType = val.userType();
	if (!propType)
		return;

	int valType = valueType(property);

	//判断数据类型是否可用
	if (propType != valType && !val.canConvert(static_cast<QVariant::Type>(valType)))
		return;

	//判断是不是可编辑下拉框属性
	if (propertyType(property) == intComboPropertyTypeId())
	{
		//没有属性类型则退出
		auto it = theValues.find(property);
		if (it == theValues.end())
		{
			return ;
		}
		//属性值相同则退出
		auto &data = it.value();
		if (data.current_value == val.toInt())
		{
			return;
		}
		//大小限制
		const int oldVal = data.current_value;
		data.current_value = qBound<int>(data.min, val.toInt(), data.max);
		if (data.current_value == oldVal)
		{
			return;
		}
		//发送变更信号
		emit propertyChanged(property);
		emit valueChanged(property, data.current_value);
		return;
	}
	
	QtVariantPropertyManager::setValue(property, val);
}

void CSVariantPropertyManager::setAttribute(QtProperty *property, const QString &attribute, const QVariant &value)
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		QVariant oldAttr = attributeValue(property, attribute);
		if (!oldAttr.isValid())
			return;

		int attrType = value.userType();
		if (!attrType)
			return;

		if (attrType != attributeType(propertyType(property), attribute) &&
			!value.canConvert((QVariant::Type)attrType))
			return;

		//没有属性类型则退出
		auto it = theValues.find(property);
		if (it == theValues.end())
		{
			return;
		}
		//根据种类写入属性参数
		if (attribute == m_maximumAttribute)
		{
			setMax(property, qvariant_cast<int>(value));
			return ;
		}
		else if (attribute == m_minimumAttribute)
		{
			setMin(property, qvariant_cast<int>(value));
			return ;
		}
		else if (attribute == m_selectionsAttribute)
		{
			setSelections(property, value.toStringList());
			return ;
		}

		return ;
	}
	QtVariantPropertyManager::setAttribute(property, attribute, value);
}

QString CSVariantPropertyManager::valueText(const QtProperty *property) const
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		const auto it = theValues.constFind(property);
		if (it == theValues.constEnd())
		{
			return QString("NULL");
		}
		return QString::number(it.value().current_value);
	}
	return QtVariantPropertyManager::valueText(property);
}

void CSVariantPropertyManager::initializeProperty(QtProperty *property)
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		theValues[property] = IntComboSelections{};
		return;
	}
	QtVariantPropertyManager::initializeProperty(property);
}

void CSVariantPropertyManager::uninitializeProperty(QtProperty *property)
{
	if (propertyType(property) == intComboPropertyTypeId())
	{
		theValues.remove(property);
		return;
	}
	QtVariantPropertyManager::uninitializeProperty(property);
}

void CSVariantPropertyManager::setMax(QtProperty *property, int max)
{
	//没有属性类型则退出
	auto it = theValues.find(property);
	if (it == theValues.end())
	{
		return;
	}
	//属性值相同则退出
	auto &data = it.value();
	if (data.max == max)
	{
		return;
	}
	data.max = max;
	//发送变更信号
	emit propertyChanged(property);
	emit attributeChanged(property, m_maximumAttribute,data.max);
	return;
}

void CSVariantPropertyManager::setMin(QtProperty *property, int min)
{
	//没有属性类型则退出
	auto it = theValues.find(property);
	if (it == theValues.end())
	{
		return;
	}
	//属性值相同则退出
	auto &data = it.value();
	if (data.min == min)
	{
		return;
	}
	data.min = min;
	//发送变更信号
	emit propertyChanged(property);
	emit attributeChanged(property, m_minimumAttribute, data.min);
	return;
}

void CSVariantPropertyManager::setSelections(QtProperty *property, QStringList list)
{
	//没有属性类型则退出
	auto it = theValues.find(property);
	if (it == theValues.end())
	{
		return;
	}
	//属性值相同则退出
	auto &data = it.value();
	if (data.selection_list == list)
	{
		return;
	}
	data.selection_list = list;
	//发送变更信号
	emit propertyChanged(property);
	emit attributeChanged(property, m_selectionsAttribute, data.selection_list);
	return;
}



CSVariantEditorFactory::CSVariantEditorFactory(QObject * parent /*= 0*/)
	: QtVariantEditorFactory(parent)
{

}

CSVariantEditorFactory::~CSVariantEditorFactory()
{

}

void CSVariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty*, const QVariant &)),
			this, SLOT(slotPropertyChanged(QtProperty*, const QVariant &)));
	connect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
			this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));
	
	QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget * CSVariantEditorFactory::createEditor(QtVariantPropertyManager *manager, QtProperty *property, QWidget *parent)
{
	const int propType = manager->propertyType(property);
	if (propType == CSVariantPropertyManager::intComboPropertyTypeId())
	{
		QComboBox *editor = new CSComboBox(parent);
		auto it = createdEditors.find(property);
		if (it == createdEditors.end())
		{
			it = createdEditors.insert(property, QList<QComboBox*>());
		}
		it.value().append(editor);
		editorToProperty.insert(editor, property);
		editor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		editor->view()->setTextElideMode(Qt::ElideRight);
		editor->setEditable(true);
		editor->setInsertPolicy(QComboBox::NoInsert);
		editor->setValidator(new QFixIntValidator(this));
		editor->setAutoCompletion(false);
		int min = manager->attributeValue(property, "minimum").toInt();
		int max = manager->attributeValue(property, "maximum").toInt();
		QIntValidator* validator = dynamic_cast<QIntValidator*>(const_cast<QValidator*>(editor->validator()));
		if (validator)
		{
			dynamic_cast<QIntValidator*>(validator)->setRange(min, max);
		}
		QStringList enumNames = manager->attributeValue(property, "selections").toStringList();
		editor->addItems(enumNames);
		editor->setCurrentText(manager->value(property).toString());
		QString strvalue = manager->value(property).toString();
		if (enumNames.contains(strvalue))
		{
			int i = 0;
			auto it = enumNames.begin();
			while (it != enumNames.end())
			{
				if (strvalue == *it)
				{
					break;
				}
				i++;
				it++;
			}
			if (i < editor->count())
			{
				editor->setCurrentIndex(i);
			}
		}

		connect(editor, SIGNAL(editTextFinished(const QString&)), this, SLOT(slotSetValue(const QString&)));
		connect(editor, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, 
			&CSVariantEditorFactory::slotSetValue);
		connect(editor, SIGNAL(destroyed(QObject*)),
			this, SLOT(slotEditorDestroyed(QObject*)));
		return editor;

	}
	return QtVariantEditorFactory::createEditor(manager, property, parent);
}

void CSVariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty*, const QVariant &)),
		this, SLOT(slotPropertyChanged(QtProperty*, const QVariant &)));
	disconnect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
		this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));

	QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void CSVariantEditorFactory::slotPropertyChanged(QtProperty * property, const QVariant &val)
{
	if (!createdEditors.contains(property))
		return;

	QListIterator<QComboBox *> itEditor(createdEditors[property]);
	while (itEditor.hasNext())
	{
		QComboBox *editor = itEditor.next();
		editor->blockSignals(true);
		editor->setCurrentText(val.toString());
		editor->blockSignals(false);
	}
}

void CSVariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property, const QString &attribute, const QVariant &val)
{
	if (!createdEditors.contains(property))
		return;

	QtVariantPropertyManager *manager = propertyManager(property);
	if (!manager)
		return;


	QListIterator<QComboBox *> itEditor(createdEditors[property]);
	while (itEditor.hasNext())
	{
		QComboBox *editor = itEditor.next();
		//根据属性类型响应变更
		if (attribute == "maximum")
		{
			QIntValidator* validator = dynamic_cast<QIntValidator*>(const_cast<QValidator*>(editor->validator()));
			if (validator)
			{
				dynamic_cast<QIntValidator*>(validator)->setTop(val.toInt());
			}
		}
		else if (attribute == "minimum")
		{
			QIntValidator* validator = dynamic_cast<QIntValidator*>(const_cast<QValidator*>(editor->validator()));
			if (validator)
			{
				dynamic_cast<QIntValidator*>(validator)->setBottom(val.toInt());
			}
		}
		else if (attribute == "selections")
		{
			editor->blockSignals(true);
			editor->clear();
			editor->addItems(val.toStringList());
			editor->setCurrentText(manager->value(property).toString());
			editor->blockSignals(false);
		}
	}
}

void CSVariantEditorFactory::slotSetValue(const QString & value)
{
	QObject *object = sender();
	const  QMap<QComboBox *, QtProperty *>::ConstIterator ecend = editorToProperty.constEnd();
	for (QMap<QComboBox *, QtProperty *>::ConstIterator itEditor = editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
	{
		if (itEditor.key() == object) 
		{
			QtProperty *property = itEditor.value();
			QtVariantPropertyManager *manager = propertyManager(property);
			if (!manager)
			{
				return;
			}
			manager->setValue(property, value);
			return;
		}
	}
}

void CSVariantEditorFactory::slotEditorDestroyed(QObject * object)
{
	const auto ecend = editorToProperty.end();
	for (auto itEditor = editorToProperty.begin(); itEditor != ecend; ++itEditor)
	{
		if (itEditor.key() == object) 
		{
			QComboBox *editor = itEditor.key();
			QtProperty *property = itEditor.value();
			const auto pit = createdEditors.find(property);
			if (pit != createdEditors.end())
			{
				pit.value().removeAll(editor);
				if (pit.value().empty())
				{
					createdEditors.erase(pit);
				}
			}
			editorToProperty.erase(itEditor);
			return;
		}
	}
}
