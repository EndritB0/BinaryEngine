/*
Logging System
Core Designed inspired by The Cherno's Logging System, with a Singleton Logger Class and Macros used for logging messages with different levels
Adapted slightly, having small changes to the macros and also haveing more customization options for the app logger names
Source: (YouTube) "Logging | Game Engine Series" by The Cherno
URL: https://youtu.be/dZr-53LAlOw
*/

#pragma once

#include <memory>
#include <string>

#include <spdlog/logger.h>

namespace BinaryEngine {

	class Log {
	public:
		enum class Level {
			Trace = 0,
			Debug,
			Info,
			Warn,
			Error,
			Critical,
			Off
		};

		static void Init(const std::string& appName = "Application");

		Log(const Log&) = delete;
		Log& operator=(const Log&) = delete;
		Log(Log&&) = delete;
		Log& operator=(Log&&) = delete;

		inline static Log& Get() { return *s_Instance; }
		inline static bool IsInitialised() { return s_Instance != nullptr; }
		static void SetCoreLevel(Level level);
		static void SetAppLevel(Level level);

		inline std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }
		inline std::shared_ptr<spdlog::logger>& GetAppLogger() { return m_AppLogger; }

	private:
		friend struct std::default_delete<Log>;
		Log() = default;
		~Log();

	private:
		static std::unique_ptr<Log> s_Instance;

		std::shared_ptr<spdlog::logger>m_CoreLogger{ nullptr };
		std::shared_ptr<spdlog::logger>m_AppLogger{ nullptr };
	};
}

#ifndef BINARY_ENGINE_LOGGING_DISABLE

#define CORE_TRACE(...)         do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetCoreLogger()->trace(__VA_ARGS__);} } while (0)
#define CORE_INFO(...)          do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetCoreLogger()->info(__VA_ARGS__);} } while (0)
#define CORE_WARN(...)          do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetCoreLogger()->warn(__VA_ARGS__);} } while (0)
#define CORE_ERROR(...)         do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetCoreLogger()->error(__VA_ARGS__);} } while (0)
#define CORE_CRITICAL(...)		do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetCoreLogger()->critical(__VA_ARGS__);} } while (0)

#define APP_TRACE(...)          do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetAppLogger()->trace(__VA_ARGS__);} } while (0)
#define APP_INFO(...)           do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetAppLogger()->info(__VA_ARGS__);} } while (0)
#define APP_WARN(...)           do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetAppLogger()->warn(__VA_ARGS__);} } while (0)
#define APP_ERROR(...)          do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetAppLogger()->error(__VA_ARGS__);} } while (0)
#define APP_CRITICAL(...)       do { if (::BinaryEngine::Log::IsInitialised()){::BinaryEngine::Log::Get().GetAppLogger()->critical(__VA_ARGS__);} } while (0)

#define CORE_SET_LEVEL(level)   ::BinaryEngine::Log::SetCoreLevel(level)
#define APP_SET_LEVEL(level)    ::BinaryEngine::Log::SetAppLevel(level)

#else

#define CORE_TRACE(...)         
#define CORE_INFO(...)          
#define CORE_WARN(...)          
#define CORE_ERROR(...)         
#define CORE_CRITICAL(...)		

#define APP_TRACE(...)          
#define APP_INFO(...)           
#define APP_WARN(...)           
#define APP_ERROR(...)          
#define APP_CRITICAL(...)       

#define CORE_SET_LEVEL(level)
#define APP_SET_LEVEL(level)

#endif