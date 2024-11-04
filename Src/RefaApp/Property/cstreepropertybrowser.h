#ifndef CSTREEPROPERTYBROWSER_H
#define CSTREEPROPERTYBROWSER_H

#include <QObject>
#include <QWidget>
#include <qttreepropertybrowser.h>


/**
 * @brief	树结构属性浏览器
 //QtTreePorpertyBrowser使用实例
 // 	QtStringPropertyManager* testManager;//字符串属性管理类
 // 	QtLineEditFactory* testFactory;//LineEdit控件工厂类,需要属性可以修改的话必须使用该类添加管理类
 // 	QtProperty* testProperty;//属性项
 //
 // 	testManager = new QtStringPropertyManager(ui->widget);
 // 	testFactory = new QtLineEditFactory(ui->widget);

 // 	testProperty = testManager->addProperty("test");//创建属性项
 // 	ui->widget->addProperty(testProperty);//添加属性项到表格内
 //
 // 	ui->widget->setFactoryForManager(testManager, testFactory);//设置属性可编辑
 //
 // 	testManager->setValue(testProperty, "test value");//设置属性值
 // 	testProperty->setEnabled(false);//设置属性使能
 */
class CSTreePropertyBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT
public:
    CSTreePropertyBrowser(QWidget* parent = 0);
    ~CSTreePropertyBrowser();

	/**
	**@ Brife	根据名称查找属性项
	**@ Param	property_name 属性名
	**@ Return	QtProperty * 属性项指针
	**@ Note	如果没有找到则返回空指针
	*/
	QtProperty * findPropertyByName(QString property_name);

private:
	
	/**
	**@ Brife	根据名称递归查找子属性项
	**@ Param	property_parent 父节点
	**@ Param	property_name 属性名
	**@ Return	QtProperty * 属性项指针
	**@ Note	如果没有找到则返回空指针
	*/
	QtProperty * findSubPropertyByName(QtProperty* property_parent, QString property_name);
};

#endif // CSTREEPROPERTYBROWSER_H
