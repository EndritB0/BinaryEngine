#include "pch.h"
#include "BinaryEngine/Physics/PhysicsWorld.h"

#include <box2d/box2d.h>

#include "BinaryEngine/Physics/Detail/PhysicsComponents.h"
#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"
#include "BinaryEngine/Physics/PhysicsBody.h"
#include "BinaryEngine/Scene/Components.h"
#include "BinaryEngine/Scene/Scene.h"

namespace {

	constexpr float s_EarthGravityMetresPerSecondSquared{ -9.81f };

	std::string_view GetEntityName(const entt::registry& registry, entt::entity entity)
	{
		const BinaryEngine::TagComponent* tag{ registry.try_get<BinaryEngine::TagComponent>(entity) };
		return tag ? std::string_view{ tag->Tag } : std::string_view{ "Entity" };
	}

	std::string_view GetComponentName([[maybe_unused]] const BinaryEngine::BoxColliderSpecification& specification)
	{
		return "BoxColliderComponent";
	}

	std::string_view GetComponentName([[maybe_unused]] const BinaryEngine::CircleColliderSpecification& specification)
	{
		return "CircleColliderComponent";
	}

	b2BodyType ToBox2DBodyType(BinaryEngine::BodyType type)
	{
		switch (type)
		{
			case BinaryEngine::BodyType::Static: return b2_staticBody;
			case BinaryEngine::BodyType::Kinematic: return b2_kinematicBody;
			case BinaryEngine::BodyType::Dynamic: return b2_dynamicBody;
		}

		return b2_dynamicBody;
	}

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

	float ResolveGravityScale(float gravityScale, const entt::registry& registry, entt::entity entity)
	{
		if (std::isfinite(gravityScale))
		{
			return gravityScale;
		}

		const float defaultGravityScale{ BinaryEngine::RigidBodySpecification{}.GravityScale };
		CORE_WARN("[Physics] '{}' ({}) has RigidBodySpecification::GravityScale {}, which isn't finite. Using {}", GetEntityName(registry, entity), entity, gravityScale, defaultGravityScale);
		return defaultGravityScale;
	}

	float ResolveDamping(float damping, float defaultDamping, std::string_view fieldName, const entt::registry& registry, entt::entity entity)
	{
		if (std::isfinite(damping) && damping >= 0.0f)
		{
			return damping;
		}

		CORE_WARN("[Physics] '{}' ({}) has RigidBodySpecification::{} {}, which must be finite and at least 0. Using {}", GetEntityName(registry, entity), entity, fieldName, damping, defaultDamping);
		return defaultDamping;
	}

	bool IsValidMaterialValue(float value, std::string_view fieldName, std::string_view componentName, const entt::registry& registry, entt::entity entity)
	{
		if (std::isfinite(value) && value >= 0.0f)
		{
			return true;
		}

		CORE_ERROR("[Physics] '{}' ({}) has a {} with Material.{} {}, which must be finite and at least 0, so it gets no shape", GetEntityName(registry, entity), entity, componentName, fieldName, value);
		return false;
	}

	bool IsValidMaterial(const BinaryEngine::PhysicsMaterial& material, std::string_view componentName, const entt::registry& registry, entt::entity entity)
	{
		return IsValidMaterialValue(material.Density, "Density", componentName, registry, entity)
			&& IsValidMaterialValue(material.Friction, "Friction", componentName, registry, entity)
			&& IsValidMaterialValue(material.Restitution, "Restitution", componentName, registry, entity);
	}

	bool IsValidOffset(BinaryEngine::Vector2f offset, std::string_view componentName, const entt::registry& registry, entt::entity entity)
	{
		if (BinaryEngine::Detail::IsFiniteVector(offset))
		{
			return true;
		}

		CORE_ERROR("[Physics] '{}' ({}) has a {} with Offset ({}, {}), which isn't finite, so it gets no shape", GetEntityName(registry, entity), entity, componentName, offset.x, offset.y);
		return false;
	}

