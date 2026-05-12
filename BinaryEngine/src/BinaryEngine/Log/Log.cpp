#include "pch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace BinaryEngine {

	std::unique_ptr<Log> Log::s_Instance{ nullptr };

	static spdlog::level::level_enum ConvertLogLevel(Log::Level level)
	{
		switch (level)
		{
			case Log::Level::Trace:
				return spdlog::level::trace;

			case Log::Level::Info:
				return spdlog::level::info;

			case Log::Level::Warn:
				return spdlog::level::warn;

			case Log::Level::Error:
				return spdlog::level::err;

			case Log::Level::Critical:
				return spdlog::level::critical;

			case Log::Level::Off:
				return spdlog::level::off;

			default:
				return spdlog::level::trace;
		}
	}

	void Log::Init(const std::string& appName)
	{
		if (s_Instance)
		{
			CORE_WARN("[Logger] Logger already initialised, skipping initialization");
			return;
		}

		s_Instance.reset(new Log());

		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_Instance->s_CoreLogger = spdlog::stdout_color_mt("Binary Engine");
		s_Instance->s_CoreLogger->set_level(spdlog::level::trace);
		CORE_INFO("[Logger] Binary Engine Logger Initialised");

		s_Instance->s_AppLogger = spdlog::stdout_color_mt(appName);
		s_Instance->s_AppLogger->set_level(spdlog::level::trace);
		APP_INFO("[Logger] Application Logger Initialised");
	}

	void Log::SetCoreLevel(Level level)
	{
		if (s_Instance && s_Instance->s_CoreLogger)
		{
			s_Instance->s_CoreLogger->set_level(ConvertLogLevel(level));
		}
	}

	void Log::SetAppLevel(Level level)
	{
		if (s_Instance && s_Instance->s_AppLogger)
		{
			s_Instance->s_AppLogger->set_level(ConvertLogLevel(level));
		}
	}

	Log::~Log()
	{
		if (this != s_Instance.get())
		{
			return;
		}

		APP_INFO("[Logger] Application Logger Shutdown");
		CORE_INFO("[Logger] Binary Engine Logger Shutdown");
		spdlog::shutdown();
	}
}