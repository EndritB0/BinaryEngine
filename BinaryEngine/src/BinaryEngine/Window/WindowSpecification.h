#pragma once
#include <string>
#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	struct WindowSpecification {
		std::string title{ "Application" };
		Vector2i resolution{ 1280, 720 };
		std::uint16_t fpsLimit{ 120 };
		bool fullscreen{ false };
	};

}