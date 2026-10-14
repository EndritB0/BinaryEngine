#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Audio/AudioScope.h"
#include "BinaryEngine/Audio/AudioSpecification.h"
#include "BinaryEngine/Audio/AudioTypes.h"
#include "BinaryEngine/Audio/Detail/AudioCommand.h"

namespace BinaryEngine {

	class AssetManager;

	namespace Detail {

		class Mixer;
		class VoiceSource;
	}

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

		bool IsAvailable() const { return m_Stream != nullptr; }
		AudioBusHandle GetBus(std::string_view name) const;
		void SetBusVolume(AudioBusHandle bus, float volume);
		float GetBusVolume(AudioBusHandle bus) const;
		void Stop(VoiceHandle voice, std::optional<float> fadeSeconds = std::nullopt);
		void SetVolume(VoiceHandle voice, float volume);
		bool IsPlaying(VoiceHandle voice) const;
		AudioScope& GetGlobalScope() { return *m_GlobalScope; }
		Statistics GetStatistics() const;
		void Update();

	private:
		friend class AudioScope;

		struct BusRecord {
			std::string Name;
			std::uint32_t ParentIndex{ 0 };
			float Volume{ 1.0f };
		};

		struct VoiceRecord {
			std::unique_ptr<Detail::VoiceSource> Source;
			AudioScope* Owner{ nullptr };
			std::uint32_t Generation{ 1 };
		};

		void BuildBusGraph(const std::vector<AudioBusSpecification>& buses);
		std::optional<std::uint32_t> FindBusIndex(std::string_view name) const;
		bool IsValidBus(AudioBusHandle bus) const;
		void OpenDevice();
		std::uint32_t SecondsToFrames(float seconds) const;
		void SendCommand(const Detail::AudioCommand& command);
		void FlushCommandBacklog();
		VoiceHandle PlayInScope(AudioScope& owner, AssetHandle clipHandle, const PlaySpecification& specification);
		VoiceHandle DropPlay();
		void ReleaseVoice(const Detail::VoiceFinishedEvent& finishedVoice);
		void ReportAudioThreadErrors();
		void RegisterScope();
		void DetachScope(AudioScope& scope);

	private:
		AssetManager& m_AssetManager;
		AudioSpecification m_Specification;
		std::vector<BusRecord> m_Buses;
		std::vector<VoiceRecord> m_Voices;
		std::vector<std::uint32_t> m_FreeSlots;
		std::vector<Detail::AudioCommand> m_CommandBacklog;
		std::unordered_set<AssetHandle> m_InvalidClipWarnings;
		std::unique_ptr<Detail::Mixer> m_Mixer;
		struct SDL_AudioStream* m_Stream{ nullptr };
		std::optional<AudioScope> m_GlobalScope;
		std::uint64_t m_LastDroppedPlayNanoseconds{ 0 };
		std::uint32_t m_LiveScopeCount{ 0 };
		std::uint32_t m_DroppedPlays{ 0 };
		std::uint32_t m_ReportedDecodeErrors{ 0 };
		bool m_IsSubsystemInitialised{ false };
		bool m_HasWarnedDroppedPlay{ false };
		bool m_HasWarnedInvalidBus{ false };
	};

}
