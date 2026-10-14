#include "pch.h"
#include "BinaryEngine/Audio/AudioEngine.h"

#include "BinaryEngine/Asset/AssetManager.h"
#include "BinaryEngine/Audio/AudioAsset.h"
#include "BinaryEngine/Audio/Detail/Mixer.h"
#include "BinaryEngine/Audio/Detail/VoiceSource.h"

namespace {

	constexpr int s_BytesPerOutputFrame{ static_cast<int>(sizeof(float) * BinaryEngine::Detail::Mixer::s_OutputChannelCount) };
	constexpr std::string_view s_MasterBusName{ "Master" };
	constexpr float s_MaxFadeSeconds{ 3600.0f };
	constexpr std::uint64_t s_DroppedPlayWarningRearmNanoseconds{ 1'000'000'000 };

	void NormaliseAudioSpecification(BinaryEngine::AudioSpecification& specification)
	{
		const BinaryEngine::AudioSpecification defaults{};

		if (specification.MaxVoices == 0)
		{
			CORE_ERROR("[AudioEngine] MaxVoices is 0, using the default of {}", defaults.MaxVoices);
			specification.MaxVoices = defaults.MaxVoices;
		}

		if (specification.SampleRate == 0)
		{
			CORE_ERROR("[AudioEngine] SampleRate is 0, using the default of {} Hz", defaults.SampleRate);
			specification.SampleRate = defaults.SampleRate;
		}

		if (std::isnan(specification.StopFadeSeconds) || specification.StopFadeSeconds < 0.0f)
		{
			CORE_ERROR("[AudioEngine] StopFadeSeconds ({}) is negative, using the default of {} s", specification.StopFadeSeconds, defaults.StopFadeSeconds);
			specification.StopFadeSeconds = defaults.StopFadeSeconds;
		}
	}

	float ClampVolume(float volume)
	{
		return std::isnan(volume) ? 0.0f : std::clamp(volume, 0.0f, 1.0f);
	}

	std::uint32_t NextGeneration(std::uint32_t generation)
	{
		const std::uint32_t nextGeneration{ generation + 1 };
		return nextGeneration == 0 ? 1 : nextGeneration;
	}

	void SDLCALL FeedAudioStream(void* userdata, SDL_AudioStream* stream, int additionalAmount, [[maybe_unused]] int totalAmount)
	{
		if (additionalAmount <= 0)
		{
			return;
		}

		BinaryEngine::Detail::Mixer& mixer{ *static_cast<BinaryEngine::Detail::Mixer*>(userdata) };
		std::uint32_t framesToMix{ static_cast<std::uint32_t>((additionalAmount + s_BytesPerOutputFrame - 1) / s_BytesPerOutputFrame) };

		while (framesToMix > 0)
		{
			const std::uint32_t blockFrames{ std::min(framesToMix, BinaryEngine::Detail::Mixer::s_MaxBlockFrames) };
			const float* block{ mixer.MixBlock(blockFrames) };
			SDL_PutAudioStreamData(stream, block, static_cast<int>(blockFrames) * s_BytesPerOutputFrame);
			framesToMix -= blockFrames;
		}
	}
}

namespace BinaryEngine {

	AudioEngine::AudioEngine(const AudioSpecification& specification, AssetManager& assetManager) :
		m_AssetManager(assetManager), m_Specification(specification)
	{
		NormaliseAudioSpecification(m_Specification);
		BuildBusGraph(m_Specification.Buses);

		m_Voices.resize(m_Specification.MaxVoices);
		m_FreeSlots.reserve(m_Specification.MaxVoices);
		for (std::uint32_t slot{ m_Specification.MaxVoices }; slot > 0; slot--)
		{
			m_FreeSlots.push_back(slot - 1);
		}

		m_GlobalScope.emplace(*this);
		OpenDevice();
	}

	AudioEngine::~AudioEngine()
	{
		m_GlobalScope.reset();
		if (m_LiveScopeCount != 0)
		{
			CORE_ERROR("[AudioEngine] {} AudioScope(s) are still alive; every scope must be destroyed before the AudioEngine", m_LiveScopeCount);
		}

		if (m_Stream != nullptr)
		{
			SDL_DestroyAudioStream(m_Stream);
			m_Stream = nullptr;
		}

		m_Voices.clear();
		const double mixedSeconds{ m_Mixer != nullptr ? static_cast<double>(m_Mixer->GetMixedFrameCount()) / m_Specification.SampleRate : 0.0 };
		m_Mixer.reset();

		if (m_IsSubsystemInitialised)
		{
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}

		CORE_INFO("[AudioEngine] Audio Engine Shutdown (mixed {:.2f} s of audio)", mixedSeconds);
	}

