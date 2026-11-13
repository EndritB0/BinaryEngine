#include "pch.h"
#include "Sandbox/States/PhysicsDemoState.h"

#include <BinaryEngine/Audio/AudioClip.h>
#include <BinaryEngine/Audio/AudioEngine.h>
#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Random.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Physics/PhysicsBody.h>
#include <BinaryEngine/Physics/PhysicsEvents.h>
#include <BinaryEngine/Physics/PhysicsWorld.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_HitSoundPath{ "./resources/audio/one_shot_1.wav" };
		constexpr std::string_view s_KeyHelp{ "Mouse  left crate, right ball   1 icy   2 bouncy   3 heavy   4 upright   B balloon\nP panes   G bullet   H no bullet   E explode   D debris\nHold W wind   Hold T twist   X clear   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1A, 0x22, 0x30 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BodyZ{ 0.0f };
		constexpr float s_CrateSize{ 32.0f };
		constexpr float s_BallRadius{ 16.0f };
		constexpr float s_StackCenterX{ -200.0f };
		constexpr float s_StackSpacing{ 36.0f };
		constexpr std::int32_t s_StackRowCount{ 3 };
		constexpr float s_IcyFriction{ 0.02f };
		constexpr float s_BouncyRestitution{ 0.9f };
		constexpr float s_HeavyDensity{ 10.0f };
		constexpr float s_BalloonRadius{ 12.0f };
		constexpr float s_BalloonGravityScale{ -0.25f };
		constexpr float s_BalloonLinearDamping{ 0.5f };
		constexpr float s_PaddleSpinDegreesPerSecond{ 60.0f };
		constexpr std::int32_t s_PaneCount{ 3 };
		constexpr float s_PaneSpacing{ 24.0f };
		constexpr float s_ShotRadius{ 4.0f };
		constexpr float s_ShotDensity{ 8.0f };
		constexpr float s_ShotSpeed{ 9000.0f };
		constexpr float s_ExplosionRadius{ 140.0f };
		constexpr float s_ExplosionImpulse{ 600.0f };
		constexpr float s_ExplosionMaxSpeedChange{ 1200.0f };
		constexpr float s_ExplosionLeverArm{ 6.0f };
		constexpr std::int32_t s_DebrisPieceCount{ 20 };
		constexpr float s_DebrisSpread{ 12.0f };
		constexpr float s_DebrisMaxSideSpeed{ 250.0f };
		constexpr float s_DebrisMinUpwardSpeed{ 100.0f };
		constexpr float s_DebrisMaxUpwardSpeed{ 400.0f };
		constexpr std::uint64_t s_DebrisCategory{ 0x2 };
		constexpr float s_TwistRadius{ 80.0f };
		constexpr float s_TwistTorque{ 4000.0f };
		const BinaryEngine::Vector2f s_GroundCenter{ 0.0f, -170.0f };
		const BinaryEngine::Vector2f s_GroundSize{ 640.0f, 20.0f };
		const BinaryEngine::Vector2f s_CeilingCenter{ 0.0f, 190.0f };
		const BinaryEngine::Vector2f s_LeftWallCenter{ -310.0f, 0.0f };
		const BinaryEngine::Vector2f s_RightWallCenter{ 310.0f, 0.0f };
		const BinaryEngine::Vector2f s_WallSize{ 20.0f, 400.0f };
		const BinaryEngine::Vector2f s_BucketFloorCenter{ 200.0f, -8.0f };
		const BinaryEngine::Vector2f s_BucketFloorSize{ 84.0f, 6.0f };
		const BinaryEngine::Vector2f s_BucketLeftWallCenter{ 161.0f, 22.0f };
		const BinaryEngine::Vector2f s_BucketRightWallCenter{ 239.0f, 22.0f };
		const BinaryEngine::Vector2f s_BucketWallSize{ 6.0f, 66.0f };
		const BinaryEngine::Vector2f s_BucketTriggerCenter{ 200.0f, 22.0f };
		const BinaryEngine::Vector2f s_BucketTriggerSize{ 64.0f, 48.0f };
		const BinaryEngine::Vector2f s_PaddleCenter{ 0.0f, -60.0f };
		const BinaryEngine::Vector2f s_PaddleSize{ 120.0f, 10.0f };
		const BinaryEngine::Vector2f s_PaneSize{ 4.0f, 56.0f };
		const BinaryEngine::Vector2f s_PaneWallCenter{ 256.0f, -132.0f };
		const BinaryEngine::Vector2f s_DebrisSize{ 8.0f, 8.0f };
		const BinaryEngine::Vector2f s_WindForce{ 300.0f, 0.0f };

		bool IsDebris(BinaryEngine::Entity entity)
		{
			return entity.HasComponent<BinaryEngine::BoxColliderComponent>() && entity.GetComponent<BinaryEngine::BoxColliderComponent>().Specification.Collider.Filter.Category == s_DebrisCategory;
		}

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
		m_HitSoundHandle = m_Context.assetManager.LoadAsset<BinaryEngine::AudioClip>(s_HitSoundPath);
		m_SfxBus = m_Context.audio.GetBus("SFX");

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
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W))
		{
			ApplyWind();
		}

		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::T))
		{
			ApplyTwist(GetCursorWorldPosition());
		}

		m_Scene.OnFixedUpdate(fixedStep);
		ReadPhysicsEvents();
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
			case BinaryEngine::Key::Num1:
			{
				const BinaryEngine::ColliderSpecification icyCollider{ .Material{ .Friction{ s_IcyFriction } } };
				RecordSpawn(CreateBoxBody("IcyCrate", GetCursorWorldPosition(), { s_CrateSize, s_CrateSize }, AtlasSquare::IcyCrate, {}, icyCollider));
				return true;
			}

			case BinaryEngine::Key::Num2:
			{
				const BinaryEngine::ColliderSpecification bouncyCollider{ .Material{ .Restitution{ s_BouncyRestitution } } };
				RecordSpawn(CreateCircleBody("BouncyBall", GetCursorWorldPosition(), s_BallRadius, AtlasDisc::BouncyBall, {}, bouncyCollider));
				return true;
			}

			case BinaryEngine::Key::Num3:
			{
				const BinaryEngine::ColliderSpecification heavyCollider{ .Material{ .Density{ s_HeavyDensity } } };
				RecordSpawn(CreateBoxBody("HeavyCrate", GetCursorWorldPosition(), { s_CrateSize, s_CrateSize }, AtlasSquare::HeavyCrate, {}, heavyCollider));
				return true;
			}

			case BinaryEngine::Key::Num4:
			{
				const BinaryEngine::RigidBodySpecification uprightBody{ .FixedRotation{ true } };
				RecordSpawn(CreateBoxBody("UprightCrate", GetCursorWorldPosition(), { s_CrateSize, s_CrateSize }, AtlasSquare::UprightCrate, uprightBody, {}));
				return true;
			}

			case BinaryEngine::Key::B:
			{
				const BinaryEngine::RigidBodySpecification balloonBody{ .GravityScale{ s_BalloonGravityScale }, .LinearDamping{ s_BalloonLinearDamping } };
				RecordSpawn(CreateCircleBody("Balloon", GetCursorWorldPosition(), s_BalloonRadius, AtlasDisc::Balloon, balloonBody, {}));
				return true;
			}

			case BinaryEngine::Key::P:
			{
				SpawnPanes();
				return true;
			}

			case BinaryEngine::Key::G:
			{
				FireShot(GetCursorWorldPosition(), true);
				return true;
			}

			case BinaryEngine::Key::H:
			{
				FireShot(GetCursorWorldPosition(), false);
				return true;
			}

			case BinaryEngine::Key::E:
			{
				Explode(GetCursorWorldPosition());
				return true;
			}

			case BinaryEngine::Key::D:
			{
				SpawnDebris(GetCursorWorldPosition());
				return true;
			}

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

	bool PhysicsDemoState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		const BinaryEngine::Vector2f position{ m_Camera.ScreenToWorld(event.GetPosition()) };

		if (event.GetButton() == BinaryEngine::Mouse::Left)
		{
			RecordSpawn(CreateBoxBody("Crate", position, { s_CrateSize, s_CrateSize }, AtlasSquare::Crate, {}, {}));
			return true;
		}

		if (event.GetButton() == BinaryEngine::Mouse::Right)
		{
			RecordSpawn(CreateCircleBody("Ball", position, s_BallRadius, AtlasDisc::Ball, {}, {}));
			return true;
		}

		return false;
	}

	BinaryEngine::Vector2f PhysicsDemoState::GetCursorWorldPosition() const
	{
		return m_Camera.ScreenToWorld(BinaryEngine::Input::GetMousePosition());
	}

	void PhysicsDemoState::CreateScenery()
	{
		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };
		const BinaryEngine::ColliderSpecification sceneryCollider{ .EnableTriggerEvents{ false } };

		CreateBoxBody("Ground", s_GroundCenter, s_GroundSize, AtlasSquare::Scenery, staticBody, sceneryCollider);
		CreateBoxBody("Ceiling", s_CeilingCenter, s_GroundSize, AtlasSquare::Scenery, staticBody, sceneryCollider);
		CreateBoxBody("LeftWall", s_LeftWallCenter, s_WallSize, AtlasSquare::Scenery, staticBody, sceneryCollider);
		CreateBoxBody("RightWall", s_RightWallCenter, s_WallSize, AtlasSquare::Scenery, staticBody, sceneryCollider);
		CreateBoxBody("BucketFloor", s_BucketFloorCenter, s_BucketFloorSize, AtlasSquare::Bucket, staticBody, sceneryCollider);
		CreateBoxBody("BucketLeftWall", s_BucketLeftWallCenter, s_BucketWallSize, AtlasSquare::Bucket, staticBody, sceneryCollider);
		CreateBoxBody("BucketRightWall", s_BucketRightWallCenter, s_BucketWallSize, AtlasSquare::Bucket, staticBody, sceneryCollider);

		m_BucketTriggerEntity = m_Scene.CreateEntity("BucketTrigger");
		m_BucketTriggerEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_BucketTriggerCenter.x, s_BucketTriggerCenter.y, s_BodyZ };
		m_BucketTriggerEntity.AddComponent<BinaryEngine::RigidBodyComponent>(staticBody);
		m_BucketTriggerEntity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ s_BucketTriggerSize }, .Collider{ .IsTrigger{ true } } });

		m_PaddleEntity = CreateBoxBody("Paddle", s_PaddleCenter, s_PaddleSize, AtlasSquare::Paddle, BinaryEngine::RigidBodySpecification{ .Type{ BinaryEngine::BodyType::Kinematic } }, sceneryCollider);
		m_Scene.GetPhysicsBody(m_PaddleEntity).SetAngularVelocity(s_PaddleSpinDegreesPerSecond);
	}

	BinaryEngine::Entity PhysicsDemoState::CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetBoxSpriteScale(size);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(square));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ collider } });
		return entity;
	}

	BinaryEngine::Entity PhysicsDemoState::CreateCircleBody(std::string_view name, BinaryEngine::Vector2f position, float radius, AtlasDisc disc, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider)
	{
		BinaryEngine::Entity entity{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ entity.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetCircleSpriteScale(radius);

		entity.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(disc));
		entity.AddComponent<BinaryEngine::RigidBodyComponent>(body);
		entity.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ radius }, .Collider{ collider } });
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
				RecordSpawn(CreateBoxBody("Crate", { s_StackCenterX + offset, rowY }, { s_CrateSize, s_CrateSize }, AtlasSquare::Crate, {}, {}));
			}
		}
	}

	void PhysicsDemoState::SpawnPanes()
	{
		for (const BinaryEngine::Entity& pane : m_PaneEntities)
		{
			if (pane.IsValid())
			{
				m_Scene.DestroyEntity(pane);
			}
		}
		m_PaneEntities.clear();

		for (std::int32_t paneIndex{ 0 }; paneIndex < s_PaneCount; paneIndex++)
		{
			const float offset{ (static_cast<float>(paneIndex) - static_cast<float>(s_PaneCount - 1) * 0.5f) * s_PaneSpacing };
			BinaryEngine::Entity pane{ CreateBoxBody("Pane", { s_PaneWallCenter.x + offset, s_PaneWallCenter.y }, s_PaneSize, AtlasSquare::Pane, {}, {}) };
			m_PaneEntities.push_back(pane);
			RecordSpawn(pane);
		}
	}

	void PhysicsDemoState::FireShot(BinaryEngine::Vector2f start, bool isBullet)
	{
		const BinaryEngine::Vector2f toTarget{ s_PaneWallCenter - start };
		const float distance{ glm::length(toTarget) };
		if (distance <= 0.0f)
		{
			return;
		}

		const BinaryEngine::RigidBodySpecification shotBody{ .Bullet{ isBullet } };
		const BinaryEngine::ColliderSpecification shotCollider{ .Material{ .Density{ s_ShotDensity } } };
		BinaryEngine::Entity shot{ CreateCircleBody(isBullet ? "BulletShot" : "Shot", start, s_ShotRadius, AtlasDisc::Shot, shotBody, shotCollider) };
		m_Scene.GetPhysicsBody(shot).SetLinearVelocity(toTarget / distance * s_ShotSpeed);
		RecordSpawn(shot);
	}

	void PhysicsDemoState::Explode(BinaryEngine::Vector2f center)
	{
		for (const entt::entity entityHandle : m_Scene.GetAllEntitiesWith<BinaryEngine::RigidBodyComponent>())
		{
			BinaryEngine::PhysicsBody body{ m_Scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &m_Scene }) };
			const BinaryEngine::Vector2f offset{ body.GetPosition() - center };
			const float distance{ glm::length(offset) };
			if (!body.IsValid() || distance <= 0.0f || distance >= s_ExplosionRadius)
			{
				continue;
			}

			const float falloff{ 1.0f - distance / s_ExplosionRadius };
			const float impulseMagnitude{ std::min(s_ExplosionImpulse * falloff, body.GetMass() * s_ExplosionMaxSpeedChange) };
			const BinaryEngine::Vector2f leverPoint{ body.GetPosition() + BinaryEngine::Vector2f{ 0.0f, s_ExplosionLeverArm } };
			body.ApplyLinearImpulse(offset / distance * impulseMagnitude, leverPoint);
		}
	}

	void PhysicsDemoState::SpawnDebris(BinaryEngine::Vector2f center)
	{
		const BinaryEngine::ColliderSpecification debrisCollider{ .Filter{ .Category{ s_DebrisCategory }, .Mask{ ~s_DebrisCategory } } };

		for (std::int32_t pieceIndex{ 0 }; pieceIndex < s_DebrisPieceCount; pieceIndex++)
		{
			const BinaryEngine::Vector2f position{ center + BinaryEngine::Random::Vector2f({ -s_DebrisSpread, -s_DebrisSpread }, { s_DebrisSpread, s_DebrisSpread }) };
			BinaryEngine::Entity piece{ CreateBoxBody("Debris", position, s_DebrisSize, AtlasSquare::Debris, {}, debrisCollider) };
			m_Scene.GetPhysicsBody(piece).SetLinearVelocity(BinaryEngine::Random::Vector2f({ -s_DebrisMaxSideSpeed, s_DebrisMinUpwardSpeed }, { s_DebrisMaxSideSpeed, s_DebrisMaxUpwardSpeed }));
			RecordSpawn(piece);
		}
	}

	void PhysicsDemoState::ApplyWind()
	{
		for (const entt::entity entityHandle : m_Scene.GetAllEntitiesWith<BinaryEngine::RigidBodyComponent>())
		{
			m_Scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &m_Scene }).ApplyForce(s_WindForce);
		}
	}

	void PhysicsDemoState::ApplyTwist(BinaryEngine::Vector2f center)
	{
		for (const entt::entity entityHandle : m_Scene.GetAllEntitiesWith<BinaryEngine::RigidBodyComponent>())
		{
			BinaryEngine::PhysicsBody body{ m_Scene.GetPhysicsBody(BinaryEngine::Entity{ entityHandle, &m_Scene }) };
			if (glm::length(body.GetPosition() - center) < s_TwistRadius)
			{
				body.ApplyTorque(s_TwistTorque);
			}
		}
	}

	void PhysicsDemoState::RecordSpawn(BinaryEngine::Entity entity)
	{
		m_SpawnedEntities.push_back(entity);
		m_LastSpawnedMass = m_Scene.GetPhysicsBody(entity).GetMass();
	}

	void PhysicsDemoState::RemoveSpawnedBodies()
	{
		for (const BinaryEngine::Entity& entity : m_SpawnedEntities)
		{
			if (entity.IsValid())
			{
				m_Scene.DestroyEntity(entity);
			}
		}

		m_SpawnedEntities.clear();
		m_PaneEntities.clear();
	}

	void PhysicsDemoState::ReadPhysicsEvents()
	{
		const BinaryEngine::PhysicsWorld& world{ m_Scene.GetPhysicsWorld() };

		m_TouchingPairCount += static_cast<std::int64_t>(world.GetContactBeginEvents().size()) - static_cast<std::int64_t>(world.GetContactEndEvents().size());
		m_LastStepHitCount = world.GetContactHitEvents().size();

		std::vector<BinaryEngine::ContactHitEvent> soundHitEvents;
		soundHitEvents.reserve(world.GetContactHitEvents().size());
		for (const BinaryEngine::ContactHitEvent& hitEvent : world.GetContactHitEvents())
		{
			if (!IsDebris(hitEvent.FirstEntity) && !IsDebris(hitEvent.SecondEntity))
			{
				soundHitEvents.push_back(hitEvent);
			}
		}
		PlayHitSounds(m_Audio, soundHitEvents, m_HitSoundHandle, m_SfxBus);

		for (const BinaryEngine::TriggerBeginEvent& triggerEvent : world.GetTriggerBeginEvents())
		{
			const bool isAlreadyInside{ std::find(m_BucketVisitors.begin(), m_BucketVisitors.end(), triggerEvent.VisitorEntity) != m_BucketVisitors.end() };
			if (triggerEvent.TriggerEntity == m_BucketTriggerEntity && !isAlreadyInside)
			{
				m_BucketVisitors.push_back(triggerEvent.VisitorEntity);
			}
		}

		for (const BinaryEngine::TriggerEndEvent& triggerEvent : world.GetTriggerEndEvents())
		{
			if (triggerEvent.TriggerEntity == m_BucketTriggerEntity)
			{
				std::erase(m_BucketVisitors, triggerEvent.VisitorEntity);
			}
		}
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
		text += std::format("Bodies {}, awake {}   Touching {}   Hits {}   Bucket {}   Last mass {:.2f} kg", world.GetBodyCount(), awakeCount, m_TouchingPairCount, m_LastStepHitCount, m_BucketVisitors.size(), m_LastSpawnedMass);
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
