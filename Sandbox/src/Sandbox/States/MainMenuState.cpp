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
		m_CharacterTexture.LoadFromFile(m_Context.renderer, "./resources/character/herochar_idle_anim_strip_4.png");
	}

	void MainMenuState::OnDetach()
	{
		APP_INFO("[MainMenuState] Detached");
	}

	void MainMenuState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
	}

	void MainMenuState::OnUpdate([[maybe_unused]] BinaryEngine::TimeStep dt)
	{
	}

	void MainMenuState::OnRender()
	{
		m_Context.renderer.SetDrawColor(BinaryEngine::Color::Blue);
		m_Context.renderer.DrawTexture(m_CharacterTexture, { 100, 100 }, { m_CharacterTexture.GetWidth(), m_CharacterTexture.GetHeight() });
	}

	bool MainMenuState::OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event)
	{
		switch (event.GetButton())
		{
			case BinaryEngine::Mouse::Right:
			{
				APP_INFO("[MainMenuState] Button 1 Pressed");
				m_StateManager.RequestClearStates();
				m_StateManager.RequestPushState<IntroState>();
				return true;
			}
		}
		return false;
	}

}