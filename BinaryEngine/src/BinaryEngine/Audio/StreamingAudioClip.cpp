#include "pch.h"
#include "BinaryEngine/Audio/StreamingAudioClip.h"

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include "BinaryEngine/Audio/Detail/VoiceSource.h"

namespace BinaryEngine {

	void StreamingAudioClip::Load(const std::filesystem::path& filePath)
	{
		const std::string pathString{ filePath.string() };

		std::ifstream file(pathString, std::ios::binary | std::ios::ate);
		if (!file)
		{
			CORE_ERROR("[StreamingAudioClip] Failed to open {}", pathString);
			return;
		}

		const std::streamsize fileSize{ file.tellg() };
		if (fileSize <= 0 || fileSize > std::numeric_limits<int>::max())
		{
			CORE_ERROR("[StreamingAudioClip] {} is empty or too large ({} bytes)", pathString, fileSize);
			return;
		}
		file.seekg(0, std::ios::beg);

		std::vector<std::uint8_t> encodedData(static_cast<std::size_t>(fileSize));
		if (!file.read(reinterpret_cast<char*>(encodedData.data()), fileSize))
		{
			CORE_ERROR("[StreamingAudioClip] Failed to read {}", pathString);
			return;
		}

		int openError{ 0 };
		stb_vorbis* decoder{ stb_vorbis_open_memory(encodedData.data(), static_cast<int>(encodedData.size()), &openError, nullptr) };
		if (decoder == nullptr)
		{
			CORE_ERROR("[StreamingAudioClip] {} isn't a valid Ogg Vorbis file (stb_vorbis error {})", pathString, openError);
			return;
		}

		const stb_vorbis_info info{ stb_vorbis_get_info(decoder) };
		const unsigned int frameCount{ stb_vorbis_stream_length_in_samples(decoder) };
		stb_vorbis_close(decoder);

		if (info.channels < 1 || info.channels > 2)
		{
			CORE_ERROR("[StreamingAudioClip] {} has {} channels; only mono and stereo are supported", pathString, info.channels);
			return;
		}

		if (frameCount == 0)
		{
			CORE_ERROR("[StreamingAudioClip] {} contains no audio, or its length can't be read", pathString);
			return;
		}

		m_EncodedData = std::make_shared<const std::vector<std::uint8_t>>(std::move(encodedData));
		m_SampleRate = info.sample_rate;
		m_ChannelCount = static_cast<std::uint32_t>(info.channels);
		CORE_TRACE("[StreamingAudioClip] {}: {} frames, {} channel(s), {} Hz", pathString, frameCount, m_ChannelCount, m_SampleRate);
	}

	std::unique_ptr<Detail::VoiceSource> StreamingAudioClip::CreateSource(bool loop) const
	{
		return Detail::CreateStreamingVoiceSource(m_EncodedData, loop);
	}

}
