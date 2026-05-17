#include "pch.h"
#include "Sandbox/States/IntroState.h"

#include "Sandbox/States/MainMenuState.h"

namespace Sandbox {

	IntroState::IntroState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[IntroState] Created");
	}

	IntroState::~IntroState()
	{
		APP_TRACE("[IntroState] Destroyed");
	}

	void IntroState::OnAttach()
	{
		APP_INFO("[IntroState] Attached");
	}

	void IntroState::OnDetach()
	{
		APP_INFO("[IntroState] Detached");
	}

	void IntroState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::MouseButtonReleasedEvent>(BIND_FUNCTION(OnMouseButtonReleased));
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void IntroState::OnUpdate([[maybe_unused]] BinaryEngine::TimeStep dt)
	{
	}

	void IntroState::OnRender()
	{
		m_Context.renderer.SetDrawColor(BinaryEngine::Color::Red);
	}

	bool IntroState::OnMouseButtonReleased(BinaryEngine::MouseButtonReleasedEvent& event)
	{
		switch (event.GetButton())
		{
			case BinaryEngine::Mouse::Button1:
			{
				m_StateManager.RequestClearStates();
				m_StateManager.RequestPushState<MainMenuState>();
				return true;
			}
		}
		return false;
	}

	bool IntroState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Q:
			{
				m_StateManager.RequestClearStates();
				return true;
			}
		}

		return false;
	}

}