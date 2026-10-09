#pragma once

#include "BinaryEngine/State/State.h"
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Renderer/AnimationClip.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>

namespace Sandbox {

	class MainMenuState : public BinaryEngine::State {
	public:
		MainMenuState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~MainMenuState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnMouseButtonPressed(BinaryEngine::MouseButtonPressedEvent& event);
		bool OnMouseScrolled(BinaryEngine::MouseScrolledEvent& event);
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::AssetHandle m_CharacterTextureHandle;
		BinaryEngine::AssetHandle m_FontHandle;
		BinaryEngine::Scene m_ActiveScene;
		BinaryEngine::Entity m_PlayerEntity;
		BinaryEngine::Entity m_NameTagEntity;

		BinaryEngine::AnimationClip m_IdleClip;
		BinaryEngine::AnimationClip m_MovingClip;
		BinaryEngine::AnimationClip m_WaveClip;
	};

}
