#pragma once

#include <cstdint>
#include <optional>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	struct PhysicsWorldSpecification {
		std::optional<Vector2f> Gravity{};
		std::int32_t SubStepCount{ 4 };
	};

}
