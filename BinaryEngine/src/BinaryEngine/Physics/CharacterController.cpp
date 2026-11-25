#include "pch.h"
#include "BinaryEngine/Physics/CharacterController.h"

#include "BinaryEngine/Physics/Detail/CharacterMovement.h"
#include "BinaryEngine/Physics/Detail/PhysicsComponents.h"
#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"
#include "BinaryEngine/Scene/Components.h"

namespace {

	BinaryEngine::Detail::CharacterControllerState* FindCharacterState(BinaryEngine::Entity entity)
	{
		if (!entity.IsValid() || !entity.HasComponent<BinaryEngine::Detail::CharacterControllerState>())
		{
			return nullptr;
		}

		return &entity.GetComponent<BinaryEngine::Detail::CharacterControllerState>();
	}

}

namespace BinaryEngine {

	bool CharacterController::IsValid() const
	{
		return FindCharacterState(m_Entity) != nullptr;
	}

	Vector2f CharacterController::GetPosition() const
	{
		if (!IsValid())
		{
			return Vector2f{ 0.0f, 0.0f };
		}

		Entity entity{ m_Entity };
		return entity.GetComponent<Detail::RigidBodyState>().CurrentPose.Position;
	}

	void CharacterController::SetPosition(Vector2f position)
	{
		if (!IsValid() || !Detail::IsFiniteVector(position))
		{
			return;
		}

		Detail::TeleportCharacter(m_Entity.GetComponent<Detail::RigidBodyState>(), position);

		if (m_Entity.HasComponent<TransformComponent>())
		{
			Transform& transform{ m_Entity.GetComponent<TransformComponent>().transform };
			transform.Position.x = position.x;
			transform.Position.y = position.y;
			transform.Rotation.z = 0.0f;
		}
	}

	Vector2f CharacterController::GetVelocity() const
	{
		const Detail::CharacterControllerState* characterState{ FindCharacterState(m_Entity) };
		return characterState ? characterState->Velocity : Vector2f{ 0.0f, 0.0f };
	}

	void CharacterController::SetVelocity(Vector2f velocity)
	{
		Detail::CharacterControllerState* characterState{ FindCharacterState(m_Entity) };
		if (characterState && Detail::IsFiniteVector(velocity))
		{
			characterState->Velocity = velocity;
		}
	}

}
