#pragma once

#include <cstdint>
#include <type_traits>

namespace BinaryEngine::Detail {

	class VoiceSource;

	enum class AudioCommandType : std::uint8_t {
		Play,
		Stop,
		SetVoiceVolume,
		SetBusVolume,
	};

	struct AudioCommand {
		AudioCommandType Type{ AudioCommandType::Play };
		std::uint32_t Slot{ 0 };
		std::uint32_t Generation{ 0 };
		VoiceSource* Source{ nullptr };
		std::uint32_t BusIndex{ 0 };
		float Volume{ 1.0f };
		std::uint32_t FadeFrames{ 0 };
	};

	struct VoiceFinishedEvent {
		std::uint32_t Slot{ 0 };
		std::uint32_t Generation{ 0 };
	};

	static_assert(std::is_trivially_copyable_v<AudioCommand>);
	static_assert(std::is_trivially_copyable_v<VoiceFinishedEvent>);

}
