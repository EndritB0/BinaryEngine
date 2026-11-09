#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Audio/AudioTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	class PhysicsDemoState : public BinaryEngine::State {
	public:
		PhysicsDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~PhysicsDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
		void CreateScenery();
		BinaryEngine::Entity CreateBoxBody(std::string_view name, BinaryEngine::Vector2f position, BinaryEngine::Vector2f size, AtlasSquare square, const BinaryEngine::RigidBodySpecification& body, const BinaryEngine::ColliderSpecification& collider);
		BinaryEngine::Entity CreateCircleBody(std::string_view name, BinaryEngine::Vector2f position, float radius, AtlasDisc disc);
		void SpawnCrateStack();
		void RecordSpawn(BinaryEngine::Entity entity);
		void RemoveSpawnedBodies();
		void ReadPhysicsEvents();
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_HitSoundHandle;
		BinaryEngine::AudioBusHandle m_SfxBus;
		BinaryEngine::Entity m_BucketTriggerEntity;

		std::vector<BinaryEngine::Entity> m_SpawnedEntities;
		std::vector<BinaryEngine::Entity> m_BucketVisitors;
		std::int64_t m_TouchingPairCount{ 0 };
		std::size_t m_LastStepHitCount{ 0 };
		float m_LastSpawnedMass{ 0.0f };
		bool m_ShowOutlines{ false };
	};

}
