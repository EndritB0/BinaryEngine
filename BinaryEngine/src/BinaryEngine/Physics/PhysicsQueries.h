#pragma once

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/PhysicsTypes.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	enum class QueryShapeType {
		Point,
		Circle,
		Box,
		Capsule
	};

	class QueryShape {
	public:
		static QueryShape Point(Vector2f position);
		static QueryShape Circle(Vector2f center, float radius);
		static QueryShape Box(Vector2f center, Vector2f size, float rotationDegrees = 0.0f);
		static QueryShape Capsule(Vector2f center, Vector2f size, float rotationDegrees = 0.0f);

		QueryShapeType GetType() const { return m_Type; }
		Vector2f GetCenter() const { return m_Center; }
		Vector2f GetSize() const { return m_Size; }
		float GetRotation() const { return m_RotationDegrees; }

	private:
		QueryShape(QueryShapeType type, Vector2f center, Vector2f size, float rotationDegrees);

		QueryShapeType m_Type{ QueryShapeType::Point };
		Vector2f m_Center{ 0.0f, 0.0f };
		Vector2f m_Size{ 0.0f, 0.0f };
		float m_RotationDegrees{ 0.0f };
	};

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

	struct OverlapHit {
		BinaryEngine::Entity Entity;
	};

}
