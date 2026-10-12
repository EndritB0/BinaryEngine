#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BinaryEngine {

	struct AudioBusSpecification {
		std::string Name;
		std::string Parent{ "Master" };
		float Volume{ 1.0f };
	};

	struct AudioSpecification {
		std::vector<AudioBusSpecification> Buses;
		std::uint32_t MaxVoices{ 64 };
		std::uint32_t SampleRate{ 48000 };
		float StopFadeSeconds{ 0.01f };
	};

}
