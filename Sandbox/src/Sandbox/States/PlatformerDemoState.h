#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Renderer/AnimationClip.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	class PlatformerDemoState : public BinaryEngine::State {
	public:
		PlatformerDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~PlatformerDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		struct MovingPlatform {
			BinaryEngine::Entity Entity;
			BinaryEngine::Vector2f From{ 0.0f, 0.0f };
			BinaryEngine::Vector2f To{ 0.0f, 0.0f };
			float Speed{ 0.0f };
		};

		struct Zombie {
			BinaryEngine::Entity Entity;
			float Direction{ 1.0f };
		};

		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		bool OnKeyReleased(BinaryEngine::KeyReleasedEvent& event);
		void CreateFrame();
		void CreateTerrain();
		BinaryEngine::Entity CreateBlock(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square);
		void CreateSlope(std::string_view name, BinaryEngine::Vector2f start, float angleDegrees, float length);
		void CreateBodies();
		void CreatePlatforms();
		BinaryEngine::Entity CreateBoxBody(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider);
		BinaryEngine::Entity CreateCircleBody(std::string_view name, BinaryEngine::Vector2f center, float radius, AtlasDisc disc, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider);
		void CreateMovingPlatform(std::string_view name, BinaryEngine::Vector2f from, BinaryEngine::Vector2f to, BinaryEngine::Vector2f size, float speed);
		void CreatePlayer();
		void CreateZombie(BinaryEngine::Vector2f start, float direction);
		void MovePlatforms();
		void MovePlayer(float stepSeconds);
		void MoveZombies(float stepSeconds);
		void TurnZombiesAtWalls();
		void KnockBackPlayer(float stepSeconds);
		bool IsZombie(BinaryEngine::Entity entity) const;
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_CharacterHandle;
		BinaryEngine::AnimationClip m_IdleClip;

		std::vector<MovingPlatform> m_MovingPlatforms;
		BinaryEngine::Entity m_PlayerEntity;
		float m_CoyoteSecondsLeft{ 0.0f };
		float m_JumpBufferSecondsLeft{ 0.0f };
		bool m_IsJumpReleased{ false };
		std::int32_t m_JumpCount{ 0 };
		std::vector<Zombie> m_Zombies;
		float m_KnockbackSecondsLeft{ 0.0f };
		std::int32_t m_KnockbackCount{ 0 };
		bool m_ShowOutlines{ false };
	};

}
