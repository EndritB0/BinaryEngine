#pragma once

#include <cstdint>

namespace BinaryEngine {

	struct FrameClockSpecification {
		std::uint16_t FixedUpdateRate{ 60 };
		float MaxFrameTimeSeconds{ 0.25f };
	};

}
