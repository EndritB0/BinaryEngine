#include "pch.h"
#include "BinaryEngine/Physics/PhysicsWorld.h"

#include <box2d/box2d.h>

#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"

namespace {

	constexpr float s_EarthGravityMetresPerSecondSquared{ -9.81f };

	BinaryEngine::Vector2f GetDefaultGravity()
	{
		return BinaryEngine::Vector2f{ 0.0f, s_EarthGravityMetresPerSecondSquared * b2GetLengthUnitsPerMeter() };
	}

	BinaryEngine::Vector2f ResolveGravity(const std::optional<BinaryEngine::Vector2f>& gravity)
	{
		if (!gravity)
		{
			return GetDefaultGravity();
		}

		if (!BinaryEngine::Detail::IsFiniteVector(*gravity))
		{
			const BinaryEngine::Vector2f defaultGravity{ GetDefaultGravity() };
			CORE_WARN("[Physics] PhysicsWorldSpecification::Gravity ({}, {}) isn't finite. Using the default, ({:.2f}, {:.2f})", gravity->x, gravity->y, defaultGravity.x, defaultGravity.y);
			return defaultGravity;
		}

		return *gravity;
	}

	std::int32_t ResolveSubStepCount(std::int32_t subStepCount)
	{
		if (subStepCount >= 1)
		{
			return subStepCount;
		}

		const std::int32_t defaultSubStepCount{ BinaryEngine::PhysicsWorldSpecification{}.SubStepCount };
		CORE_WARN("[Physics] PhysicsWorldSpecification::SubStepCount is {}, below 1. Using {}", subStepCount, defaultSubStepCount);
		return defaultSubStepCount;
	}

}

namespace BinaryEngine {

	PhysicsWorld::PhysicsWorld(Passkey, const std::optional<PhysicsWorldSpecification>& specification)
	{
		if (!specification)
		{
			return;
		}

		const Vector2f gravity{ ResolveGravity(specification->Gravity) };
		m_SubStepCount = ResolveSubStepCount(specification->SubStepCount);

		b2WorldDef worldDef{ b2DefaultWorldDef() };
		worldDef.gravity = Detail::ToBox2DVector(gravity);

		const b2WorldId worldId{ b2CreateWorld(&worldDef) };
		if (B2_IS_NULL(worldId))
		{
			CORE_ERROR("[Physics] Failed to create a physics world: all {} Box2D world slots are in use", B2_MAX_WORLDS);
			return;
		}

		m_WorldId = b2StoreWorldId(worldId);
		CORE_INFO("[Physics] World created: gravity ({:.2f}, {:.2f}) units/s^2, {} sub-steps", gravity.x, gravity.y, m_SubStepCount);
	}

	PhysicsWorld::~PhysicsWorld()
	{
		if (!IsValid())
		{
			return;
		}

		const b2WorldId worldId{ b2LoadWorldId(m_WorldId) };
		const std::int32_t bodyCount{ b2World_GetCounters(worldId).bodyCount };
		b2DestroyWorld(worldId);
		CORE_INFO("[Physics] World destroyed after {} steps ({} bodies)", m_StepCount, bodyCount);
	}

	bool PhysicsWorld::IsValid() const
	{
		return m_WorldId != 0;
	}

	Vector2f PhysicsWorld::GetGravity() const
	{
		if (!IsValid())
		{
			return Vector2f{ 0.0f, 0.0f };
		}

		return Detail::ToEngineVector(b2World_GetGravity(b2LoadWorldId(m_WorldId)));
	}

	void PhysicsWorld::SetGravity(Vector2f gravity)
	{
		if (!IsValid() || !Detail::IsFiniteVector(gravity))
		{
			return;
		}

		b2World_SetGravity(b2LoadWorldId(m_WorldId), Detail::ToBox2DVector(gravity));
	}

	std::int32_t PhysicsWorld::GetBodyCount() const
	{
		if (!IsValid())
		{
			return 0;
		}

		return b2World_GetCounters(b2LoadWorldId(m_WorldId)).bodyCount;
	}

	void PhysicsWorld::Step(TimeStep fixedStep)
	{
		if (!IsValid())
		{
			return;
		}

		b2World_Step(b2LoadWorldId(m_WorldId), fixedStep.GetSeconds(), m_SubStepCount);
		m_StepCount++;
	}

}
