#ifndef CSHEALTHMANAGER_H
#define CSHEALTHMANAGER_H

#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <QString>
#include <QPointer>
#include <QHostAddress>

#pragma pack(push, 1)

// 健康记录，定长192字节
struct HealthRecord
{
	uint8_t head_mark[4]{ 'X', 'J', 'X', 'G' }; // 帧头
	uint16_t device_index{ 0 }; // 设备编号：1~65535
	uint16_t fault_count{ 0 }; // 故障数：0~32
	uint32_t fault_codes[32]{ 0 }; // 故障代码
	float temperatures[8]{ 0.0 }; // 温度：[0]-FPGA主板温度，[1]-FPGA从板温度，[2]-ARM芯片温度，[3~7]-预留
	uint8_t work_mode{ 0 }; // 工作模式：0-未连接，1-正在连接，2-正在重连，3-已连接，4-正在断开，5-断开，6-预览，7-高速采集，8-正在录制，9-正在断电数据恢复，10-慢速回放，11-待机
	uint8_t res[23]{ 0 }; // 预留
};

#pragma pack(pop)

class QUdpSocket;
class QHostAddress;

namespace Bosma
{
	class Scheduler;
}

//西光所设备健康管理单例类
class CSHealthManager
{
public:
	static CSHealthManager & instance()
	{
		static CSHealthManager inst;
		return inst;
	}

	/**
	* @brief 获取接收数据主机的ip
	* @return QString  ip
	* @note
	*/
	QString getHostIp() const;

	/**
	* @brief 获取主机对应端口
	* @return 端口号
	* @note
	*/
	uint16_t getPort() const;

	/**
	* @brief 获取消息发送周期
	* @return 发送周期
	* @note
	*/
	uint16_t getPeriod() const;

	/**
	* @brief 应用相关参数
	* @param ip - 主机ip port - 主机端口 period - 发送周期
	* @note 
	*/
	void apply(const QString  ip, uint16_t port, uint16_t period);

	/**
	* @brief 开始发送健康管理消息
	*/
	void start();

	/**
	* @brief 停止发送消息
	*/
	void stop();

	/**
	* @brief 更新设备健康参数列表
	* @param [in]device_ip  设备ip
	* @param [in]record 该设备的健康记录
	*/
	void updateRecord(const QString & device_ip, const HealthRecord & record);
private:
    CSHealthManager();
	~CSHealthManager();

	/**
	* @brief 执行发送消息
	*/
	void doSendRecord();

	/**
	* @brief 获取当前全部设备健康记录
	* @param [out] 全部设备健康记录
	*/
	void getRecords(std::vector<std::shared_ptr<HealthRecord>> & records);

private:
	std::map<QString, std::shared_ptr<HealthRecord>> map_record_;//全部设备健康记录,<ip , 对应健康记录>
	std::mutex record_mtx_;

	QPointer<QUdpSocket> udp_socket_;
	QHostAddress host_address_;
	uint16_t port_= 8080;


	std::unique_ptr<Bosma::Scheduler> scheduler_ptr_;//任务定时启动器

};

#endif // CSHEALTHMANAGER_H