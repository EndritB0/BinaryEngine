#include "pch.h"
#include "Sandbox/States/ShowcaseDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/RenderSpace.h>
#include <BinaryEngine/Renderer/ShapeSpecification.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_HudText{ "Binary Engine\nWASD to move, scroll to zoom\nWalk over the red square, under the green circle" };
		constexpr float s_HudTextSize{ 16.0f };
		constexpr float s_HudTextLeft{ 8.0f };
		constexpr float s_HudTextBaseline{ 20.0f };
		constexpr float s_HudTextZ{ 10.0f };
		constexpr float s_HudPanelZ{ 9.0f };
		constexpr float s_HudPanelPadding{ 3.0f };
		constexpr BinaryEngine::Color s_HudPanelColor{ 0x10, 0x10, 0x30, 0xC0 };
		constexpr BinaryEngine::Color s_HudBorderColor{ 0xFF, 0xFF, 0xFF, 0xFF };
		constexpr float s_ShapeSpinDegreesPerSecond{ 30.0f };
		constexpr float s_ShapeZ{ 2.0f };
		constexpr float s_ShapeSize{ 40.0f };
		constexpr float s_OutlineThickness{ 3.0f };
		constexpr float s_LineThickness{ 2.0f };
		constexpr float s_LineHalfLength{ 30.0f };
		constexpr BinaryEngine::Color s_FilledShapeColor{ 0xFF, 0xD7, 0x40, 0xFF };
		constexpr BinaryEngine::Color s_OutlinedShapeColor{ 0xFF, 0xFF, 0xFF, 0xFF };
		constexpr BinaryEngine::Color s_BelowPlayerColor{ 0xFF, 0x50, 0x50, 0xB0 };
		constexpr BinaryEngine::Color s_AbovePlayerColor{ 0x50, 0xFF, 0x70, 0xB0 };
	}

	ShowcaseDemoState::ShowcaseDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[ShowcaseDemoState] Created");
	}

	ShowcaseDemoState::~ShowcaseDemoState()
	{
		APP_TRACE("[ShowcaseDemoState] Destroyed");
	}

	void ShowcaseDemoState::OnAttach()
	{
		APP_INFO("[ShowcaseDemoState] Attached");

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
			.Size{ 16.0f },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::World },
			.Alignment{ BinaryEngine::TextAlignment::Center },
		};
		m_NameTagEntity = m_ActiveScene.CreateEntity("PlayerNameTag");
		m_NameTagEntity.AddComponent<BinaryEngine::TextComponent>(m_FontHandle, "Player", nameTagSpec);

		BinaryEngine::TextSpecification hudSpec{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::RenderSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		BinaryEngine::Entity hudEntity = m_ActiveScene.CreateEntity("Hud");
		auto& hudTransform = hudEntity.GetComponent<BinaryEngine::TransformComponent>();
		hudEntity.AddComponent<BinaryEngine::TextComponent>(m_FontHandle, std::string{ s_HudText }, hudSpec);
		hudTransform.transform.Position = { s_HudTextLeft, s_HudTextBaseline, s_HudTextZ };

		const std::shared_ptr<BinaryEngine::Font> font{ m_Context.assetManager.GetAsset<BinaryEngine::Font>(m_FontHandle) };
		if (font && font->IsValid())
		{
			const BinaryEngine::Vector2f hudTextBounds{ m_Context.renderer.MeasureText(*font, s_HudText, hudSpec) };
			const float textScale{ s_HudTextSize / font->GetPixelSize() };
			const float hudTextTop{ s_HudTextBaseline - font->GetAscent() * textScale };
			m_HudPanelCenter = { s_HudTextLeft + hudTextBounds.x * 0.5f, hudTextTop + hudTextBounds.y * 0.5f };
			m_HudPanelSize = { hudTextBounds.x + 2.0f * s_HudPanelPadding, hudTextBounds.y + 2.0f * s_HudPanelPadding };
		}
	}

	void ShowcaseDemoState::OnDetach()
	{
		APP_INFO("[ShowcaseDemoState] Detached");
	}

	void ShowcaseDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::MouseScrolledEvent>(BIND_FUNCTION(OnMouseScrolled));
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void ShowcaseDemoState::OnUpdate(BinaryEngine::TimeStep dt)
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

		m_ShapeAngleDegrees = std::fmod(m_ShapeAngleDegrees + s_ShapeSpinDegreesPerSecond * dt.GetSeconds(), 360.0f);
		m_ActiveScene.OnUpdate(dt, m_Context.clock.GetInterpolationAlpha());
	}

	void ShowcaseDemoState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);
		m_ActiveScene.OnRender(m_Context.renderer, m_Context.assetManager);
		DrawShapes();
		m_Context.renderer.EndScene();
	}

	void ShowcaseDemoState::DrawShapes()
	{
		BinaryEngine::Renderer& renderer{ m_Context.renderer };

		const BinaryEngine::Vector3f spin{ 0.0f, 0.0f, m_ShapeAngleDegrees };
		const BinaryEngine::Vector3f noRotation{ 0.0f, 0.0f, 0.0f };
		const BinaryEngine::Vector3f shapeScale{ s_ShapeSize, s_ShapeSize, 1.0f };

		const BinaryEngine::ShapeSpecification filled{ .Color{ s_FilledShapeColor } };
		renderer.DrawRectangle({ { 270.0f, 320.0f, s_ShapeZ }, spin, shapeScale }, filled);
		renderer.DrawCircle({ { 330.0f, 320.0f, s_ShapeZ }, spin, shapeScale }, filled);
		renderer.DrawTriangle({ { 390.0f, 320.0f, s_ShapeZ }, spin, shapeScale }, filled);

		const BinaryEngine::ShapeSpecification outlined{ .Color{ s_OutlinedShapeColor }, .OutlineThickness{ s_OutlineThickness } };
		renderer.DrawRectangle({ { 270.0f, 270.0f, s_ShapeZ }, spin, shapeScale }, outlined);
		renderer.DrawCircle({ { 330.0f, 270.0f, s_ShapeZ }, spin, shapeScale }, outlined);
		renderer.DrawTriangle({ { 390.0f, 270.0f, s_ShapeZ }, spin, shapeScale }, outlined);

		const float angleRadians{ glm::radians(m_ShapeAngleDegrees) };
		const BinaryEngine::Vector2f lineCenter{ 450.0f, 295.0f };
		const BinaryEngine::Vector2f lineOffset{ std::cos(angleRadians) * s_LineHalfLength, std::sin(angleRadians) * s_LineHalfLength };
		renderer.DrawLine(lineCenter - lineOffset, lineCenter + lineOffset, { .Color{ s_OutlinedShapeColor }, .Thickness{ s_LineThickness }, .SortZ{ s_ShapeZ } });

		renderer.DrawRectangle({ { 260.0f, 200.0f, 0.5f }, noRotation, { 50.0f, 50.0f, 1.0f } }, { .Color{ s_BelowPlayerColor } });
		renderer.DrawCircle({ { 140.0f, 200.0f, 1.5f }, noRotation, { 50.0f, 50.0f, 1.0f } }, { .Color{ s_AbovePlayerColor } });

		const BinaryEngine::Transform hudPanel{ { m_HudPanelCenter.x, m_HudPanelCenter.y, s_HudPanelZ }, noRotation, { m_HudPanelSize.x, m_HudPanelSize.y, 1.0f } };
		renderer.DrawRectangle(hudPanel, { .Color{ s_HudPanelColor }, .Space{ BinaryEngine::RenderSpace::Screen } });
		renderer.DrawRectangle(hudPanel, { .Color{ s_HudBorderColor }, .OutlineThickness{ 1.0f }, .Space{ BinaryEngine::RenderSpace::Screen } });
	}

	bool ShowcaseDemoState::OnMouseScrolled(BinaryEngine::MouseScrolledEvent& event)
	{
		const float zoomStep{ event.GetOffsetY() > 0.0f ? 1.1f : (1.0f / 1.1f) };
		m_Camera.SetZoom(m_Camera.GetZoom() * zoomStep);
		return true;
	}

	bool ShowcaseDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Space:
			{
				m_PlayerEntity.GetComponent<BinaryEngine::AnimationComponent>().Play(&m_WaveClip);
				return true;
			}

			default: return false;
		}
	}

}