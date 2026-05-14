#include "pch.h"
#include "Sandbox/States/MainMenuState.h"

#include "Sandbox/States/IntroState.h"

namespace Sandbox {

	MainMenuState::MainMenuState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[MainMenuState] Created");
	}

	MainMenuState::~MainMenuState()
	{
		APP_TRACE("[MainMenuState] Destroyed");
	}

	void MainMenuState::OnAttach()
	{
		APP_INFO("[MainMenuState] Attached");
	}

	void MainMenuState::OnDetach()
	{
		APP_INFO("[MainMenuState] Detached");
	}

	void MainMenuState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::WindowResizedEvent>([this](BinaryEngine::WindowResizedEvent& e) { e; m_StateManager.RequestClearStates(); m_StateManager.RequestPushState<IntroState>(); return true; });
	}

	void MainMenuState::OnUpdate()
	{
	}

	void MainMenuState::OnRender()
	{
		m_Context.renderer.SetDrawColor(BinaryEngine::Color::Blue);
	}

}