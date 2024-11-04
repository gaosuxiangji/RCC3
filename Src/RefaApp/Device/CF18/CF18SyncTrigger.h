/***********************************************************
**文 件 名：CF18SyncTriggerSDK.h
**描    述：CF18 同步触发器API接口
**内    容：
**功    能：
**文件关系：
**作    者：
**生成日期：2023-06-19
**版 本 号：V1.0
**修改日志：
************************************************************/

#ifndef __CF18_SDK_H__
#define __CF18_SDK_H__

#ifdef _MSC_VER
#ifdef CF18SDK_EXPORTS
#define CF18SDK_API __declspec(dllexport)
#else
#define CF18SDK_API __declspec(dllimport)
#endif
#elif __GNUC__ >= 4 || defined(__clang__)
#define CF18SDK_API __attribute__((visibility("default")))
#endif
#ifndef CF18SDK_API
#define CF18SDK_API
#endif

#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct CmdChnStatus {
	uint8_t  channel;       // 通道号(0~7)(mode==0有效)
	uint8_t  single;        // 0-内同步 1-外同步 2-内触发 3-外触发 4-序列发生器 5-B码
	uint8_t  loopSwitch;    // 1:无限循环开 0：无限循环关
	uint8_t  globalCtl;     // 1:使能（启动/停止是否对当前通道有效） 0：不使能
	uint8_t  outputReset;   // 1:复位输出脉冲 0：不复位
	uint8_t  currentStatus; // 0:停止状态 1：运行中状态 rw==0读有效
	uint32_t circleCnt;     // 循环次数
	uint32_t delayPs;       // 延时皮秒数（0~1000）
	uint32_t outputCnt;     // 读取已经输出的脉冲数 rw==0读有效
};

struct CmdInputTrigger {
	uint8_t  polarityReversal; // 0：不反转  1：反转
	uint8_t  edgeReset;        // 触发输入沿复位0:不复位 1：复位
	uint32_t ditheringTime;    // 触发消抖时间设置0~65535
	uint32_t risingEdgeCnt;    // ，0~65535，rw==0读有效
	uint32_t fallingEdgeCnt;   // 触发下降沿计数，0~65535，rw==0读有效
};

struct CmdInputSync {
	uint8_t  polarityReversal;  // 0：不反转  1：反转
	uint8_t  edgeReset;         // 同步输入沿复位0:不复位 1：复位
	uint32_t ditheringTime;     // 同步输入消抖时间设置0~65535
	uint32_t risingEdgeCnt;     // 同步上升沿计数，0~65535，rw==0读有效
	uint32_t fallingEdgeCnt;    // 同步下降沿计数，0~65535，rw==0读有效
};

struct CmdInputBcode {
	uint32_t BcodeDitheringTime;  // B码输入消抖时间0~65535
};

struct CmdInternalSync {
	uint8_t  channel;         // 通道号(0~7)(mode==0有效)
	uint32_t cycle;           // 内同步周期(us)
	uint32_t highLevelWidth;  // 内同步高电平宽度(us)
	uint32_t risingDelay;     // 内同步上升沿延时
};

struct CmdInternalTrigger {
	uint8_t  channel;         // 通道号(0~7)(mode==0有效)
	uint32_t cycle;           // 内同步周期(us)
	uint32_t highLevelWidth;  // 内同步高电平宽度(us)
	uint32_t risingDelay;     // 内同步上升沿延时
};

struct CmdWaveGen {
	uint8_t  status;   // 命令返回状态 0：成功  1：
};


// General operation begin
//*************************************************
//说 明： 打开CF18客户端接口
//参 数：
//   [in] : host : std::string , 主机地址，如192.168.8.35
//   [in] : port : int , 端口号
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool CF18Open(std::string host, int port);
CF18SDK_API bool CF18Close();
// General operation end 

// Version operation begin
//*************************************************
//说 明： 获取版本信息
//参 数：
//   [out] : version : std::string ,版本信息，json字符串，详见CF18接口协议
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetVersionData(std::string &arm_version, std::string &fpga_version); // 获取版本信息
// Version operation end

// control operation begin

//*************************************************
//说 明： 控制设备通道启动/停止
//参 数：
//   [in] : mode : uint8_t , 0:单通道模式 1：所有通道
//   [in] : channel : uint8_t , 通道号(0~7)(mode==0有效)
//   [in] : work : uint8_t , 0:停止 1：启动
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool SetChannelSwitch(uint8_t mode, uint8_t channel, uint8_t work);

// control operation end 

// channel info operation begin

//*************************************************
//说 明： 获取通道参数
//参 数：
//   [in] : chn : int , 通道号(0~7)(mode==0有效)
//   [out] : channelchnIno : CmdChnStatus , 通道信息结构体
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetChnStatus(int chn, CmdChnStatus &chnIno);
CF18SDK_API bool SetChnStatus(CmdChnStatus chnIno); // 设置通道参数
// channel info operation end 


// input signal operation begin
CF18SDK_API bool GetInputTrigger(CmdInputTrigger &inputTrigger); // 获取输入触发信号参数
CF18SDK_API bool SetInputTrigger(CmdInputTrigger inputTrigger); // 设置输入触发信号参数
CF18SDK_API bool GetInputSync(CmdInputSync &inputSync); // 获取输入同步信号参数
CF18SDK_API bool SetInputSync(CmdInputSync inputSync); // 设置输入同步信号参数

//*************************************************
//说 明： 获取输入B码参数
//参 数：
//   [out] : BcodeDitheringTime : uint32_t , B码输入消抖时间0~65535
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetInputBcode(uint32_t &BcodeDitheringTime); // 获取输入B码参数
CF18SDK_API bool SetInputBcode(uint32_t BcodeDitheringTime); // 设置输入B码参数
// input signal operation end


// internal sync signal operation begin

//*************************************************
//说 明： 获取内同步信号参数
//参 数：
//   [in] : chn : int , 通道号(0~7)(mode==0有效)
//   [out] : internalSyncInfo : CmdInternalSync , 内同步信号信息结构体
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetInternalSync(int chn, CmdInternalSync &internalSyncInfo);
CF18SDK_API bool SetInternalSync(CmdInternalSync internalSyncInfo); // 设置内同步信号参数
// internal sync signal operation end

// internal trigger signal operation begin
//*************************************************
//说 明： 获取内触发信号参数
//参 数：
//   [in] : chn : int , 通道号(0~7)(mode==0有效)
//   [out] : internalSyncInfo : CmdInternalSync , 内触发信号信息结构体
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetInternalTrigger(int chn, CmdInternalTrigger &internalTrigInfo); 
CF18SDK_API bool SetInternalTrigger(CmdInternalTrigger internalTrigInfo); // 设置内触发信号参数
// internal trigger signal operation end

// wave generator operation begin
//*************************************************
//说 明： 获取内同步信号参数
//参 数：
//   [out] : wave : std::string , 波形序列数据，json字符串，详见CF18接口协议
//返回值：执行是否成功
//备注： 
//*************************************************
CF18SDK_API bool GetWaveData(std::string &wave); // 获取内触发信号参数
CF18SDK_API bool SetWaveData(std::string wave); // 设置内触发信号参数
// wave generator operation end

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // ! __CF18_SDK_H__