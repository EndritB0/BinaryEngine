#include "pch.h"
#include "Sandbox/States/MainMenuState.h"

#include <BinaryEngine/Scene/Components.h>

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

		m_Context.renderer.SetClearColor(BinaryEngine::Color::Blue);

		m_CharacterTextureHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>("./resources/character/character_idle.png", m_Context.renderer);
		m_FontHandle = m_Context.assetManager.LoadAsset<BinaryEngine::Font>("./resources/fonts/OpenSans-Regular.ttf", m_Context.renderer);

		m_IdleClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.15f, true);
		m_MovingClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.06f, true);
		m_WaveClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.10f, false);

		m_PlayerEntity = m_ActiveScene.CreateEntity("Player");
		auto& playerTransform = m_PlayerEntity.GetComponent<BinaryEngine::TransformComponent>();
		m_PlayerEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterTextureHandle, BinaryEngine::TextureRegion{ { 0, 0 }, { 16, 16 } });
		m_PlayerEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		playerTransform.transform.Position = { 200.0f, 200.0f, 1.f };

		BinaryEngine::Entity allyEntity = m_ActiveScene.CreateEntity("Ally");
		auto& allyTransform = allyEntity.GetComponent<BinaryEngine::TransformComponent>();
		allyEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterTextureHandle, BinaryEngine::TextureRegion{ { 0, 0 }, { 16, 16 } });
		allyEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		allyTransform.transform.Position = { 100.f, 100.f, 0.f };
		allyTransform.transform.Scale = { 2.f, 1.f, 1.f };

		BinaryEngine::TextSpecification nameTagSpec{
			.Size { 16.0f },
			.FillColor {BinaryEngine::Color::White},
			.Space {BinaryEngine::TextSpace::World},
			.Alignment {BinaryEngine::TextAlignment::Center},
		};
		m_NameTagEntity = m_ActiveScene.CreateEntity("PlayerNameTag");
		m_NameTagEntity.AddComponent<BinaryEngine::TextComponent>(m_FontHandle, "Player", nameTagSpec);

		BinaryEngine::TextSpecification hudSpec{
			.Size { 16.0f },
			.FillColor {BinaryEngine::Color::White},
			.Space {BinaryEngine::TextSpace::Screen},
			.Alignment {BinaryEngine::TextAlignment::Left},
		};
		BinaryEngine::Entity hudEntity = m_ActiveScene.CreateEntity("Hud");
		auto& hudTransform = hudEntity.GetComponent<BinaryEngine::TransformComponent>();
		hudEntity.AddComponent<BinaryEngine::TextComponent>(m_FontHandle, "Binary Engine\nWASD to move", hudSpec);
		hudTransform.transform.Position = { 8.0f, 20.0f, 10.0f };
	}

	void MainMenuState::OnDetach()
	{
		APP_INFO("[MainMenuState] Detached");
	}

	void MainMenuState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
		dispatcher.Dispatch<BinaryEngine::MouseScrolledEvent>(BIND_FUNCTION(OnMouseScrolled));
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void MainMenuState::OnUpdate([[maybe_unused]] BinaryEngine::TimeStep dt)
	{
		auto& playerTransform = m_PlayerEntity.GetComponent<BinaryEngine::TransformComponent>().transform;

		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W))
		{
			playerTransform.Position.y += 100.f * dt.GetSeconds();
		}
		else if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S))
		{
			playerTransform.Position.y -= 100.f * dt.GetSeconds();
		}

		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A))
		{
			playerTransform.Position.x -= 100.f * dt.GetSeconds();
		}
		else if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D))
		{
			playerTransform.Position.x += 100.f * dt.GetSeconds();
		}

		m_Camera.SetPosition({ playerTransform.Position.x, playerTransform.Position.y, 1 });

		auto& nameTagTransform = m_NameTagEntity.GetComponent<BinaryEngine::TransformComponent>().transform;
		nameTagTransform.Position = { playerTransform.Position.x, playerTransform.Position.y + 14.0f, 5.0f };

		auto& playerAnimation = m_PlayerEntity.GetComponent<BinaryEngine::AnimationComponent>();
		const bool isPlayingOneShot{ playerAnimation.CurrentClip == &m_WaveClip && !playerAnimation.Finished };
		if (!isPlayingOneShot)
		{
			const bool isMoving{ BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D) };

			playerAnimation.Play(isMoving ? &m_MovingClip : &m_IdleClip);
		}

		m_ActiveScene.OnUpdate(dt);
	}

	void MainMenuState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);
		m_ActiveScene.OnRender(m_Context.renderer, m_Context.assetManager);
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

	bool MainMenuState::OnMouseScrolled(BinaryEngine::MouseScrolledEvent& event)
	{
		const float zoomStep{ event.GetOffsetY() > 0.0f ? 1.1f : (1.0f / 1.1f) };
		m_Camera.SetZoom(m_Camera.GetZoom() * zoomStep);
		return true;
	}

	bool MainMenuState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Space:
			{
				m_PlayerEntity.GetComponent<BinaryEngine::AnimationComponent>().Play(&m_WaveClip);
				return true;
			}

			default:return false;
		}
	}

}