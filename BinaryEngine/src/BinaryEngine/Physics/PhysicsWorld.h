#pragma once

#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Core/Timestep.h"
#include "BinaryEngine/Physics/PhysicsEvents.h"
#include "BinaryEngine/Physics/PhysicsWorldSpecification.h"
#include "BinaryEngine/Scene/Entity.h"

namespace BinaryEngine {

	class PhysicsBody;
	class Scene;

	class PhysicsWorld {
	private:
		struct Passkey {
			explicit Passkey() = default;
		};

	public:
		PhysicsWorld(Passkey, Scene& scene, const PhysicsWorldSpecification& specification);
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
		std::span<const ContactBeginEvent> GetContactBeginEvents() const { return m_ContactBeginEvents; }
		std::span<const ContactEndEvent> GetContactEndEvents() const { return m_ContactEndEvents; }
		std::span<const ContactHitEvent> GetContactHitEvents() const { return m_ContactHitEvents; }
		std::span<const TriggerBeginEvent> GetTriggerBeginEvents() const { return m_TriggerBeginEvents; }
		std::span<const TriggerEndEvent> GetTriggerEndEvents() const { return m_TriggerEndEvents; }

	private:
		friend class Scene;

		void Connect(entt::registry& registry);
		void Step(entt::registry& registry, TimeStep fixedStep);
		void Interpolate(entt::registry& registry, float interpolationAlpha);
		PhysicsBody FindBody(entt::registry& registry, Entity entity);
		void ConvertEvents();
		void RecordDestroyedShape(std::uint64_t shapeId, entt::entity entity);

		void OnRigidBodyAdded(entt::registry& registry, entt::entity entity);
		void OnRigidBodyStateRemoved(entt::registry& registry, entt::entity entity);
		template<typename ColliderComponent>
		void OnColliderStateRemoved(entt::registry& registry, entt::entity entity);

	private:
		Scene& m_Scene;
		std::uint32_t m_WorldId{ 0 };
		std::int32_t m_SubStepCount{ 0 };
		std::uint64_t m_StepCount{ 0 };
		std::vector<ContactBeginEvent> m_ContactBeginEvents;
		std::vector<ContactEndEvent> m_ContactEndEvents;
		std::vector<ContactHitEvent> m_ContactHitEvents;
		std::vector<TriggerBeginEvent> m_TriggerBeginEvents;
		std::vector<TriggerEndEvent> m_TriggerEndEvents;
		std::unordered_map<std::uint64_t, entt::entity> m_DestroyedShapes;
	};

}
