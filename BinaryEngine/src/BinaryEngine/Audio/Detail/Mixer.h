#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include "BinaryEngine/Audio/Detail/AudioCommand.h"
#include "BinaryEngine/Audio/Detail/SpscQueue.h"

namespace BinaryEngine::Detail {

	class VoiceSource;

	struct MixerBusSetup {
		std::uint32_t ParentIndex{ 0 };
		float Volume{ 1.0f };
	};

	class Mixer {
	public:
		static constexpr std::uint32_t s_OutputChannelCount{ 2 };
		static constexpr std::uint32_t s_MaxBlockFrames{ 512 };
		static constexpr std::uint32_t s_MaxSourceRateRatio{ 4 };
		static constexpr std::uint32_t s_SourceScratchFrames{ (s_MaxBlockFrames + 2) * s_MaxSourceRateRatio };

		Mixer(std::uint32_t sampleRate, std::uint32_t maxVoices, const std::vector<MixerBusSetup>& buses);

		Mixer(const Mixer&) = delete;
		Mixer& operator=(const Mixer&) = delete;
		Mixer(Mixer&&) = delete;
		Mixer& operator=(Mixer&&) = delete;

		bool PushCommand(const AudioCommand& command) { return m_Commands.TryPush(command); }
		bool PopFinishedVoice(VoiceFinishedEvent& finishedVoice) { return m_FinishedVoices.TryPop(finishedVoice); }
		std::uint32_t GetDecodeErrorCount() const { return m_DecodeErrorCount.load(std::memory_order_relaxed); }
		std::uint64_t GetMixedFrameCount() const { return m_MixedFrameCount.load(std::memory_order_relaxed); }

		const float* MixBlock(std::uint32_t frameCount);

	private:
		struct GainRamp {
			float Current{ 1.0f };
			float Target{ 1.0f };
			float Step{ 0.0f };
			std::uint32_t FramesRemaining{ 0 };

			void Reset(float value);
			void RampTo(float target, std::uint32_t frameCount);
			float Advance();
		};

		struct VoiceState {
			VoiceSource* Source{ nullptr };
			std::uint32_t Generation{ 0 };
			std::uint32_t BusIndex{ 0 };
			std::uint32_t ChannelCount{ 1 };
			std::uint64_t Phase{ 0 };
			std::uint64_t PhaseStep{ 0 };
			float CurrentFrame[s_OutputChannelCount]{};
			float NextFrame[s_OutputChannelCount]{};
			GainRamp Volume;
			GainRamp Envelope;
			bool IsActive{ false };
			bool IsStopping{ false };
		};

		struct BusState {
			std::uint32_t ParentIndex{ 0 };
			GainRamp Volume;
			std::vector<float> Samples;
		};

		void ProcessCommands();
		void StartVoice(const AudioCommand& command);
		void StopVoice(const AudioCommand& command);
		VoiceState* FindVoice(std::uint32_t slot, std::uint32_t generation);
		void MixVoice(std::uint32_t slot, std::uint32_t frameCount);
		void FinishVoice(std::uint32_t slot);
		void FoldBuses(std::uint32_t frameCount);

	private:
		SpscQueue<AudioCommand> m_Commands;
		SpscQueue<VoiceFinishedEvent> m_FinishedVoices;
		std::vector<VoiceState> m_Voices;
		std::vector<BusState> m_Buses;
		std::vector<float> m_SourceScratch;
		std::uint32_t m_SampleRate{ 0 };
		std::uint32_t m_VolumeRampFrames{ 0 };
		std::atomic<std::uint32_t> m_DecodeErrorCount{ 0 };
		std::atomic<std::uint64_t> m_MixedFrameCount{ 0 };
	};

}
