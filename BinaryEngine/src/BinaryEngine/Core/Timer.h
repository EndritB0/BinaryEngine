#pragma once

#include <chrono>
#include <string>

namespace BinaryEngine {

	enum class Time {
		Seconds,
		Milliseconds,
		Microseconds,
	};

	class Timer {
	public:
		Timer(const std::string& context = std::string(), Time unit = Time::Milliseconds) :
			m_Context(context),
			m_Unit(unit),
			m_Start(std::chrono::high_resolution_clock::now())
		{
		}

		~Timer();

	private:
		const std::string m_Context;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
		Time m_Unit;
	};

}