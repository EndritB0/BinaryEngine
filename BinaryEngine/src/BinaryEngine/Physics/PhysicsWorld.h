#pragma once

#include <cstdint>
#include <optional>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Core/Timestep.h"
#include "BinaryEngine/Physics/PhysicsWorldSpecification.h"

namespace BinaryEngine {

	class PhysicsWorld {
	private:
		struct Passkey {
			explicit Passkey() = default;
		};

	public:
		PhysicsWorld(Passkey, const std::optional<PhysicsWorldSpecification>& specification);
		~PhysicsWorld();

		PhysicsWorld(const PhysicsWorld&) = delete;
		PhysicsWorld& operator=(const PhysicsWorld&) = delete;
		PhysicsWorld(PhysicsWorld&&) = delete;
		PhysicsWorld& operator=(PhysicsWorld&&) = delete;

		bool IsValid() const;
		Vector2f GetGravity() const;
		void SetGravity(Vector2f gravity);
		std::int32_t GetBodyCount() const;
		std::uint64_t GetStepCount() const { return m_StepCount; }

	private:
		friend class Scene;

		void Step(TimeStep fixedStep);

	private:
		std::uint32_t m_WorldId{ 0 };
		std::int32_t m_SubStepCount{ 0 };
		std::uint64_t m_StepCount{ 0 };
	};

}
