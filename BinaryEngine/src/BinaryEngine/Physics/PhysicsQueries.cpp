#include "pch.h"
#include "BinaryEngine/Physics/PhysicsQueries.h"

namespace BinaryEngine {

	QueryShape QueryShape::Point(Vector2f position)
	{
		return QueryShape{ QueryShapeType::Point, position, Vector2f{ 0.0f, 0.0f }, 0.0f };
	}

	QueryShape QueryShape::Circle(Vector2f center, float radius)
	{
		return QueryShape{ QueryShapeType::Circle, center, Vector2f{ radius * 2.0f, radius * 2.0f }, 0.0f };
	}

	QueryShape QueryShape::Box(Vector2f center, Vector2f size, float rotationDegrees)
	{
		return QueryShape{ QueryShapeType::Box, center, size, rotationDegrees };
	}

	QueryShape QueryShape::Capsule(Vector2f center, Vector2f size, float rotationDegrees)
	{
		return QueryShape{ QueryShapeType::Capsule, center, size, rotationDegrees };
	}

	QueryShape::QueryShape(QueryShapeType type, Vector2f center, Vector2f size, float rotationDegrees) :
		m_Type(type), m_Center(center), m_Size(size), m_RotationDegrees(rotationDegrees)
	{}

}
