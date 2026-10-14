#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

struct stb_vorbis;

namespace BinaryEngine::Detail {

	enum class SourceStatus {
		Playing,
		Starved,
		Finished,
		Failed,
	};

	struct SourceReadResult {
		std::uint32_t FramesRead{ 0 };
		SourceStatus Status{ SourceStatus::Playing };
	};

	class VoiceSource {
	public:
		VoiceSource() = default;
		virtual ~VoiceSource() = default;

		VoiceSource(const VoiceSource&) = delete;
		VoiceSource& operator=(const VoiceSource&) = delete;
		VoiceSource(VoiceSource&&) = delete;
		VoiceSource& operator=(VoiceSource&&) = delete;

		virtual std::uint32_t GetSampleRate() const = 0;
		virtual std::uint32_t GetChannelCount() const = 0;
		virtual SourceReadResult Read(float* destination, std::uint32_t frameCount) = 0;
	};

	class ResidentVoiceSource : public VoiceSource {
	public:
		ResidentVoiceSource(std::shared_ptr<const std::vector<float>> samples, std::uint32_t sampleRate, std::uint32_t channelCount, bool loop);

		virtual std::uint32_t GetSampleRate() const override { return m_SampleRate; }
		virtual std::uint32_t GetChannelCount() const override { return m_ChannelCount; }
		virtual SourceReadResult Read(float* destination, std::uint32_t frameCount) override;

	private:
		std::shared_ptr<const std::vector<float>> m_Samples;
		std::size_t m_FrameCount{ 0 };
		std::size_t m_ReadFrame{ 0 };
		std::uint32_t m_SampleRate{ 0 };
		std::uint32_t m_ChannelCount{ 0 };
		bool m_Loop{ false };
	};

	class StreamingVoiceSource : public VoiceSource {
	public:
		StreamingVoiceSource(std::shared_ptr<const std::vector<std::uint8_t>> encodedData, bool loop);
		virtual ~StreamingVoiceSource() override;

		bool IsValid() const { return m_Decoder != nullptr; }
		virtual std::uint32_t GetSampleRate() const override { return m_SampleRate; }
		virtual std::uint32_t GetChannelCount() const override { return m_ChannelCount; }
		virtual SourceReadResult Read(float* destination, std::uint32_t frameCount) override;

	private:
		std::shared_ptr<const std::vector<std::uint8_t>> m_EncodedData;
		::stb_vorbis* m_Decoder{ nullptr };
		std::uint32_t m_SampleRate{ 0 };
		std::uint32_t m_ChannelCount{ 0 };
		bool m_Loop{ false };
	};
}
