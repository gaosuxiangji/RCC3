#include "cshealthmanager.h"
#include "Common/scheduler/Scheduler.h"
#include <QUdpSocket>
#include "System/SystemSettings/systemsettingsmanager.h"
QString CSHealthManager::getHostIp() const
{
	return 	SystemSettingsManager::instance().getHealthManagerIp();
}

uint16_t CSHealthManager::getPort() const
{
	return SystemSettingsManager::instance().getHealthManagerPort();
}

uint16_t CSHealthManager::getPeriod() const
{
	return SystemSettingsManager::instance().getHealthManagerPeriod();
}

void CSHealthManager::apply(const QString  ip, uint16_t port, uint16_t period)
{
	SystemSettingsManager::instance().setHealthManagerIp(ip);
	SystemSettingsManager::instance().setHealthManagerPort(port);
	SystemSettingsManager::instance().setHealthManagerPeriod(period);
	stop();
	start();

}

void CSHealthManager::start()
{
	QString ip =getHostIp();
	port_ = getPort();
	uint16_t period = getPeriod();

	scheduler_ptr_.reset(new Bosma::Scheduler(1));
	
	if (!udp_socket_)
	{
		udp_socket_ = new QUdpSocket();
	}
	
	host_address_.setAddress(ip);

	scheduler_ptr_->every(std::chrono::seconds(period), [this] {
		doSendRecord();
	});
}

void CSHealthManager::stop()
{
	scheduler_ptr_.reset();
	if (udp_socket_)
	{
		udp_socket_->close();
		delete udp_socket_;
		udp_socket_ = nullptr;
	}
}

void CSHealthManager::updateRecord(const QString & device_ip, const HealthRecord & record)
{
	std::lock_guard<std::mutex> lock(record_mtx_);

	std::shared_ptr<HealthRecord> record_ptr;
	auto iter = map_record_.find(device_ip);
	if (iter == map_record_.end())
	{
		record_ptr.reset(new HealthRecord);
		map_record_[device_ip] = record_ptr;
	}
	else
	{
		record_ptr = iter->second;
	}

	*record_ptr = record;
}

CSHealthManager::CSHealthManager()
{

}

CSHealthManager::~CSHealthManager()
{
	stop();
}

void CSHealthManager::doSendRecord()
{
	if (udp_socket_)
	{
		std::vector<std::shared_ptr<HealthRecord>> records;
		getRecords(records);
		for (auto record_ptr : records)
		{
			if (record_ptr->device_index == 0)
			{
				continue;
			}
			if (-1 == udp_socket_->writeDatagram((char*)record_ptr.get(),sizeof(HealthRecord),host_address_,port_))
			{
				udp_socket_->error();
				break;
			}
		}

	}
}

void CSHealthManager::getRecords(std::vector<std::shared_ptr<HealthRecord>>& records)
{
	std::lock_guard<std::mutex> lock(record_mtx_);

	for (auto pair : map_record_)
	{
		records.push_back(pair.second);
	}
}
