#pragma once

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/PhysicsTypes.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	struct QueryFilter {
		CollisionFilter Filter{};
		bool IncludeTriggers{ false };
	};

	struct CastHit {
		BinaryEngine::Entity Entity;
		Vector2f Point{ 0.0f, 0.0f };
		Vector2f Normal{ 0.0f, 0.0f };
		float Fraction{ 0.0f };
	};

}
