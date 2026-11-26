#include "pch.h"
#include "Sandbox/States/ArenaDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Physics/CharacterController.h>
#include <BinaryEngine/Physics/CharacterControllerTypes.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_CharacterPath{ "./resources/character/character_idle.png" };
		constexpr std::string_view s_KeyHelp{ "WASD move   R reset   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x2A, 0x24, 0x30 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BlockZ{ 0.0f };
		constexpr float s_CharacterZ{ 1.0f };
		constexpr float s_RoomWallX{ 270.0f };
		constexpr float s_RoomWallY{ 150.0f };
		constexpr float s_PillarX{ 120.0f };
		constexpr float s_PillarY{ 60.0f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_CrateDamping{ 3.0f };
		constexpr float s_LightCrateDensity{ 1.0f };
		constexpr float s_HeavyCrateDensity{ 10.0f };
		constexpr float s_PlayerScale{ 2.5f };
		constexpr float s_PlayerPushForce{ 400.0f };
		constexpr float s_PlayerSpeed{ 150.0f };
		constexpr float s_PlayerAcceleration{ 1200.0f };
		constexpr float s_PlayerFriction{ 1500.0f };
		const BinaryEngine::Vector2f s_HorizontalWallSize{ 560.0f, 20.0f };
		const BinaryEngine::Vector2f s_VerticalWallSize{ 20.0f, 320.0f };
		const BinaryEngine::Vector2f s_PillarSize{ 32.0f, 32.0f };
		const std::array<BinaryEngine::Vector2f, 2> s_LightCrateCenters{ BinaryEngine::Vector2f{ -40.0f, 100.0f }, BinaryEngine::Vector2f{ 40.0f, 100.0f } };
		const std::array<BinaryEngine::Vector2f, 2> s_HeavyCrateCenters{ BinaryEngine::Vector2f{ -40.0f, -100.0f }, BinaryEngine::Vector2f{ 40.0f, -100.0f } };
		const BinaryEngine::Vector2f s_PlayerStart{ -200.0f, 0.0f };
		const BinaryEngine::Vector2f s_PlayerCapsuleSize{ 24.0f, 36.0f };

		BinaryEngine::Vector2f MoveTowards(BinaryEngine::Vector2f current, BinaryEngine::Vector2f target, float maxChange)
		{
			const BinaryEngine::Vector2f change{ target - current };
			const float distance{ glm::length(change) };
			return distance <= maxChange ? target : current + change / distance * maxChange;
		}

	}

	ArenaDemoState::ArenaDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[ArenaDemoState] Created");
	}

	ArenaDemoState::~ArenaDemoState()
	{
		APP_TRACE("[ArenaDemoState] Destroyed");
	}

	void ArenaDemoState::OnAttach()
	{
		APP_INFO("[ArenaDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);
		m_AtlasHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_PhysicsAtlasPath, m_Context.renderer);
		m_CharacterHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_CharacterPath, m_Context.renderer);
		m_IdleClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.15f, true);

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("ArenaHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateRoom();
		CreateCrates();
		CreatePlayer();
	}

	void ArenaDemoState::OnDetach()
	{
		APP_INFO("[ArenaDemoState] Detached");
	}

	void ArenaDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void ArenaDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		MovePlayer(fixedStep.GetSeconds());
		m_Scene.OnFixedUpdate(fixedStep);
	}

	void ArenaDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
		UpdateHud();
	}

	void ArenaDemoState::OnRender()
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

	bool ArenaDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::R:
			{
				m_Scene.GetCharacterController(m_PlayerEntity).SetPosition(s_PlayerStart);
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

	void ArenaDemoState::CreateRoom()
	{
		CreateBlock("RoomWall", { 0.0f, s_RoomWallY }, s_HorizontalWallSize, AtlasSquare::Scenery);
		CreateBlock("RoomWall", { 0.0f, -s_RoomWallY }, s_HorizontalWallSize, AtlasSquare::Scenery);
		CreateBlock("RoomWall", { -s_RoomWallX, 0.0f }, s_VerticalWallSize, AtlasSquare::Scenery);
		CreateBlock("RoomWall", { s_RoomWallX, 0.0f }, s_VerticalWallSize, AtlasSquare::Scenery);

		CreateBlock("Pillar", { -s_PillarX, s_PillarY }, s_PillarSize, AtlasSquare::TableWall);
		CreateBlock("Pillar", { s_PillarX, s_PillarY }, s_PillarSize, AtlasSquare::TableWall);
		CreateBlock("Pillar", { -s_PillarX, -s_PillarY }, s_PillarSize, AtlasSquare::TableWall);
		CreateBlock("Pillar", { s_PillarX, -s_PillarY }, s_PillarSize, AtlasSquare::TableWall);
	}

	void ArenaDemoState::CreateBlock(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, AtlasSquare square)
	{
		BinaryEngine::Entity block{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ block.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { center.x, center.y, s_BlockZ };
		transform.Scale = GetBoxSpriteScale(size);
		block.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		block.AddComponent<BinaryEngine::RigidBodyComponent>(BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Static } });
		block.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size } });
	}

	void ArenaDemoState::CreateCrates()
	{
		for (const BinaryEngine::Vector2f& center : s_LightCrateCenters)
		{
			CreateCrate("Crate", center, AtlasSquare::Crate, s_LightCrateDensity);
		}

		for (const BinaryEngine::Vector2f& center : s_HeavyCrateCenters)
		{
			CreateCrate("HeavyCrate", center, AtlasSquare::HeavyCrate, s_HeavyCrateDensity);
		}
	}

	void ArenaDemoState::CreateCrate(std::string_view name, BinaryEngine::Vector2f center, AtlasSquare square, float density)
	{
		const BinaryEngine::Vector2f size{ s_CrateSize, s_CrateSize };
		BinaryEngine::Entity crate{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ crate.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { center.x, center.y, s_BlockZ };
		transform.Scale = GetBoxSpriteScale(size);
		crate.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		crate.AddComponent<BinaryEngine::RigidBodyComponent>(BinaryEngine::RigidBodySpecification{ .LinearDamping{ s_CrateDamping }, .AngularDamping{ s_CrateDamping } });
		crate.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ .Material{ .Density{ density } } } });
	}

	void ArenaDemoState::CreatePlayer()
	{
		m_PlayerEntity = m_Scene.CreateEntity("Player");
		BinaryEngine::Transform& transform{ m_PlayerEntity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { s_PlayerStart.x, s_PlayerStart.y, s_CharacterZ };
		transform.Scale = { s_PlayerScale, s_PlayerScale, 1.0f };
		m_PlayerEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterHandle, m_IdleClip.Frames.front());
		m_PlayerEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		m_PlayerEntity.AddComponent<BinaryEngine::CharacterControllerComponent>(BinaryEngine::CharacterControllerSpecification{ .Mode{ BinaryEngine::CharacterMode::Floating }, .Capsule{ .Size{ s_PlayerCapsuleSize } }, .MaxPushForce{ s_PlayerPushForce } });
	}

	void ArenaDemoState::MovePlayer(float stepSeconds)
	{
		BinaryEngine::Vector2f input{ 0.0f, 0.0f };
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D))
		{
			input.x += 1.0f;
		}
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A))
		{
			input.x -= 1.0f;
		}
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W))
		{
			input.y += 1.0f;
		}
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S))
		{
			input.y -= 1.0f;
		}

		const bool isMoving{ input != BinaryEngine::Vector2f{ 0.0f, 0.0f } };
		const BinaryEngine::Vector2f targetVelocity{ isMoving ? glm::normalize(input) * s_PlayerSpeed : BinaryEngine::Vector2f{ 0.0f, 0.0f } };
		const float rate{ isMoving ? s_PlayerAcceleration : s_PlayerFriction };

		BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
		player.SetVelocity(MoveTowards(player.GetVelocity(), targetVelocity, rate * stepSeconds));
	}

	void ArenaDemoState::UpdateHud()
	{
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::format("{}: {}", s_KeyHelp, m_ShowOutlines ? "on" : "off");
	}

}
