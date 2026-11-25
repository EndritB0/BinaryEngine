#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine::Detail {

	struct CharacterControllerState;
	struct RigidBodyState;

	void MoveCharacter(std::uint32_t worldId, const RigidBodyState& bodyState, CharacterControllerState& characterState, float stepSeconds);
	void TeleportCharacter(RigidBodyState& bodyState, Vector2f position);

}
