#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "BinaryEngine/Audio/AudioSpecification.h"
#include "BinaryEngine/Audio/AudioTypes.h"

namespace BinaryEngine {

	class AssetManager;

	namespace Detail {

		class Mixer;

	}

	class AudioEngine {
	public:
		AudioEngine(const AudioSpecification& specification, AssetManager& assetManager);
		~AudioEngine();

		AudioEngine(const AudioEngine&) = delete;
		AudioEngine& operator=(const AudioEngine&) = delete;
		AudioEngine(AudioEngine&&) = delete;
		AudioEngine& operator=(AudioEngine&&) = delete;

		bool IsAvailable() const { return m_Stream != nullptr; }
		AudioBusHandle GetBus(std::string_view name) const;
		void SetBusVolume(AudioBusHandle bus, float volume);
		float GetBusVolume(AudioBusHandle bus) const;
		void Update();

	private:
		struct BusRecord {
			std::string Name;
			std::uint32_t ParentIndex{ 0 };
			float Volume{ 1.0f };
		};

		void BuildBusGraph(const std::vector<AudioBusSpecification>& buses);
		std::optional<std::uint32_t> FindBusIndex(std::string_view name) const;
		bool IsValidBus(AudioBusHandle bus) const;
		void OpenDevice();

	private:
		AudioSpecification m_Specification;
		std::vector<BusRecord> m_Buses;
		std::unique_ptr<Detail::Mixer> m_Mixer;
		struct SDL_AudioStream* m_Stream{ nullptr };
		bool m_IsSubsystemInitialised{ false };
	};

}
