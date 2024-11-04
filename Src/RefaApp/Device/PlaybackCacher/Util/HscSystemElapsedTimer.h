#pragma once

#include <chrono>
#include <ratio>

// ��ʱ������
class HscSystemElapsedTimer
{
public:
	HscSystemElapsedTimer() : tpStart(std::chrono::system_clock::now()) {}

	// ��ʱ�����룩
	int64_t elapsed() const
	{
		return msecsElapsed();
	}

	// ��ʱ���룩
	int64_t secsElapsed() const
	{
		return std::chrono::duration<double>(std::chrono::system_clock::now() - tpStart).count();
	}

	// ��ʱ�����룩
	int64_t msecsElapsed() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::system_clock::now() - tpStart).count();
	}


	// ��ʱ��΢�룩
	int64_t usecsEplased() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000000>>(std::chrono::system_clock::now() - tpStart).count();
	}

	// ��ʱ�����룩
	int64_t nsecsElapsed() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000000000>>(std::chrono::system_clock::now() - tpStart).count();
	}

	// ��ʱ�ж������룩
	bool hasExpired(int64_t timeout) const {
		return std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::system_clock::now() - tpStart).count() >= timeout;
	}

	// ������ʱ
	void start()
	{
		tpStart = std::chrono::system_clock::now();
	}

	// ����������ʱ�������غ�ʱ�����룩
	int64_t restart()
	{
		std::chrono::time_point<std::chrono::system_clock> tpStartLast = tpStart;
		tpStart = std::chrono::system_clock::now();
		return std::chrono::duration<double, std::ratio<1, 1000>>(tpStart - tpStartLast).count();
	}

private:
	std::chrono::time_point<std::chrono::system_clock> tpStart;
};

