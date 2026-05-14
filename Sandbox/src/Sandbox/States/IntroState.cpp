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
		dispatcher.Dispatch<BinaryEngine::WindowResizedEvent>([this](BinaryEngine::WindowResizedEvent& e) { e; m_StateManager.RequestClearStates(); m_StateManager.RequestPushState<MainMenuState>(); return true; });
	}

	void IntroState::OnUpdate()
	{
	}

	void IntroState::OnRender()
	{
		m_Context.renderer.SetDrawColor(BinaryEngine::Color::Red);
	}

}