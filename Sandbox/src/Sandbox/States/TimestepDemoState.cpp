#include "pch.h"
#include "Sandbox/States/TimestepDemoState.h"

#include <BinaryEngine/Core/Color.h>
#include <BinaryEngine/Core/FrameClock.h>
#include <BinaryEngine/Core/Transform.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Renderer/Texture2D.h>
#include <BinaryEngine/Renderer/TextureRegion.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_CharacterTexturePath{ "./resources/character/character_idle.png" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x18, 0x30, 0x20 };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr float s_LabelTextSize{ 10.0f };
		constexpr std::array<std::uint16_t, 6> s_FixedUpdateRates{ 5, 10, 20, 30, 60, 120 };
		constexpr std::size_t s_StartFixedUpdateRateIndex{ 1 };
		constexpr std::chrono::seconds s_StallDuration{ 1 };
		constexpr float s_TrackLeft{ -200.0f };
		constexpr float s_TrackLength{ 400.0f };
		constexpr float s_Speed{ 160.0f };
		constexpr float s_SpriteScale{ 1.5f };
		constexpr float s_LabelOffsetY{ 18.0f };
		constexpr std::array<float, TimestepDemoState::s_RowCount> s_RowY{ 80.0f, 30.0f, -20.0f, -70.0f };
		constexpr std::array<std::string_view, TimestepDemoState::s_RowCount> s_RowLabels{
			"Frame time (unclamped)",
			"dt (clamped)",
			"Fixed step",
			"Fixed step + interpolation",
		};
		const BinaryEngine::TextureRegion s_CharacterRegion{ { 0, 0 }, { 16, 16 } };

		float AdvanceDistance(float distance, float travel)
		{
			return std::fmod(distance + travel, 2.0f * s_TrackLength);
		}

		float GetTrackPosition(float distance)
		{
			return s_TrackLeft + s_TrackLength - std::abs(s_TrackLength - distance);
		}

	}

	TimestepDemoState::TimestepDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[TimestepDemoState] Created");
	}

	TimestepDemoState::~TimestepDemoState()
	{
		APP_TRACE("[TimestepDemoState] Destroyed");
	}

	void TimestepDemoState::OnAttach()
	{
		APP_INFO("[TimestepDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);

		m_SavedFixedUpdateRate = m_Context.clock.GetFixedUpdateRate();
		SelectFixedUpdateRate(s_StartFixedUpdateRateIndex);

		const BinaryEngine::AssetHandle characterTextureHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Texture2D>(s_CharacterTexturePath, m_Context.renderer) };
		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::TextSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("TimestepHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, 10.0f };

		CreateRows(characterTextureHandle, fontHandle);
		LineUp();
	}

	void TimestepDemoState::OnDetach()
	{
		m_Context.clock.SetFixedUpdateRate(m_SavedFixedUpdateRate);
		APP_INFO("[TimestepDemoState] Detached");
	}

	void TimestepDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void TimestepDemoState::OnFixedUpdate(BinaryEngine::TimeStep fixedStep)
	{
		m_PreviousFixedStepPosition = m_CurrentFixedStepPosition;
		m_FixedStepDistance = AdvanceDistance(m_FixedStepDistance, s_Speed * fixedStep.GetSeconds());
		m_CurrentFixedStepPosition = GetTrackPosition(m_FixedStepDistance);
	}

	void TimestepDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		const BinaryEngine::FrameClock& frameClock{ m_Context.clock };

		m_FrameTimeDistance = AdvanceDistance(m_FrameTimeDistance, s_Speed * frameClock.GetFrameTime().GetSeconds());
		m_DeltaTimeDistance = AdvanceDistance(m_DeltaTimeDistance, s_Speed * dt.GetSeconds());

		if (frameClock.GetFrameTime().GetNanoseconds() > frameClock.GetDeltaTime().GetNanoseconds())
		{
			m_HasClampedFrame = true;
			m_LastClampedFrameTimeMilliseconds = frameClock.GetFrameTime().GetMilliseconds();
			m_LastClampedDeltaTimeMilliseconds = frameClock.GetDeltaTime().GetMilliseconds();
		}

		UpdateRowPositions();
		UpdateHud();
	}

	void TimestepDemoState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);
		m_Scene.OnRender(m_Context.renderer, m_Context.assetManager);
		m_Context.renderer.EndScene();
	}

	bool TimestepDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::UpArrow:
			{
				if (!event.IsRepeat() && m_FixedUpdateRateIndex + 1 < s_FixedUpdateRates.size())
				{
					SelectFixedUpdateRate(m_FixedUpdateRateIndex + 1);
				}
				return true;
			}

			case BinaryEngine::Key::DownArrow:
			{
				if (!event.IsRepeat() && m_FixedUpdateRateIndex > 0)
				{
					SelectFixedUpdateRate(m_FixedUpdateRateIndex - 1);
				}
				return true;
			}

			case BinaryEngine::Key::Space:
			{
				if (!event.IsRepeat())
				{
					APP_INFO("[TimestepDemoState] Stalling the game thread for {} s", s_StallDuration.count());
					std::this_thread::sleep_for(s_StallDuration);
				}
				return true;
			}

			case BinaryEngine::Key::R:
			{
				if (!event.IsRepeat())
				{
					LineUp();
				}
				return true;
			}

			default: return false;
		}
	}

	void TimestepDemoState::SelectFixedUpdateRate(std::size_t rateIndex)
	{
		m_FixedUpdateRateIndex = rateIndex;
		m_Context.clock.SetFixedUpdateRate(s_FixedUpdateRates[rateIndex]);
	}

	void TimestepDemoState::LineUp()
	{
		m_FrameTimeDistance = 0.0f;
		m_DeltaTimeDistance = 0.0f;
		m_FixedStepDistance = 0.0f;
		m_PreviousFixedStepPosition = s_TrackLeft;
		m_CurrentFixedStepPosition = s_TrackLeft;
	}

	void TimestepDemoState::CreateRows(BinaryEngine::AssetHandle characterTextureHandle, BinaryEngine::AssetHandle fontHandle)
	{
		const BinaryEngine::TextSpecification labelSpecification{
			.Size{ s_LabelTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::TextSpace::World },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};

		for (std::size_t rowIndex{ 0 }; rowIndex < s_RowCount; rowIndex++)
		{
			BinaryEngine::Entity& rowEntity{ m_RowEntities[rowIndex] };
			rowEntity = m_Scene.CreateEntity("TimestepRow");
			rowEntity.AddComponent<BinaryEngine::SpriteComponent>(characterTextureHandle, s_CharacterRegion);
			BinaryEngine::Transform& rowTransform{ rowEntity.GetComponent<BinaryEngine::TransformComponent>().transform };
			rowTransform.Position = { s_TrackLeft, s_RowY[rowIndex], 0.0f };
			rowTransform.Scale = { s_SpriteScale, s_SpriteScale, 1.0f };

			BinaryEngine::Entity labelEntity{ m_Scene.CreateEntity("TimestepRowLabel") };
			labelEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{ s_RowLabels[rowIndex] }, labelSpecification);
			labelEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_TrackLeft, s_RowY[rowIndex] + s_LabelOffsetY, 1.0f };
		}
	}

	void TimestepDemoState::UpdateRowPositions()
	{
		const float alpha{ m_Context.clock.GetInterpolationAlpha() };
		const std::array<float, s_RowCount> rowPositions{
			GetTrackPosition(m_FrameTimeDistance),
			GetTrackPosition(m_DeltaTimeDistance),
			m_CurrentFixedStepPosition,
			std::lerp(m_PreviousFixedStepPosition, m_CurrentFixedStepPosition, alpha),
		};

		for (std::size_t rowIndex{ 0 }; rowIndex < s_RowCount; rowIndex++)
		{
			m_RowEntities[rowIndex].GetComponent<BinaryEngine::TransformComponent>().transform.Position.x = rowPositions[rowIndex];
		}
	}

	void TimestepDemoState::UpdateHud()
	{
		const BinaryEngine::FrameClock& frameClock{ m_Context.clock };

		std::string text{ std::format("Fixed update: {} Hz (Up/Down)   Steps this frame: {}   Alpha: {:.2f}\n", frameClock.GetFixedUpdateRate(), frameClock.GetFixedStepCount(), frameClock.GetInterpolationAlpha()) };
		text += std::format("Space: stall {} s   R: line up\n", s_StallDuration.count());
		if (m_HasClampedFrame)
		{
			text += std::format("Last clamped frame: {:.0f} ms -> {:.0f} ms", m_LastClampedFrameTimeMilliseconds, m_LastClampedDeltaTimeMilliseconds);
		}
		else
		{
			text += "Last clamped frame: none";
		}

		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