	bool IsValidGeometry(const BinaryEngine::BoxColliderSpecification& specification, const entt::registry& registry, entt::entity entity)
	{
		const BinaryEngine::Vector2f size{ specification.Size };
		if (!BinaryEngine::Detail::IsFiniteVector(size) || size.x <= 0.0f || size.y <= 0.0f)
		{
			CORE_ERROR("[Physics] '{}' ({}) has a BoxColliderComponent with Size ({}, {}). Both must be finite and above 0, so it gets no shape", GetEntityName(registry, entity), entity, size.x, size.y);
			return false;
		}

		return IsValidOffset(specification.Offset, GetComponentName(specification), registry, entity);
	}

	bool IsValidGeometry(const BinaryEngine::CircleColliderSpecification& specification, const entt::registry& registry, entt::entity entity)
	{
		if (!std::isfinite(specification.Radius) || specification.Radius <= 0.0f)
		{
			CORE_ERROR("[Physics] '{}' ({}) has a CircleColliderComponent with Radius {}, which must be finite and above 0, so it gets no shape", GetEntityName(registry, entity), entity, specification.Radius);
			return false;
		}

		return IsValidOffset(specification.Offset, GetComponentName(specification), registry, entity);
	}

	b2ShapeDef MakeShapeDef(const BinaryEngine::ColliderSpecification& collider, entt::entity entity)
	{
		const float worldUnitsPerMeter{ b2GetLengthUnitsPerMeter() };

		b2ShapeDef shapeDef{ b2DefaultShapeDef() };
		shapeDef.userData = BinaryEngine::Detail::ToUserData(entity);
		shapeDef.density = collider.Material.Density / (worldUnitsPerMeter * worldUnitsPerMeter);
		shapeDef.material.friction = collider.Material.Friction;
		shapeDef.material.restitution = collider.Material.Restitution;
		shapeDef.filter.categoryBits = collider.Filter.Category;
		shapeDef.filter.maskBits = collider.Filter.Mask;
		shapeDef.isSensor = collider.IsTrigger;
		shapeDef.enableContactEvents = collider.EnableContactEvents;
		shapeDef.enableHitEvents = collider.EnableHitEvents;
		shapeDef.enableSensorEvents = collider.EnableTriggerEvents;
		return shapeDef;
	}

	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef, const BinaryEngine::BoxColliderSpecification& specification)
	{
		const b2Polygon box{ b2MakeOffsetBox(specification.Size.x * 0.5f, specification.Size.y * 0.5f, BinaryEngine::Detail::ToBox2DVector(specification.Offset), b2Rot_identity) };
		return b2CreatePolygonShape(bodyId, &shapeDef, &box);
	}

	b2ShapeId CreateShape(b2BodyId bodyId, const b2ShapeDef& shapeDef, const BinaryEngine::CircleColliderSpecification& specification)
	{
		const b2Circle circle{ BinaryEngine::Detail::ToBox2DVector(specification.Offset), specification.Radius };
		return b2CreateCircleShape(bodyId, &shapeDef, &circle);
	}

	BinaryEngine::Detail::PhysicsPose ToPose(const b2WorldTransform& transform)
	{
		return BinaryEngine::Detail::PhysicsPose{ BinaryEngine::Detail::ToEngineVector(transform.p), BinaryEngine::Detail::ToEngineDegrees(transform.q) };
	}

	float WrapDegrees(float degrees)
	{
		return std::remainder(degrees, 360.0f);
	}

	void UpdateCurrentPoses(entt::registry& registry, const b2BodyEvents& bodyEvents)
	{
		for (std::int32_t eventIndex{ 0 }; eventIndex < bodyEvents.moveCount; eventIndex++)
		{
			const b2BodyMoveEvent& moveEvent{ bodyEvents.moveEvents[eventIndex] };
			BinaryEngine::Detail::RigidBodyState* bodyState{ registry.try_get<BinaryEngine::Detail::RigidBodyState>(BinaryEngine::Detail::ToEntity(moveEvent.userData)) };
			if (bodyState)
			{
				bodyState->CurrentPose = ToPose(moveEvent.transform);
			}
		}
	}

	template<typename ColliderComponent>
	void AttachCollider(entt::registry& registry, entt::entity entity, std::uint64_t bodyId)
	{
		BinaryEngine::Detail::ColliderState<ColliderComponent>* colliderState{ registry.try_get<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entity) };
		if (!colliderState || colliderState->ShapeId != 0)
		{
			return;
		}

		const auto& specification{ registry.get<ColliderComponent>(entity).Specification };
		if (!IsValidGeometry(specification, registry, entity) || !IsValidMaterial(specification.Collider.Material, GetComponentName(specification), registry, entity))
		{
			return;
		}

		const b2ShapeDef shapeDef{ MakeShapeDef(specification.Collider, entity) };
		colliderState->ShapeId = b2StoreShapeId(CreateShape(b2LoadBodyId(bodyId), shapeDef, specification));
	}

	template<typename ColliderComponent>
	void AddColliderState(entt::registry& registry, entt::entity entity)
	{
		registry.emplace<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entity);

		const BinaryEngine::Detail::RigidBodyState* bodyState{ registry.try_get<BinaryEngine::Detail::RigidBodyState>(entity) };
		if (bodyState)
		{
			AttachCollider<ColliderComponent>(registry, entity, bodyState->BodyId);
		}
	}

	template<typename ColliderComponent>
	void RemoveColliderState(entt::registry& registry, entt::entity entity)
	{
		registry.remove<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entity);
	}

	void RemoveRigidBodyState(entt::registry& registry, entt::entity entity)
	{
		registry.remove<BinaryEngine::Detail::RigidBodyState>(entity);
	}

	template<typename ColliderComponent>
	std::uint64_t DetachColliderShape(entt::registry& registry, entt::entity entity)
	{
		BinaryEngine::Detail::ColliderState<ColliderComponent>* colliderState{ registry.try_get<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entity) };
		if (!colliderState)
		{
			return 0;
		}

		const std::uint64_t shapeId{ colliderState->ShapeId };
		colliderState->ShapeId = 0;
		return shapeId;
	}

	template<typename ColliderComponent>
	void WarnAboutWaitingColliders(entt::registry& registry)
	{
		auto waitingColliders = registry.view<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entt::exclude<BinaryEngine::RigidBodyComponent>);
		waitingColliders.each([&registry](entt::entity entity, BinaryEngine::Detail::ColliderState<ColliderComponent>& colliderState) {
			if (colliderState.HasWarnedWaiting)
			{
				return;
			}

			colliderState.HasWarnedWaiting = true;
			const std::string_view componentName{ GetComponentName(registry.get<ColliderComponent>(entity).Specification) };
			CORE_WARN("[Physics] '{}' ({}) has a {} but no RigidBodyComponent, so it doesn't collide. Add a RigidBodyComponent to give it a body", GetEntityName(registry, entity), entity, componentName);
		});
	}

	std::optional<entt::entity> FindShapeEntity(b2ShapeId shapeId, const std::unordered_map<std::uint64_t, entt::entity>& destroyedShapes)
	{
		if (b2Shape_IsValid(shapeId))
		{
			return BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(shapeId));
		}

		const auto destroyedShape{ destroyedShapes.find(b2StoreShapeId(shapeId)) };
		if (destroyedShape != destroyedShapes.end())
		{
			return destroyedShape->second;
		}

		CORE_ERROR("[Physics] A Box2D event named a shape the engine never recorded, so the event was dropped");
		return std::nullopt;
	}

}

