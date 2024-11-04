#ifndef LOGUTILS_H
#define LOGUTILS_H
#include <QDateTime>
#include <QStandardPaths>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/cfg/argv.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#define CSLOG_DEBUG(...) {auto p = spdlog::get("Revealer_Camera_Control");if(p){  SPDLOG_LOGGER_DEBUG(p,__VA_ARGS__);}}
#define CSLOG_INFO(...) {auto p = spdlog::get("Revealer_Camera_Control");if(p){  SPDLOG_LOGGER_INFO(p,__VA_ARGS__);}}
#define CSLOG_WARN(...) {auto p = spdlog::get("Revealer_Camera_Control");if(p){  SPDLOG_LOGGER_WARN(p,__VA_ARGS__);}}
#define CSLOG_ERROR(...) {auto p = spdlog::get("Revealer_Camera_Control");if(p){  SPDLOG_LOGGER_ERROR(p,__VA_ARGS__);}}
/**
* @brief 日志工具类(TODO:期望按照日期和大小分割文件,文件数量超过最大限制后自动删除以往文件,支持同时在控制台和文件中输出)
*/
class LogUtils
{
public:
	/**
	* @brief 初始化日志文件
	*/
	static void initLog() {
		// 配置日志
		//文件输出
		QString log_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
			"/AgileDevice/Revealer_Camera_Control/Revealer_Camera_Control" +
			/* QDateTime::currentDateTime().toString("_yyyy_MM_dd") +*/".log";
		auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path.toLocal8Bit().data(), 5 * 1024 * 1024, 5);
		//rotating_sink->set_pattern("[%n][%D %H:%M:%S.%e|thread:%t|%s:%#][%l]%^%v%$");
		rotating_sink->set_level(spdlog::level::info);
		//rotating_sink->flush_on(spdlog::level::info);

		//控制台输出
		auto std_logger = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		//std_logger->set_pattern("[%n][%D %H:%M:%S.%e|thread:%t|%s:%#][%l]%^%v%$");
		std_logger->set_level(spdlog::level::trace);


		auto logger = std::make_shared<spdlog::logger>("Revealer_Camera_Control", spdlog::sinks_init_list{ std_logger, rotating_sink });
		logger->set_pattern("[%n][%D %H:%M:%S.%e|thread:%t|%s:%#][%l]%^%v%$");
		logger->set_level(spdlog::level::trace);
		logger->flush_on(spdlog::level::info);
		spdlog::register_logger(logger);
	}

	static void releaseLog()
	{
		spdlog::shutdown();
	}

};


#endif // LOGUTILS_H
