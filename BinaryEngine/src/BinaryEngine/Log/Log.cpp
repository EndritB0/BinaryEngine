#include "pch.h"
#include "BinaryEngine/Log/Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace {

	std::optional<BinaryEngine::Log> s_Instance{ std::nullopt };

	spdlog::level::level_enum ConvertLogLevel(BinaryEngine::Log::Level level)
	{
		switch (level)
		{
			case BinaryEngine::Log::Level::Trace:
				return spdlog::level::trace;

			case BinaryEngine::Log::Level::Debug:
				return spdlog::level::debug;

			case BinaryEngine::Log::Level::Info:
				return spdlog::level::info;

			case BinaryEngine::Log::Level::Warn:
				return spdlog::level::warn;

			case BinaryEngine::Log::Level::Error:
				return spdlog::level::err;

			case BinaryEngine::Log::Level::Critical:
				return spdlog::level::critical;

			case BinaryEngine::Log::Level::Off:
				return spdlog::level::off;

			default:
				return spdlog::level::trace;
		}
	}

}

namespace BinaryEngine {

	void Log::Init(const std::string& appName)
	{
		if (s_Instance)
		{
			CORE_WARN("[Logger] Logger already initialised, skipping initialization");
			return;
		}

		s_Instance.emplace(Passkey{});

		spdlog::set_pattern("%^[%T] [%l] [%n]: %v%$");
		s_Instance->m_CoreLogger = spdlog::stdout_color_mt("Binary Engine");
		s_Instance->m_CoreLogger->set_level(spdlog::level::trace);
		CORE_INFO("[Logger] Binary Engine Logger Initialised");

		s_Instance->m_AppLogger = spdlog::stdout_color_mt(appName);
		s_Instance->m_AppLogger->set_level(spdlog::level::trace);
		CORE_INFO("[Logger] {} Logger Initialised", appName);
	}

	void Log::Shutdown()
	{
		if (!s_Instance)
		{
			return;
		}

		s_Instance.reset();
		spdlog::shutdown();
	}

	Log& Log::Get()
	{
		return *s_Instance;
	}

	bool Log::IsInitialised()
	{
		return s_Instance.has_value();
	}

	void Log::SetCoreLevel(Level level)
	{
		if (s_Instance && s_Instance->m_CoreLogger)
		{
			s_Instance->m_CoreLogger->set_level(ConvertLogLevel(level));
		}
	}

	void Log::SetAppLevel(Level level)
	{
		if (s_Instance && s_Instance->m_AppLogger)
		{
			s_Instance->m_AppLogger->set_level(ConvertLogLevel(level));
		}
	}

	Log::~Log()
	{
		CORE_INFO("[Logger] {} Logger Shutdown", m_AppLogger->name());
		CORE_INFO("[Logger] Binary Engine Logger Shutdown");

		m_AppLogger.reset();
		m_CoreLogger.reset();
	}
}