namespace BinaryEngine {

	PhysicsWorld::PhysicsWorld(Passkey, Scene& scene, const PhysicsWorldSpecification& specification) :
		m_Scene(scene)
	{
		const Vector2f gravity{ ResolveGravity(specification.Gravity) };
		m_SubStepCount = ResolveSubStepCount(specification.SubStepCount);

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

	template<typename ColliderComponent>
	void PhysicsWorld::OnColliderStateRemoved(entt::registry& registry, entt::entity entity)
	{
		const std::uint64_t shapeId{ registry.get<Detail::ColliderState<ColliderComponent>>(entity).ShapeId };
		const b2ShapeId box2DShapeId{ b2LoadShapeId(shapeId) };
		if (shapeId == 0 || !b2Shape_IsValid(box2DShapeId))
		{
			return;
		}

		RecordDestroyedShape(shapeId, entity);
		b2DestroyShape(box2DShapeId, true);
	}

	void PhysicsWorld::Connect(entt::registry& registry)
	{
		registry.storage<Detail::RigidBodyState>();
		registry.storage<Detail::ColliderState<BoxColliderComponent>>();
		registry.storage<Detail::ColliderState<CircleColliderComponent>>();

		registry.on_construct<RigidBodyComponent>().connect<&PhysicsWorld::OnRigidBodyAdded>(*this);
		registry.on_destroy<RigidBodyComponent>().connect<&RemoveRigidBodyState>();
		registry.on_destroy<Detail::RigidBodyState>().connect<&PhysicsWorld::OnRigidBodyStateRemoved>(*this);

		registry.on_construct<BoxColliderComponent>().connect<&AddColliderState<BoxColliderComponent>>();
		registry.on_destroy<BoxColliderComponent>().connect<&RemoveColliderState<BoxColliderComponent>>();
		registry.on_destroy<Detail::ColliderState<BoxColliderComponent>>().connect<&PhysicsWorld::OnColliderStateRemoved<BoxColliderComponent>>(*this);

		registry.on_construct<CircleColliderComponent>().connect<&AddColliderState<CircleColliderComponent>>();
		registry.on_destroy<CircleColliderComponent>().connect<&RemoveColliderState<CircleColliderComponent>>();
		registry.on_destroy<Detail::ColliderState<CircleColliderComponent>>().connect<&PhysicsWorld::OnColliderStateRemoved<CircleColliderComponent>>(*this);
	}

	void PhysicsWorld::Step(entt::registry& registry, TimeStep fixedStep)
	{
		if (!IsValid())
		{
			return;
		}

		registry.view<Detail::RigidBodyState>().each([](Detail::RigidBodyState& bodyState) {
			bodyState.PreviousPose = bodyState.CurrentPose;
		});

		const b2WorldId worldId{ b2LoadWorldId(m_WorldId) };
		b2World_Step(worldId, fixedStep.GetSeconds(), m_SubStepCount);
		UpdateCurrentPoses(registry, b2World_GetBodyEvents(worldId));
		ConvertEvents();
		m_DestroyedShapes.clear();

		WarnAboutWaitingColliders<BoxColliderComponent>(registry);
		WarnAboutWaitingColliders<CircleColliderComponent>(registry);
		m_StepCount++;
	}

	void PhysicsWorld::Interpolate(entt::registry& registry, float interpolationAlpha)
	{
		if (!IsValid())
		{
			return;
		}

		registry.view<const Detail::RigidBodyState, TransformComponent>().each([interpolationAlpha](const Detail::RigidBodyState& bodyState, TransformComponent& transformComponent) {
			if (bodyState.Type == BodyType::Static)
			{
				return;
			}

			const Detail::PhysicsPose& previousPose{ bodyState.PreviousPose };
			const Detail::PhysicsPose& currentPose{ bodyState.CurrentPose };
			const Vector2f position{ glm::mix(previousPose.Position, currentPose.Position, interpolationAlpha) };
			const float rotationChange{ WrapDegrees(currentPose.RotationDegrees - previousPose.RotationDegrees) };

			Transform& transform{ transformComponent.transform };
			transform.Position.x = position.x;
			transform.Position.y = position.y;
			transform.Rotation.z = previousPose.RotationDegrees + rotationChange * interpolationAlpha;
		});
	}

	PhysicsBody PhysicsWorld::FindBody(entt::registry& registry, Entity entity)
	{
		const entt::entity entityHandle{ entity };
		if (!entity || entity != Entity{ entityHandle, &m_Scene } || !registry.valid(entityHandle))
		{
			return PhysicsBody{};
		}

		const Detail::RigidBodyState* bodyState{ registry.try_get<Detail::RigidBodyState>(entityHandle) };
		if (!bodyState)
		{
			return PhysicsBody{};
		}

		return PhysicsBody{ entity, bodyState->BodyId };
	}

	void PhysicsWorld::ConvertEvents()
	{
		m_ContactBeginEvents.clear();
		m_ContactEndEvents.clear();
		m_ContactHitEvents.clear();
		m_TriggerBeginEvents.clear();
		m_TriggerEndEvents.clear();

		const auto findEntity = [this](b2ShapeId shapeId) -> std::optional<Entity> {
			const std::optional<entt::entity> entity{ FindShapeEntity(shapeId, m_DestroyedShapes) };
			if (!entity)
			{
				return std::nullopt;
			}

			return Entity{ *entity, &m_Scene };
		};

		const b2WorldId worldId{ b2LoadWorldId(m_WorldId) };
		const b2ContactEvents contactEvents{ b2World_GetContactEvents(worldId) };

		m_ContactBeginEvents.reserve(static_cast<std::size_t>(contactEvents.beginCount));
		for (std::int32_t eventIndex{ 0 }; eventIndex < contactEvents.beginCount; eventIndex++)
		{
			const b2ContactBeginTouchEvent& beginEvent{ contactEvents.beginEvents[eventIndex] };
			const std::optional<Entity> firstEntity{ findEntity(beginEvent.shapeIdA) };
			const std::optional<Entity> secondEntity{ findEntity(beginEvent.shapeIdB) };
			if (firstEntity && secondEntity)
			{
				m_ContactBeginEvents.emplace_back(*firstEntity, *secondEntity);
			}
		}

		m_ContactEndEvents.reserve(static_cast<std::size_t>(contactEvents.endCount));
		for (std::int32_t eventIndex{ 0 }; eventIndex < contactEvents.endCount; eventIndex++)
		{
			const b2ContactEndTouchEvent& endEvent{ contactEvents.endEvents[eventIndex] };
			const std::optional<Entity> firstEntity{ findEntity(endEvent.shapeIdA) };
			const std::optional<Entity> secondEntity{ findEntity(endEvent.shapeIdB) };
			if (firstEntity && secondEntity)
			{
				m_ContactEndEvents.emplace_back(*firstEntity, *secondEntity);
			}
		}

		m_ContactHitEvents.reserve(static_cast<std::size_t>(contactEvents.hitCount));
		for (std::int32_t eventIndex{ 0 }; eventIndex < contactEvents.hitCount; eventIndex++)
		{
			const b2ContactHitEvent& hitEvent{ contactEvents.hitEvents[eventIndex] };
			const std::optional<Entity> firstEntity{ findEntity(hitEvent.shapeIdA) };
			const std::optional<Entity> secondEntity{ findEntity(hitEvent.shapeIdB) };
			if (firstEntity && secondEntity)
			{
				m_ContactHitEvents.emplace_back(*firstEntity, *secondEntity, Detail::ToEngineVector(hitEvent.point), Detail::ToEngineVector(hitEvent.normal), hitEvent.approachSpeed);
			}
		}

		const b2SensorEvents sensorEvents{ b2World_GetSensorEvents(worldId) };

		m_TriggerBeginEvents.reserve(static_cast<std::size_t>(sensorEvents.beginCount));
		for (std::int32_t eventIndex{ 0 }; eventIndex < sensorEvents.beginCount; eventIndex++)
		{
			const b2SensorBeginTouchEvent& beginEvent{ sensorEvents.beginEvents[eventIndex] };
			const std::optional<Entity> triggerEntity{ findEntity(beginEvent.sensorShapeId) };
			const std::optional<Entity> visitorEntity{ findEntity(beginEvent.visitorShapeId) };
			if (triggerEntity && visitorEntity)
			{
				m_TriggerBeginEvents.emplace_back(*triggerEntity, *visitorEntity);
			}
		}

		m_TriggerEndEvents.reserve(static_cast<std::size_t>(sensorEvents.endCount));
		for (std::int32_t eventIndex{ 0 }; eventIndex < sensorEvents.endCount; eventIndex++)
		{
			const b2SensorEndTouchEvent& endEvent{ sensorEvents.endEvents[eventIndex] };
			const std::optional<Entity> triggerEntity{ findEntity(endEvent.sensorShapeId) };
			const std::optional<Entity> visitorEntity{ findEntity(endEvent.visitorShapeId) };
			if (triggerEntity && visitorEntity)
			{
				m_TriggerEndEvents.emplace_back(*triggerEntity, *visitorEntity);
			}
		}
	}

	void PhysicsWorld::RecordDestroyedShape(std::uint64_t shapeId, entt::entity entity)
	{
		if (shapeId != 0 && b2Shape_IsValid(b2LoadShapeId(shapeId)))
		{
			m_DestroyedShapes.insert_or_assign(shapeId, entity);
		}
	}

	void PhysicsWorld::OnRigidBodyAdded(entt::registry& registry, entt::entity entity)
	{
		if (!IsValid())
		{
			return;
		}

		const TransformComponent* transformComponent{ registry.try_get<TransformComponent>(entity) };
		if (!transformComponent)
		{
			CORE_ERROR("[Physics] '{}' ({}) has a RigidBodyComponent but no TransformComponent, so it gets no body", GetEntityName(registry, entity), entity);
			return;
		}

		const Transform& transform{ transformComponent->transform };
		const Detail::PhysicsPose creationPose{ Vector2f{ transform.Position.x, transform.Position.y }, transform.Rotation.z };
		if (!Detail::IsFiniteVector(creationPose.Position) || !std::isfinite(creationPose.RotationDegrees))
		{
			CORE_ERROR("[Physics] '{}' ({}) has a TransformComponent that isn't finite (position ({}, {}), rotation {}), so it gets no body", GetEntityName(registry, entity), entity, creationPose.Position.x, creationPose.Position.y, creationPose.RotationDegrees);
			return;
		}

		const RigidBodySpecification& specification{ registry.get<RigidBodyComponent>(entity).Specification };
		const RigidBodySpecification defaults{};

		b2BodyDef bodyDef{ b2DefaultBodyDef() };
		bodyDef.type = ToBox2DBodyType(specification.Type);
		bodyDef.position = Detail::ToBox2DVector(creationPose.Position);
		bodyDef.rotation = Detail::ToBox2DRotation(creationPose.RotationDegrees);
		bodyDef.gravityScale = ResolveGravityScale(specification.GravityScale, registry, entity);
		bodyDef.linearDamping = ResolveDamping(specification.LinearDamping, defaults.LinearDamping, "LinearDamping", registry, entity);
		bodyDef.angularDamping = ResolveDamping(specification.AngularDamping, defaults.AngularDamping, "AngularDamping", registry, entity);
		bodyDef.motionLocks.angularZ = specification.FixedRotation;
		bodyDef.isBullet = specification.Bullet;
		bodyDef.userData = Detail::ToUserData(entity);

		const std::uint64_t bodyId{ b2StoreBodyId(b2CreateBody(b2LoadWorldId(m_WorldId), &bodyDef)) };
		registry.emplace<Detail::RigidBodyState>(entity, bodyId, specification.Type, creationPose, creationPose);

		AttachCollider<BoxColliderComponent>(registry, entity, bodyId);
		AttachCollider<CircleColliderComponent>(registry, entity, bodyId);
	}

	void PhysicsWorld::OnRigidBodyStateRemoved(entt::registry& registry, entt::entity entity)
	{
		RecordDestroyedShape(DetachColliderShape<BoxColliderComponent>(registry, entity), entity);
		RecordDestroyedShape(DetachColliderShape<CircleColliderComponent>(registry, entity), entity);

		const b2BodyId bodyId{ b2LoadBodyId(registry.get<Detail::RigidBodyState>(entity).BodyId) };
		if (b2Body_IsValid(bodyId))
		{
			b2DestroyBody(bodyId);
		}
	}

}
