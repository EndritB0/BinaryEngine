#pragma once

#include <optional>

#include "BinaryEngine/Physics/PhysicsWorldSpecification.h"

namespace BinaryEngine {

	struct SceneSpecification {
		std::optional<PhysicsWorldSpecification> Physics{};
	};

}