	AudioBusHandle AudioEngine::GetBus(std::string_view name) const
	{
		const std::optional<std::uint32_t> busIndex{ FindBusIndex(name) };
		if (!busIndex.has_value())
		{
			return AudioBusHandle{};
		}

		return AudioBusHandle{ *busIndex };
	}

	void AudioEngine::SetBusVolume(AudioBusHandle bus, float volume)
	{
		if (!IsValidBus(bus))
		{
			return;
		}

		BusRecord& busRecord{ m_Buses[bus.m_Index] };
		busRecord.Volume = ClampVolume(volume);

		if (IsAvailable())
		{
			SendCommand(Detail::AudioCommand{ .Type{ Detail::AudioCommandType::SetBusVolume }, .BusIndex{ bus.m_Index }, .Volume{ busRecord.Volume } });
		}
	}

	float AudioEngine::GetBusVolume(AudioBusHandle bus) const
	{
		if (!IsValidBus(bus))
		{
			return 0.0f;
		}

		return m_Buses[bus.m_Index].Volume;
	}

	void AudioEngine::Stop(VoiceHandle voice, std::optional<float> fadeSeconds)
	{
		if (!IsPlaying(voice))
		{
			return;
		}

		SendCommand(Detail::AudioCommand{
			.Type{ Detail::AudioCommandType::Stop },
			.Slot{ voice.m_Slot },
			.Generation{ voice.m_Generation },
			.FadeFrames{ SecondsToFrames(fadeSeconds.value_or(m_Specification.StopFadeSeconds)) },
		});
	}

	void AudioEngine::SetVolume(VoiceHandle voice, float volume)
	{
		if (!IsPlaying(voice))
		{
			return;
		}

		SendCommand(Detail::AudioCommand{
			.Type{ Detail::AudioCommandType::SetVoiceVolume },
			.Slot{ voice.m_Slot },
			.Generation{ voice.m_Generation },
			.Volume{ ClampVolume(volume) },
		});
	}

	bool AudioEngine::IsPlaying(VoiceHandle voice) const
	{
		if (!voice.IsValid() || voice.m_Slot >= m_Voices.size())
		{
			return false;
		}

		const VoiceRecord& voiceRecord{ m_Voices[voice.m_Slot] };
		return voiceRecord.Source != nullptr && voiceRecord.Generation == voice.m_Generation;
	}

	AudioEngine::Statistics AudioEngine::GetStatistics() const
	{
		return Statistics{
			.IsAvailable{ IsAvailable() },
			.ActiveVoices{ static_cast<std::uint32_t>(m_Voices.size() - m_FreeSlots.size()) },
			.MaxVoices{ m_Specification.MaxVoices },
			.DroppedPlays{ m_DroppedPlays },
			.DecodeErrors{ IsAvailable() ? m_Mixer->GetDecodeErrorCount() : 0u },
		};
	}

	void AudioEngine::Update()
	{
		if (!IsAvailable())
		{
			return;
		}

		FlushCommandBacklog();

		Detail::VoiceFinishedEvent finishedVoice{};
		while (m_Mixer->PopFinishedVoice(finishedVoice))
		{
			ReleaseVoice(finishedVoice);
		}

		if (m_HasWarnedDroppedPlay && SDL_GetTicksNS() - m_LastDroppedPlayNanoseconds >= s_DroppedPlayWarningRearmNanoseconds)
		{
			m_HasWarnedDroppedPlay = false;
		}

		ReportAudioThreadErrors();
	}

	void AudioEngine::BuildBusGraph(const std::vector<AudioBusSpecification>& buses)
	{
		m_Buses.reserve(buses.size() + 1);
		m_Buses.emplace_back(std::string{ s_MasterBusName }, 0u, 1.0f);

		for (const AudioBusSpecification& bus : buses)
		{
			if (bus.Name.empty())
			{
				CORE_ERROR("[AudioEngine] Skipping a bus with an empty name");
				continue;
			}

			if (bus.Name == s_MasterBusName)
			{
				CORE_ERROR("[AudioEngine] Skipping bus 'Master': the engine adds it itself");
				continue;
			}

			if (FindBusIndex(bus.Name).has_value())
			{
				CORE_ERROR("[AudioEngine] Skipping bus '{}': a bus with that name already exists", bus.Name);
				continue;
			}

			std::optional<std::uint32_t> parentIndex{ FindBusIndex(bus.Parent) };
			if (!parentIndex.has_value())
			{
				CORE_ERROR("[AudioEngine] Bus '{}' has parent '{}', which isn't declared before it, so it routes to Master", bus.Name, bus.Parent);
				parentIndex = 0;
			}

			m_Buses.emplace_back(bus.Name, *parentIndex, ClampVolume(bus.Volume));
		}
	}

