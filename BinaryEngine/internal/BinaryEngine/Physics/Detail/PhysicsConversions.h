#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine::Detail {

	static_assert(std::is_same_v<b2Pos, b2Vec2>, "The engine passes positions as b2Vec2, which needs Box2D's single-precision build");
	static_assert(sizeof(void*) >= sizeof(entt::entity), "An entity is stored in Box2D's user data pointer");

	inline b2Vec2 ToBox2DVector(Vector2f vector)
	{
		return b2Vec2{ vector.x, vector.y };
	}

	inline Vector2f ToEngineVector(b2Vec2 vector)
	{
		return Vector2f{ vector.x, vector.y };
	}

	inline b2Rot ToBox2DRotation(float degrees)
	{
		return b2MakeRot(glm::radians(degrees));
	}

	inline float ToEngineDegrees(b2Rot rotation)
	{
		return glm::degrees(b2Rot_GetAngle(rotation));
	}

	inline void* ToUserData(entt::entity entity)
	{
		return reinterpret_cast<void*>(static_cast<std::uintptr_t>(entt::to_integral(entity)));
	}

	inline entt::entity ToEntity(void* userData)
	{
		return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(userData));
	}

	inline bool IsFiniteVector(Vector2f vector)
	{
		return std::isfinite(vector.x) && std::isfinite(vector.y);
	}

}
