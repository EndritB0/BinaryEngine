#include "pch.h"
#include "BinaryEngine/Core/FrameClock.h"

namespace BinaryEngine {

	namespace {

		constexpr std::uint64_t s_NanosecondsPerSecond{ 1'000'000'000 };
		constexpr FrameClockSpecification s_DefaultSpecification{};

		std::uint64_t CalculateFixedStepNanoseconds(std::uint16_t rate)
		{
			return (s_NanosecondsPerSecond + rate / 2u) / rate;
		}

	}

	FrameClock::FrameClock(const FrameClockSpecification& specification)
	{
		std::uint16_t fixedUpdateRate{ specification.FixedUpdateRate };
		if (fixedUpdateRate == 0)
		{
			CORE_WARN("[FrameClock] A fixed update rate of 0 Hz is invalid, using {} Hz", s_DefaultSpecification.FixedUpdateRate);
			fixedUpdateRate = s_DefaultSpecification.FixedUpdateRate;
		}

		float maxFrameTimeSeconds{ specification.MaxFrameTimeSeconds };
		if (!(maxFrameTimeSeconds > 0.0f))
		{
			CORE_WARN("[FrameClock] A max frame time of {} s is invalid, using {} s", maxFrameTimeSeconds, s_DefaultSpecification.MaxFrameTimeSeconds);
			maxFrameTimeSeconds = s_DefaultSpecification.MaxFrameTimeSeconds;
		}

		m_AppliedFixedUpdateRate = fixedUpdateRate;
		m_RequestedFixedUpdateRate = fixedUpdateRate;
		m_FixedStepNanoseconds = CalculateFixedStepNanoseconds(fixedUpdateRate);
		m_MaxFrameTimeNanoseconds = static_cast<std::uint64_t>(static_cast<double>(maxFrameTimeSeconds) * static_cast<double>(s_NanosecondsPerSecond));
	}

	float FrameClock::GetInterpolationAlpha() const
	{
		return static_cast<float>(static_cast<double>(m_AccumulatorNanoseconds) / static_cast<double>(m_FixedStepNanoseconds));
	}

	void FrameClock::SetFixedUpdateRate(std::uint16_t rate)
	{
		if (rate == 0)
		{
			CORE_WARN("[FrameClock] Ignoring a fixed update rate of 0 Hz");
			return;
		}

		m_RequestedFixedUpdateRate = rate;
	}

	void FrameClock::Start(std::uint64_t nowNanoseconds)
	{
		m_LastFrameStartNanoseconds = nowNanoseconds;
		m_AccumulatorNanoseconds = 0;
		m_FrameTimeNanoseconds = 0;
		m_DeltaTimeNanoseconds = 0;
		m_FixedStepCount = 0;
	}

	void FrameClock::BeginFrame(std::uint64_t frameStartNanoseconds)
	{
		m_FrameTimeNanoseconds = frameStartNanoseconds - m_LastFrameStartNanoseconds;
		m_LastFrameStartNanoseconds = frameStartNanoseconds;

		ApplyRequestedFixedUpdateRate();

		m_DeltaTimeNanoseconds = std::min(m_FrameTimeNanoseconds, m_MaxFrameTimeNanoseconds);
		m_AccumulatorNanoseconds += m_DeltaTimeNanoseconds;
		m_FixedStepCount = 0;
	}

	bool FrameClock::ConsumeFixedStep()
	{
		if (m_AccumulatorNanoseconds < m_FixedStepNanoseconds)
		{
			return false;
		}

		m_AccumulatorNanoseconds -= m_FixedStepNanoseconds;
		m_FixedStepCount++;
		return true;
	}

	void FrameClock::ApplyRequestedFixedUpdateRate()
	{
		if (m_RequestedFixedUpdateRate == m_AppliedFixedUpdateRate)
		{
			return;
		}

		const std::uint64_t newFixedStepNanoseconds{ CalculateFixedStepNanoseconds(m_RequestedFixedUpdateRate) };
		m_AccumulatorNanoseconds = m_AccumulatorNanoseconds * newFixedStepNanoseconds / m_FixedStepNanoseconds;
		m_FixedStepNanoseconds = newFixedStepNanoseconds;
		m_AppliedFixedUpdateRate = m_RequestedFixedUpdateRate;
		CORE_INFO("[FrameClock] Fixed update rate: {} Hz", m_AppliedFixedUpdateRate);
	}

}
