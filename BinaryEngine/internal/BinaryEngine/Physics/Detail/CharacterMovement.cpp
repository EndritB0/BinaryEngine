#include "pch.h"
#include "BinaryEngine/Physics/Detail/CharacterMovement.h"

#include <box2d/box2d.h>

#include "BinaryEngine/Physics/Detail/PhysicsComponents.h"
#include "BinaryEngine/Physics/Detail/PhysicsConversions.h"

namespace {

	constexpr int s_MaxMoveIterations{ 5 };
	constexpr std::size_t s_MaxCollisionPlanes{ 16 };
	constexpr float s_MinMoveMeters{ 0.01f };
	constexpr float s_LandingSpeedMetersPerSecond{ 0.01f };

	struct PlaneGather {
		b2ShapeId OwnShapeId;
		std::array<b2CollisionPlane, s_MaxCollisionPlanes> Planes{};
		int PlaneCount{ 0 };
	};

	struct CapsuleSweep {
		b2ShapeId OwnShapeId;
		b2Vec2 Position;
		b2Capsule Capsule;
		b2Vec2 Translation;
		float Fraction{ 1.0f };
	};

	struct FootRay {
		b2ShapeId OwnShapeId;
		b2ShapeId ShapeId{};
		b2Vec2 Normal{};
		float Fraction{ 1.0f };
		bool Hit{ false };
	};

	bool IsSolidShape(b2ShapeId shapeId, b2ShapeId ownShapeId)
	{
		return !B2_ID_EQUALS(shapeId, ownShapeId) && !b2Shape_IsSensor(shapeId);
	}

	bool GatherPlane(b2ShapeId shapeId, const b2PlaneResult* planeResult, void* context)
	{
		PlaneGather& gather{ *static_cast<PlaneGather*>(context) };
		if (!IsSolidShape(shapeId, gather.OwnShapeId))
		{
			return true;
		}

		gather.Planes[static_cast<std::size_t>(gather.PlaneCount)] = b2CollisionPlane{ planeResult->plane, std::numeric_limits<float>::max(), 0.0f, true };
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
		if (!IsSolidShape(shapeId, sweep.OwnShapeId))
		{
			return true;
		}

		const b2Transform transform{ b2Body_GetTransform(b2Shape_GetBody(shapeId)) };
		const std::array<b2Vec2, 2> centers{ b2InvTransformPoint(transform, sweep.Position + sweep.Capsule.center1), b2InvTransformPoint(transform, sweep.Position + sweep.Capsule.center2) };

		b2ShapeCastInput input{};
		input.proxy = b2MakeProxy(centers.data(), static_cast<int>(centers.size()), sweep.Capsule.radius);
		input.translation = b2InvRotateVector(transform.q, sweep.Translation);
		input.maxFraction = 1.0f;
		input.canEncroach = true;

		const b2CastOutput output{ CastAgainstShape(shapeId, input) };
		if (output.hit && output.fraction > 0.0f)
		{
			sweep.Fraction = std::min(sweep.Fraction, output.fraction);
		}

		return true;
	}

	float SweepCapsule(b2WorldId worldId, b2ShapeId ownShapeId, b2Vec2 position, const b2Capsule& capsule, b2Vec2 translation, b2QueryFilter filter)
	{
		const std::array<b2Vec2, 2> centers{ capsule.center1, capsule.center2 };
		const b2AABB startBox{ b2MakeAABB(centers.data(), static_cast<int>(centers.size()), capsule.radius) };
		const b2AABB endBox{ startBox.lowerBound + translation, startBox.upperBound + translation };

		CapsuleSweep sweep{ ownShapeId, position, capsule, translation };
		b2World_OverlapAABB(worldId, position, b2AABB_Union(startBox, endBox), filter, SweepAgainstShape, &sweep);
		return sweep.Fraction;
	}

