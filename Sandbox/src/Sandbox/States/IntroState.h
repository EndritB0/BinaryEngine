#pragma once

#include "BinaryEngine/State/State.h"
#include "BinaryEngine/Event/EventTypes.h"

namespace Sandbox {

	class IntroState : public BinaryEngine::State {
	public:
		IntroState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~IntroState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnMouseButtonReleased(BinaryEngine::MouseButtonReleasedEvent& event);
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
	};

}