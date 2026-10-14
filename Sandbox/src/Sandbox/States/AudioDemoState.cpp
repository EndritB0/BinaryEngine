#include "pch.h"
#include "Sandbox/States/AudioDemoState.h"

#include <BinaryEngine/Audio/AudioClip.h>
#include <BinaryEngine/Audio/AudioEngine.h>
#include <BinaryEngine/Audio/StreamingAudioClip.h>
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
		constexpr std::string_view s_AmbiencePath{ "./resources/audio/ambience_loop.wav" };
		constexpr std::string_view s_MusicPath{ "./resources/audio/music_loop.ogg" };
		constexpr std::string_view s_KeyHelp{ "Z / X  one-shot     Hold C  flood the voices\nA  ambience loop (0.5 s fade-in)     M  music     S  music, 2 s fade-out\nUp / Down  Master volume     [ / ]  SFX volume     - / =  Music volume     , / .  Ambience volume" };
		constexpr float s_MusicSlowFadeSeconds{ 2.0f };
		constexpr float s_AmbienceFadeInSeconds{ 0.5f };
		constexpr float s_VolumeStep{ 0.1f };
		constexpr BinaryEngine::Color s_ClearColor{ 0x1E, 0x14, 0x2D };
		constexpr float s_HudTextSize{ 12.0f };
		constexpr int s_FloodPlaysPerFrame{ 4 };
		BinaryEngine::VoiceHandle s_MusicVoice;
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
		m_AmbienceHandle = m_Context.assetManager.LoadAsset<BinaryEngine::AudioClip>(s_AmbiencePath);
		m_MusicHandle = m_Context.assetManager.LoadAsset<BinaryEngine::StreamingAudioClip>(s_MusicPath);

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
				PlayOneShot(m_SecondOneShotHandle);
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

			case BinaryEngine::Key::A:
			{
				if (!event.IsRepeat())
				{
					ToggleAmbience();
				}
				return true;
			}

			case BinaryEngine::Key::UpArrow:
			{
				ChangeBusVolume(BinaryEngine::AudioBusHandle::Master(), s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::DownArrow:
			{
				ChangeBusVolume(BinaryEngine::AudioBusHandle::Master(), -s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::RightBracket:
			{
				ChangeBusVolume(m_SfxBus, s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::LeftBracket:
			{
				ChangeBusVolume(m_SfxBus, -s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::Equal:
			{
				ChangeBusVolume(m_MusicBus, s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::Minus:
			{
				ChangeBusVolume(m_MusicBus, -s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::Period:
			{
				ChangeBusVolume(m_AmbienceBus, s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::Comma:
			{
				ChangeBusVolume(m_AmbienceBus, -s_VolumeStep);
				return true;
			}

			case BinaryEngine::Key::M:
			{
				if (!event.IsRepeat())
				{
					ToggleMusic();
				}
				return true;
			}

			case BinaryEngine::Key::S:
			{
				if (!event.IsRepeat())
				{
					StopMusicSlowly();
				}
				return true;
			}

			default: return false;
		}
	}

	void AudioDemoState::PlayOneShot(BinaryEngine::AssetHandle clip)
	{
		m_Audio.Play(clip, { .Bus{ m_SfxBus } });
	}

	void AudioDemoState::ToggleAmbience()
	{
		BinaryEngine::AudioEngine& audio{ m_Context.audio };
		if (audio.IsPlaying(m_AmbienceVoice))
		{
			audio.Stop(m_AmbienceVoice);
			return;
		}

		m_AmbienceVoice = m_Audio.Play(m_AmbienceHandle, { .Bus{ m_AmbienceBus }, .Loop{ true }, .FadeInSeconds{ s_AmbienceFadeInSeconds } });
	}

	void AudioDemoState::ChangeBusVolume(BinaryEngine::AudioBusHandle bus, float change)
	{
		BinaryEngine::AudioEngine& audio{ m_Context.audio };
		audio.SetBusVolume(bus, audio.GetBusVolume(bus) + change);
	}

	void AudioDemoState::ToggleMusic()
	{
		BinaryEngine::AudioEngine& audio{ m_Context.audio };
		if (audio.IsPlaying(s_MusicVoice))
		{
			audio.Stop(s_MusicVoice);
			return;
		}

		s_MusicVoice = audio.GetGlobalScope().Play(m_MusicHandle, { .Bus{ m_MusicBus }, .Loop{ true } });
	}

	void AudioDemoState::StopMusicSlowly()
	{
		m_Context.audio.Stop(s_MusicVoice, s_MusicSlowFadeSeconds);
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
