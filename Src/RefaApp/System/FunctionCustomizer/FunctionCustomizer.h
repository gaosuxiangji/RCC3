#pragma once
#include <QString>
#include <stdint.h>

/**
*@brief 自定义属性解析/获取类
*@note 第一次调用需要在创建QApplication之后
**/
class FunctionCustomizer
{
private:
	FunctionCustomizer();

public:
	~FunctionCustomizer();

	static FunctionCustomizer & GetInstance()
	{
		static FunctionCustomizer instance;
		return instance;
	}

	/**
	*@brief 控制台是否使能
	*@return : bool : true-打开控制台，false-不打开控制台
	**/
	bool isConsoleEnabled() const;

	/**
	*@brief 报靶功能是否使能
	*@return : bool : true-打开报靶功能，false-不打开
	**/
	bool isTargetScoringSupported() const;

	/**
	*@brief 易用性版本
	*@return : bool : true-易用性版本，false-非易用性版本
	**/
	bool isUsabilityVersion() const;

	/**
	*@brief 西光所版本是否使能
	*@return : bool : true-西光所版本，false-非西光所版本
	**/
	bool isXiguangsuoVersion() const;

	/**
	*@brief 积分球版本是否使能
	*@return : bool : true-积分球版本，false-非积分球版本
	**/
	bool isIntegratingSphereVersion() const;

	/**
	*@brief H150版本是否使能
	*@return : bool : true-H150版本，false-非H150版本
	**/
	bool isH150Enabled() const;


	/**
	*@brief 设置是否开机自启动
	*@param [in] : bauto_run : bool，是否自启动
	**/
	void setAutoRun(bool bauto_run) const;

	/**
	*@brief 是否开机自启动
	*@return : bool : true-开机自启动，false-不开机自启动
	**/
	bool isAutoRun() const;

	bool isCF18ControlSupported() const;
private:
	bool bconsole_eanble_{ false };

	bool btarget_scoring_support_{ false };

	bool busability_enable_{ true };

	bool bxiguangsuo_version_{ false };

	bool bintegrating_sphere_version_{ false };

	bool bh150_enable_{ false };

	bool m_bcf18_version{ true };

};

