#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

namespace Sandbox {

	class TimestepDemoState : public BinaryEngine::State {
	public:
		TimestepDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~TimestepDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnFixedUpdate(BinaryEngine::TimeStep fixedStep) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	public:
		static constexpr std::size_t s_RowCount{ 4 };

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void SelectFixedUpdateRate(std::size_t rateIndex);
		void LineUp();
		void CreateRows(BinaryEngine::AssetHandle characterTextureHandle, BinaryEngine::AssetHandle fontHandle);
		void UpdateRowPositions();
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;
		std::array<BinaryEngine::Entity, s_RowCount> m_RowEntities;

		float m_FrameTimeDistance{ 0.0f };
		float m_DeltaTimeDistance{ 0.0f };
		float m_FixedStepDistance{ 0.0f };
		float m_PreviousFixedStepPosition{ 0.0f };
		float m_CurrentFixedStepPosition{ 0.0f };

		std::size_t m_FixedUpdateRateIndex{ 0 };
		std::uint16_t m_SavedFixedUpdateRate{ 0 };

		bool m_HasClampedFrame{ false };
		float m_LastClampedFrameTimeMilliseconds{ 0.0f };
		float m_LastClampedDeltaTimeMilliseconds{ 0.0f };
	};

}
