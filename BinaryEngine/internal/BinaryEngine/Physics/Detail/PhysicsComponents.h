#pragma once

#include <cstdint>
#include <vector>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/CharacterControllerTypes.h"
#include "BinaryEngine/Physics/PhysicsTypes.h"
#include "BinaryEngine/Scene/Entity.h"

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

	struct CharacterControllerState {
		std::uint64_t ShapeId{ 0 };
		CharacterMode Mode{ CharacterMode::Grounded };
		float HoverHeight{ 0.0f };
		float SpringFrequency{ 0.0f };
		float SpringDampingRatio{ 0.0f };
		float MinFloorNormalY{ 0.0f };
		float MaxPushForce{ 0.0f };
		float Mass{ 0.0f };
		bool IsSoft{ false };
		Vector2f Velocity{ 0.0f, 0.0f };
		float SpringVelocity{ 0.0f };
		bool IsOnFloor{ false };
		Vector2f FloorNormal{ 0.0f, 0.0f };
		Entity FloorEntity{};
		Vector2f FloorVelocity{ 0.0f, 0.0f };
		std::vector<CharacterTouch> Touches;
	};

}
