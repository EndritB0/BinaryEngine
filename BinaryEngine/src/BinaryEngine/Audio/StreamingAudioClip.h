#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Audio/AudioAsset.h"

namespace BinaryEngine {

	class StreamingAudioClip : public AudioAsset {
	public:
		StreamingAudioClip() = default;
		virtual ~StreamingAudioClip() override = default;

		StreamingAudioClip(const StreamingAudioClip&) = delete;
		StreamingAudioClip& operator=(const StreamingAudioClip&) = delete;
		StreamingAudioClip(StreamingAudioClip&&) = delete;
		StreamingAudioClip& operator=(StreamingAudioClip&&) = delete;

		static AssetType GetStaticAssetType() { return AssetType::StreamingAudioClip; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		void Load(const std::filesystem::path& filePath);
		virtual bool IsValid() const override { return m_EncodedData != nullptr; }
		virtual std::uint32_t GetSampleRate() const override { return m_SampleRate; }
		virtual std::uint32_t GetChannelCount() const override { return m_ChannelCount; }

	private:
		virtual std::unique_ptr<Detail::VoiceSource> CreateSource(bool loop) const override;

	private:
		std::shared_ptr<const std::vector<std::uint8_t>> m_EncodedData;
		std::uint32_t m_SampleRate{ 0 };
		std::uint32_t m_ChannelCount{ 0 };
	};

}
