#include "pch.h"
#include "BinaryEngine/Audio/Detail/VoiceSource.h"

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

namespace BinaryEngine::Detail {

	ResidentVoiceSource::ResidentVoiceSource(std::shared_ptr<const std::vector<float>> samples, std::uint32_t sampleRate, std::uint32_t channelCount, bool loop) :
		m_Samples(std::move(samples)), m_SampleRate(sampleRate), m_ChannelCount(channelCount), m_Loop(loop)
	{
		m_FrameCount = m_ChannelCount == 0 ? 0 : m_Samples->size() / m_ChannelCount;
	}

	SourceReadResult ResidentVoiceSource::Read(float* destination, std::uint32_t frameCount)
	{
		if (m_FrameCount == 0)
		{
			return { 0, SourceStatus::Finished };
		}

		const float* samples{ m_Samples->data() };
		std::uint32_t framesRead{ 0 };

		while (framesRead < frameCount)
		{
			if (m_ReadFrame == m_FrameCount)
			{
				if (!m_Loop)
				{
					return { framesRead, SourceStatus::Finished };
				}

				m_ReadFrame = 0;
			}

			const std::size_t framesToCopy{ std::min<std::size_t>(m_FrameCount - m_ReadFrame, frameCount - framesRead) };
			std::copy_n(samples + m_ReadFrame * m_ChannelCount, framesToCopy * m_ChannelCount, destination + static_cast<std::size_t>(framesRead) * m_ChannelCount);
			m_ReadFrame += framesToCopy;
			framesRead += static_cast<std::uint32_t>(framesToCopy);
		}

		const bool hasEnded{ !m_Loop && m_ReadFrame == m_FrameCount };
		return { framesRead, hasEnded ? SourceStatus::Finished : SourceStatus::Playing };
	}

	StreamingVoiceSource::StreamingVoiceSource(std::shared_ptr<const std::vector<std::uint8_t>> encodedData, bool loop) :
		m_EncodedData(std::move(encodedData)), m_Loop(loop)
	{
		int openError{ 0 };
		m_Decoder = stb_vorbis_open_memory(m_EncodedData->data(), static_cast<int>(m_EncodedData->size()), &openError, nullptr);
		if (m_Decoder == nullptr)
		{
			CORE_ERROR("[StreamingVoiceSource] stb_vorbis_open_memory failed with error {}", openError);
			return;
		}

		const stb_vorbis_info info{ stb_vorbis_get_info(m_Decoder) };
		m_SampleRate = info.sample_rate;
		m_ChannelCount = static_cast<std::uint32_t>(info.channels);
	}

	StreamingVoiceSource::~StreamingVoiceSource()
	{
		if (m_Decoder != nullptr)
		{
			stb_vorbis_close(m_Decoder);
		}
	}

	SourceReadResult StreamingVoiceSource::Read(float* destination, std::uint32_t frameCount)
	{
		std::uint32_t framesRead{ 0 };
		bool hasJustRestarted{ false };

		while (framesRead < frameCount)
		{
			const int floatsWanted{ static_cast<int>((frameCount - framesRead) * m_ChannelCount) };
			const int framesDecoded{ stb_vorbis_get_samples_float_interleaved(m_Decoder, static_cast<int>(m_ChannelCount), destination + static_cast<std::size_t>(framesRead) * m_ChannelCount, floatsWanted) };
			framesRead += static_cast<std::uint32_t>(framesDecoded);
			if (framesRead == frameCount)
			{
				break;
			}

			if (stb_vorbis_get_error(m_Decoder) != VORBIS__no_error)
			{
				return { framesRead, SourceStatus::Failed };
			}

			if (!m_Loop || (hasJustRestarted && framesDecoded == 0))
			{
				return { framesRead, SourceStatus::Finished };
			}

			if (stb_vorbis_seek_start(m_Decoder) == 0)
			{
				return { framesRead, SourceStatus::Failed };
			}
			hasJustRestarted = true;
		}

		return { framesRead, SourceStatus::Playing };
	}
}
