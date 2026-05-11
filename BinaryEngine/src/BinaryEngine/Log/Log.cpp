#include "pch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace BinaryEngine {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger{ nullptr };
	std::shared_ptr<spdlog::logger> Log::s_AppLogger{ nullptr };

	void Log::Init(const std::string& appName)
	{
		if (s_CoreLogger || s_AppLogger)
		{
			CORE_WARN("[Logger] Logger already initialised, skipping initialization");
			return;
		}

		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Binary Engine");
		s_CoreLogger->set_level(spdlog::level::trace);
		CORE_INFO("[Logger] Binary Engine Logger Initialised");

		s_AppLogger = spdlog::stdout_color_mt(appName);
		s_AppLogger->set_level(spdlog::level::trace);
		APP_INFO("[Logger] Application Logger Initialised");
	}

	void Log::Shutdown()
	{
		APP_INFO("[Logger] Application Logger Shutdown");
		CORE_INFO("[Logger] Binary Engine Logger Shutdown");
		spdlog::shutdown();
	}
}