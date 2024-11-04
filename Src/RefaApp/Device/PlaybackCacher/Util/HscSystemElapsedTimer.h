#pragma once

#include <chrono>
#include <ratio>

// 计时工具类
class HscSystemElapsedTimer
{
public:
	HscSystemElapsedTimer() : tpStart(std::chrono::system_clock::now()) {}

	// 耗时（毫秒）
	int64_t elapsed() const
	{
		return msecsElapsed();
	}

	// 耗时（秒）
	int64_t secsElapsed() const
	{
		return std::chrono::duration<double>(std::chrono::system_clock::now() - tpStart).count();
	}

	// 耗时（毫秒）
	int64_t msecsElapsed() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::system_clock::now() - tpStart).count();
	}


	// 耗时（微秒）
	int64_t usecsEplased() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000000>>(std::chrono::system_clock::now() - tpStart).count();
	}

	// 耗时（纳秒）
	int64_t nsecsElapsed() const
	{
		return std::chrono::duration<double, std::ratio<1, 1000000000>>(std::chrono::system_clock::now() - tpStart).count();
	}

	// 超时判定（毫秒）
	bool hasExpired(int64_t timeout) const {
		return std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::system_clock::now() - tpStart).count() >= timeout;
	}

	// 启动计时
	void start()
	{
		tpStart = std::chrono::system_clock::now();
	}

	// 重新启动计时，并返回耗时（毫秒）
	int64_t restart()
	{
		std::chrono::time_point<std::chrono::system_clock> tpStartLast = tpStart;
		tpStart = std::chrono::system_clock::now();
		return std::chrono::duration<double, std::ratio<1, 1000>>(tpStart - tpStartLast).count();
	}

private:
	std::chrono::time_point<std::chrono::system_clock> tpStart;
};

