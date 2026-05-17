#pragma once

#include <cstdint>

namespace BinaryEngine {

	class TimeStep {
	public:
		TimeStep(std::uint64_t time = 0) : m_Time(time) {}

		float GetSeconds() const { return m_Time / 1000000000.f; }
		float GetMilliseconds() const { return m_Time / 1000000.f; }
		float GetMicroseconds() const { return m_Time / 1000.f; }
		double GetSecondsDouble() const { return m_Time / 1000000000.; }
		double GetMillisecondsDouble() const { return m_Time / 1000000.; }
		double GetMicrosecondsDouble() const { return m_Time / 1000.; }
		std::uint64_t GetNanoseconds() const { return m_Time; }

	private:
		std::uint64_t m_Time{};
	};

}