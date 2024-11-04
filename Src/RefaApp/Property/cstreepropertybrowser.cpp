#include "cstreepropertybrowser.h"

CSTreePropertyBrowser::CSTreePropertyBrowser(QWidget* parent)
    :QtTreePropertyBrowser(parent)
{

}

CSTreePropertyBrowser::~CSTreePropertyBrowser()
{

}

QtProperty * CSTreePropertyBrowser::findPropertyByName(QString property_name)
{
	QtProperty * prop_res = nullptr;
	QList<QtProperty*> prop_list = properties();
	for (auto prop_item: prop_list)
	{
		//遍历子节点
		QtProperty* find_res = findSubPropertyByName(prop_item, property_name);
		if (find_res)
		{
			prop_res = find_res;
			break;
		}
	}
	return prop_res;
}

QtProperty * CSTreePropertyBrowser::findSubPropertyByName(QtProperty* property_parent, QString property_name)
{
	//判断自己是不是要找的节点,不是的话查看是否有子节点,有子节点就看递归找,没有子节点就返回空指针
	if (property_parent)
	{
		if (property_parent->propertyName() == property_name)//自己是要找的节点,返回自己
		{
			return property_parent;
		}
		else//自己不是要找的节点,看子节点
		{
			QList<QtProperty*> sub_items = property_parent->subProperties();
			if (sub_items.count())//有子节点,遍历
			{
				QtProperty * find_res;
				for (auto sub_item : sub_items)
				{
					find_res = findSubPropertyByName(sub_item, property_name);
					if (find_res)
					{
						//在子节点中找到了,返回节点
						return find_res;
					}
				}
				//没在子节点中找到,返回空
				return nullptr;
			}
			else//没有子节点,返回空
			{
				return nullptr;
			}
		}
	}
	else//传入了空指针,返回空
	{
		return nullptr;
	}
}
