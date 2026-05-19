#pragma once

#include "BinaryEngine/State/State.h"
#include <BinaryEngine/Event/EventTypes.h>

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

	private:
	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution() };
		BinaryEngine::AssetHandle m_CharacterTexture;
		BinaryEngine::Transform m_CharacterTransform{ { 200.0f, 200.0f, 1.f }, { 0.f, 0.f, 0.f }, { 1.f, 1.f, 1.f } };
		BinaryEngine::Transform m_CharacterTransform2{ { 100.0f, 100.0f, 0.f }, { 0.f, 0.f, 0.f }, { 2.f, 1.f, 1.f } };
	};

}