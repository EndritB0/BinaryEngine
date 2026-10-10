#include "pch.h"
#include "Sandbox/States/SandboxShellState.h"

#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Scene/Components.h>
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

		constexpr const char* s_OverlayFontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr float s_OverlayTextSize{ 12.0f };
		constexpr float s_OverlayMargin{ 8.0f };
		constexpr float s_TimingRefreshSeconds{ 0.5f };
	}

	SandboxShellState::SandboxShellState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context, const SandboxOptions& options) :
		State(stateManager, context), m_Options(options)
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
		CreateOverlay();

		if (m_Options.Frames.has_value())
		{
			const std::size_t demoCount{ m_Options.AllDemos ? GetDemos().size() : 1 };
			APP_INFO("[SandboxShellState] Smoke run: {} frames per demo, {} demo(s)", *m_Options.Frames, demoCount);
		}
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

	void SandboxShellState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		UpdateFrameTiming(dt);

		if (m_IsQuitting)
		{
			return;
		}

		if (!m_ActiveDemoIndex.has_value())
		{
			SwitchToDemo(GetStartDemoIndex());
		}
		else if (m_Options.Frames.has_value())
		{
			++m_FramesOnActiveDemo;
			if (m_FramesOnActiveDemo >= *m_Options.Frames)
			{
				AdvanceSmokeRun();
			}
		}

		UpdateOverlayStatistics();
	}

	void SandboxShellState::OnRender()
	{
		if (!m_OverlayStatisticsEntity)
		{
			return;
		}

		m_Context.renderer.BeginScene(m_Camera);

		const BinaryEngine::Vector2f screenSize{ m_Camera.GetScreenViewSize() };
		const float lastStatisticsBaselineY{ screenSize.y - s_OverlayMargin - m_OverlayDescent };
		const float firstStatisticsBaselineY{ lastStatisticsBaselineY - static_cast<float>(m_OverlayStatisticsLineCount - 1) * m_OverlayLineAdvance };
		const float headerBaselineY{ firstStatisticsBaselineY - m_OverlayLineAdvance };
		m_OverlayStatisticsEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_OverlayMargin, firstStatisticsBaselineY, 0.0f };
		m_OverlayHeaderEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { s_OverlayMargin, headerBaselineY, 0.0f };

		m_OverlayScene.OnRender(m_Context.renderer, m_Context.assetManager);
		m_Context.renderer.EndScene();
	}

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
		m_FramesOnActiveDemo = 0;
		UpdateOverlayHeader();
	}

	void SandboxShellState::AdvanceSmokeRun()
	{
		const std::size_t nextDemoIndex{ *m_ActiveDemoIndex + 1 };
		if (m_Options.AllDemos && nextDemoIndex < GetDemos().size())
		{
			SwitchToDemo(nextDemoIndex);
			return;
		}

		APP_INFO("[SandboxShellState] Smoke run finished");
		Quit();
	}

	void SandboxShellState::Quit()
	{
		m_IsQuitting = true;
		m_StateManager.RequestClearStates();
	}

	std::size_t SandboxShellState::GetStartDemoIndex() const
	{
		const DemoEntry* startDemo{ FindDemo(m_Options.StartDemo) };
		return startDemo != nullptr ? static_cast<std::size_t>(startDemo - GetDemos().data()) : 0;
	}

	void SandboxShellState::CreateOverlay()
	{
		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_OverlayFontPath, m_Context.renderer) };
		const std::shared_ptr<BinaryEngine::Font> font{ m_Context.assetManager.GetAsset<BinaryEngine::Font>(fontHandle) };
		if (!font || !font->IsValid())
		{
			APP_ERROR("[SandboxShellState] Overlay font failed to load, so the overlay is disabled");
			return;
		}

		const BinaryEngine::TextSpecification overlaySpecification{
			.Size{ s_OverlayTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::TextSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};

		const float textScale{ s_OverlayTextSize / font->GetPixelSize() };
		m_OverlayLineAdvance = font->GetLineHeight() * overlaySpecification.LineSpacing * textScale;
		m_OverlayDescent = -font->GetDescent() * textScale;

		m_OverlayHeaderEntity = m_OverlayScene.CreateEntity("OverlayHeader");
		m_OverlayHeaderEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, overlaySpecification);

		m_OverlayStatisticsEntity = m_OverlayScene.CreateEntity("OverlayStatistics");
		m_OverlayStatisticsEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, overlaySpecification);
	}

	void SandboxShellState::UpdateFrameTiming(BinaryEngine::TimeStep dt)
	{
		m_TimingAccumulatedSeconds += dt.GetSeconds();
		++m_TimingAccumulatedFrames;

		if (m_TimingAccumulatedSeconds >= s_TimingRefreshSeconds)
		{
			const float frameCount{ static_cast<float>(m_TimingAccumulatedFrames) };
			m_DisplayedFramesPerSecond = frameCount / m_TimingAccumulatedSeconds;
			m_DisplayedFrameMilliseconds = 1000.0f * m_TimingAccumulatedSeconds / frameCount;
			m_TimingAccumulatedSeconds = 0.0f;
			m_TimingAccumulatedFrames = 0;
		}
	}

	void SandboxShellState::UpdateOverlayHeader()
	{
		if (!m_OverlayHeaderEntity || !m_ActiveDemoIndex.has_value())
		{
			return;
		}

		const std::span<const DemoEntry> demos{ GetDemos() };
		std::string text{ std::format("{} (F{})   ", demos[*m_ActiveDemoIndex].Name, *m_ActiveDemoIndex + 1) };
		for (std::size_t demoIndex{ 0 }; demoIndex < demos.size(); ++demoIndex)
		{
			text += std::format("F{} {}   ", demoIndex + 1, demos[demoIndex].Name);
		}
		text += "Esc quit";

		m_OverlayHeaderEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

	void SandboxShellState::UpdateOverlayStatistics()
	{
		if (!m_OverlayStatisticsEntity)
		{
			return;
		}

		std::string text{ std::format("FPS {:.0f}  {:.1f} ms\n", m_DisplayedFramesPerSecond, m_DisplayedFrameMilliseconds) };

		const BinaryEngine::Renderer::Statistics& statistics{ m_Context.renderer.GetStatistics() };
		text += std::format("Quads {}  Draw calls {}  Glyphs {}  Culled {}", statistics.quadCount, statistics.drawCalls, statistics.glyphCount, statistics.culledSprites);

		if (m_Options.Frames.has_value())
		{
			text += std::format("\nSmoke run: frame {} / {}", m_FramesOnActiveDemo, *m_Options.Frames);
		}

		m_OverlayStatisticsLineCount = static_cast<std::size_t>(std::ranges::count(text, '\n')) + 1;
		m_OverlayStatisticsEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}
}
