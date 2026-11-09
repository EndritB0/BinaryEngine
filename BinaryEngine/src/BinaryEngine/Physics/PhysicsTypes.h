#pragma once

#include <cstdint>
#include <limits>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	enum class BodyType {
		Static,
		Kinematic,
		Dynamic
	};

	struct PhysicsMaterial {
		float Density{ 1.0f };
		float Friction{ 0.6f };
		float Restitution{ 0.0f };
	};

	struct CollisionFilter {
		std::uint64_t Category{ 1 };
		std::uint64_t Mask{ std::numeric_limits<std::uint64_t>::max() };
	};

	struct RigidBodySpecification {
		BodyType Type{ BodyType::Dynamic };
		float GravityScale{ 1.0f };
		float LinearDamping{ 0.0f };
		float AngularDamping{ 0.0f };
		bool FixedRotation{ false };
		bool Bullet{ false };
	};

	struct ColliderSpecification {
		PhysicsMaterial Material{};
		CollisionFilter Filter{};
		bool IsTrigger{ false };
		bool EnableContactEvents{ true };
		bool EnableHitEvents{ true };
		bool EnableTriggerEvents{ true };
	};

	struct BoxColliderSpecification {
		Vector2f Size{ 0.0f, 0.0f };
		Vector2f Offset{ 0.0f, 0.0f };
		ColliderSpecification Collider{};
	};

	struct CircleColliderSpecification {
		float Radius{ 0.0f };
		Vector2f Offset{ 0.0f, 0.0f };
		ColliderSpecification Collider{};
	};

}
