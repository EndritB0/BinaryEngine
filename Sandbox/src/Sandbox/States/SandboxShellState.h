#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/State/State.h>

#include "Sandbox/SandboxOptions.h"

namespace Sandbox {

	class SandboxShellState : public BinaryEngine::State {
	public:
		SandboxShellState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context, const SandboxOptions& options);
		virtual ~SandboxShellState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void SwitchToDemo(std::size_t demoIndex);
		void AdvanceSmokeRun();
		void Quit();
		std::size_t GetStartDemoIndex() const;

	private:
		SandboxOptions m_Options;
		std::optional<std::size_t> m_ActiveDemoIndex;
		std::uint32_t m_FramesOnActiveDemo{ 0 };
		bool m_IsQuitting{ false };
	};

}
