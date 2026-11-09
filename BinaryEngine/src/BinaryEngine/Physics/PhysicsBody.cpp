#include "pch.h"
#include "BinaryEngine/Physics/PhysicsBody.h"

#include <box2d/box2d.h>

#include "BinaryEngine/Physics/Detail/PhysicsComponents.h"
#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"
#include "BinaryEngine/Scene/Components.h"

namespace BinaryEngine {

	bool PhysicsBody::IsValid() const
	{
		return m_BodyId != 0 && b2Body_IsValid(b2LoadBodyId(m_BodyId));
	}

	Vector2f PhysicsBody::GetPosition() const
	{
		if (!IsValid())
		{
			return Vector2f{ 0.0f, 0.0f };
		}

		return Detail::ToEngineVector(b2Body_GetPosition(b2LoadBodyId(m_BodyId)));
	}

	float PhysicsBody::GetRotation() const
	{
		if (!IsValid())
		{
			return 0.0f;
		}

		return Detail::ToEngineDegrees(b2Body_GetRotation(b2LoadBodyId(m_BodyId)));
	}

	void PhysicsBody::SetTransform(Vector2f position, float rotationDegrees)
	{
		if (!IsValid() || !m_Entity.IsValid() || !Detail::IsFiniteVector(position) || !std::isfinite(rotationDegrees))
		{
			return;
		}

		b2Body_SetTransform(b2LoadBodyId(m_BodyId), Detail::ToBox2DVector(position), Detail::ToBox2DRotation(rotationDegrees));

		const Detail::PhysicsPose pose{ position, rotationDegrees };
		if (m_Entity.HasComponent<Detail::RigidBodyState>())
		{
			Detail::RigidBodyState& bodyState{ m_Entity.GetComponent<Detail::RigidBodyState>() };
			bodyState.PreviousPose = pose;
			bodyState.CurrentPose = pose;
		}

		if (m_Entity.HasComponent<TransformComponent>())
		{
			Transform& transform{ m_Entity.GetComponent<TransformComponent>().transform };
			transform.Position.x = position.x;
			transform.Position.y = position.y;
			transform.Rotation.z = rotationDegrees;
		}
	}

	Vector2f PhysicsBody::GetLinearVelocity() const
	{
		if (!IsValid())
		{
			return Vector2f{ 0.0f, 0.0f };
		}

		return Detail::ToEngineVector(b2Body_GetLinearVelocity(b2LoadBodyId(m_BodyId)));
	}

	void PhysicsBody::SetLinearVelocity(Vector2f velocity)
	{
		if (!IsValid() || !Detail::IsFiniteVector(velocity))
		{
			return;
		}

		b2Body_SetLinearVelocity(b2LoadBodyId(m_BodyId), Detail::ToBox2DVector(velocity));
	}

	float PhysicsBody::GetAngularVelocity() const
	{
		if (!IsValid())
		{
			return 0.0f;
		}

		return glm::degrees(b2Body_GetAngularVelocity(b2LoadBodyId(m_BodyId)));
	}

	void PhysicsBody::SetAngularVelocity(float degreesPerSecond)
	{
		if (!IsValid() || !std::isfinite(degreesPerSecond))
		{
			return;
		}

		b2Body_SetAngularVelocity(b2LoadBodyId(m_BodyId), glm::radians(degreesPerSecond));
	}

	void PhysicsBody::ApplyForce(Vector2f force)
	{
		if (!IsValid() || !Detail::IsFiniteVector(force))
		{
			return;
		}

		b2Body_ApplyForceToCenter(b2LoadBodyId(m_BodyId), Detail::ToBox2DVector(force), true);
	}

	void PhysicsBody::ApplyTorque(float torque)
	{
		if (!IsValid() || !std::isfinite(torque))
		{
			return;
		}

		b2Body_ApplyTorque(b2LoadBodyId(m_BodyId), torque, true);
	}

	void PhysicsBody::ApplyLinearImpulse(Vector2f impulse)
	{
		if (!IsValid() || !Detail::IsFiniteVector(impulse))
		{
			return;
		}

		b2Body_ApplyLinearImpulseToCenter(b2LoadBodyId(m_BodyId), Detail::ToBox2DVector(impulse), true);
	}

	void PhysicsBody::ApplyLinearImpulse(Vector2f impulse, Vector2f worldPoint)
	{
		if (!IsValid() || !Detail::IsFiniteVector(impulse) || !Detail::IsFiniteVector(worldPoint))
		{
			return;
		}

		b2Body_ApplyLinearImpulse(b2LoadBodyId(m_BodyId), Detail::ToBox2DVector(impulse), Detail::ToBox2DVector(worldPoint), true);
	}

	void PhysicsBody::ApplyAngularImpulse(float impulse)
	{
		if (!IsValid() || !std::isfinite(impulse))
		{
			return;
		}

		b2Body_ApplyAngularImpulse(b2LoadBodyId(m_BodyId), impulse, true);
	}

	float PhysicsBody::GetMass() const
	{
		if (!IsValid())
		{
			return 0.0f;
		}

		return b2Body_GetMass(b2LoadBodyId(m_BodyId));
	}

	bool PhysicsBody::IsAwake() const
	{
		return IsValid() && b2Body_IsAwake(b2LoadBodyId(m_BodyId));
	}

	void PhysicsBody::SetAwake(bool awake)
	{
		if (!IsValid())
		{
			return;
		}

		b2Body_SetAwake(b2LoadBodyId(m_BodyId), awake);
	}

}
