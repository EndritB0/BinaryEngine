#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "BinaryEngine/Log/LogSource.h"

namespace BinaryEngine {

	enum class TimeUnit {
		Seconds,
		Milliseconds,
		Microseconds,
	};

	class Timer {
	public:
		Timer(LogSource source, std::string_view context = {}, TimeUnit unit = TimeUnit::Milliseconds) :
			m_Context{ context },
			m_Source{ source },
			m_Unit{ unit },
			m_Start{ std::chrono::steady_clock::now() }
		{}

		Timer(LogSource source, TimeUnit unit) :
			Timer{ source, {}, unit }
		{}

		~Timer();

		Timer(const Timer&) = delete;
		Timer& operator=(const Timer&) = delete;
		Timer(Timer&&) = delete;
		Timer& operator=(Timer&&) = delete;

	private:
		const std::string m_Context;
		LogSource m_Source;
		TimeUnit m_Unit;
		std::chrono::steady_clock::time_point m_Start;
	};

}

#define BINARY_ENGINE_CONCAT_INNER(first, second)	first##second
#define BINARY_ENGINE_CONCAT(first, second)			BINARY_ENGINE_CONCAT_INNER(first, second)
#define BINARY_ENGINE_TIMER_NAME					BINARY_ENGINE_CONCAT(binaryEngineTimer, __LINE__)

#if !defined(BINARY_ENGINE_TIMERS_DISABLE) && !defined(BINARY_ENGINE_LOGGING_DISABLE)

#define CORE_TIMER(...)		::BinaryEngine::Timer BINARY_ENGINE_TIMER_NAME{ ::BinaryEngine::LogSource::Core, __VA_ARGS__ }
#define APP_TIMER(...)		::BinaryEngine::Timer BINARY_ENGINE_TIMER_NAME{ ::BinaryEngine::LogSource::App, __VA_ARGS__ }

#else

#define CORE_TIMER(...)		do { if constexpr (false) { ::BinaryEngine::Timer BINARY_ENGINE_TIMER_NAME{ ::BinaryEngine::LogSource::Core, __VA_ARGS__ }; } } while (0)
#define APP_TIMER(...)		do { if constexpr (false) { ::BinaryEngine::Timer BINARY_ENGINE_TIMER_NAME{ ::BinaryEngine::LogSource::App, __VA_ARGS__ }; } } while (0)

#endif