#include "pch.h"
#include "Sandbox/States/PlatformerDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Physics/CharacterController.h>
#include <BinaryEngine/Physics/CharacterControllerTypes.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsEvents.h>
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
		constexpr std::string_view s_KeyHelp{ "A/D move   Space jump   S+Space drop   R reset   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x22, 0x30, 0x3C };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BlockZ{ 0.0f };
		constexpr float s_CharacterZ{ 1.0f };
		constexpr float s_FloorY{ -160.0f };
		constexpr float s_HillStartX{ -110.0f };
		constexpr float s_HillHeight{ 30.0f };
		constexpr float s_PlateauWidth{ 40.0f };
		constexpr float s_GentleSlopeDegrees{ 30.0f };
		constexpr float s_SteepSlopeDegrees{ 60.0f };
		constexpr float s_SlopeThickness{ 16.0f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_IcyFriction{ 0.02f };
		constexpr float s_HeavyDensity{ 10.0f };
		constexpr float s_BallRadius{ 12.0f };
		constexpr float s_PivotRadius{ 8.0f };
		constexpr float s_SlidingPlatformSpeed{ 60.0f };
		constexpr float s_ElevatorSpeed{ 50.0f };
		constexpr float s_PlayerScale{ 3.0f };
		constexpr float s_PlayerPushForce{ 400.0f };
		constexpr float s_PlayerMass{ 2.0f };
		constexpr float s_RunSpeed{ 160.0f };
		constexpr float s_GroundAcceleration{ 1600.0f };
		constexpr float s_GroundFriction{ 2000.0f };
		constexpr float s_AirAcceleration{ 800.0f };
		constexpr float s_GravityScale{ 2.0f };
		constexpr float s_JumpSpeed{ 340.0f };
		constexpr float s_JumpCutFactor{ 0.5f };
		constexpr float s_CoyoteSeconds{ 0.1f };
		constexpr float s_JumpBufferSeconds{ 0.1f };
		constexpr float s_ZombieScale{ 4.0f };
		constexpr float s_ZombieSpeed{ 50.0f };
		constexpr float s_KnockbackSeconds{ 0.5f };
		constexpr float s_CoinRadius{ 8.0f };
		const BinaryEngine::Vector2f s_GroundCenter{ 0.0f, -170.0f };
		const BinaryEngine::Vector2f s_GroundSize{ 640.0f, 20.0f };
		const BinaryEngine::Vector2f s_CeilingCenter{ 0.0f, 190.0f };
		const BinaryEngine::Vector2f s_LeftWallCenter{ -310.0f, 10.0f };
		const BinaryEngine::Vector2f s_RightWallCenter{ 310.0f, 10.0f };
		const BinaryEngine::Vector2f s_WallSize{ 20.0f, 380.0f };
		const BinaryEngine::Vector2f s_StepCenter{ -215.0f, -157.0f };
		const BinaryEngine::Vector2f s_StepSize{ 40.0f, 6.0f };
		const BinaryEngine::Vector2f s_LedgeCenter{ -150.0f, -148.0f };
		const BinaryEngine::Vector2f s_LedgeSize{ 40.0f, 24.0f };
		const BinaryEngine::Vector2f s_ShelfCenter{ 180.0f, 14.0f };
		const BinaryEngine::Vector2f s_ShelfSize{ 100.0f, 12.0f };
		const BinaryEngine::Vector2f s_LightCrateCenter{ 50.0f, -144.0f };
		const BinaryEngine::Vector2f s_IcyCrateCenter{ 116.0f, -144.0f };
		const BinaryEngine::Vector2f s_HeavyCrateCenter{ 182.0f, -144.0f };
		const BinaryEngine::Vector2f s_BallCenter{ -38.0f, -118.0f };
		const BinaryEngine::Vector2f s_PivotCenter{ 180.0f, 28.0f };
		const BinaryEngine::Vector2f s_PlankCenter{ 180.0f, 39.0f };
		const BinaryEngine::Vector2f s_PlankSize{ 64.0f, 6.0f };
		const BinaryEngine::Vector2f s_SlidingPlatformFrom{ -240.0f, -70.0f };
		const BinaryEngine::Vector2f s_SlidingPlatformTo{ -100.0f, -70.0f };
		const BinaryEngine::Vector2f s_SlidingPlatformSize{ 70.0f, 8.0f };
		const BinaryEngine::Vector2f s_ElevatorFrom{ 260.0f, -157.0f };
		const BinaryEngine::Vector2f s_ElevatorTo{ 260.0f, 17.0f };
		const BinaryEngine::Vector2f s_ElevatorSize{ 56.0f, 6.0f };
		const BinaryEngine::Vector2f s_LowOneWayCenter{ 83.0f, -84.0f };
		const BinaryEngine::Vector2f s_HighOneWayCenter{ -60.0f, -10.0f };
		const BinaryEngine::Vector2f s_OneWaySize{ 90.0f, 8.0f };
		const std::array<BinaryEngine::Vector2f, 4> s_CoinSpots{
			BinaryEngine::Vector2f{ 83.0f, -60.0f },
			BinaryEngine::Vector2f{ -60.0f, 16.0f },
			BinaryEngine::Vector2f{ -150.0f, -112.0f },
			BinaryEngine::Vector2f{ 260.0f, 60.0f },
		};
		const BinaryEngine::Vector2f s_PlayerStart{ 17.0f, -136.0f };
		const BinaryEngine::Vector2f s_PlayerCapsuleSize{ 28.0f, 40.0f };
		const BinaryEngine::Vector2f s_PlayerCapsuleOffset{ 0.0f, 4.0f };
		const BinaryEngine::Vector2f s_LeftZombieStart{ -270.0f, -128.0f };
		const BinaryEngine::Vector2f s_RightZombieStart{ -205.0f, -122.0f };
		const BinaryEngine::Vector2f s_ZombieCapsuleSize{ 36.0f, 52.0f };
		const BinaryEngine::Vector2f s_ZombieCapsuleOffset{ 0.0f, 2.0f };
		const BinaryEngine::Vector2f s_KnockbackVelocity{ 220.0f, 200.0f };

		float MoveTowards(float current, float target, float maxChange)
		{
			return current + std::clamp(target - current, -maxChange, maxChange);
		}

	}

	PlatformerDemoState::PlatformerDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[PlatformerDemoState] Created");
	}

	PlatformerDemoState::~PlatformerDemoState()
	{
		APP_TRACE("[PlatformerDemoState] Destroyed");
	}

	void PlatformerDemoState::OnAttach()
	{
		APP_INFO("[PlatformerDemoState] Attached");

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
		m_HudEntity = m_Scene.CreateEntity("PlatformerHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateFrame();
		CreateTerrain();
		CreateBodies();
		CreatePlatforms();
		CreateCoin();
		CreatePlayer();
		CreateZombie(s_LeftZombieStart, 1.0f);
		CreateZombie(s_RightZombieStart, -1.0f);
	}

	void PlatformerDemoState::OnDetach()
	{
		const BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
		const BinaryEngine::Vector2f position{ player.GetPosition() };
		APP_INFO("[PlatformerDemoState] player grounded {} at ({:.2f}, {:.2f}), {} jumps, {} drops, {} coins, {} knockbacks", player.IsOnFloor() ? "yes" : "no", position.x, position.y, m_JumpCount, m_DropCount, m_CoinCount, m_KnockbackCount);
		APP_INFO("[PlatformerDemoState] Detached");
	}

	void PlatformerDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
		dispatcher.Dispatch<BinaryEngine::KeyReleasedEvent>(BIND_FUNCTION(OnKeyReleased));
	}

	void PlatformerDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		MovePlatforms();
		MovePlayer(fixedStep.GetSeconds());
		MoveZombies(fixedStep.GetSeconds());
		m_Scene.OnFixedUpdate(fixedStep);
		TurnZombiesAtWalls();
		KnockBackPlayer(fixedStep.GetSeconds());
		CollectCoin();
	}

	void PlatformerDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
		UpdateHud();
	}

	void PlatformerDemoState::OnRender()
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

	bool PlatformerDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat())
		{
			return false;
		}

		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Space:
			{
				BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
				if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S) && player.IsOnFloor() && IsOneWayPlatform(player.GetFloorEntity()))
				{
					player.DropThrough();
					m_DropCount++;
					return true;
				}

				m_JumpBufferSecondsLeft = s_JumpBufferSeconds;
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

	bool PlatformerDemoState::OnKeyReleased(BinaryEngine::KeyReleasedEvent& event)
	{
		if (event.GetKeyCode() != BinaryEngine::Key::Space)
		{
			return false;
		}

		m_IsJumpReleased = true;
		return true;
	}

	void PlatformerDemoState::CreateFrame()
	{
		CreateBlock("Ground", s_GroundCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery);
		CreateBlock("Ceiling", s_CeilingCenter, s_GroundSize, 0.0f, AtlasSquare::Scenery);
		CreateBlock("LeftWall", s_LeftWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery);
		CreateBlock("RightWall", s_RightWallCenter, s_WallSize, 0.0f, AtlasSquare::Scenery);
	}

	void PlatformerDemoState::CreateTerrain()
	{
		CreateBlock("Step", s_StepCenter, s_StepSize, 0.0f, AtlasSquare::Scenery);
		CreateBlock("Ledge", s_LedgeCenter, s_LedgeSize, 0.0f, AtlasSquare::Scenery);

		const float plateauStartX{ s_HillStartX + s_HillHeight / std::tan(glm::radians(s_GentleSlopeDegrees)) };
		const float plateauEndX{ plateauStartX + s_PlateauWidth };
		const float plateauTopY{ s_FloorY + s_HillHeight };
		CreateSlope("GentleSlope", { s_HillStartX, s_FloorY }, s_GentleSlopeDegrees, s_HillHeight / std::sin(glm::radians(s_GentleSlopeDegrees)));
		CreateBlock("Plateau", { (plateauStartX + plateauEndX) * 0.5f, s_FloorY + s_HillHeight * 0.5f }, { s_PlateauWidth, s_HillHeight }, 0.0f, AtlasSquare::Scenery);
		CreateSlope("SteepSlope", { plateauEndX, plateauTopY }, -s_SteepSlopeDegrees, s_HillHeight / std::sin(glm::radians(s_SteepSlopeDegrees)));
		CreateBlock("Shelf", s_ShelfCenter, s_ShelfSize, 0.0f, AtlasSquare::Scenery);
	}

	BinaryEngine::Entity PlatformerDemoState::CreateBlock(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square)
	{
		BinaryEngine::Entity block{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ block.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { center.x, center.y, s_BlockZ };
		transform.Rotation.z = rotationDegrees;
		transform.Scale = GetBoxSpriteScale(size);
		block.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		block.AddComponent<BinaryEngine::RigidBodyComponent>(BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Static } });
		block.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size } });
		return block;
	}

	void PlatformerDemoState::CreateSlope(std::string_view name, BinaryEngine::Vector2f start, float angleDegrees, float length)
	{
		const float angleRadians{ glm::radians(angleDegrees) };
		const BinaryEngine::Vector2f direction{ std::cos(angleRadians), std::sin(angleRadians) };
		const BinaryEngine::Vector2f surfaceNormal{ -direction.y, direction.x };
		const BinaryEngine::Vector2f center{ start + direction * (length * 0.5f) - surfaceNormal * (s_SlopeThickness * 0.5f) };
		CreateBlock(name, center, { length, s_SlopeThickness }, angleDegrees, AtlasSquare::Scenery);
	}

	void PlatformerDemoState::CreateBodies()
	{
		const BinaryEngine::Vector2f crateSize{ s_CrateSize, s_CrateSize };
		const BinaryEngine::RigidBodySpecification crateBody{ .FixedRotation{ true } };
		CreateBoxBody("Crate", s_LightCrateCenter, crateSize, AtlasSquare::Crate, crateBody, {});
		CreateBoxBody("IcyCrate", s_IcyCrateCenter, crateSize, AtlasSquare::IcyCrate, crateBody, BinaryEngine::ColliderSpecification{ .Material{ .Friction{ s_IcyFriction } } });
		CreateBoxBody("HeavyCrate", s_HeavyCrateCenter, crateSize, AtlasSquare::HeavyCrate, crateBody, BinaryEngine::ColliderSpecification{ .Material{ .Density{ s_HeavyDensity } } });
		CreateCircleBody("Ball", s_BallCenter, s_BallRadius, AtlasDisc::Ball, {}, {});
		CreateCircleBody("Pivot", s_PivotCenter, s_PivotRadius, AtlasDisc::Pocket, BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Static } }, {});
		CreateBoxBody("Seesaw", s_PlankCenter, s_PlankSize, AtlasSquare::Bucket, {}, {});
	}

	void PlatformerDemoState::CreatePlatforms()
	{
		CreateMovingPlatform("SlidingPlatform", s_SlidingPlatformFrom, s_SlidingPlatformTo, s_SlidingPlatformSize, s_SlidingPlatformSpeed);
		CreateMovingPlatform("Elevator", s_ElevatorFrom, s_ElevatorTo, s_ElevatorSize, s_ElevatorSpeed);

		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };
		const BinaryEngine::ColliderSpecification oneWayCollider{ .OneWay{ true } };
		m_OneWayPlatforms.push_back(CreateBoxBody("LowOneWay", s_LowOneWayCenter, s_OneWaySize, AtlasSquare::Pane, staticBody, oneWayCollider));
		m_OneWayPlatforms.push_back(CreateBoxBody("HighOneWay", s_HighOneWayCenter, s_OneWaySize, AtlasSquare::Pane, staticBody, oneWayCollider));
	}

	BinaryEngine::Entity PlatformerDemoState::CreateBoxBody(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { center.x, center.y, s_BlockZ };
		transform.Scale = GetBoxSpriteScale(size);
		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ collider } });
		return entity;
	}

	BinaryEngine::Entity PlatformerDemoState::CreateCircleBody(std::string_view name, BinaryEngine::Vector2f center, float radius, AtlasDisc disc, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { center.x, center.y, s_BlockZ };
		transform.Scale = GetCircleSpriteScale(radius);
		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(disc));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ radius }, .Collider{ collider } });
		return entity;
	}

	void PlatformerDemoState::CreateMovingPlatform(std::string_view name, BinaryEngine::Vector2f from, BinaryEngine::Vector2f to, BinaryEngine::Vector2f size, float speed)
	{
		const BinaryEngine::Entity platform{ CreateBoxBody(name, from, size, AtlasSquare::Paddle, BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Kinematic } }, {}) };
		m_MovingPlatforms.emplace_back(platform, from, to, speed);
	}

	void PlatformerDemoState::CreateCoin()
	{
		m_CoinEntity = CreateCircleBody("Coin", s_CoinSpots[m_CoinSpotIndex], s_CoinRadius, AtlasDisc::YellowPuck, BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Static } }, BinaryEngine::ColliderSpecification{ .IsTrigger{ true } });
	}

	void PlatformerDemoState::CreatePlayer()
	{
		m_PlayerEntity = m_Scene.CreateEntity("Player");
		BinaryEngine::Transform& transform{ m_PlayerEntity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { s_PlayerStart.x, s_PlayerStart.y, s_CharacterZ };
		transform.Scale = { s_PlayerScale, s_PlayerScale, 1.0f };
		m_PlayerEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterHandle, m_IdleClip.Frames.front());
		m_PlayerEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		m_PlayerEntity.AddComponent<BinaryEngine::CharacterControllerComponent>(BinaryEngine::CharacterControllerSpecification{ .Capsule{ .Size{ s_PlayerCapsuleSize }, .Offset{ s_PlayerCapsuleOffset } }, .MaxPushForce{ s_PlayerPushForce }, .Mass{ s_PlayerMass } });
	}

	void PlatformerDemoState::CreateZombie(BinaryEngine::Vector2f start, float direction)
	{
		BinaryEngine::Entity zombie{ m_Scene.CreateEntity("Zombie") };
		BinaryEngine::Transform& transform{ zombie.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { start.x, start.y, s_CharacterZ };
		transform.Scale = { s_ZombieScale, s_ZombieScale, 1.0f };
		zombie.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterHandle, m_IdleClip.Frames.front());
		zombie.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		zombie.AddComponent<BinaryEngine::CharacterControllerComponent>(BinaryEngine::CharacterControllerSpecification{ .Capsule{ .Size{ s_ZombieCapsuleSize }, .Offset{ s_ZombieCapsuleOffset } }, .IsSoft{ true } });
		m_Zombies.emplace_back(zombie, direction);
	}

	void PlatformerDemoState::MovePlatforms()
	{
		for (MovingPlatform& platform : m_MovingPlatforms)
		{
			BinaryEngine::PhysicsBody body{ m_Scene.GetPhysicsBody(platform.Entity) };
			if (glm::dot(platform.To - body.GetPosition(), platform.To - platform.From) <= 0.0f)
			{
				std::swap(platform.From, platform.To);
			}

			body.SetLinearVelocity(glm::normalize(platform.To - platform.From) * platform.Speed);
		}
	}

	void PlatformerDemoState::MovePlayer(float stepSeconds)
	{
		BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
		const bool isOnFloor{ player.IsOnFloor() };

		float input{ 0.0f };
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D))
		{
			input += 1.0f;
		}
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A))
		{
			input -= 1.0f;
		}

		BinaryEngine::Vector2f velocity{ player.GetVelocity() };
		const float acceleration{ isOnFloor ? (input != 0.0f ? s_GroundAcceleration : s_GroundFriction) : s_AirAcceleration };
		velocity.x = MoveTowards(velocity.x, input * s_RunSpeed, acceleration * stepSeconds);
		velocity.y += s_GravityScale * m_Scene.GetPhysicsWorld().GetGravity().y * stepSeconds;

		if (m_IsJumpReleased && velocity.y > 0.0f)
		{
			velocity.y *= s_JumpCutFactor;
		}
		m_IsJumpReleased = false;

		m_CoyoteSecondsLeft = isOnFloor ? s_CoyoteSeconds : m_CoyoteSecondsLeft - stepSeconds;
		m_JumpBufferSecondsLeft -= stepSeconds;
		if (m_JumpBufferSecondsLeft > 0.0f && m_CoyoteSecondsLeft > 0.0f)
		{
			velocity.y = s_JumpSpeed;
			m_JumpBufferSecondsLeft = 0.0f;
			m_CoyoteSecondsLeft = 0.0f;
			m_JumpCount++;
		}

		player.SetVelocity(velocity);
	}

	void PlatformerDemoState::MoveZombies(float stepSeconds)
	{
		const float gravity{ s_GravityScale * m_Scene.GetPhysicsWorld().GetGravity().y };
		for (const Zombie& zombie : m_Zombies)
		{
			BinaryEngine::CharacterController controller{ m_Scene.GetCharacterController(zombie.Entity) };
			controller.SetVelocity({ zombie.Direction * s_ZombieSpeed, controller.GetVelocity().y + gravity * stepSeconds });
		}
	}

	void PlatformerDemoState::TurnZombiesAtWalls()
	{
		for (Zombie& zombie : m_Zombies)
		{
			const BinaryEngine::CharacterController controller{ m_Scene.GetCharacterController(zombie.Entity) };
			if (!controller.IsOnWall())
			{
				continue;
			}

			for (const BinaryEngine::CharacterTouch& touch : controller.GetTouches())
			{
				if (touch.Normal.x * zombie.Direction < 0.0f)
				{
					zombie.Direction = -zombie.Direction;
					break;
				}
			}
		}
	}

	void PlatformerDemoState::KnockBackPlayer(float stepSeconds)
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
				player.SetVelocity({ touch.Normal.x < 0.0f ? -s_KnockbackVelocity.x : s_KnockbackVelocity.x, s_KnockbackVelocity.y });
				m_KnockbackSecondsLeft = s_KnockbackSeconds;
				m_KnockbackCount++;
				return;
			}
		}
	}

	bool PlatformerDemoState::IsZombie(BinaryEngine::Entity entity) const
	{
		return std::ranges::any_of(m_Zombies, [entity](const Zombie& zombie) { return zombie.Entity == entity; });
	}

	bool PlatformerDemoState::IsOneWayPlatform(BinaryEngine::Entity entity) const
	{
		return std::ranges::find(m_OneWayPlatforms, entity) != m_OneWayPlatforms.end();
	}

	void PlatformerDemoState::CollectCoin()
	{
		for (const BinaryEngine::TriggerBeginEvent& triggerEvent : m_Scene.GetPhysicsWorld().GetTriggerBeginEvents())
		{
			if (triggerEvent.TriggerEntity == m_CoinEntity && triggerEvent.VisitorEntity == m_PlayerEntity)
			{
				m_CoinSpotIndex = (m_CoinSpotIndex + 1) % s_CoinSpots.size();
				m_Scene.GetPhysicsBody(m_CoinEntity).SetTransform(s_CoinSpots[m_CoinSpotIndex], 0.0f);
				m_CoinCount++;
				return;
			}
		}
	}

	void PlatformerDemoState::UpdateHud()
	{
		const BinaryEngine::CharacterController player{ m_Scene.GetCharacterController(m_PlayerEntity) };
		const BinaryEngine::Vector2f velocity{ player.GetVelocity() };

		std::string text{ std::format("{}: {}\n", s_KeyHelp, m_ShowOutlines ? "on" : "off") };
		if (player.IsOnFloor())
		{
			BinaryEngine::Entity floor{ player.GetFloorEntity() };
			const float floorDegrees{ glm::degrees(std::acos(std::clamp(player.GetFloorNormal().y, -1.0f, 1.0f))) };
			const std::string_view floorTag{ floor.IsValid() ? std::string_view{ floor.GetComponent<BinaryEngine::TagComponent>().Tag } : std::string_view{ "?" } };
			text += std::format("Grounded on '{}' ({:.1f} deg)", floorTag, floorDegrees);
		}
		else
		{
			text += "Airborne";
		}
		text += std::format("   Velocity ({:.0f}, {:.0f})\nTouches {}   On wall {}   Coins {}   Knockbacks {}", velocity.x, velocity.y, player.GetTouches().size(), player.IsOnWall() ? "yes" : "no", m_CoinCount, m_KnockbackCount);
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
