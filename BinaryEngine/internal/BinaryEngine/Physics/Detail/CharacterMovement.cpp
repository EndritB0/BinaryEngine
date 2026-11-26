#include "pch.h"
#include "BinaryEngine/Physics/Detail/CharacterMovement.h"

#include <box2d/box2d.h>

#include "BinaryEngine/Physics/Detail/PhysicsComponents.h"
#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"
#include "BinaryEngine/Scene/Components.h"

namespace {

	constexpr int s_MaxMoveIterations{ 5 };
	constexpr std::size_t s_MaxCollisionPlanes{ 16 };
	constexpr float s_MinMoveMeters{ 0.01f };
	constexpr float s_LandingSpeedMetersPerSecond{ 0.01f };
	constexpr float s_SoftPushLimitMeters{ 0.025f };

	struct TouchedShape {
		b2ShapeId ShapeId;
		b2Vec2 Point;
	};

	struct PlaneGather {
		const entt::registry& Registry;
		b2ShapeId OwnShapeId;
		const BinaryEngine::Detail::CharacterControllerState& Character;
		std::array<b2CollisionPlane, s_MaxCollisionPlanes> Planes{};
		std::array<TouchedShape, s_MaxCollisionPlanes> TouchedShapes{};
		int PlaneCount{ 0 };
		bool SawDropShape{ false };
	};

	struct CapsuleSweep {
		const entt::registry& Registry;
		b2ShapeId OwnShapeId;
		const BinaryEngine::Detail::CharacterControllerState& Character;
		b2Vec2 Position;
		b2Capsule Capsule;
		b2Vec2 Translation;
		float Fraction{ 1.0f };
	};

	struct FootRay {
		const entt::registry& Registry;
		const BinaryEngine::Detail::CharacterControllerState& Character;
		b2ShapeId OwnShapeId;
		b2ShapeId ShapeId{};
		b2Vec2 Point{};
		b2Vec2 Normal{};
		float Fraction{ 1.0f };
		bool Hit{ false };
	};

	bool IsSolidShape(b2ShapeId shapeId, b2ShapeId ownShapeId)
	{
		return !B2_ID_EQUALS(shapeId, ownShapeId) && !b2Shape_IsSensor(shapeId);
	}

	bool IsDynamicShape(b2ShapeId shapeId)
	{
		return b2Body_GetType(b2Shape_GetBody(shapeId)) == b2_dynamicBody;
	}

