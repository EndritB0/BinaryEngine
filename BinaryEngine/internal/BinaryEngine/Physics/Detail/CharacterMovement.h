#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class Scene;

}

namespace BinaryEngine::Detail {

	struct CharacterControllerState;
	struct RigidBodyState;

	void MoveCharacter(std::uint32_t worldId, Scene& scene, const RigidBodyState& bodyState, CharacterControllerState& characterState, float stepSeconds);
	void TeleportCharacter(CharacterControllerState& characterState, RigidBodyState& bodyState, Vector2f position);

}
