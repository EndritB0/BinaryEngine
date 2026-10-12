#pragma once

#include <cstdint>
#include <limits>

namespace BinaryEngine {

	class VoiceHandle {
	public:
		VoiceHandle() = default;

		bool IsValid() const { return m_Generation != 0; }
		bool operator==(const VoiceHandle&) const = default;

	private:
		friend class AudioEngine;

		VoiceHandle(std::uint32_t slot, std::uint32_t generation) :
			m_Slot(slot), m_Generation(generation) {}

		std::uint32_t m_Slot{ 0 };
		std::uint32_t m_Generation{ 0 };
	};

	class AudioBusHandle {
	public:
		AudioBusHandle() = default;

		static AudioBusHandle Master() { return AudioBusHandle{ 0 }; }
		bool IsValid() const { return m_Index != s_InvalidIndex; }
		bool operator==(const AudioBusHandle&) const = default;

	private:
		friend class AudioEngine;

		static constexpr std::uint32_t s_InvalidIndex{ std::numeric_limits<std::uint32_t>::max() };

		explicit AudioBusHandle(std::uint32_t index) :
			m_Index(index) {}

		std::uint32_t m_Index{ s_InvalidIndex };
	};

	struct PlaySpecification {
		AudioBusHandle Bus{ AudioBusHandle::Master() };
		float Volume{ 1.0f };
		bool Loop{ false };
		float FadeInSeconds{ 0.0f };
	};

}
