#pragma once

#include <cstdint>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	class PhysicsBody {
	public:
		PhysicsBody() = default;

		bool IsValid() const;
		Entity GetEntity() const { return m_Entity; }

		Vector2f GetPosition() const;
		float GetRotation() const;
		void SetTransform(Vector2f position, float rotationDegrees);

		Vector2f GetLinearVelocity() const;
		void SetLinearVelocity(Vector2f velocity);
		float GetAngularVelocity() const;
		void SetAngularVelocity(float degreesPerSecond);

		void ApplyForce(Vector2f force);
		void ApplyTorque(float torque);
		void ApplyLinearImpulse(Vector2f impulse);
		void ApplyLinearImpulse(Vector2f impulse, Vector2f worldPoint);
		void ApplyAngularImpulse(float impulse);

		float GetMass() const;
		bool IsAwake() const;
		void SetAwake(bool awake);

	private:
		friend class PhysicsWorld;

		PhysicsBody(Entity entity, std::uint64_t bodyId) :
			m_Entity(entity), m_BodyId(bodyId) {}

		Entity m_Entity{};
		std::uint64_t m_BodyId{ 0 };
	};

}
