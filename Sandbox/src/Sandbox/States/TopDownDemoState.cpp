#include "pch.h"
#include "Sandbox/States/TopDownDemoState.h"

#include <BinaryEngine/Audio/AudioClip.h>
#include <BinaryEngine/Audio/AudioEngine.h>
#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
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
		constexpr std::string_view s_HitSoundPath{ "./resources/audio/one_shot_2.wav" };
		constexpr std::string_view s_KeyHelp{ "Left mouse  shoot the cue puck (further is harder)   Right mouse  spin it   C outlines" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1E, 0x5A, 0x32 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_HudZ{ 10.0f };
		constexpr float s_BodyZ{ 0.0f };
		constexpr float s_PocketZ{ -1.0f };
		constexpr float s_TableWallX{ 270.0f };
		constexpr float s_TableWallY{ 150.0f };
		constexpr float s_WallRestitution{ 0.9f };
		constexpr float s_WallFriction{ 0.1f };
		constexpr float s_PocketRadius{ 14.0f };
		constexpr float s_PuckRadius{ 10.0f };
		constexpr float s_PuckRestitution{ 0.9f };
		constexpr float s_PuckLinearDamping{ 0.8f };
		constexpr float s_PuckAngularDamping{ 1.0f };
		constexpr std::int32_t s_RackRowCount{ 4 };
		constexpr float s_RackRowSpacing{ 17.5f };
		constexpr float s_RackColumnSpacing{ 20.3f };
		constexpr float s_ShotSpeedPerUnit{ 4.0f };
		constexpr float s_MaxShotSpeed{ 1000.0f };
		constexpr float s_SpinImpulse{ 300.0f };
		const BinaryEngine::Vector2f s_HorizontalWallSize{ 560.0f, 20.0f };
		const BinaryEngine::Vector2f s_VerticalWallSize{ 20.0f, 320.0f };
		const BinaryEngine::Vector2f s_RackApex{ 90.0f, 0.0f };
		const BinaryEngine::Vector2f s_CueStart{ -150.0f, 0.0f };
		const std::array<BinaryEngine::Vector2f, 6> s_PocketCenters{
			BinaryEngine::Vector2f{ -248.0f, 128.0f },
			BinaryEngine::Vector2f{ 0.0f, 128.0f },
			BinaryEngine::Vector2f{ 248.0f, 128.0f },
			BinaryEngine::Vector2f{ -248.0f, -128.0f },
			BinaryEngine::Vector2f{ 0.0f, -128.0f },
			BinaryEngine::Vector2f{ 248.0f, -128.0f },
		};

	}

	TopDownDemoState::TopDownDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[TopDownDemoState] Created");
	}

	TopDownDemoState::~TopDownDemoState()
	{
		APP_TRACE("[TopDownDemoState] Destroyed");
	}

	void TopDownDemoState::OnAttach()
	{
		APP_INFO("[TopDownDemoState] Attached");

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
		m_HudEntity = m_Scene.CreateEntity("TopDownHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, s_HudZ };

		CreateTable();
		RackPucks();
		m_CueEntity = CreatePuck("CuePuck", s_CueStart, AtlasDisc::CuePuck, true);
	}

	void TopDownDemoState::OnDetach()
	{
		APP_INFO("[TopDownDemoState] Detached");
	}

	void TopDownDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
	}

	void TopDownDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_Scene.OnFixedUpdate(fixedStep);
		ReadPhysicsEvents();
	}

	void TopDownDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		m_Scene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
		UpdateHud();
	}

	void TopDownDemoState::OnRender()
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

	bool TopDownDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat() || event.GetKeyCode() != BinaryEngine::Key::C)
		{
			return false;
		}

		m_ShowOutlines = !m_ShowOutlines;
		return true;
	}

	bool TopDownDemoState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		if (event.GetButton() == BinaryEngine::Mouse::Left)
		{
			ShootCuePuck(m_Camera.ScreenToWorld(event.GetPosition()));
			return true;
		}

		if (event.GetButton() == BinaryEngine::Mouse::Right)
		{
			SpinCuePuck();
			return true;
		}

		return false;
	}

	void TopDownDemoState::CreateTable()
	{
		const BinaryEngine::RigidBodySpecification staticBody{ .Type{ BinaryEngine::BodyType::Static } };
		const BinaryEngine::ColliderSpecification wallCollider{ .Material{ .Friction{ s_WallFriction }, .Restitution{ s_WallRestitution } }, .EnableTriggerEvents{ false } };
		const std::array<std::pair<BinaryEngine::Vector2f, BinaryEngine::Vector2f>, 4> walls{
			std::pair{ BinaryEngine::Vector2f{ 0.0f, s_TableWallY }, s_HorizontalWallSize },
			std::pair{ BinaryEngine::Vector2f{ 0.0f, -s_TableWallY }, s_HorizontalWallSize },
			std::pair{ BinaryEngine::Vector2f{ -s_TableWallX, 0.0f }, s_VerticalWallSize },
			std::pair{ BinaryEngine::Vector2f{ s_TableWallX, 0.0f }, s_VerticalWallSize },
		};

		for (const auto& [center, size] : walls)
		{
			BinaryEngine::Entity wall{ m_Scene.CreateEntity("TableWall") };
			BinaryEngine::Transform& transform{ wall.GetComponent<BinaryEngine::TransformComponent>().transform };
			transform.Position = { center.x, center.y, s_BodyZ };
			transform.Scale = GetBoxSpriteScale(size);
			wall.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(AtlasSquare::TableWall));
			wall.AddComponent<BinaryEngine::RigidBodyComponent>(staticBody);
			wall.AddComponent<BinaryEngine::BoxColliderComponent>(BinaryEngine::BoxColliderSpecification{ .Size{ size }, .Collider{ wallCollider } });
		}

		for (const BinaryEngine::Vector2f& center : s_PocketCenters)
		{
			BinaryEngine::Entity pocket{ m_Scene.CreateEntity("Pocket") };
			BinaryEngine::Transform& transform{ pocket.GetComponent<BinaryEngine::TransformComponent>().transform };
			transform.Position = { center.x, center.y, s_PocketZ };
			transform.Scale = GetCircleSpriteScale(s_PocketRadius);
			pocket.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(AtlasDisc::Pocket));
			pocket.AddComponent<BinaryEngine::RigidBodyComponent>(staticBody);
			pocket.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ s_PocketRadius }, .Collider{ .IsTrigger{ true } } });
		}
	}

	BinaryEngine::Entity TopDownDemoState::CreatePuck(std::string_view name, BinaryEngine::Vector2f position, AtlasDisc disc, bool isBullet)
	{
		const BinaryEngine::RigidBodySpecification puckBody{ .LinearDamping{ s_PuckLinearDamping }, .AngularDamping{ s_PuckAngularDamping }, .Bullet{ isBullet } };
		const BinaryEngine::ColliderSpecification puckCollider{ .Material{ .Restitution{ s_PuckRestitution } } };

		BinaryEngine::Entity puck{ m_Scene.CreateEntity(std::string{ name }) };
		BinaryEngine::Transform& transform{ puck.GetComponent<BinaryEngine::TransformComponent>().transform };
		transform.Position = { position.x, position.y, s_BodyZ };
		transform.Scale = GetCircleSpriteScale(s_PuckRadius);
		puck.AddComponent<BinaryEngine::SpriteComponent>(m_AtlasHandle, GetAtlasRegion(disc));
		puck.AddComponent<BinaryEngine::RigidBodyComponent>(puckBody);
		puck.AddComponent<BinaryEngine::CircleColliderComponent>(BinaryEngine::CircleColliderSpecification{ .Radius{ s_PuckRadius }, .Collider{ puckCollider } });
		return puck;
	}

	void TopDownDemoState::RackPucks()
	{
		for (std::int32_t rowIndex{ 0 }; rowIndex < s_RackRowCount; rowIndex++)
		{
			for (std::int32_t puckIndex{ 0 }; puckIndex <= rowIndex; puckIndex++)
			{
				const float x{ s_RackApex.x + static_cast<float>(rowIndex) * s_RackRowSpacing };
				const float y{ s_RackApex.y + (static_cast<float>(puckIndex) - static_cast<float>(rowIndex) * 0.5f) * s_RackColumnSpacing };
				const AtlasDisc disc{ (rowIndex + puckIndex) % 2 == 0 ? AtlasDisc::RedPuck : AtlasDisc::YellowPuck };
				m_PuckEntities.push_back(CreatePuck("Puck", { x, y }, disc, false));
			}
		}
	}

	void TopDownDemoState::ShootCuePuck(BinaryEngine::Vector2f target)
	{
		BinaryEngine::PhysicsBody cueBody{ m_Scene.GetPhysicsBody(m_CueEntity) };
		const BinaryEngine::Vector2f toTarget{ target - cueBody.GetPosition() };
		const float distance{ glm::length(toTarget) };
		if (!cueBody.IsValid() || distance <= 0.0f)
		{
			return;
		}

		const float speed{ std::min(distance * s_ShotSpeedPerUnit, s_MaxShotSpeed) };
		cueBody.ApplyLinearImpulse(toTarget / distance * (speed * cueBody.GetMass()));
		m_ShotCount++;
	}

	void TopDownDemoState::SpinCuePuck()
	{
		m_Scene.GetPhysicsBody(m_CueEntity).ApplyAngularImpulse(s_SpinImpulse);
	}

	void TopDownDemoState::ResetCuePuck()
	{
		BinaryEngine::PhysicsBody cueBody{ m_Scene.GetPhysicsBody(m_CueEntity) };
		cueBody.SetTransform(s_CueStart, 0.0f);
		cueBody.SetLinearVelocity({ 0.0f, 0.0f });
		cueBody.SetAngularVelocity(0.0f);
	}

	void TopDownDemoState::ReadPhysicsEvents()
	{
		const BinaryEngine::PhysicsWorld& world{ m_Scene.GetPhysicsWorld() };
		PlayHitSounds(m_Audio, world.GetContactHitEvents(), m_HitSoundHandle, m_SfxBus);

		for (const BinaryEngine::TriggerBeginEvent& triggerEvent : world.GetTriggerBeginEvents())
		{
			const BinaryEngine::Entity& visitor{ triggerEvent.VisitorEntity };
			if (visitor == m_CueEntity)
			{
				ResetCuePuck();
				continue;
			}

			const auto puck{ std::find(m_PuckEntities.begin(), m_PuckEntities.end(), visitor) };
			if (puck != m_PuckEntities.end() && visitor.IsValid())
			{
				m_PuckEntities.erase(puck);
				m_Scene.DestroyEntity(visitor);
				m_PocketedCount++;
			}
		}
	}

	void TopDownDemoState::UpdateHud()
	{
		std::string text{ std::format("{}: {}\n", s_KeyHelp, m_ShowOutlines ? "on" : "off") };
		text += std::format("Pucks left {}   Pocketed {}   Shots {}", m_PuckEntities.size(), m_PocketedCount, m_ShotCount);
		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