	float KeepClosestFloorHit(b2ShapeId shapeId, [[maybe_unused]] b2Pos point, b2Vec2 normal, float fraction, void* context)
	{
		FootRay& ray{ *static_cast<FootRay*>(context) };
		if (fraction == 0.0f || !IsSolidShape(shapeId, ray.OwnShapeId))
		{
			return -1.0f;
		}

		if (!ray.Hit || fraction < ray.Fraction)
		{
			ray = FootRay{ ray.OwnShapeId, shapeId, normal, fraction, true };
		}

		return ray.Fraction;
	}

	b2Vec2 GetLowerCenter(const b2Capsule& capsule)
	{
		return b2Vec2{ (capsule.center1.x + capsule.center2.x) * 0.5f, std::min(capsule.center1.y, capsule.center2.y) };
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
	}

	void UpdateFloor(b2WorldId worldId, b2ShapeId shapeId, const b2Capsule& capsule, b2QueryFilter filter, b2Vec2 position, b2Vec2& velocity, BinaryEngine::Detail::CharacterControllerState& characterState, float stepSeconds, BinaryEngine::Scene& scene)
	{
		const float restLength{ capsule.radius + characterState.HoverHeight };
		const float rayLength{ characterState.IsOnFloor ? 2.0f * restLength : restLength };
		FootRay ray{ shapeId };
		b2World_CastRay(worldId, position + GetLowerCenter(capsule), b2Vec2{ 0.0f, -rayLength }, filter, KeepClosestFloorHit, &ray);

		const float landingSpeed{ s_LandingSpeedMetersPerSecond * b2GetLengthUnitsPerMeter() };
		if (!ray.Hit || ray.Normal.y < characterState.MinFloorNormalY || velocity.y > landingSpeed)
		{
			LeaveFloor(characterState);
			return;
		}

		velocity.y = std::max(velocity.y, 0.0f);
		characterState.SpringVelocity = b2SpringDamper(characterState.SpringFrequency, characterState.SpringDampingRatio, ray.Fraction * rayLength - restLength, characterState.SpringVelocity, stepSeconds);
		characterState.IsOnFloor = true;
		characterState.FloorNormal = BinaryEngine::Detail::ToEngineVector(ray.Normal);
		characterState.FloorEntity = BinaryEngine::Entity{ BinaryEngine::Detail::ToEntity(b2Shape_GetUserData(ray.ShapeId)), &scene };
	}

}

namespace BinaryEngine::Detail {

	void MoveCharacter(std::uint32_t worldId, Scene& scene, const RigidBodyState& bodyState, CharacterControllerState& characterState, float stepSeconds)
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
			UpdateFloor(world, shapeId, capsule, filter, position, velocity, characterState, stepSeconds, scene);
		}

		const b2Vec2 target{ position + stepSeconds * (velocity + b2Vec2{ 0.0f, characterState.SpringVelocity }) };
		PlaneGather gather{ shapeId };
		for (int iteration{ 0 }; iteration < s_MaxMoveIterations; iteration++)
		{
			gather.PlaneCount = 0;
			b2World_CollideMover(world, position, &capsule, filter, GatherPlane, &gather);
			const b2PlaneSolverResult solution{ b2SolvePlanes(target - position, gather.Planes.data(), gather.PlaneCount) };
			const b2Vec2 move{ SweepCapsule(world, shapeId, position, capsule, solution.delta, filter) * solution.delta };
			position = position + move;
			if (b2LengthSquared(move) < minMove * minMove)
			{
				break;
			}
		}

		if (characterState.IsOnFloor)
		{
			ExcludeWalkablePlanesFromClip(gather, characterState.MinFloorNormalY);
		}

		characterState.Velocity = ToEngineVector(b2ClipVector(velocity, gather.Planes.data(), gather.PlaneCount));
		b2Body_SetTargetTransform(bodyId, b2WorldTransform{ position, b2Rot_identity }, stepSeconds, true);
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
