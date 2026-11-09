#include "pch.h"
#include "Sandbox/PhysicsDemoCommon.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/Transform.h>
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
