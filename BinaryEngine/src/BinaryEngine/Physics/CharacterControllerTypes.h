#pragma once

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/PhysicsTypes.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	enum class CharacterMode {
		Grounded,
		Floating
	};

	struct CharacterControllerSpecification {
		CharacterMode Mode{ CharacterMode::Grounded };
		CapsuleColliderSpecification Capsule{};
		float HoverHeight{ 8.0f };
		float SpringFrequency{ 5.0f };
		float SpringDampingRatio{ 0.8f };
		float MaxSlopeAngle{ 45.0f };
		float MaxPushForce{ 0.0f };
		float Mass{ 1.0f };
		bool IsSoft{ false };
	};

	struct CharacterTouch {
		BinaryEngine::Entity Entity;
		Vector2f Point{ 0.0f, 0.0f };
		Vector2f Normal{ 0.0f, 0.0f };
	};

}
