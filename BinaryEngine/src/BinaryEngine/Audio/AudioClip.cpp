#include "pch.h"
#include "BinaryEngine/Audio/AudioClip.h"

#include "BinaryEngine/Audio/Detail/VoiceSource.h"

namespace BinaryEngine {

	void AudioClip::Load(const std::filesystem::path& filePath)
	{
		const std::string pathString{ filePath.string() };

		SDL_AudioSpec fileSpecification{};
		std::uint8_t* fileData{ nullptr };
		std::uint32_t fileDataByteCount{ 0 };
		if (!SDL_LoadWAV(pathString.c_str(), &fileSpecification, &fileData, &fileDataByteCount))
		{
			CORE_ERROR("[AudioClip] Failed to load WAV {}: {}", pathString, SDL_GetError());
			return;
		}

		if (fileSpecification.channels < 1 || fileSpecification.channels > 2 || fileSpecification.freq <= 0)
		{
			CORE_ERROR("[AudioClip] {} has {} channels at {} Hz; only mono and stereo are supported", pathString, fileSpecification.channels, fileSpecification.freq);
			SDL_free(fileData);
			return;
		}

		const SDL_AudioSpec floatSpecification{
			.format{ SDL_AUDIO_F32 },
			.channels{ fileSpecification.channels },
			.freq{ fileSpecification.freq },
		};

		std::uint8_t* floatData{ nullptr };
		int floatDataByteCount{ 0 };
		const bool isConverted{ SDL_ConvertAudioSamples(&fileSpecification, fileData, static_cast<int>(fileDataByteCount), &floatSpecification, &floatData, &floatDataByteCount) };
		SDL_free(fileData);

		if (!isConverted)
		{
			CORE_ERROR("[AudioClip] Failed to convert {} to 32-bit float: {}", pathString, SDL_GetError());
			return;
		}

		const std::size_t channelCount{ static_cast<std::size_t>(fileSpecification.channels) };
		const std::size_t frameCount{ static_cast<std::size_t>(floatDataByteCount) / sizeof(float) / channelCount };
		if (frameCount == 0)
		{
			CORE_ERROR("[AudioClip] {} contains no audio", pathString);
			SDL_free(floatData);
			return;
		}

		std::shared_ptr<std::vector<float>> samples{ std::make_shared<std::vector<float>>(frameCount * channelCount) };
		std::memcpy(samples->data(), floatData, frameCount * channelCount * sizeof(float));
		SDL_free(floatData);

		m_Samples = std::move(samples);
		m_SampleRate = static_cast<std::uint32_t>(fileSpecification.freq);
		m_ChannelCount = static_cast<std::uint32_t>(fileSpecification.channels);
		CORE_TRACE("[AudioClip] {}: {} frames, {} channel(s), {} Hz", pathString, frameCount, m_ChannelCount, m_SampleRate);
	}

	std::unique_ptr<Detail::VoiceSource> AudioClip::CreateSource(bool loop) const
	{
		return std::make_unique<Detail::ResidentVoiceSource>(m_Samples, m_SampleRate, m_ChannelCount, loop);
	}

}
