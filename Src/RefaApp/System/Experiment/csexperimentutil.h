#ifndef CSEXPERIMENTUTIL_H
#define CSEXPERIMENTUTIL_H

#include <QString>

//实验
struct CSExperiment {
	QString name;	//实验名称
	QString code;	//以实验生成时间为准的唯一实验代号，格式YYYY-MM-DD-HH-mm-SS,如果为空则代表当前未定义实验
	QString info;	//实验描述信息
	bool captured;		//是否采集过图像，采集过的实验不再允许修改实验配置参数
	QString iniPath;	//记录实验保存信息的ini文件

	CSExperiment()
	{
		captured = false;
	};

	//赋值运算符
	CSExperiment& operator=(const CSExperiment& other)
	{
		if (this != &other)
		{
			name = other.name;
			code = other.code;
			info = other.info;
			iniPath = other.iniPath;
			captured = other.captured;
		}

		return *this;
	};
};

/**
* @brief 实验项目相关功能类
*/
class CSExperimentUtil
{
public:
	/**
	* @brief 根据当前时间生成实验代号
	* @return 实验代号
	*/
	static QString GenerateExpCode();



};

#endif // CSEXPERIMENTUTIL_H