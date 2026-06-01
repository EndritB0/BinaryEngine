#include "pch.h"
#include "BinaryEngine/Core/Timer.h"

namespace BinaryEngine {


	BinaryEngine::Timer::~Timer()
	{
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = end - m_Start;

		switch (m_Unit)
		{
		case Time::Seconds:
		{
			double seconds{ std::chrono::duration<double>(duration).count() };
			CORE_INFO("{} took: {} seconds", m_Context, seconds);
			break;
		}

		case Time::Milliseconds:
		{
			double milliseconds{ std::chrono::duration<double, std::milli>(duration).count() };
			CORE_INFO("{} took: {} ms", m_Context, milliseconds);
			break;
		}

		case Time::Microseconds:
		{
			std::int64_t microseconds{ std::chrono::duration_cast<std::chrono::microseconds>(duration).count() };
			CORE_INFO("{} took: {} us", m_Context, microseconds);
			break;
		}
		}
	}

}