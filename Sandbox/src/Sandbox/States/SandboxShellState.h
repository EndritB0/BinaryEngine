#pragma once

#include <cstddef>
#include <optional>

#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/State/State.h>

namespace Sandbox {

	class SandboxShellState : public BinaryEngine::State {
	public:
		SandboxShellState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~SandboxShellState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void SwitchToDemo(std::size_t demoIndex);
		void Quit();

	private:
		std::optional<std::size_t> m_ActiveDemoIndex;
		bool m_IsQuitting{ false };
	};

}
