#include "pch.h"
#include "Sandbox/PhysicsDemoCommon.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Physics/CharacterController.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Scene/Components.h>
#include <BinaryEngine/Scene/Entity.h>

namespace Sandbox {

	namespace {

		constexpr int s_AtlasSquareSize{ 8 };
		constexpr int s_AtlasSquareStride{ 16 };
		constexpr int s_AtlasDiscSize{ 32 };
		constexpr int s_AtlasDiscStride{ 40 };
		constexpr int s_AtlasDiscTop{ 16 };
		constexpr float s_OutlineZ{ 5.0f };
		constexpr float s_OutlineThickness{ 1.5f };
		constexpr float s_RadiusLineThickness{ 1.0f };
		constexpr BinaryEngine::Color s_SolidOutlineColor{ 0x50, 0xFF, 0x50, 0xFF };
		constexpr BinaryEngine::Color s_TriggerOutlineColor{ 0xFF, 0x50, 0xFF, 0xFF };
		constexpr std::size_t s_MaxHitSoundsPerStep{ 4 };
		constexpr float s_LoudestHitSpeed{ 600.0f };
		constexpr float s_QuietestHitVolume{ 0.1f };
		constexpr float s_HitSoundVolume{ 0.4f };

		BinaryEngine::Color GetOutlineColor(const BinaryEngine::ColliderSpecification& collider)
		{
			return collider.IsTrigger ? s_TriggerOutlineColor : s_SolidOutlineColor;
		}

	}

	BinaryEngine::TextureRegion GetAtlasRegion(AtlasSquare square)
	{
		return BinaryEngine::TextureRegion{ { static_cast<int>(square) * s_AtlasSquareStride, 0 }, { s_AtlasSquareSize, s_AtlasSquareSize } };
	}

	BinaryEngine::TextureRegion GetAtlasRegion(AtlasDisc disc)
	{
		return BinaryEngine::TextureRegion{ { static_cast<int>(disc) * s_AtlasDiscStride, s_AtlasDiscTop }, { s_AtlasDiscSize, s_AtlasDiscSize } };
	}

	BinaryEngine::Vector3f GetBoxSpriteScale(BinaryEngine::Vector2f size)
	{
		return { size.x / static_cast<float>(s_AtlasSquareSize), size.y / static_cast<float>(s_AtlasSquareSize), 1.0f };
	}

	BinaryEngine::Vector3f GetCircleSpriteScale(float radius)
	{
		const float scale{ radius * 2.0f / static_cast<float>(s_AtlasDiscSize) };
		return { scale, scale, 1.0f };
	}

	void DrawCapsule(BinaryEngine::Renderer& renderer, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, float rotationDegrees, float sortZ, const BinaryEngine::ShapeSpecification& specification)
	{
		const float radius{ std::min(size.x, size.y) * 0.5f };
		const float centerDistance{ std::abs(size.y - size.x) * 0.5f };
		const float axisDegrees{ rotationDegrees + (size.y >= size.x ? 90.0f : 0.0f) };
		const float axisRadians{ glm::radians(axisDegrees) };
		const BinaryEngine::Vector2f axis{ std::cos(axisRadians), std::sin(axisRadians) };
		const BinaryEngine::Vector2f firstCenter{ center - axis * centerDistance };
		const BinaryEngine::Vector2f secondCenter{ center + axis * centerDistance };
		const BinaryEngine::Vector3f circleScale{ radius * 2.0f, radius * 2.0f, 1.0f };

		renderer.DrawCircle(BinaryEngine::Transform{ { firstCenter.x, firstCenter.y, sortZ }, { 0.0f, 0.0f, 0.0f }, circleScale }, specification);
		renderer.DrawCircle(BinaryEngine::Transform{ { secondCenter.x, secondCenter.y, sortZ }, { 0.0f, 0.0f, 0.0f }, circleScale }, specification);

		if (specification.OutlineThickness <= 0.0f)
		{
			const BinaryEngine::Transform middle{ { center.x, center.y, sortZ }, { 0.0f, 0.0f, axisDegrees }, { centerDistance * 2.0f, radius * 2.0f, 1.0f } };
			renderer.DrawRectangle(middle, specification);
			return;
		}

		const BinaryEngine::Vector2f side{ -axis.y, axis.x };
		const float sideDistance{ radius - specification.OutlineThickness * 0.5f };
		const BinaryEngine::LineSpecification line{ .Color{ specification.Color }, .Thickness{ specification.OutlineThickness }, .SortZ{ sortZ }, .Space{ specification.Space } };
		renderer.DrawLine(firstCenter + side * sideDistance, secondCenter + side * sideDistance, line);
		renderer.DrawLine(firstCenter - side * sideDistance, secondCenter - side * sideDistance, line);
	}

