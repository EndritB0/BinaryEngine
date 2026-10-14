#pragma once

#include <BinaryEngine/Asset/AssetTypes.h>
#include <BinaryEngine/Audio/AudioTypes.h>
#include <BinaryEngine/Event/EventTypes.h>
#include <BinaryEngine/Renderer/OrthographicCamera.h>
#include <BinaryEngine/Scene/Entity.h>
#include <BinaryEngine/Scene/Scene.h>
#include <BinaryEngine/State/State.h>
#include <BinaryEngine/Window/Window.h>

namespace Sandbox {

	class AudioDemoState : public BinaryEngine::State {
	public:
		AudioDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context);
		virtual ~AudioDemoState() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(BinaryEngine::Event& event) override;
		virtual void OnUpdate(BinaryEngine::TimeStep dt) override;
		virtual void OnRender() override;

	private:
		bool OnKeyPressed(BinaryEngine::KeyPressedEvent& event);
		void PlayOneShot(BinaryEngine::AssetHandle clip);
		void ToggleAmbience();
		void ChangeBusVolume(BinaryEngine::AudioBusHandle bus, float change);
		void UpdateHud();

	private:
		BinaryEngine::OrthographicCamera m_Camera{ m_Context.window.GetResolution(), m_Context.camera };
		BinaryEngine::Scene m_Scene;
		BinaryEngine::Entity m_HudEntity;

		BinaryEngine::AssetHandle m_FirstOneShotHandle;
		BinaryEngine::AssetHandle m_SecondOneShotHandle;
		BinaryEngine::AssetHandle m_AmbienceHandle;

		BinaryEngine::AudioBusHandle m_SfxBus;
		BinaryEngine::AudioBusHandle m_MusicBus;
		BinaryEngine::AudioBusHandle m_AmbienceBus;

		BinaryEngine::VoiceHandle m_AmbienceVoice;
	};

}
