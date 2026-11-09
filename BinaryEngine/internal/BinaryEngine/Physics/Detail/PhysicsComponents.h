#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/PhysicsTypes.h"

namespace BinaryEngine::Detail {

	struct PhysicsPose {
		Vector2f Position{ 0.0f, 0.0f };
		float RotationDegrees{ 0.0f };
	};

	struct RigidBodyState {
		std::uint64_t BodyId{ 0 };
		BodyType Type{ BodyType::Dynamic };
		PhysicsPose PreviousPose{};
		PhysicsPose CurrentPose{};
	};

	template<typename ColliderComponent>
	struct ColliderState {
		std::uint64_t ShapeId{ 0 };
		bool HasWarnedWaiting{ false };
	};

}
