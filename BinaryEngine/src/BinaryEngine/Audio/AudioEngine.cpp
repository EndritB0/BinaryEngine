#include "pch.h"
#include "BinaryEngine/Audio/AudioEngine.h"

namespace {

	constexpr int s_OutputChannelCount{ 2 };
	constexpr std::string_view s_MasterBusName{ "Master" };

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

}

namespace BinaryEngine {

	AudioEngine::AudioEngine(const AudioSpecification& specification, [[maybe_unused]] AssetManager& assetManager) :
		m_Specification(specification)
	{
		NormaliseAudioSpecification(m_Specification);
		BuildBusGraph(m_Specification.Buses);
		OpenDevice();
	}

	AudioEngine::~AudioEngine()
	{
		if (m_Stream != nullptr)
		{
			SDL_DestroyAudioStream(m_Stream);
			m_Stream = nullptr;
		}

		if (m_IsSubsystemInitialised)
		{
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
		}

		CORE_INFO("[AudioEngine] Audio Engine Shutdown");
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

		m_Buses[bus.m_Index].Volume = ClampVolume(volume);
	}

	float AudioEngine::GetBusVolume(AudioBusHandle bus) const
	{
		if (!IsValidBus(bus))
		{
			return 0.0f;
		}

		return m_Buses[bus.m_Index].Volume;
	}

	void AudioEngine::Update()
	{}

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
		for (std::size_t busIndex{ 0 }; busIndex < m_Buses.size(); ++busIndex)
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

		const SDL_AudioSpec outputSpecification{
			.format{ SDL_AUDIO_F32 },
			.channels{ s_OutputChannelCount },
			.freq{ static_cast<int>(m_Specification.SampleRate) },
		};

		m_Stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &outputSpecification, nullptr, nullptr);
		if (m_Stream == nullptr)
		{
			CORE_WARN("[AudioEngine] Failed to open the audio device, running without sound: {}", SDL_GetError());
			return;
		}

		if (!SDL_ResumeAudioStreamDevice(m_Stream))
		{
			CORE_WARN("[AudioEngine] Failed to start the audio device, running without sound: {}", SDL_GetError());
			SDL_DestroyAudioStream(m_Stream);
			m_Stream = nullptr;
			return;
		}

		CORE_INFO("[AudioEngine] Audio Engine Initialised: {} Hz stereo, {} voices", m_Specification.SampleRate, m_Specification.MaxVoices);
	}

}
