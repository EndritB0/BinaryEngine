#pragma once
#include <string>
#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	struct WindowSpecification {
		std::string Title{ "Application" };
		Vector2i Resolution{ 1280, 720 };
		std::uint16_t FpsLimit{ 120 };
		bool Fullscreen{ false };
	};

}