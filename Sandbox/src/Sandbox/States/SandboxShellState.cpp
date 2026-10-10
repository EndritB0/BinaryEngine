#include "pch.h"
#include "Sandbox/States/SandboxShellState.h"

#include <array>

#include <BinaryEngine/State/StateManager.h>

#include "Sandbox/DemoRegistry.h"

namespace Sandbox {

	namespace {

		constexpr std::array<BinaryEngine::KeyCode, 12> s_DemoKeys{
			BinaryEngine::Key::F1,
			BinaryEngine::Key::F2,
			BinaryEngine::Key::F3,
			BinaryEngine::Key::F4,
			BinaryEngine::Key::F5,
			BinaryEngine::Key::F6,
			BinaryEngine::Key::F7,
			BinaryEngine::Key::F8,
			BinaryEngine::Key::F9,
			BinaryEngine::Key::F10,
			BinaryEngine::Key::F11,
			BinaryEngine::Key::F12,
		};

	}

	SandboxShellState::SandboxShellState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[SandboxShellState] Created");
	}

	SandboxShellState::~SandboxShellState()
	{
		APP_TRACE("[SandboxShellState] Destroyed");
	}

	void SandboxShellState::OnAttach()
	{
		APP_INFO("[SandboxShellState] Attached");

		m_Context.renderer.SetClearColor(BinaryEngine::Color::Black);
	}

	void SandboxShellState::OnDetach()
	{
		APP_INFO("[SandboxShellState] Detached");
	}

	void SandboxShellState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void SandboxShellState::OnUpdate([[maybe_unused]] BinaryEngine::TimeStep dt)
	{
		if (m_IsQuitting)
		{
			return;
		}

		if (!m_ActiveDemoIndex.has_value())
		{
			SwitchToDemo(0);
		}
	}

	void SandboxShellState::OnRender()
	{}

	bool SandboxShellState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		if (event.IsRepeat() || m_IsQuitting)
		{
			return false;
		}

		if (event.GetKeyCode() == BinaryEngine::Key::Escape)
		{
			APP_INFO("[SandboxShellState] ESC pressed, closing application");
			Quit();
			return true;
		}

		const std::size_t demoCount{ GetDemos().size() };
		for (std::size_t demoIndex{ 0 }; demoIndex < demoCount; ++demoIndex)
		{
			if (event.GetKeyCode() == s_DemoKeys[demoIndex])
			{
				SwitchToDemo(demoIndex);
				return true;
			}
		}

		return false;
	}

	void SandboxShellState::SwitchToDemo(std::size_t demoIndex)
	{
		const DemoEntry& demo{ GetDemos()[demoIndex] };

		if (m_ActiveDemoIndex.has_value())
		{
			m_StateManager.RequestPopState();
		}
		demo.Push(m_StateManager);

		APP_INFO("[SandboxShellState] Switching to demo '{}' (F{})", demo.Name, demoIndex + 1);
		m_ActiveDemoIndex = demoIndex;
	}

	void SandboxShellState::Quit()
	{
		m_IsQuitting = true;
		m_StateManager.RequestClearStates();
	}

}