	bool IsSoftController(const entt::registry& registry, b2ShapeId shapeId)
	{
		const BinaryEngine::Detail::CharacterControllerState* characterState{ registry.try_get<BinaryEngine::Detail::CharacterControllerState>(BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(shapeId))) };
		return characterState && characterState->IsSoft;
	}

	template<typename ColliderComponent>
	bool IsOneWayCollider(const entt::registry& registry, entt::entity entity, std::uint64_t shapeId)
	{
		const BinaryEngine::Detail::ColliderState<ColliderComponent>* colliderState{ registry.try_get<BinaryEngine::Detail::ColliderState<ColliderComponent>>(entity) };
		return colliderState && colliderState->ShapeId == shapeId && colliderState->OneWay;
	}

	bool IsOneWayShape(const entt::registry& registry, b2ShapeId shapeId)
	{
		const entt::entity entity{ BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(shapeId)) };
		const std::uint64_t storedShapeId{ b2StoreShapeId(shapeId) };
		return IsOneWayCollider<BinaryEngine::BoxColliderComponent>(registry, entity, storedShapeId) || IsOneWayCollider<BinaryEngine::CircleColliderComponent>(registry, entity, storedShapeId) || IsOneWayCollider<BinaryEngine::CapsuleColliderComponent>(registry, entity, storedShapeId);
	}

	bool IsOneWayFor(const entt::registry& registry, const BinaryEngine::Detail::CharacterControllerState& character, b2ShapeId shapeId)
	{
		return character.Mode == BinaryEngine::CharacterMode::Grounded && IsOneWayShape(registry, shapeId);
	}

	bool IsDroppedShape(const BinaryEngine::Detail::CharacterControllerState& character, b2ShapeId shapeId)
	{
		return character.DropShapeId != 0 && character.DropShapeId == b2StoreShapeId(shapeId);
	}

	bool GatherPlane(b2ShapeId shapeId, const b2PlaneResult* planeResult, void* context)
	{
		PlaneGather& gather{ *static_cast<PlaneGather*>(context) };
		if (static_cast<std::size_t>(gather.PlaneCount) >= s_MaxCollisionPlanes)
		{
			return false;
		}

		if (!IsSolidShape(shapeId, gather.OwnShapeId))
		{
			return true;
		}

		if (IsDroppedShape(gather.Character, shapeId))
		{
			gather.SawDropShape = true;
			return true;
		}

		b2Plane plane{ planeResult->plane };
		if (!IsDynamicShape(shapeId))
		{
			plane.offset -= B2_LINEAR_SLOP;
		}

		if (IsOneWayFor(gather.Registry, gather.Character, shapeId) && (plane.normal.y < gather.Character.MinFloorNormalY || plane.offset > B2_LINEAR_SLOP))
		{
			return true;
		}

		const bool isSoft{ IsSoftController(gather.Registry, shapeId) };
		const float pushLimit{ isSoft ? s_SoftPushLimitMeters * b2GetLengthUnitsPerMeter() : std::numeric_limits<float>::max() };
		const std::size_t planeIndex{ static_cast<std::size_t>(gather.PlaneCount) };
		gather.Planes[planeIndex] = b2CollisionPlane{ plane, pushLimit, 0.0f, !isSoft };
		gather.TouchedShapes[planeIndex] = TouchedShape{ shapeId, b2TransformPoint(b2Body_GetTransform(b2Shape_GetBody(shapeId)), planeResult->point) };
		gather.PlaneCount++;
		return static_cast<std::size_t>(gather.PlaneCount) < s_MaxCollisionPlanes;
	}

	b2CastOutput CastAgainstShape(b2ShapeId shapeId, const b2ShapeCastInput& input)
	{
		switch (b2Shape_GetType(shapeId))
		{
			case b2_circleShape:
			{
				const b2Circle circle{ b2Shape_GetCircle(shapeId) };
				return b2CastShapeCircle(&circle, &input);
			}

			case b2_capsuleShape:
			{
				const b2Capsule capsule{ b2Shape_GetCapsule(shapeId) };
				return b2CastShapeCapsule(&capsule, &input);
			}

			case b2_polygonShape:
			{
				const b2Polygon polygon{ b2Shape_GetPolygon(shapeId) };
				return b2CastShapePolygon(&polygon, &input);
			}

			default: return b2CastOutput{};
		}
	}

	bool SweepAgainstShape(b2ShapeId shapeId, void* context)
	{
		CapsuleSweep& sweep{ *static_cast<CapsuleSweep*>(context) };
		if (!IsSolidShape(shapeId, sweep.OwnShapeId) || IsSoftController(sweep.Registry, shapeId) || IsDroppedShape(sweep.Character, shapeId))
		{
			return true;
		}

		const bool isOneWay{ IsOneWayFor(sweep.Registry, sweep.Character, shapeId) };
		if (isOneWay && sweep.Translation.y >= 0.0f)
		{
			return true;
		}

		const b2Transform transform{ b2Body_GetTransform(b2Shape_GetBody(shapeId)) };
		const std::array<b2Vec2, 2> centers{ b2InvTransformPoint(transform, sweep.Position + sweep.Capsule.center1), b2InvTransformPoint(transform, sweep.Position + sweep.Capsule.center2) };

		b2ShapeCastInput input{};
		input.proxy = b2MakeProxy(centers.data(), static_cast<int>(centers.size()), sweep.Capsule.radius + (IsDynamicShape(shapeId) ? B2_LINEAR_SLOP : 0.0f));
		input.translation = b2InvRotateVector(transform.q, sweep.Translation);
		input.maxFraction = 1.0f;
		input.canEncroach = true;

		const b2CastOutput output{ CastAgainstShape(shapeId, input) };
		const bool blocks{ !isOneWay || b2RotateVector(transform.q, output.normal).y >= sweep.Character.MinFloorNormalY };
		if (output.hit && output.fraction > 0.0f && blocks)
		{
			sweep.Fraction = std::min(sweep.Fraction, output.fraction);
		}

		return true;
	}

	float SweepCapsule(b2WorldId worldId, const entt::registry& registry, b2ShapeId ownShapeId, const BinaryEngine::Detail::CharacterControllerState& character, b2Vec2 position, const b2Capsule& capsule, b2Vec2 translation, b2QueryFilter filter)
	{
		const std::array<b2Vec2, 2> centers{ capsule.center1, capsule.center2 };
		const b2AABB startBox{ b2MakeAABB(centers.data(), static_cast<int>(centers.size()), capsule.radius + B2_LINEAR_SLOP) };
		const b2AABB endBox{ startBox.lowerBound + translation, startBox.upperBound + translation };

		CapsuleSweep sweep{ registry, ownShapeId, character, position, capsule, translation };
		b2World_OverlapAABB(worldId, position, b2AABB_Union(startBox, endBox), filter, SweepAgainstShape, &sweep);
		return sweep.Fraction;
	}

	b2Vec2 GatherAndSolve(b2WorldId worldId, b2Vec2 position, const b2Capsule& capsule, b2QueryFilter filter, b2Vec2 target, PlaneGather& gather)
	{
		gather.PlaneCount = 0;
		gather.SawDropShape = false;
		b2World_CollideMover(worldId, position, &capsule, filter, GatherPlane, &gather);
		return b2SolvePlanes(target - position, gather.Planes.data(), gather.PlaneCount).delta;
	}

	float KeepClosestFloorHit(b2ShapeId shapeId, b2Pos point, b2Vec2 normal, float fraction, void* context)
	{
		FootRay& ray{ *static_cast<FootRay*>(context) };
		if (fraction == 0.0f || !IsSolidShape(shapeId, ray.OwnShapeId) || IsDroppedShape(ray.Character, shapeId) || (IsOneWayFor(ray.Registry, ray.Character, shapeId) && normal.y < ray.Character.MinFloorNormalY))
		{
			return -1.0f;
		}

		if (!ray.Hit || fraction < ray.Fraction)
		{
			ray.ShapeId = shapeId;
			ray.Point = point;
			ray.Normal = normal;
			ray.Fraction = fraction;
			ray.Hit = true;
		}

		return ray.Fraction;
	}

	b2Vec2 GetLowerCenter(const b2Capsule& capsule)
	{
		return b2Vec2{ (capsule.center1.x + capsule.center2.x) * 0.5f, std::min(capsule.center1.y, capsule.center2.y) };
	}

	void StoreTouches(const PlaneGather& gather, BinaryEngine::Scene& scene, BinaryEngine::Detail::CharacterControllerState& characterState)
	{
		characterState.Touches.clear();
		characterState.Touches.reserve(static_cast<std::size_t>(gather.PlaneCount));
		for (int planeIndex{ 0 }; planeIndex < gather.PlaneCount; planeIndex++)
		{
			const std::size_t index{ static_cast<std::size_t>(planeIndex) };
			const BinaryEngine::Entity entity{ BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(gather.TouchedShapes[index].ShapeId)), &scene };
			characterState.Touches.emplace_back(entity, BinaryEngine::Detail::ToEngineVector(gather.TouchedShapes[index].Point), BinaryEngine::Detail::ToEngineVector(gather.Planes[index].plane.normal));
		}
	}

	void PushBodies(const PlaneGather& gather, b2Vec2 characterVelocity, float maxImpulse)
	{
		for (int planeIndex{ 0 }; planeIndex < gather.PlaneCount; planeIndex++)
		{
			const TouchedShape& touchedShape{ gather.TouchedShapes[static_cast<std::size_t>(planeIndex)] };
			const b2BodyId bodyId{ b2Shape_GetBody(touchedShape.ShapeId) };
			if (b2Body_GetType(bodyId) != b2_dynamicBody)
			{
				continue;
			}

			const b2Vec2 pushNormal{ -gather.Planes[static_cast<std::size_t>(planeIndex)].plane.normal };
			const float mass{ b2Body_GetMass(bodyId) };
			const float rotationalInertia{ b2Body_GetRotationalInertia(bodyId) };
			const float leverCrossNormal{ b2Cross(touchedShape.Point - b2Body_GetWorldCenter(bodyId), pushNormal) };
			const float inverseEffectiveMass{ (mass > 0.0f ? 1.0f / mass : 0.0f) + (rotationalInertia > 0.0f ? leverCrossNormal * leverCrossNormal / rotationalInertia : 0.0f) };
			if (inverseEffectiveMass <= 0.0f)
			{
				continue;
			}

			const float normalSpeed{ b2Dot(b2Body_GetWorldPointVelocity(bodyId, touchedShape.Point) - characterVelocity, pushNormal) };
			const float impulse{ std::min(std::max(-normalSpeed / inverseEffectiveMass, 0.0f), maxImpulse) };
			if (impulse > 0.0f)
			{
				b2Body_ApplyLinearImpulse(bodyId, impulse * pushNormal, touchedShape.Point, true);
			}
		}
	}

	void DisableContactWhileMovingIn(b2ShapeId shapeIdA, b2ShapeId shapeIdB, b2Manifold* manifold, [[maybe_unused]] void* context)
	{
		const b2BodyId bodyIdA{ b2Shape_GetBody(shapeIdA) };
		const b2BodyId bodyIdB{ b2Shape_GetBody(shapeIdB) };
		float approachSpeed{ 0.0f };
		if (b2Body_GetType(bodyIdA) == b2_kinematicBody)
		{
			approachSpeed = b2Dot(b2Body_GetLinearVelocity(bodyIdA), manifold->normal);
		}
		else if (b2Body_GetType(bodyIdB) == b2_kinematicBody)
		{
			approachSpeed = -b2Dot(b2Body_GetLinearVelocity(bodyIdB), manifold->normal);
		}

		if (approachSpeed > 0.0f)
		{
			manifold->pointCount = 0;
		}
	}

	void ExcludeWalkablePlanesFromClip(PlaneGather& gather, float minFloorNormalY)
	{
		for (int planeIndex{ 0 }; planeIndex < gather.PlaneCount; planeIndex++)
		{
			b2CollisionPlane& plane{ gather.Planes[static_cast<std::size_t>(planeIndex)] };
			if (plane.plane.normal.y >= minFloorNormalY)
			{
				plane.clipVelocity = false;
			}
		}
	}

	void LeaveFloor(BinaryEngine::Detail::CharacterControllerState& characterState)
	{
		characterState.SpringVelocity = 0.0f;
		characterState.IsOnFloor = false;
		characterState.FloorNormal = BinaryEngine::Vector2f{ 0.0f, 0.0f };
		characterState.FloorEntity = BinaryEngine::Entity{};
		characterState.FloorShapeId = 0;
		characterState.IsFloorOneWay = false;
		characterState.FloorVelocity = BinaryEngine::Vector2f{ 0.0f, 0.0f };
	}

	void ForgetDroppedShape(BinaryEngine::Detail::CharacterControllerState& characterState)
	{
		characterState.DropShapeId = 0;
		characterState.HasOverlappedDropShape = false;
	}

	void UpdateDroppedShape(const PlaneGather& gather, BinaryEngine::Detail::CharacterControllerState& characterState)
	{
		if (characterState.DropShapeId == 0)
		{
			return;
		}

		if (gather.SawDropShape)
		{
			characterState.HasOverlappedDropShape = true;
		}
		else if (characterState.HasOverlappedDropShape)
		{
			ForgetDroppedShape(characterState);
		}
	}

	void UpdateFloor(b2WorldId worldId, const entt::registry& registry, b2ShapeId shapeId, const b2Capsule& capsule, b2QueryFilter filter, b2Vec2 position, b2Vec2& velocity, BinaryEngine::Detail::CharacterControllerState& characterState, float stepSeconds, BinaryEngine::Scene& scene)
	{
		const float restLength{ capsule.radius + characterState.HoverHeight };
		const float rayLength{ characterState.IsOnFloor ? 2.0f * restLength : restLength };
		FootRay ray{ registry, characterState, shapeId };
		b2World_CastRay(worldId, position + GetLowerCenter(capsule), b2Vec2{ 0.0f, -rayLength }, filter, KeepClosestFloorHit, &ray);

		const float landingSpeed{ s_LandingSpeedMetersPerSecond * b2GetLengthUnitsPerMeter() };
		if (!ray.Hit || ray.Normal.y < characterState.MinFloorNormalY || velocity.y > landingSpeed)
		{
			if (characterState.IsOnFloor)
			{
				velocity = velocity + BinaryEngine::Detail::ToBox2DVector(characterState.FloorVelocity);
			}

			LeaveFloor(characterState);
			return;
		}

		velocity.y = std::max(velocity.y, 0.0f);
		if (!characterState.IsOnFloor)
		{
			ForgetDroppedShape(characterState);
		}

		characterState.SpringVelocity = b2SpringDamper(characterState.SpringFrequency, characterState.SpringDampingRatio, ray.Fraction * rayLength - restLength, characterState.SpringVelocity, stepSeconds);
		characterState.IsOnFloor = true;
		characterState.FloorNormal = BinaryEngine::Detail::ToEngineVector(ray.Normal);
		characterState.FloorEntity = BinaryEngine::Entity{ BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(ray.ShapeId)), &scene };
		characterState.FloorShapeId = b2StoreShapeId(ray.ShapeId);
		characterState.IsFloorOneWay = IsOneWayShape(registry, ray.ShapeId);

		const b2BodyId floorBodyId{ b2Shape_GetBody(ray.ShapeId) };
		characterState.FloorVelocity = BinaryEngine::Detail::ToEngineVector(b2Body_GetWorldPointVelocity(floorBodyId, ray.Point));
		if (b2Body_GetType(floorBodyId) == b2_dynamicBody)
		{
			b2Body_ApplyForce(floorBodyId, characterState.Mass * b2World_GetGravity(worldId), ray.Point, true);
		}
	}

}

