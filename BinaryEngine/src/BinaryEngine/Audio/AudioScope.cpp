#include "pch.h"
#include "BinaryEngine/Audio/AudioScope.h"

#include "BinaryEngine/Audio/AudioEngine.h"

namespace BinaryEngine {

	AudioScope::AudioScope(AudioEngine& audioEngine) :
		m_AudioEngine(audioEngine)
	{
		m_AudioEngine.RegisterScope();
	}

	AudioScope::~AudioScope()
	{
		StopAll();
		m_AudioEngine.DetachScope(*this);
	}

	VoiceHandle AudioScope::Play(AssetHandle clip, const PlaySpecification& specification)
	{
		return m_AudioEngine.PlayInScope(*this, clip, specification);
	}

	void AudioScope::StopAll(std::optional<float> fadeSeconds)
	{
		for (const VoiceHandle voice : m_Voices)
		{
			m_AudioEngine.Stop(voice, fadeSeconds);
		}
	}
}