	std::optional<std::uint32_t> AudioEngine::FindBusIndex(std::string_view name) const
	{
		for (std::size_t busIndex{ 0 }; busIndex < m_Buses.size(); busIndex++)
		{
			if (m_Buses[busIndex].Name == name)
			{
				return static_cast<std::uint32_t>(busIndex);
			}
		}

		return std::nullopt;
	}

	bool AudioEngine::IsValidBus(AudioBusHandle bus) const
	{
		return bus.IsValid() && bus.m_Index < m_Buses.size();
	}

	void AudioEngine::OpenDevice()
	{
		if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
		{
			CORE_WARN("[AudioEngine] Failed to initialise the audio driver, running without sound: {}", SDL_GetError());
			return;
		}
		m_IsSubsystemInitialised = true;

		std::vector<Detail::MixerBusSetup> busSetups;
		busSetups.reserve(m_Buses.size());
		for (const BusRecord& bus : m_Buses)
		{
			busSetups.emplace_back(bus.ParentIndex, bus.Volume);
		}
		m_Mixer = std::make_unique<Detail::Mixer>(m_Specification.SampleRate, m_Specification.MaxVoices, busSetups);

		const SDL_AudioSpec outputSpecification{
			.format{ SDL_AUDIO_F32 },
			.channels{ static_cast<int>(Detail::Mixer::s_OutputChannelCount) },
			.freq{ static_cast<int>(m_Specification.SampleRate) },
		};

		m_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &outputSpecification, FeedAudioStream, m_Mixer.get());
		if (m_Stream == nullptr)
		{
			CORE_WARN("[AudioEngine] Failed to open the audio device, running without sound: {}", SDL_GetError());
			m_Mixer.reset();
			return;
		}

		if (!SDL_ResumeAudioStreamDevice(m_Stream))
		{
			CORE_WARN("[AudioEngine] Failed to start the audio device, running without sound: {}", SDL_GetError());
			SDL_DestroyAudioStream(m_Stream);
			m_Stream = nullptr;
			m_Mixer.reset();
			return;
		}

		CORE_INFO("[AudioEngine] Audio Engine Initialised: {} Hz stereo, {} voices", m_Specification.SampleRate, m_Specification.MaxVoices);
	}

	std::uint32_t AudioEngine::SecondsToFrames(float seconds) const
	{
		if (std::isnan(seconds) || seconds <= 0.0f)
		{
			return 0;
		}

		const float clampedSeconds{ std::min(seconds, s_MaxFadeSeconds) };
		return static_cast<std::uint32_t>(clampedSeconds * static_cast<float>(m_Specification.SampleRate));
	}

	void AudioEngine::SendCommand(const Detail::AudioCommand& command)
	{
		if (!m_CommandBacklog.empty() || !m_Mixer->PushCommand(command))
		{
			m_CommandBacklog.push_back(command);
		}
	}

	void AudioEngine::FlushCommandBacklog()
	{
		std::size_t sentCount{ 0 };
		while (sentCount < m_CommandBacklog.size() && m_Mixer->PushCommand(m_CommandBacklog[sentCount]))
		{
			++sentCount;
		}

		m_CommandBacklog.erase(m_CommandBacklog.begin(), m_CommandBacklog.begin() + static_cast<std::ptrdiff_t>(sentCount));
	}

