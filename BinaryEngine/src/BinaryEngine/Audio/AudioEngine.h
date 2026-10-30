#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Audio/AudioTypes.h"

namespace BinaryEngine {

	class AssetManager;
	class AudioScope;
	struct AudioSpecification;

	class AudioEngine {
	public:
		struct Statistics {
			bool IsAvailable{ false };
			std::uint32_t ActiveVoices{ 0 };
			std::uint32_t MaxVoices{ 0 };
			std::uint32_t DroppedPlays{ 0 };
			std::uint32_t DecodeErrors{ 0 };
		};

		AudioEngine(const AudioSpecification& specification, AssetManager& assetManager);
		~AudioEngine();

		AudioEngine(const AudioEngine&) = delete;
		AudioEngine& operator=(const AudioEngine&) = delete;
		AudioEngine(AudioEngine&&) = delete;
		AudioEngine& operator=(AudioEngine&&) = delete;

		bool IsAvailable() const;
		AudioBusHandle GetBus(std::string_view name) const;
		void SetBusVolume(AudioBusHandle bus, float volume);
		float GetBusVolume(AudioBusHandle bus) const;
		void Stop(VoiceHandle voice, std::optional<float> fadeSeconds = std::nullopt);
		void SetVolume(VoiceHandle voice, float volume);
		bool IsPlaying(VoiceHandle voice) const;
		AudioScope& GetGlobalScope();
		Statistics GetStatistics() const;
		void Update();

	private:
		friend class AudioScope;

		VoiceHandle PlayInScope(AudioScope& owner, AssetHandle clipHandle, const PlaySpecification& specification);
		void RegisterScope();
		void DetachScope(AudioScope& scope);

	private:
		struct AudioEngineData;
		std::unique_ptr<AudioEngineData> m_Data;
	};

}
