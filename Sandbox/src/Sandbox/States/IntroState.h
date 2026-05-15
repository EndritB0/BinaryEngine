#pragma once

#include "BinaryEngine/State/State.h"

namespace Sandbox {

	class IntroState : public BinaryEngine::State {
	public:
		IntroState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~IntroState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate() override;
		virtual void OnRender() override;

	private:
		bool OnMouseButtonReleased(BinaryEngine::MouseButtonReleasedEvent& event);
	};

}