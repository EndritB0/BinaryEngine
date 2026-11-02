#pragma once

#include <cstdint>

struct SDL_GPUBuffer;
struct SDL_GPUCopyPass;
struct SDL_GPUDevice;
struct SDL_GPURenderPass;
struct SDL_GPUTransferBuffer;

namespace BinaryEngine::Detail {

	inline constexpr std::uint32_t s_VerticesPerQuad{ 4 };
	inline constexpr std::uint32_t s_IndicesPerQuad{ 6 };

	class QuadIndexBuffer {
	public:
		QuadIndexBuffer() = default;

		QuadIndexBuffer(const QuadIndexBuffer&) = delete;
		QuadIndexBuffer& operator=(const QuadIndexBuffer&) = delete;
		QuadIndexBuffer(QuadIndexBuffer&&) = delete;
		QuadIndexBuffer& operator=(QuadIndexBuffer&&) = delete;

		bool Initialise(::SDL_GPUDevice* device, std::uint32_t initialQuadCapacity, std::uint32_t maxQuadCapacity);
		void Release(::SDL_GPUDevice* device);
		void ResetFrame();
		void EnsureCapacity(::SDL_GPUDevice* device, std::uint32_t neededQuadCount);
		void UploadPending(::SDL_GPUDevice* device, ::SDL_GPUCopyPass* copyPass);
		void Bind(::SDL_GPURenderPass* renderPass) const;

		bool IsValid() const { return m_Buffer != nullptr; }
		std::uint32_t GetCapacity() const { return m_QuadCapacity; }

	private:
		bool CreateBuffers(::SDL_GPUDevice* device, std::uint32_t newCapacity);

	private:
		::SDL_GPUBuffer* m_Buffer{ nullptr };
		::SDL_GPUTransferBuffer* m_PendingTransferBuffer{ nullptr };
		std::uint32_t m_QuadCapacity{ 0 };
		std::uint32_t m_MaxQuadCapacity{ 0 };
		bool m_GrowthFailed{ false };
		bool m_FrameOverflowed{ false };
	};

}
