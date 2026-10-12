#include "pch.h"
#include "BinaryEngine/Audio/Detail/VoiceSource.h"

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

}
