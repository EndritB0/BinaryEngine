#pragma once

#include <cstdint>

#include "BinaryEngine/Renderer/BlendMode.h"

namespace BinaryEngine {

	struct RendererSpecification {
		bool vSync{ false };
		BlendMode blendMode{ BlendMode::Blend };
		std::uint8_t defaultAlpha{ 255 };
	};

}