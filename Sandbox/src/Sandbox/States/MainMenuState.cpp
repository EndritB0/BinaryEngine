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
		m_CharacterTexture = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>("./resources/character/character_idle.png", m_Context.renderer);
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
		m_Camera.SetPosition({ m_CharacterTransform.Position.x, m_CharacterTransform.Position.y, 20 });

		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W))
		{
			m_CharacterTransform.Position.y -= 100.f * dt.GetSeconds();
		}
		else if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S))
		{
			m_CharacterTransform.Position.y += 100.f * dt.GetSeconds();
		}

		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A))
		{
			m_CharacterTransform.Position.x -= 100.f * dt.GetSeconds();
		}
		else if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D))
		{
			m_CharacterTransform.Position.x += 100.f * dt.GetSeconds();
		}

	}

	void MainMenuState::OnRender()
	{
		m_Context.renderer.SetDrawColor(BinaryEngine::Color::Blue);
		m_Context.renderer.BeginScene(m_Camera);
		auto characterAsset = m_Context.assetManager.GetAsset<BinaryEngine::Texture2D>(m_CharacterTexture);
		m_Context.renderer.DrawTexture(*characterAsset, m_CharacterTransform);
		m_Context.renderer.DrawTexture(*characterAsset, m_CharacterTransform2);
		m_Context.renderer.EndScene();
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