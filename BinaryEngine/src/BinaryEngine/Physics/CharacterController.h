#pragma once

#include <span>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Physics/CharacterControllerTypes.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	class CharacterController {
	public:
		CharacterController() = default;

		bool IsValid() const;
		Entity GetEntity() const { return m_Entity; }

		Vector2f GetPosition() const;
		void SetPosition(Vector2f position);
		Vector2f GetVelocity() const;
		void SetVelocity(Vector2f velocity);

		bool IsOnFloor() const;
		bool IsOnWall() const;
		Vector2f GetFloorNormal() const;
		Entity GetFloorEntity() const;
		std::span<const CharacterTouch> GetTouches() const;

	private:
		friend class PhysicsWorld;

		explicit CharacterController(Entity entity) :
			m_Entity(entity) {}

		Entity m_Entity{};
	};

}
