#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Audio/AudioTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Physics/PhysicsWorldSpecification.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/Scene/SceneSpecification.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/PhysicsDemoCommon.h"

namespace Sandbox {

	class TopDownDemoState : public BinaryEngine::State {
	public:
		TopDownDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~TopDownDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
		void CreateTable();
		BinaryEngine::Entity CreatePuck(std::string_view name, BinaryEngine::Vector2f position, AtlasDisc disc, bool isBullet);
		void RackPucks();
		void ShootCuePuck(BinaryEngine::Vector2f target);
		void SpinCuePuck();
		void ResetCuePuck();
		void ReadPhysicsEvents();
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene{ BinaryEngine::SceneSpecification{ .Physics{ BinaryEngine::PhysicsWorldSpecification{ .Gravity{ BinaryEngine::Vector2f{ 0.0f, 0.0f } } } } } };
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_HitSoundHandle;
		BinaryEngine::AudioBusHandle m_SfxBus;

		BinaryEngine::Entity m_CueEntity;
		std::vector<BinaryEngine::Entity> m_PuckEntities;
		std::int32_t m_PocketedCount{ 0 };
		std::int32_t m_ShotCount{ 0 };
		bool m_ShowOutlines{ false };
	};

}
