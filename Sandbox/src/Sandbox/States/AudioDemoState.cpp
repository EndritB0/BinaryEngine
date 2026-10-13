#include "pch.h"
#include "Sandbox/States/AudioDemoState.h"

#include <BinaryEngine/Audio/AudioClip.h>
#include <BinaryEngine/Audio/AudioEngine.h>
#include <BinaryEngine/Input/Input.h>
#include <BinaryEngine/Renderer/Font.h>
#include <BinaryEngine/Renderer/Renderer.h>
#include <BinaryEngine/Renderer/TextSpecification.h>
#include <BinaryEngine/Scene/Components.h>

namespace Sandbox {

	namespace {

		constexpr std::string_view s_FontPath{ "./resources/fonts/OpenSans-Regular.ttf" };
		constexpr std::string_view s_FirstOneShotPath{ "./resources/audio/one_shot_1.wav" };
		constexpr std::string_view s_SecondOneShotPath{ "./resources/audio/one_shot_2.wav" };
		constexpr std::string_view s_KeyHelp{ "Z / X  one-shot     Hold C  flood the voices" };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1E, 0x14, 0x2D };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr int s_FloodPlaysPerFrame{ 4 };

	}

	AudioDemoState::AudioDemoState(BinaryEngine::StateManager& stateManager, const BinaryEngine::Context& context) :
		State(stateManager, context)
	{
		APP_TRACE("[AudioDemoState] Created");
	}

	AudioDemoState::~AudioDemoState()
	{
		APP_TRACE("[AudioDemoState] Destroyed");
	}

	void AudioDemoState::OnAttach()
	{
		APP_INFO("[AudioDemoState] Attached");

		m_Context.renderer.SetClearColor(s_ClearColor);

		m_FirstOneShotHandle = m_Context.assetManager.LoadAsset<BinaryEngine::AudioClip>(s_FirstOneShotPath);
		m_SecondOneShotHandle = m_Context.assetManager.LoadAsset<BinaryEngine::AudioClip>(s_SecondOneShotPath);

		m_SfxBus = m_Context.audio.GetBus("SFX");
		m_MusicBus = m_Context.audio.GetBus("Music");
		m_AmbienceBus = m_Context.audio.GetBus("Ambience");

		const BinaryEngine::AssetHandle fontHandle{ m_Context.assetManager.LoadAsset<BinaryEngine::Font>(s_FontPath, m_Context.renderer) };
		const BinaryEngine::TextSpecification hudSpecification{
			.Size{ s_HudTextSize },
			.FillColor{ BinaryEngine::Color::White },
			.Space{ BinaryEngine::TextSpace::Screen },
			.Alignment{ BinaryEngine::TextAlignment::Left },
		};
		m_HudEntity = m_Scene.CreateEntity("AudioHud");
		m_HudEntity.AddComponent<BinaryEngine::TextComponent>(fontHandle, std::string{}, hudSpecification);
		m_HudEntity.GetComponent<BinaryEngine::TransformComponent>().transform.Position = { 8.0f, 20.0f, 0.0f };
	}

	void AudioDemoState::OnDetach()
	{
		APP_INFO("[AudioDemoState] Detached");
	}

	void AudioDemoState::OnEvent(BinaryEngine::Event& event)
	{
		BinaryEngine::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<BinaryEngine::KeyPressedEvent>(BIND_FUNCTION(OnKeyPressed));
	}

	void AudioDemoState::OnUpdate(BinaryEngine::TimeStep dt)
	{
		if (BinaryEngine::Input::IsKeyPressed(BinaryEngine::Key::C))
		{
			for (int playIndex{ 0 }; playIndex < s_FloodPlaysPerFrame; playIndex++)
			{
				PlayOneShot(m_FirstOneShotHandle);
			}
		}

		UpdateHud();
		m_Scene.OnUpdate(dt);
	}

	void AudioDemoState::OnRender()
	{
		m_Context.renderer.BeginScene(m_Camera);
		m_Scene.OnRender(m_Context.renderer, m_Context.assetManager);
		m_Context.renderer.EndScene();
	}

	bool AudioDemoState::OnKeyPressed(BinaryEngine::KeyPressedEvent& event)
	{
		switch (event.GetKeyCode())
		{
			case BinaryEngine::Key::Z:
			{
				PlayOneShot(m_FirstOneShotHandle);
				return true;
			}

			case BinaryEngine::Key::X:
			{
				PlayOneShot(m_SecondOneShotHandle);
				return true;
			}

			default: return false;
		}
	}

	void AudioDemoState::PlayOneShot(BinaryEngine::AssetHandle clip)
	{
		m_Audio.Play(clip, { .Bus{ m_SfxBus } });
	}

	void AudioDemoState::UpdateHud()
	{
		const BinaryEngine::AudioEngine& audio{ m_Context.audio };
		const BinaryEngine::AudioEngine::Statistics statistics{ audio.GetStatistics() };

		std::string text{ statistics.IsAvailable ? "Audio device: available\n" : "Audio device: unavailable, running silently\n" };
		text += std::format("Voices {} / {}   Dropped plays {}   Decode errors {}\n", statistics.ActiveVoices, statistics.MaxVoices, statistics.DroppedPlays, statistics.DecodeErrors);
		text += std::format("Master {:.1f}   SFX {:.1f}   Music {:.1f}   Ambience {:.1f}\n", audio.GetBusVolume(BinaryEngine::AudioBusHandle::Master()), audio.GetBusVolume(m_SfxBus), audio.GetBusVolume(m_MusicBus), audio.GetBusVolume(m_AmbienceBus));
		text += s_KeyHelp;

		m_HudEntity.GetComponent<BinaryEngine::TextComponent>().Text = std::move(text);
	}

}