	VoiceHandle AudioEngine::PlayInScope(AudioScope& owner, AssetHandle clipHandle, const PlaySpecification& specification)
	{
		if (!IsAvailable())
		{
			return VoiceHandle{};
		}

		if (m_InvalidClipWarnings.contains(clipHandle))
		{
			return VoiceHandle{};
		}

		const std::shared_ptr<AudioAsset> clip{ m_AssetManager.GetAsset<AudioAsset>(clipHandle) };
		if (!clip)
		{
			m_InvalidClipWarnings.insert(clipHandle);
			return VoiceHandle{};
		}

		if (!clip->IsValid())
		{
			m_InvalidClipWarnings.insert(clipHandle);
			CORE_WARN("[AudioEngine] Can't play {}: the asset failed to load", clip->GetFilePath().string());
			return VoiceHandle{};
		}

		const std::uint32_t maxSourceRate{ m_Specification.SampleRate * Detail::Mixer::s_MaxSourceRateRatio };
		if (clip->GetSampleRate() > maxSourceRate)
		{
			CORE_ERROR("[AudioEngine] Can't play {}: its sample rate is {} Hz and the limit is {} Hz (4 x the mix rate)", clip->GetFilePath().string(), clip->GetSampleRate(), maxSourceRate);
			return VoiceHandle{};
		}

		std::uint32_t busIndex{ specification.Bus.m_Index };
		if (!IsValidBus(specification.Bus))
		{
			if (!m_HasWarnedInvalidBus)
			{
				CORE_WARN("[AudioEngine] Play was given an invalid bus, so the voice routes to Master");
				m_HasWarnedInvalidBus = true;
			}
			busIndex = 0;
		}

		if (m_FreeSlots.empty() || !m_CommandBacklog.empty())
		{
			return DropPlay();
		}

		std::unique_ptr<Detail::VoiceSource> source{ clip->CreateSource(specification.Loop) };
		if (!source)
		{
			CORE_ERROR("[AudioEngine] Can't play {}: its voice source failed to open", clip->GetFilePath().string());
			return VoiceHandle{};
		}

		const std::uint32_t slot{ m_FreeSlots.back() };
		VoiceRecord& voice{ m_Voices[slot] };
		const Detail::AudioCommand playCommand{
			.Type{ Detail::AudioCommandType::Play },
			.Slot{ slot },
			.Generation{ voice.Generation },
			.Source{ source.get() },
			.BusIndex{ busIndex },
			.Volume{ ClampVolume(specification.Volume) },
			.FadeFrames{ SecondsToFrames(specification.FadeInSeconds) },
		};

		if (!m_Mixer->PushCommand(playCommand))
		{
			return DropPlay();
		}

		m_FreeSlots.pop_back();
		voice.Source = std::move(source);
		voice.Owner = &owner;

		const VoiceHandle handle{ slot, voice.Generation };
		owner.m_Voices.push_back(handle);
		return handle;
	}

	VoiceHandle AudioEngine::DropPlay()
	{
		++m_DroppedPlays;
		m_LastDroppedPlayNanoseconds = SDL_GetTicksNS();
		if (!m_HasWarnedDroppedPlay)
		{
			CORE_WARN("[AudioEngine] Play dropped: all {} voices are in use or the command queue is full", m_Specification.MaxVoices);
			m_HasWarnedDroppedPlay = true;
		}

		return VoiceHandle{};
	}

	void AudioEngine::ReleaseVoice(const Detail::VoiceFinishedEvent& finishedVoice)
	{
		if (finishedVoice.Slot >= m_Voices.size())
		{
			return;
		}

		VoiceRecord& voice{ m_Voices[finishedVoice.Slot] };
		if (!voice.Source || voice.Generation != finishedVoice.Generation)
		{
			return;
		}

		voice.Source.reset();
		if (voice.Owner != nullptr)
		{
			std::erase(voice.Owner->m_Voices, VoiceHandle{ finishedVoice.Slot, finishedVoice.Generation });
			voice.Owner = nullptr;
		}

		voice.Generation = NextGeneration(voice.Generation);
		m_FreeSlots.push_back(finishedVoice.Slot);
	}

	void AudioEngine::ReportAudioThreadErrors()
	{
		const std::uint32_t decodeErrors{ m_Mixer->GetDecodeErrorCount() };
		if (decodeErrors == m_ReportedDecodeErrors)
		{
			return;
		}

		CORE_ERROR("[AudioEngine] {} new decode error(s) on the audio thread; those voices stopped", decodeErrors - m_ReportedDecodeErrors);
		m_ReportedDecodeErrors = decodeErrors;
	}

	void AudioEngine::RegisterScope()
	{
		++m_LiveScopeCount;
	}

	void AudioEngine::DetachScope(AudioScope& scope)
	{
		for (const VoiceHandle voice : scope.m_Voices)
		{
			if (IsPlaying(voice))
			{
				m_Voices[voice.m_Slot].Owner = nullptr;
			}
		}

		scope.m_Voices.clear();
		--m_LiveScopeCount;
	}
}
