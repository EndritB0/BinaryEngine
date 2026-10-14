#pragma once

#include <optional>
#include <vector>

#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Audio/AudioTypes.h"

namespace BinaryEngine {

	class AudioEngine;

	class AudioScope {
	public:
		explicit AudioScope(AudioEngine& audioEngine);
		~AudioScope();

		AudioScope(const AudioScope&) = delete;
		AudioScope& operator=(const AudioScope&) = delete;
		AudioScope(AudioScope&&) = delete;
		AudioScope& operator=(AudioScope&&) = delete;

		VoiceHandle Play(AssetHandle clip, const PlaySpecification& specification = {});
		void StopAll(std::optional<float> fadeSeconds = std::nullopt);

	private:
		friend class AudioEngine;

		AudioEngine& m_AudioEngine;
		std::vector<VoiceHandle> m_Voices;
	};

}
