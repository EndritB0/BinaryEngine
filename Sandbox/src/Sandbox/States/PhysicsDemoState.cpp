#include "pch.h"
#include "Sandbox/States/PhysicsDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_KeyHelp{ "Mouse  left crate, right ball   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1A, 0x22, 0x30 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BodyZ{ 0.0f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_BallRadius{ 16.0f };
		constexpr float s_StackCenterX{ -200.0f };
		constexpr float s_StackSpacing{ 36.0f };
		constexpr std::int32_t s_StackRowCount{ 3 };
		const BinaryEngine::Vector2f s_GroundCenter{ 0.0f, -170.0f };
		const BinaryEngine::Vector2f s_GroundSize{ 640.0f, 20.0f };
		const BinaryEngine::Vector2f s_CeilingCenter{ 0.0f, 190.0f };
		const BinaryEngine::Vector2f s_LeftWallCenter{ -310.0f, 0.0f };
		const BinaryEngine::Vector2f s_RightWallCenter{ 310.0f, 0.0f };
		const BinaryEngine::Vector2f s_WallSize{ 20.0f, 400.0f };

	}

	PhysicsDemoState::PhysicsDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[PhysicsDemoState] Created");
	}

	PhysicsDemoState::~PhysicsDemoState()
	{
		APP_TRACE("[PhysicsDemoState] Destroyed");
	}

	void PhysicsDemoState::OnAttach()
	{
		APP_INFO("[PhysicsDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);
		m_AtlasHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_PhysicsAtlasPath, m_Context.renderer);

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("PhysicsHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateScenery();
		SpawnCrateStack();
	}

	void PhysicsDemoState::OnDetach()
	{
		APP_INFO("[PhysicsDemoState] Detached");
	}

	void PhysicsDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
	}

	void PhysicsDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_Scene.OnFixedUpdate(fixedStep);
	}

	void PhysicsDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
		UpdateHud();
	}

	void PhysicsDemoState::OnRender()
	{
		BinaryEngine::Renderer& renderer{ m_Context.renderer };
		renderer.BeginScene(m_Camera);
		m_Scene.OnRender(renderer, m_Context.assetManager);
		if (m_ShowOutlines)
		{
			DrawColliderOutlines(m_Scene, renderer);
		}
		renderer.EndScene();
	}

	bool PhysicsDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::C:
			{
				m_ShowOutlines = !m_ShowOutlines;
				return true;
			}

			default: return false;
		}
	}

	bool PhysicsDemoState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		const BinaryEngine::Vector2f position{ m_Camera.ScreenToWorld(event.GetPosition()) };

		if (event.GetButton() == BinaryEngine::Mouse::Left)
		{
			RecordSpawn(CreateBoxBody("Crate", position, { s_CrateSize, s_CrateSize }, AtlasSquare::Crate, {}));
			return true;
		}

		if (event.GetButton() == BinaryEngine::Mouse::Right)
		{
			RecordSpawn(CreateCircleBody("Ball", position, s_BallRadius, AtlasDisc::Ball));
			return true;
		}

		return false;
	}

	void PhysicsDemoState::CreateScenery()
	{
		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };

		CreateBoxBody("Ground", s_GroundCenter, s_GroundSize, AtlasSquare::Scenery, staticBody);
		CreateBoxBody("Ceiling", s_CeilingCenter, s_GroundSize, AtlasSquare::Scenery, staticBody);
		CreateBoxBody("LeftWall", s_LeftWallCenter, s_WallSize, AtlasSquare::Scenery, staticBody);
		CreateBoxBody("RightWall", s_RightWallCenter, s_WallSize, AtlasSquare::Scenery, staticBody);
	}

	BinaryEngine::Entity PhysicsDemoState::CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetBoxSpriteScale(size);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size } });
		return entity;
	}

	BinaryEngine::Entity PhysicsDemoState::CreateCircleBody(std::string_view name, BinaryEngine::Vector2f position, float radius, AtlasDisc disc)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetCircleSpriteScale(radius);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(disc));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>();
		entity.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ radius } });
		return entity;
	}

	void PhysicsDemoState::SpawnCrateStack()
	{
		const float groundTop{ s_GroundCenter.y + s_GroundSize.y * 0.5f };
		for (std::int32_t rowIndex{ 0 }; rowIndex < s_StackRowCount; rowIndex++)
		{
			const std::int32_t crateCount{ s_StackRowCount - rowIndex };
			const float rowY{ groundTop + s_CrateSize * (static_cast<float>(rowIndex) + 0.5f) };
			for (std::int32_t crateIndex{ 0 }; crateIndex < crateCount; crateIndex++)
			{
				const float offset{ (static_cast<float>(crateIndex) - static_cast<float>(crateCount - 1) * 0.5f) * s_StackSpacing };
				RecordSpawn(CreateBoxBody("Crate", { s_StackCenterX + offset, rowY }, { s_CrateSize, s_CrateSize }, AtlasSquare::Crate, {}));
			}
		}
	}

	void PhysicsDemoState::RecordSpawn(BinaryEngine::Entity entity)
	{
		m_LastSpawnedMass = m_Scene.GetPhysicsBody(entity).GetMass();
	}

	void PhysicsDemoState::UpdateHud()
	{
		std::int32_t awakeCount{ 0 };
		for (const entt::entity entityHandle : m_Scene.GetAllEntitiesWith<BinaryEngine::RigidBodyComponent>())
		{
			if (m_Scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &m_Scene }).IsAwake())
			{
				awakeCount++;
			}
		}

		const BinaryEngine::PhysicsWorld& world{ m_Scene.GetPhysicsWorld() };
		std::string text{ std::format("{}: {}\n", s_KeyHelp, m_ShowOutlines ? "on" : "off") };
		text += std::format("Bodies {}, awake {}   Steps {}   Last mass {:.2f} kg", world.GetBodyCount(), awakeCount, world.GetStepCount(), m_LastSpawnedMass);
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