	void DrawColliderOutlines(BinaryEngine::Scene& scene, BinaryEngine::Renderer& renderer)
	{
		scene.GetAllEntitiesWith<BinaryEngine::BoxColliderComponent>().each([&scene, &renderer](entt::entity entityHandle, const BinaryEngine::BoxColliderComponent& collider) {
			const BinaryEngine::PhysicsBody body{ scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &scene }) };
			if (!body.IsValid())
			{
				return;
			}

			const BinaryEngine::Vector2f center{ body.GetPosition() };
			const BinaryEngine::Vector2f size{ collider.Specification.Size };
			const BinaryEngine::Transform outline{ { center.x, center.y, s_OutlineZ }, { 0.0f, 0.0f, body.GetRotation() }, { size.x, size.y, 1.0f } };
			renderer.DrawRectangle(outline, { .Color{ GetOutlineColor(collider.Specification.Collider) }, .OutlineThickness{ s_OutlineThickness } });
		});

		scene.GetAllEntitiesWith<BinaryEngine::CircleColliderComponent>().each([&scene, &renderer](entt::entity entityHandle, const BinaryEngine::CircleColliderComponent& collider) {
			const BinaryEngine::PhysicsBody body{ scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &scene }) };
			if (!body.IsValid())
			{
				return;
			}

			const BinaryEngine::Vector2f center{ body.GetPosition() };
			const float radius{ collider.Specification.Radius };
			const BinaryEngine::Color color{ GetOutlineColor(collider.Specification.Collider) };
			const BinaryEngine::Transform outline{ { center.x, center.y, s_OutlineZ }, { 0.0f, 0.0f, 0.0f }, { radius * 2.0f, radius * 2.0f, 1.0f } };
			renderer.DrawCircle(outline, { .Color{ color }, .OutlineThickness{ s_OutlineThickness } });

			const float radians{ glm::radians(body.GetRotation()) };
			const BinaryEngine::Vector2f rim{ center + BinaryEngine::Vector2f{ std::cos(radians), std::sin(radians) } * radius };
			renderer.DrawLine(center, rim, { .Color{ color }, .Thickness{ s_RadiusLineThickness }, .SortZ{ s_OutlineZ } });
		});

		scene.GetAllEntitiesWith<BinaryEngine::CapsuleColliderComponent>().each([&scene, &renderer](entt::entity entityHandle, const BinaryEngine::CapsuleColliderComponent& collider) {
			const BinaryEngine::PhysicsBody body{ scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &scene }) };
			if (!body.IsValid())
			{
				return;
			}

			DrawCapsule(renderer, body.GetPosition(), collider.Specification.Size, body.GetRotation(), s_OutlineZ, { .Color{ GetOutlineColor(collider.Specification.Collider) }, .OutlineThickness{ s_OutlineThickness } });
		});

		scene.GetAllEntitiesWith<BinaryEngine::CharacterControllerComponent>().each([&scene, &renderer](entt::entity entityHandle, const BinaryEngine::CharacterControllerComponent& controllerComponent) {
			const BinaryEngine::CharacterController controller{ scene.GetCharacterController(BinaryEngine::Entity{ entityHandle, &scene }) };
			if (!controller.IsValid())
			{
				return;
			}

			const BinaryEngine::CapsuleColliderSpecification& capsule{ controllerComponent.Specification.Capsule };
			DrawCapsule(renderer, controller.GetPosition() + capsule.Offset, capsule.Size, 0.0f, s_OutlineZ, { .Color{ GetOutlineColor(capsule.Collider) }, .OutlineThickness{ s_OutlineThickness } });
		});
	}

	void PlayHitSounds(BinaryEngine::AudioScope& audio, std::span<const BinaryEngine::ContactHitEvent> hitEvents, BinaryEngine::AssetHandle sound, BinaryEngine::AudioBusHandle bus)
	{
		const std::size_t playCount{ std::min(hitEvents.size(), s_MaxHitSoundsPerStep) };
		for (std::size_t hitIndex{ 0 }; hitIndex < playCount; hitIndex++)
		{
			const float volume{ s_HitSoundVolume * std::clamp(hitEvents[hitIndex].ApproachSpeed / s_LoudestHitSpeed, s_QuietestHitVolume, 1.0f) };
			audio.Play(sound, { .Bus{ bus }, .Volume{ volume } });
		}
	}

}
