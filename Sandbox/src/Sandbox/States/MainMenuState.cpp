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

		m_CharacterTexture = m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>("./resources/character/character_idle.png", m_Context.renderer);

		m_IdleClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.15f, true);
		m_MovingClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.06f, true);
		m_WaveClip = BinaryEngine::MakeStripClip({ 0, 0 }, { 16, 16 }, 4, 4, 0.10f, false);

		m_PlayerEntity = m_ActiveScene.CreateEntity("Player");
		auto& playerTransform = m_PlayerEntity.GetComponent<BinaryEngine::TransformComponent>();
		m_PlayerEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterTexture, BinaryEngine::TextureRegion{ { 0, 0 }, { 16, 16 } });
		m_PlayerEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		playerTransform.transform.Position = { 200.0f, 200.0f, 1.f };

		BinaryEngine::Entity allyEntity = m_ActiveScene.CreateEntity("Ally");
		auto& allyTransform = allyEntity.GetComponent<BinaryEngine::TransformComponent>();
		allyEntity.AddComponent<BinaryEngine::SpriteComponent>(m_CharacterTexture, BinaryEngine::TextureRegion{ { 0, 0 }, { 16, 16 } });
		allyEntity.AddComponent<BinaryEngine::AnimationComponent>(&m_IdleClip);
		allyTransform.transform.Position = { 100.f, 100.f, 0.f };
		allyTransform.transform.Scale = { 2.f, 1.f, 1.f };
	}

	void MainMenuState::OnDetach()
	{
		APP_INFO("[MainMenuState] Detached");
	}

	void MainMenuState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::MouseButtonPressedEvent>(BIND_FUNCTION(OnMouseButtonPressed));
		dispatcher.Dispatch<BinaryEngine::WindowResizedEvent>(BIND_FUNCTION(OnWindowResized));
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

		auto& playerAnimation = m_PlayerEntity.GetComponent<BinaryEngine::AnimationComponent>();
		const bool inOneShot{ playerAnimation.CurrentClip == &m_WaveClip && !playerAnimation.Finished };
		if (!inOneShot)
		{
			const bool moving{ BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::W)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::A)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::S)
				|| BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::D) };

			playerAnimation.Play(moving ? &m_MovingClip : &m_IdleClip);
		}

		m_ActiveScene.OnUpdate(dt);
	}

	void MainMenuState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);

		auto view = m_ActiveScene.GetAllEntitiesWith<BinaryEngine::TransformComponent, BinaryEngine::SpriteComponent>();

		view.each([&](auto& transform, auto& sprite) {
			auto textureAsset = m_Context.assetManager.GetAsset<BinaryEngine::Texture2D>(sprite.TextureHandle);

			if (!textureAsset)
			{
				return;
			}

			if (sprite.UseRegion)
			{
				m_Context.renderer.DrawSprite(*textureAsset, transform.transform, sprite.Region);
			}
			else
			{
				m_Context.renderer.DrawSprite(*textureAsset, transform.transform);
			}
				  });

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

	bool MainMenuState::OnWindowResized(BinaryEngine::WindowResizedEvent& event)
	{
		m_Camera.OnResize(event.GetSize());
		return false;
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