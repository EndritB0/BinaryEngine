#include "pch.h"
#include "Sandbox/States/QueriesDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_KeyHelp{ "Right mouse  capsule   X clear   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x12, 0x33, 0x38 };
		constexpr BinaryEngine::Color s_CapsuleColor{ 0xFF, 0x8C, 0x1A };
		constexpr BinaryEngine::Color s_TriggerZoneColor{ 0xFF, 0x50, 0xFF, 0x40 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BodyZ{ 0.0f };
		constexpr float s_TriggerZoneZ{ -0.5f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_BallRadius{ 16.0f };
		constexpr float s_SlopeRotationDegrees{ 20.0f };
		constexpr float s_SlopeFriction{ 0.05f };
		const BinaryEngine::Vector2f s_GroundCenter{ 0.0f, -120.0f };
		const BinaryEngine::Vector2f s_GroundSize{ 640.0f, 20.0f };
		const BinaryEngine::Vector2f s_CeilingCenter{ 0.0f, 190.0f };
		const BinaryEngine::Vector2f s_LeftWallCenter{ -310.0f, 50.0f };
		const BinaryEngine::Vector2f s_RightWallCenter{ 310.0f, 50.0f };
		const BinaryEngine::Vector2f s_WallSize{ 20.0f, 360.0f };
		const BinaryEngine::Vector2f s_SlopeCenter{ -170.0f, 50.0f };
		const BinaryEngine::Vector2f s_SlopeSize{ 160.0f, 10.0f };
		const BinaryEngine::Vector2f s_PlatformCenter{ 170.0f, 60.0f };
		const BinaryEngine::Vector2f s_PlatformSize{ 140.0f, 10.0f };
		const BinaryEngine::Vector2f s_TriggerZoneCenter{ -110.0f, -60.0f };
		const BinaryEngine::Vector2f s_TriggerZoneSize{ 40.0f, 90.0f };
		const BinaryEngine::Vector2f s_LogCenter{ 170.0f, 77.0f };
		const BinaryEngine::Vector2f s_LogSize{ 64.0f, 24.0f };
		const BinaryEngine::Vector2f s_StandingCapsuleCenter{ 150.0f, -78.0f };
		const BinaryEngine::Vector2f s_CapsuleSize{ 32.0f, 64.0f };
		const std::array<BinaryEngine::Vector2f, 4> s_CratePositions{
			BinaryEngine::Vector2f{ -40.0f, -94.0f },
			BinaryEngine::Vector2f{ 0.0f, -94.0f },
			BinaryEngine::Vector2f{ 40.0f, -94.0f },
			BinaryEngine::Vector2f{ 80.0f, -94.0f },
		};
		const std::array<BinaryEngine::Vector2f, 3> s_BallPositions{
			BinaryEngine::Vector2f{ 205.0f, -94.0f },
			BinaryEngine::Vector2f{ 245.0f, -94.0f },
			BinaryEngine::Vector2f{ 222.0f, 81.0f },
		};

	}

	QueriesDemoState::QueriesDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[QueriesDemoState] Created");
	}

	QueriesDemoState::~QueriesDemoState()
	{
		APP_TRACE("[QueriesDemoState] Destroyed");
	}

	void QueriesDemoState::OnAttach()
	{
		APP_INFO("[QueriesDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);
		m_AtlasHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_PhysicsAtlasPath, m_Context.renderer);

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("QueriesHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateLevel();
	}

	void QueriesDemoState::OnDetach()
	{
		APP_INFO("[QueriesDemoState] Detached");
	}

	void QueriesDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
	}

	void QueriesDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_Scene.OnFixedUpdate(fixedStep);
	}

	void QueriesDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
	}

	void QueriesDemoState::OnRender()
	{
		BinaryEngine::Renderer& renderer{ m_Context.renderer };
		renderer.BeginScene(m_Camera);
		UpdateHud();
		m_Scene.OnRender(renderer, m_Context.assetManager);
		DrawCapsuleBodies(renderer);
		renderer.DrawRectangle(BinaryEngine::Transform{ { s_TriggerZoneCenter.x, s_TriggerZoneCenter.y, s_TriggerZoneZ }, { 0.0f, 0.0f, 0.0f }, { s_TriggerZoneSize.x, s_TriggerZoneSize.y, 1.0f } }, { .Color{ s_TriggerZoneColor } });
		if (m_ShowOutlines)
		{
			DrawColliderOutlines(m_Scene, renderer);
		}
		renderer.EndScene();
	}

	bool QueriesDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::X:
			{
				RemoveSpawnedBodies();
				return true;
			}

			case BinaryEngine::Key::C:
			{
				m_ShowOutlines = !m_ShowOutlines;
				return true;
			}

			default: return false;
		}
	}

	bool QueriesDemoState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		const BinaryEngine::Vector2f position{ m_Camera.ScreenToWorld(event.GetPosition()) };

		if (event.GetButton() == BinaryEngine::Mouse::Right)
		{
			SpawnCapsule(position);
			return true;
		}

		return false;
	}

	void QueriesDemoState::CreateLevel()
	{
		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };
		const BinaryEngine::ColliderSpecification slopeCollider{ .Material{ .Friction{ s_SlopeFriction } } };

		CreateBoxBody("Ground", s_GroundCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("Ceiling", s_CeilingCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("LeftWall", s_LeftWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("RightWall", s_RightWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery, staticBody, {});
		CreateBoxBody("Slope", s_SlopeCenter, s_SlopeSize, s_SlopeRotationDegrees, AtlasSquare::Scenery, staticBody, slopeCollider);
		CreateBoxBody("Platform", s_PlatformCenter, s_PlatformSize, 0.0f, AtlasSquare::Scenery, staticBody, {});

		BinaryEngine::Entity triggerZone{ m_Scene.CreateEntity("TriggerZone") };
		triggerZone.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_TriggerZoneCenter.x, s_TriggerZoneCenter.y, s_BodyZ };
		triggerZone.AddComponent<BinaryEngine::RigidBodyComponent>(staticBody);
		triggerZone.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ s_TriggerZoneSize }, .Collider{ .IsTrigger{ true } } });

		for (const BinaryEngine::Vector2f& position : s_CratePositions)
		{
			CreateBoxBody("Crate", position, { s_CrateSize, s_CrateSize }, 0.0f, AtlasSquare::Crate, {}, {});
		}

		for (const BinaryEngine::Vector2f& position : s_BallPositions)
		{
			CreateBall(position);
		}

		CreateCapsuleBody("Log", s_LogCenter, s_LogSize, {});
		CreateCapsuleBody("StandingCapsule", s_StandingCapsuleCenter, s_CapsuleSize, BinaryEngine::RigidBodySpecification{ .FixedRotation{ true } });
	}

	void QueriesDemoState::CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Rotation.z = rotationDegrees;
		transform.Scale = GetBoxSpriteScale(size);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ collider } });
	}

	void QueriesDemoState::CreateBall(BinaryEngine::Vector2f position)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity("Ball") };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetCircleSpriteScale(s_BallRadius);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(AtlasDisc::Ball));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>();
		entity.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ s_BallRadius } });
	}

	BinaryEngine::Entity QueriesDemoState::CreateCapsuleBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, const BinaryEngine::RigidBodySpecification& body)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		entity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { position.x, position.y, s_BodyZ };
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::CapsuleColliderComponent>(BinaryEngine::CapsuleColliderSpecification{ .Size{ size } });
		return entity;
	}

	void QueriesDemoState::SpawnCapsule(BinaryEngine::Vector2f position)
	{
		const BinaryEngine::Entity capsule{ CreateCapsuleBody("Capsule", position, s_CapsuleSize, {}) };
		m_SpawnedEntities.push_back(capsule);
		m_LastSpawnedMass = m_Scene.GetPhysicsBody(capsule).GetMass();
	}

	void QueriesDemoState::RemoveSpawnedBodies()
	{
		for (const BinaryEngine::Entity& entity : m_SpawnedEntities)
		{
			if (entity.IsValid())
			{
				m_Scene.DestroyEntity(entity);
			}
		}

		m_SpawnedEntities.clear();
	}

	void QueriesDemoState::DrawCapsuleBodies(BinaryEngine::Renderer& renderer)
	{
		m_Scene.GetAllEntitiesWith<BinaryEngine::TransformComponent, BinaryEngine::CapsuleColliderComponent>().each([&renderer](const BinaryEngine::TransformComponent& transformComponent, const BinaryEngine::CapsuleColliderComponent& collider) {
			const BinaryEngine::Transform& transform{ transformComponent.transform };
			DrawCapsule(renderer, { transform.Position.x, transform.Position.y }, collider.Specification.Size, transform.Rotation.z, transform.Position.z, { .Color{ s_CapsuleColor } });
		});
	}

	void QueriesDemoState::UpdateHud()
	{
		std::string text{ std::format("{}: {}\n", s_KeyHelp, m_ShowOutlines ? "on" : "off") };
		text += std::format("Bodies {}   Last mass {:.2f} kg", m_Scene.GetPhysicsWorld().GetBodyCount(), m_LastSpawnedMass);
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
