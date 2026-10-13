#pragma once

#include <cstdint>
#include <memory>

#include "BinaryEngine/Asset/Asset.h"

namespace BinaryEngine {

	class AudioEngine;

	namespace Detail {

		class VoiceSource;

	}

	class AudioAsset : public Asset {
	public:
		virtual bool IsValid() const = 0;
		virtual std::uint32_t GetSampleRate() const = 0;
		virtual std::uint32_t GetChannelCount() const = 0;

	private:
		friend class AudioEngine;

		virtual std::unique_ptr<Detail::VoiceSource> CreateSource(bool loop) const = 0;
	};

}
