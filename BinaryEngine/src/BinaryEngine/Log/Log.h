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
		static void Init(const std::string& appName = "Application");
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; }

	private:
		static std::shared_ptr<spdlog::logger>s_CoreLogger;
		static std::shared_ptr<spdlog::logger>s_AppLogger;
	};
}

#ifndef BINARY_ENGINE_LOGGING_DISABLE

#define CORE_TRACE(...)         do { if (::BinaryEngine::Log::GetCoreLogger()){::BinaryEngine::Log::GetCoreLogger()->trace(__VA_ARGS__);} } while (0)
#define CORE_INFO(...)          do { if (::BinaryEngine::Log::GetCoreLogger()){::BinaryEngine::Log::GetCoreLogger()->info(__VA_ARGS__);} } while (0)
#define CORE_WARN(...)          do { if (::BinaryEngine::Log::GetCoreLogger()){::BinaryEngine::Log::GetCoreLogger()->warn(__VA_ARGS__);} } while (0)
#define CORE_ERROR(...)         do { if (::BinaryEngine::Log::GetCoreLogger()){::BinaryEngine::Log::GetCoreLogger()->error(__VA_ARGS__);} } while (0)
#define CORE_CRITICAL(...)		do { if (::BinaryEngine::Log::GetCoreLogger()){::BinaryEngine::Log::GetCoreLogger()->critical(__VA_ARGS__);} } while (0)

#define APP_TRACE(...)          do { if (::BinaryEngine::Log::GetAppLogger()){::BinaryEngine::Log::GetAppLogger()->trace(__VA_ARGS__);} } while (0)
#define APP_INFO(...)           do { if (::BinaryEngine::Log::GetAppLogger()){::BinaryEngine::Log::GetAppLogger()->info(__VA_ARGS__);} } while (0)
#define APP_WARN(...)           do { if (::BinaryEngine::Log::GetAppLogger()){::BinaryEngine::Log::GetAppLogger()->warn(__VA_ARGS__);} } while (0)
#define APP_ERROR(...)          do { if (::BinaryEngine::Log::GetAppLogger()){::BinaryEngine::Log::GetAppLogger()->error(__VA_ARGS__);} } while (0)
#define APP_CRITICAL(...)       do { if (::BinaryEngine::Log::GetAppLogger()){::BinaryEngine::Log::GetAppLogger()->critical(__VA_ARGS__);} } while (0)

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

#endif