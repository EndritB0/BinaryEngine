#pragma once

#include <cstdint>
#include <string_view>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Core/Math.h>
#include <BinaryEngine/Event/EventTypes.h>
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
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		bool OnKeyReleased(BinaryEngine::KeyReleasedEvent& event);
		void CreateFrame();
		void CreateTerrain();
		BinaryEngine::Entity CreateBlock(std::string_view name, BinaryEngine::Vector2f center, BinaryEngine::Vector2f size, float rotationDegrees, AtlasSquare square);
		void CreateSlope(std::string_view name, BinaryEngine::Vector2f start, float angleDegrees, float length);
		void CreatePlayer();
		void MovePlayer(float stepSeconds);
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		BinaryEngine::AssetHandle m_AtlasHandle;
		BinaryEngine::AssetHandle m_CharacterHandle;
		BinaryEngine::AnimationClip m_IdleClip;

		BinaryEngine::Entity m_PlayerEntity;
		float m_CoyoteSecondsLeft{ 0.0f };
		float m_JumpBufferSecondsLeft{ 0.0f };
		bool m_IsJumpReleased{ false };
		std::int32_t m_JumpCount{ 0 };
		bool m_ShowOutlines{ false };
	};

}
