#include "pch.h"
#include "BinaryEngine/Audio/Detail/Mixer.h"

#include "BinaryEngine/Audio/Detail/VoiceSource.h"

namespace {

	constexpr std::uint32_t s_CommandQueueCapacity{ 1024 };
	constexpr float s_VolumeRampSeconds{ 0.01f };
	constexpr std::uint32_t s_PhaseFractionBits{ 32 };
	constexpr std::uint64_t s_PhaseOne{ std::uint64_t{ 1 } << s_PhaseFractionBits };
	constexpr float s_PhaseToFraction{ 1.0f / static_cast<float>(s_PhaseOne) };

}

namespace BinaryEngine::Detail {

	Mixer::Mixer(std::uint32_t sampleRate, std::uint32_t maxVoices, const std::vector<MixerBusSetup>& buses) :
		m_Commands(s_CommandQueueCapacity),
		m_FinishedVoices(maxVoices),
		m_Voices(maxVoices),
		m_SampleRate(sampleRate),
		m_VolumeRampFrames(static_cast<std::uint32_t>(static_cast<float>(sampleRate) * s_VolumeRampSeconds))
	{
		m_Buses.reserve(buses.size());
		for (const MixerBusSetup& busSetup : buses)
		{
			BusState& bus{ m_Buses.emplace_back() };
			bus.ParentIndex = busSetup.ParentIndex;
			bus.Volume.Reset(busSetup.Volume);
		}
	}

	const float* Mixer::MixBlock(std::uint32_t frameCount)
	{
		const std::uint32_t blockFrames{ std::min(frameCount, s_MaxBlockFrames) };
		if (blockFrames == 0)
		{
			return m_Buses[0].Samples.data();
		}

		ProcessCommands();

		for (BusState& bus : m_Buses)
		{
			std::fill_n(bus.Samples.data(), static_cast<std::size_t>(blockFrames) * s_OutputChannelCount, 0.0f);
		}

		const std::uint32_t voiceCount{ static_cast<std::uint32_t>(m_Voices.size()) };
		for (std::uint32_t slot{ 0 }; slot < voiceCount; slot++)
		{
			if (m_Voices[slot].IsActive)
			{
				MixVoice(slot, blockFrames);
			}
		}

		FoldBuses(blockFrames);
		m_MixedFrameCount.fetch_add(blockFrames, std::memory_order_relaxed);
		return m_Buses[0].Samples.data();
	}

	void Mixer::ProcessCommands()
	{
		AudioCommand command{};
		while (m_Commands.TryPop(command))
		{
			switch (command.Type)
			{
				case AudioCommandType::Play:
				{
					StartVoice(command);
					break;
				}

				case AudioCommandType::Stop:
				{
					StopVoice(command);
					break;
				}

				case AudioCommandType::SetVoiceVolume:
				{
					VoiceState* voice{ FindVoice(command.Slot, command.Generation) };
					if (voice != nullptr)
					{
						voice->Volume.RampTo(command.Volume, m_VolumeRampFrames);
					}
					break;
				}

				case AudioCommandType::SetBusVolume:
				{
					if (command.BusIndex < m_Buses.size())
					{
						m_Buses[command.BusIndex].Volume.RampTo(command.Volume, m_VolumeRampFrames);
					}
					break;
				}
			}
		}
	}

	void Mixer::StartVoice(const AudioCommand& command)
	{
		if (command.Slot >= m_Voices.size() || command.Source == nullptr)
		{
			return;
		}

		VoiceState& voice{ m_Voices[command.Slot] };
		voice = VoiceState{};
		voice.Source = command.Source;
		voice.Generation = command.Generation;
		voice.BusIndex = command.BusIndex < m_Buses.size() ? command.BusIndex : 0;
		voice.ChannelCount = command.Source->GetChannelCount() == 2 ? 2u : 1u;
		voice.PhaseStep = (static_cast<std::uint64_t>(command.Source->GetSampleRate()) << s_PhaseFractionBits) / m_SampleRate;
		voice.Phase = 2 * s_PhaseOne;
		voice.Volume.Reset(command.Volume);
		voice.Envelope.Reset(0.0f);
		voice.Envelope.RampTo(1.0f, command.FadeFrames);
		voice.IsActive = true;
	}

	void Mixer::StopVoice(const AudioCommand& command)
	{
		VoiceState* voice{ FindVoice(command.Slot, command.Generation) };
		if (voice == nullptr)
		{
			return;
		}

		if (command.FadeFrames == 0)
		{
			FinishVoice(command.Slot);
			return;
		}

		voice->IsStopping = true;
		voice->Envelope.RampTo(0.0f, command.FadeFrames);
	}

	Mixer::VoiceState* Mixer::FindVoice(std::uint32_t slot, std::uint32_t generation)
	{
		if (slot >= m_Voices.size())
		{
			return nullptr;
		}

		VoiceState& voice{ m_Voices[slot] };
		return voice.IsActive && voice.Generation == generation ? &voice : nullptr;
	}

