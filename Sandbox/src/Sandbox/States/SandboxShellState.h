#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

#include "Sandbox/SandboxOptions.h"

namespace Sandbox {

	class SandboxShellState : public BinaryEngine::State {
	public:
		SandboxShellState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context, const SandboxOptions& options);
		virtual ~SandboxShellState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void SwitchToDemo(std::size_t demoIndex);
		void AdvanceSmokeRun();
		void Quit();
		std::size_t GetStartDemoIndex() const;
		void CreateOverlay();
		void UpdateFrameTiming(BinaryEngine::TimeStep dt);
		void UpdateOverlayHeader();
		void UpdateOverlayStatistics();

	private:
		SandboxOptions m_Options;
		std::optional<std::size_t> m_ActiveDemoIndex;
		std::uint32_t m_FramesOnActiveDemo{ 0 };
		bool m_IsQuitting{ false };

		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_OverlayScene;
		BinaryEngine::Entity m_OverlayHeaderEntity;
		BinaryEngine::Entity m_OverlayStatisticsEntity;
		float m_OverlayLineAdvance{ 0.0f };
		float m_OverlayDescent{ 0.0f };
		std::size_t m_OverlayStatisticsLineCount{ 1 };

		float m_TimingAccumulatedSeconds{ 0.0f };
		std::uint32_t m_TimingAccumulatedFrames{ 0 };
		float m_DisplayedFramesPerSecond{ 0.0f };
		float m_DisplayedFrameMilliseconds{ 0.0f };
	};

}
