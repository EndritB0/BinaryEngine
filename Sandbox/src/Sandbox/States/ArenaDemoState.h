#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsWorldSpecification.h>
#include <BinaryEngine/Renderer/AnimationClip.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/Scene/SceneSpecification.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	class ArenaDemoState : public BinaryEngine::State {
	public:
		ArenaDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~ArenaDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void CreateRoom();
		void CreateBlock(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, AtlasSquare square);
		void CreateCrates();
		void CreateCrate(std::string_view name, BinaryEngine::Vector2f center, AtlasSquare square, float density);
		void CreatePlayer();
		void CreateZombie(BinaryEngine::Vector2f position);
		void SpawnZombies();
		void RemoveSpawnedZombies();
		std::optional<BinaryEngine::Vector2f> FindFreeSpot(float clearanceRadius);
		void MovePlayer(float stepSeconds);
		void MoveZombies(float stepSeconds);
		void KnockBackPlayer(float stepSeconds);
		bool IsZombie(BinaryEngine::Entity entity) const;
		void CreateGem();
		void CollectGems();
		void LogStepSummary();
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene{ BinaryEngine::SceneSpecification{ .Physics{ BinaryEngine::PhysicsWorldSpecification{ .Gravity{ BinaryEngine::Vector2f{ 0.0f, 0.0f } } } } } };
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_CharacterHandle;
		BinaryEngine::AnimationClip m_IdleClip;

		BinaryEngine::Entity m_PlayerEntity;
		std::vector<BinaryEngine::Entity> m_Zombies;
		float m_KnockbackSecondsLeft{ 0.0f };
		std::int32_t m_KnockbackCount{ 0 };
		std::int32_t m_TimedStepCount{ 0 };
		double m_TimedStepSeconds{ 0.0 };
		std::vector<BinaryEngine::Entity> m_Gems;
		std::int32_t m_GemCount{ 0 };
		bool m_ShowOutlines{ false };
	};

}
