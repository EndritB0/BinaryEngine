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
		virtual void OnUpdate() override;
		virtual void OnRender() override;

	private:
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
	};

}