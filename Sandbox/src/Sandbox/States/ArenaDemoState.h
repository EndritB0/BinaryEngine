#pragma once

#include <string_view>

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
		void CreatePlayer();
		void MovePlayer(float stepSeconds);
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene{ BinaryEngine::SceneSpecification{ .Physics{ BinaryEngine::PhysicsWorldSpecification{ .Gravity{ BinaryEngine::Vector2f{ 0.0f, 0.0f } } } } } };
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_CharacterHandle;
		BinaryEngine::AnimationClip m_IdleClip;

		BinaryEngine::Entity m_PlayerEntity;
		bool m_ShowOutlines{ false };
	};

}