namespace BinaryEngine::Detail {

	void MoveCharacter(std::uint32_t worldId, const entt::registry& registry, Scene& scene, const RigidBodyState& bodyState, CharacterControllerState& characterState, float stepSeconds)
	{
		const b2WorldId world{ b2LoadWorldId(worldId) };
		const b2BodyId bodyId{ b2LoadBodyId(bodyState.BodyId) };
		const b2ShapeId shapeId{ b2LoadShapeId(characterState.ShapeId) };
		const b2Capsule capsule{ b2Shape_GetCapsule(shapeId) };
		const b2Filter shapeFilter{ b2Shape_GetFilter(shapeId) };
		const b2QueryFilter filter{ shapeFilter.categoryBits, shapeFilter.maskBits };
		const float minMove{ s_MinMoveMeters * b2GetLengthUnitsPerMeter() };
		b2Vec2 velocity{ ToBox2DVector(characterState.Velocity) };

		b2Vec2 position{ b2Body_GetPosition(bodyId) };
		if (characterState.Mode == CharacterMode::Grounded)
		{
			UpdateFloor(world, registry, shapeId, capsule, filter, position, velocity, characterState, stepSeconds, scene);
		}

		const b2Vec2 floorVelocity{ ToBox2DVector(characterState.FloorVelocity) };
		const b2Vec2 target{ position + stepSeconds * (velocity + floorVelocity + b2Vec2{ 0.0f, characterState.SpringVelocity }) };
		b2Capsule gatherCapsule{ capsule };
		gatherCapsule.radius += B2_LINEAR_SLOP;
		PlaneGather gather{ registry, shapeId, characterState };
		b2Vec2 delta{ GatherAndSolve(world, position, gatherCapsule, filter, target, gather) };
		for (int iteration{ 0 }; iteration < s_MaxMoveIterations; iteration++)
		{
			const b2Vec2 move{ SweepCapsule(world, registry, shapeId, characterState, position, capsule, delta, filter) * delta };
			position = position + move;
			delta = GatherAndSolve(world, position, gatherCapsule, filter, target, gather);
			if (b2LengthSquared(move) < minMove * minMove)
			{
				break;
			}
		}

		StoreTouches(gather, scene, characterState);
		UpdateDroppedShape(gather, characterState);

		if (characterState.MaxPushForce > 0.0f)
		{
			PushBodies(gather, velocity, characterState.MaxPushForce * stepSeconds);
		}

		if (characterState.IsOnFloor)
		{
			ExcludeWalkablePlanesFromClip(gather, characterState.MinFloorNormalY);
		}

		characterState.Velocity = ToEngineVector(b2ClipVector(velocity, gather.Planes.data(), gather.PlaneCount));
		b2Body_SetTargetTransform(bodyId, b2WorldTransform{ position, b2Rot_identity }, stepSeconds, true);
	}

	void EnableCharacterContactControl(std::uint32_t worldId)
	{
		b2World_SetPreSolveCallback(b2LoadWorldId(worldId), DisableContactWhileMovingIn, nullptr, nullptr);
	}

	void TeleportCharacter(CharacterControllerState& characterState, RigidBodyState& bodyState, Vector2f position)
	{
		LeaveFloor(characterState);
		b2Body_SetTransform(b2LoadBodyId(bodyState.BodyId), ToBox2DVector(position), b2Rot_identity);

		const PhysicsPose pose{ position, 0.0f };
		bodyState.PreviousPose = pose;
		bodyState.CurrentPose = pose;
	}

}
