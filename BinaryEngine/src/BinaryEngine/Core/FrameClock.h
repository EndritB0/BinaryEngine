#pragma once

#include <cstdint>

#include "BinaryEngine/Core/FrameClockSpecification.h"
#include "BinaryEngine/Core/Timestep.h"

namespace BinaryEngine {

	class FrameClock {
	public:
		explicit FrameClock(const FrameClockSpecification& specification);

		FrameClock(const FrameClock&) = delete;
		FrameClock& operator=(const FrameClock&) = delete;
		FrameClock(FrameClock&&) = delete;
		FrameClock& operator=(FrameClock&&) = delete;

		TimeStep GetFrameTime() const { return m_FrameTimeNanoseconds; }
		TimeStep GetDeltaTime() const { return m_DeltaTimeNanoseconds; }
		TimeStep GetFixedStep() const { return m_FixedStepNanoseconds; }
		TimeStep GetMaxFrameTime() const { return m_MaxFrameTimeNanoseconds; }
		std::uint16_t GetFixedUpdateRate() const { return m_RequestedFixedUpdateRate; }
		std::uint32_t GetFixedStepCount() const { return m_FixedStepCount; }
		float GetInterpolationAlpha() const;
		void SetFixedUpdateRate(std::uint16_t rate);

	private:
		friend class Application;

		void Start(std::uint64_t nowNanoseconds);
		void BeginFrame(std::uint64_t frameStartNanoseconds);
		bool ConsumeFixedStep();
		void ApplyRequestedFixedUpdateRate();

	private:
		std::uint64_t m_MaxFrameTimeNanoseconds{ 0 };
		std::uint64_t m_FixedStepNanoseconds{ 0 };
		std::uint64_t m_AccumulatorNanoseconds{ 0 };
		std::uint64_t m_LastFrameStartNanoseconds{ 0 };
		std::uint64_t m_FrameTimeNanoseconds{ 0 };
		std::uint64_t m_DeltaTimeNanoseconds{ 0 };
		std::uint32_t m_FixedStepCount{ 0 };
		std::uint16_t m_AppliedFixedUpdateRate{ 0 };
		std::uint16_t m_RequestedFixedUpdateRate{ 0 };
	};

}
