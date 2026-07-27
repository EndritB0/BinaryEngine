#pragma once

#include "BinaryEngine/State/State.h"
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>

namespace Sandbox {

	class MainMenuState : public BinaryEngine::State {
	public:
		MainMenuState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~MainMenuState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
		bool OnWindowResized(BinaryEngine::WindowResizedEvent& event);

	private:
	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution() };
		BinaryEngine::AssetHandle m_CharacterTexture;
		BinaryEngine::Scene m_ActiveScene;
		BinaryEngine::Entity m_PlayerEntity;

		static constexpr int s_IdleFrameCount{ 4 };
		float m_AnimationTimer{ 0.0f };
		float m_FrameDuration{ 0.15f };
		int m_CurrentFrame{ 0 };
	};

}