	void Mixer::MixVoice(std::uint32_t slot, std::uint32_t frameCount)
	{
		VoiceState& voice{ m_Voices[slot] };
		const std::uint32_t channelCount{ voice.ChannelCount };
		const std::uint32_t sourceFramesNeeded{ static_cast<std::uint32_t>((voice.Phase + (frameCount - 1) * voice.PhaseStep) >> s_PhaseFractionBits) };

		SourceReadResult readResult{};
		if (sourceFramesNeeded > 0)
		{
			readResult = voice.Source->Read(m_SourceScratch.data(), sourceFramesNeeded);
		}

		float* scratch{ m_SourceScratch.data() };
		const std::uint32_t framesRead{ std::min(readResult.FramesRead, sourceFramesNeeded) };
		std::fill(scratch + static_cast<std::size_t>(framesRead) * channelCount, scratch + static_cast<std::size_t>(sourceFramesNeeded) * channelCount, 0.0f);

		float* busSamples{ m_Buses[voice.BusIndex].Samples.data() };
		std::uint32_t scratchFrame{ 0 };

		for (std::uint32_t frame{ 0 }; frame < frameCount; frame++)
		{
			while (voice.Phase >= s_PhaseOne)
			{
				const float* sourceFrame{ scratch + static_cast<std::size_t>(scratchFrame) * channelCount };
				voice.CurrentFrame[0] = voice.NextFrame[0];
				voice.CurrentFrame[1] = voice.NextFrame[1];
				voice.NextFrame[0] = sourceFrame[0];
				voice.NextFrame[1] = channelCount == 2 ? sourceFrame[1] : sourceFrame[0];
				voice.Phase -= s_PhaseOne;
				++scratchFrame;
			}

			const float fraction{ static_cast<float>(voice.Phase) * s_PhaseToFraction };
			const float gain{ voice.Envelope.Advance() * voice.Volume.Advance() };
			for (std::uint32_t channel{ 0 }; channel < s_OutputChannelCount; channel++)
			{
				const float interpolated{ voice.CurrentFrame[channel] + (voice.NextFrame[channel] - voice.CurrentFrame[channel]) * fraction };
				busSamples[frame * s_OutputChannelCount + channel] += interpolated * gain;
			}

			voice.Phase += voice.PhaseStep;
		}

		if (readResult.Status == SourceStatus::Failed)
		{
			m_DecodeErrorCount.fetch_add(1, std::memory_order_relaxed);
		}

		const bool hasFadedOut{ voice.IsStopping && voice.Envelope.FramesRemaining == 0 };
		const bool hasSourceEnded{ readResult.Status == SourceStatus::Finished || readResult.Status == SourceStatus::Failed };
		if (hasFadedOut || hasSourceEnded)
		{
			FinishVoice(slot);
		}
	}

	void Mixer::FinishVoice(std::uint32_t slot)
	{
		VoiceState& voice{ m_Voices[slot] };
		voice.IsActive = false;
		voice.Source = nullptr;
		m_FinishedVoices.TryPush(VoiceFinishedEvent{ .Slot{ slot }, .Generation{ voice.Generation } });
	}

	void Mixer::FoldBuses(std::uint32_t frameCount)
	{
		for (std::size_t busIndex{ m_Buses.size() - 1 }; busIndex > 0; busIndex--)
		{
			BusState& bus{ m_Buses[busIndex] };
			float* parentSamples{ m_Buses[bus.ParentIndex].Samples.data() };
			const float* busSamples{ bus.Samples.data() };

			for (std::uint32_t frame{ 0 }; frame < frameCount; frame++)
			{
				const float gain{ bus.Volume.Advance() };
				for (std::uint32_t channel{ 0 }; channel < s_OutputChannelCount; channel++)
				{
					const std::uint32_t sampleIndex{ frame * s_OutputChannelCount + channel };
					parentSamples[sampleIndex] += busSamples[sampleIndex] * gain;
				}
			}
		}

		BusState& master{ m_Buses[0] };
		float* masterSamples{ master.Samples.data() };
		for (std::uint32_t frame{ 0 }; frame < frameCount; frame++)
		{
			const float gain{ master.Volume.Advance() };
			for (std::uint32_t channel{ 0 }; channel < s_OutputChannelCount; channel++)
			{
				float& sample{ masterSamples[frame * s_OutputChannelCount + channel] };
				sample = std::clamp(sample * gain, -1.0f, 1.0f);
			}
		}
	}

	void Mixer::GainRamp::Reset(float value)
	{
		Current = value;
		Target = value;
		Step = 0.0f;
		FramesRemaining = 0;
	}

	void Mixer::GainRamp::RampTo(float target, std::uint32_t frameCount)
	{
		if (frameCount == 0)
		{
			Reset(target);
			return;
		}

		Target = target;
		Step = (target - Current) / static_cast<float>(frameCount);
		FramesRemaining = frameCount;
	}

	float Mixer::GainRamp::Advance()
	{
		if (FramesRemaining > 0)
		{
			--FramesRemaining;
			Current = FramesRemaining == 0 ? Target : Current + Step;
		}

		return Current;
	}

}
