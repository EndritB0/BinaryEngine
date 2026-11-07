#pragma once

#include <cmath>
#include <type_traits>

#include <box2d/box2d.h>

#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine::Detail {

	static_assert(std::is_same_v<b2Pos, b2Vec2>, "The engine passes positions as b2Vec2, which needs Box2D's single-precision build");

	inline b2Vec2 ToBox2DVector(Vector2f vector)
	{
		return b2Vec2{ vector.x, vector.y };
	}

	inline Vector2f ToEngineVector(b2Vec2 vector)
	{
		return Vector2f{ vector.x, vector.y };
	}

	inline bool IsFiniteVector(Vector2f vector)
	{
		return std::isfinite(vector.x) && std::isfinite(vector.y);
	}

}
