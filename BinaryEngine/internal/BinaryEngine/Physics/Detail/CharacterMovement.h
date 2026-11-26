#pragma once

#include <cstdint>

#include <entt/entt.hpp>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class Scene;

}

namespace BinaryEngine::Detail {

	struct CharacterControllerState;
	struct RigidBodyState;

	void MoveCharacter(std::uint32_t worldId, const entt::registry& registry, Scene& scene, const RigidBodyState& bodyState, CharacterControllerState& characterState, float stepSeconds);
	void TeleportCharacter(CharacterControllerState& characterState, RigidBodyState& bodyState, Vector2f position);
	void EnableCharacterContactControl(std::uint32_t worldId);

}
