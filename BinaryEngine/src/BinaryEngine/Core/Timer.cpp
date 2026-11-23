#include "pch.h"
#include "BinaryEngine/Core/Timer.h"

namespace BinaryEngine {

	BinaryEngine::Timer::~Timer()
	{
		const auto duration{ std::chrono::steady_clock::now() - m_Start };

		std::string message;
		switch (m_Unit)
		{
			case TimeUnit::Seconds:
			{
				double seconds{ std::chrono::duration<double>(duration).count() };
				message = std::format("[Timer] {} took: {} seconds", m_Context, seconds);
				break;
			}

			case TimeUnit::Milliseconds:
			{
				double milliseconds{ std::chrono::duration<double, std::milli>(duration).count() };
				message = std::format("[Timer] {} took: {} ms", m_Context, milliseconds);
				break;
			}

			case TimeUnit::Microseconds:
			{
				std::int64_t microseconds{ std::chrono::duration_cast<std::chrono::microseconds>(duration).count() };
				message = std::format("[Timer] {} took: {} us", m_Context, microseconds);
				break;
			}
		}

		switch (m_Source)
		{
			case LogSource::Core:
			{
				CORE_INFO(message);
				break;
			}

			case LogSource::App:
			{
				APP_INFO(message);
				break;
			}
		}
	}

}