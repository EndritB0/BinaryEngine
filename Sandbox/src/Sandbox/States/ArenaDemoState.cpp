#include "pch.h"
#include "Sandbox/States/ArenaDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Physics/CharacterController.h>
#include <BinaryEngine/Core/Random.h>
#include <BinaryEngine/Physics/CharacterControllerTypes.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsEvents.h>
#include <BinaryEngine/Physics/PhysicsQueries.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_CharacterPath{ "./resources/character/character_idle.png" };
		constexpr std::string_view s_KeyHelp{ "WASD move   Z spawn ten   X remove spawned   R reset   C outlines" };
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
		constexpr float s_ZombieScale{ 2.0f };
		constexpr float s_ZombieSpeed{ 60.0f };
		constexpr float s_ZombieAcceleration{ 600.0f };
		constexpr std::int32_t s_ZombieSpawnCount{ 10 };
		constexpr std::int32_t s_SpawnAttempts{ 50 };
		constexpr float s_SpawnClearance{ 80.0f };
		constexpr float s_KnockbackSpeed{ 300.0f };
		constexpr float s_KnockbackSeconds{ 0.4f };
		constexpr std::int32_t s_GemCount{ 3 };
		constexpr float s_GemRadius{ 8.0f };
		const BinaryEngine::Vector2f s_HorizontalWallSize{ 560.0f, 20.0f };
		const BinaryEngine::Vector2f s_VerticalWallSize{ 20.0f, 320.0f };
		const BinaryEngine::Vector2f s_PillarSize{ 32.0f, 32.0f };
		const std::array<BinaryEngine::Vector2f, 2> s_LightCrateCenters{ BinaryEngine::Vector2f{ -40.0f, 100.0f }, BinaryEngine::Vector2f{ 40.0f, 100.0f } };
		const std::array<BinaryEngine::Vector2f, 2> s_HeavyCrateCenters{ BinaryEngine::Vector2f{ -40.0f, -100.0f }, BinaryEngine::Vector2f{ 40.0f, -100.0f } };
		const BinaryEngine::Vector2f s_PlayerStart{ -200.0f, 0.0f };
		const BinaryEngine::Vector2f s_PlayerCapsuleSize{ 24.0f, 36.0f };
		const BinaryEngine::Vector2f s_ZombieCapsuleSize{ 20.0f, 28.0f };
		const BinaryEngine::Vector2f s_SpawnAreaMin{ -240.0f, -120.0f };
		const BinaryEngine::Vector2f s_SpawnAreaMax{ 240.0f, 120.0f };
		const std::array<BinaryEngine::Vector2f, 10> s_ZombieStarts{
			BinaryEngine::Vector2f{ 170.0f, -100.0f },
			BinaryEngine::Vector2f{ 220.0f, -100.0f },
			BinaryEngine::Vector2f{ 170.0f, -50.0f },
			BinaryEngine::Vector2f{ 220.0f, -50.0f },
			BinaryEngine::Vector2f{ 170.0f, 0.0f },
			BinaryEngine::Vector2f{ 220.0f, 0.0f },
			BinaryEngine::Vector2f{ 170.0f, 50.0f },
			BinaryEngine::Vector2f{ 220.0f, 50.0f },
			BinaryEngine::Vector2f{ 170.0f, 100.0f },
			BinaryEngine::Vector2f{ 220.0f, 100.0f },
		};

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
		for (const BinaryEngine::Vector2f& start : s_ZombieStarts)
		{
			CreateZombie(start);
		}

		for (std::int32_t gemIndex{ 0 }; gemIndex < s_GemCount; gemIndex++)
		{
			CreateGem();
		}
	}

	void ArenaDemoState::OnDetach()
	{
		LogStepSummary();
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
		MoveZombies(fixedStep.GetSeconds());

		const std::chrono::steady_clock::time_point stepStart{ std::chrono::steady_clock::now() };
		m_Scene.OnFixedUpdate(fixedStep);
		m_TimedStepSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - stepStart).count();
		m_TimedStepCount++;

		KnockBackPlayer(fixedStep.GetSeconds());
		CollectGems();
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
			case BinaryEngine::Key::Z:
			{
				LogStepSummary();
				SpawnZombies();
				return true;
			}

			case BinaryEngine::Key::X:
			{
				if (m_Zombies.size() > s_ZombieStarts.size())
				{
					LogStepSummary();
					RemoveSpawnedZombies();
				}
				return true;
			}

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

	void ArenaDemoState::CreateZombie(BinaryEngine::Vector2f position)
	{
		BinaryEngine::Entity zombie{ m_Scene.CreateEntity("Zombie") };
		BinaryEngine::Transform& transform{ zombie.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_CharacterZ };
		transform.Scale = { s_ZombieScale, s_ZombieScale, 1.0f };
		zombie.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterHandle, m_IdleClip.Frames.front());
		zombie.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		zombie.AddComponent<BinaryEngine::CharacterControllerComponent>(BinaryEngine::CharacterControllerSpecification{ .Mode{ BinaryEngine::CharacterMode::Floating }, .Capsule{ .Size{ s_ZombieCapsuleSize } }, .IsSoft{ true } });
		m_Zombies.push_back(zombie);
	}

	void ArenaDemoState::SpawnZombies()
	{
		for (std::int32_t spawnIndex{ 0 }; spawnIndex < s_ZombieSpawnCount; spawnIndex++)
		{
			const std::optional<BinaryEngine::Vector2f> spot{ FindFreeSpot(s_ZombieCapsuleSize.y * 0.5f) };
			if (spot)
			{
				CreateZombie(*spot);
			}
		}
	}

	void ArenaDemoState::RemoveSpawnedZombies()
	{
		while (m_Zombies.size() > s_ZombieStarts.size())
		{
			m_Scene.DestroyEntity(m_Zombies.back());
			m_Zombies.pop_back();
		}
	}

	std::optional<BinaryEngine::Vector2f> ArenaDemoState::FindFreeSpot(float clearanceRadius)
	{
		const BinaryEngine::Vector2f playerPosition{ m_Scene.GetCharacterController(m_PlayerEntity).GetPosition() };
		std::vector<BinaryEngine::OverlapHit> hits;
		for (std::int32_t attempt{ 0 }; attempt < s_SpawnAttempts; attempt++)
		{
			const BinaryEngine::Vector2f spot{ BinaryEngine::Random::Vector2f(s_SpawnAreaMin, s_SpawnAreaMax) };
			if (glm::distance(spot, playerPosition) >= s_SpawnClearance && m_Scene.GetPhysicsWorld().OverlapShape(BinaryEngine::QueryShape::Circle(spot, clearanceRadius), hits) == 0)
			{
				return spot;
			}
		}

		return std::nullopt;
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

	void ArenaDemoState::MoveZombies(float stepSeconds)
	{
		const BinaryEngine::Vector2f playerPosition{ m_Scene.GetCharacterController(m_PlayerEntity).GetPosition() };
		for (const BinaryEngine::Entity& zombie : m_Zombies)
		{
			BinaryEngine::CharacterController controller{ m_Scene.GetCharacterController(zombie) };
			const BinaryEngine::Vector2f toPlayer{ playerPosition - controller.GetPosition() };
			const float distance{ glm::length(toPlayer) };
			const BinaryEngine::Vector2f targetVelocity{ distance > 0.0f ? toPlayer / distance * s_ZombieSpeed : BinaryEngine::Vector2f{ 0.0f, 0.0f } };
			controller.SetVelocity(MoveTowards(controller.GetVelocity(), targetVelocity, s_ZombieAcceleration * stepSeconds));
		}
	}

	void ArenaDemoState::KnockBackPlayer(float stepSeconds)
	{
		m_KnockbackSecondsLeft -= stepSeconds;
		if (m_KnockbackSecondsLeft > 0.0f)
		{
			return;
		}

		BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
		for (const BinaryEngine::CharacterTouch& touch : player.GetTouches())
		{
			if (IsZombie(touch.Entity))
			{
				player.SetVelocity(touch.Normal * s_KnockbackSpeed);
				m_KnockbackSecondsLeft = s_KnockbackSeconds;
				m_KnockbackCount++;
				return;
			}
		}
	}

	bool ArenaDemoState::IsZombie(BinaryEngine::Entity entity) const
	{
		return std::ranges::find(m_Zombies, entity) != m_Zombies.end();
	}

	void ArenaDemoState::CreateGem()
	{
		const std::optional<BinaryEngine::Vector2f> spot{ FindFreeSpot(s_GemRadius) };
		if (!spot)
		{
			return;
		}

		BinaryEngine::Entity gem{ m_Scene.CreateEntity("Gem") };
		BinaryEngine::Transform& transform{ gem.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { spot->x, spot->y, s_BlockZ };
		transform.Scale = GetCircleSpriteScale(s_GemRadius);
		gem.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(AtlasDisc::RedPuck));
		gem.AddComponent<BinaryEngine::RigidBodyComponent>(BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Static } });
		gem.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ s_GemRadius }, .Collider{ .IsTrigger{ true } } });
		m_Gems.push_back(gem);
	}

	void ArenaDemoState::CollectGems()
	{
		for (const BinaryEngine::TriggerBeginEvent& triggerEvent : m_Scene.GetPhysicsWorld().GetTriggerBeginEvents())
		{
			if (triggerEvent.VisitorEntity != m_PlayerEntity || std::ranges::find(m_Gems, triggerEvent.TriggerEntity) == m_Gems.end())
			{
				continue;
			}

			const std::optional<BinaryEngine::Vector2f> spot{ FindFreeSpot(s_GemRadius) };
			if (spot)
			{
				m_Scene.GetPhysicsBody(triggerEvent.TriggerEntity).SetTransform(*spot, 0.0f);
			}
			m_GemCount++;
		}
	}

	void ArenaDemoState::LogStepSummary()
	{
		const double averageMilliseconds{ m_TimedStepCount > 0 ? m_TimedStepSeconds / m_TimedStepCount * 1000.0 : 0.0 };
		APP_INFO("[ArenaDemoState] {} zombies: {} steps, average step {:.3f} ms", m_Zombies.size(), m_TimedStepCount, averageMilliseconds);
		m_TimedStepCount = 0;
		m_TimedStepSeconds = 0.0;
	}

	void ArenaDemoState::UpdateHud()
	{
		const double averageMilliseconds{ m_TimedStepCount > 0 ? m_TimedStepSeconds / m_TimedStepCount * 1000.0 : 0.0 };
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::format("{}: {}\nZombies {}   Gems {}   Knockbacks {}   Step {:.3f} ms", s_KeyHelp, m_ShowOutlines ? "on" : "off", m_Zombies.size(), m_GemCount, m_KnockbackCount, averageMilliseconds);
	}

}
