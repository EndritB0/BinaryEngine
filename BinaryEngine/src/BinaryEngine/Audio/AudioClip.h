#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Audio/AudioAsset.h"

namespace BinaryEngine {

	class AudioClip : public AudioAsset {
	public:
		AudioClip() = default;
		virtual ~AudioClip() override = default;

		AudioClip(const AudioClip&) = delete;
		AudioClip& operator=(const AudioClip&) = delete;
		AudioClip(AudioClip&&) = delete;
		AudioClip& operator=(AudioClip&&) = delete;

		static AssetType GetStaticAssetType() { return AssetType::AudioClip; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		void Load(const std::filesystem::path& filePath);
		virtual bool IsValid() const override { return m_Samples != nullptr; }
		virtual std::uint32_t GetSampleRate() const override { return m_SampleRate; }
		virtual std::uint32_t GetChannelCount() const override { return m_ChannelCount; }

	private:
		virtual std::unique_ptr<Detail::VoiceSource> CreateSource(bool loop) const override;

	private:
		std::shared_ptr<const std::vector<float>> m_Samples;
		std::uint32_t m_SampleRate{ 0 };
		std::uint32_t m_ChannelCount{ 0 };
	};

}
