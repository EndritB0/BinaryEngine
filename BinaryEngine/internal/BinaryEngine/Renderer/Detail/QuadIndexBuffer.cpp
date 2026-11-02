#include "pch.h"
#include "BinaryEngine/Renderer/Detail/QuadIndexBuffer.h"

namespace {

	std::uint32_t CalculateIndexBytes(std::uint32_t quadCapacity)
	{
		return quadCapacity * BinaryEngine::Detail::s_IndicesPerQuad * static_cast<std::uint32_t>(sizeof(std::uint32_t));
	}

	bool WriteQuadIndices(SDL_GPUDevice* device, SDL_GPUTransferBuffer* transferBuffer, std::uint32_t quadCapacity)
	{
		void* mappedIndices{ SDL_MapGPUTransferBuffer(device, transferBuffer, false) };
		if (!mappedIndices)
		{
			return false;
		}

		std::uint32_t* indices{ static_cast<std::uint32_t*>(mappedIndices) };
		for (std::uint32_t quadIndex{ 0 }; quadIndex < quadCapacity; quadIndex++)
		{
			const std::uint32_t firstVertex{ quadIndex * BinaryEngine::Detail::s_VerticesPerQuad };
			const std::uint32_t firstIndex{ quadIndex * BinaryEngine::Detail::s_IndicesPerQuad };
			indices[firstIndex + 0] = firstVertex + 0;
			indices[firstIndex + 1] = firstVertex + 1;
			indices[firstIndex + 2] = firstVertex + 2;
			indices[firstIndex + 3] = firstVertex + 2;
			indices[firstIndex + 4] = firstVertex + 3;
			indices[firstIndex + 5] = firstVertex + 0;
		}

		SDL_UnmapGPUTransferBuffer(device, transferBuffer);
		return true;
	}

}

namespace BinaryEngine::Detail {

	bool QuadIndexBuffer::Initialise(::SDL_GPUDevice* device, std::uint32_t initialQuadCapacity, std::uint32_t maxQuadCapacity)
	{
		m_MaxQuadCapacity = maxQuadCapacity;
		return CreateBuffers(device, initialQuadCapacity);
	}

	void QuadIndexBuffer::Release(::SDL_GPUDevice* device)
	{
		if (m_PendingTransferBuffer)
		{
			SDL_ReleaseGPUTransferBuffer(device, m_PendingTransferBuffer);
			m_PendingTransferBuffer = nullptr;
		}

		if (m_Buffer)
		{
			SDL_ReleaseGPUBuffer(device, m_Buffer);
			m_Buffer = nullptr;
		}
	}

	void QuadIndexBuffer::ResetFrame()
	{
		if (!m_FrameOverflowed)
		{
			m_GrowthFailed = false;
		}
		m_FrameOverflowed = false;
	}

	void QuadIndexBuffer::EnsureCapacity(::SDL_GPUDevice* device, std::uint32_t neededQuadCount)
	{
		if (neededQuadCount <= m_QuadCapacity)
		{
			return;
		}

		m_FrameOverflowed = true;

		if (m_GrowthFailed)
		{
			return;
		}

		const std::uint32_t grownCapacity{ std::min(std::max(neededQuadCount, m_QuadCapacity * 2), m_MaxQuadCapacity) };
		if (!CreateBuffers(device, grownCapacity))
		{
			m_GrowthFailed = true;
		}
	}

	void QuadIndexBuffer::UploadPending(::SDL_GPUDevice* device, ::SDL_GPUCopyPass* copyPass)
	{
		if (!m_PendingTransferBuffer)
		{
			return;
		}

		SDL_GPUTransferBufferLocation source{
			.transfer_buffer{ m_PendingTransferBuffer },
			.offset{ 0 },
		};

		SDL_GPUBufferRegion region{
			.buffer{ m_Buffer },
			.offset{ 0 },
			.size{ CalculateIndexBytes(m_QuadCapacity) },
		};

		SDL_UploadToGPUBuffer(copyPass, &source, &region, false);
		SDL_ReleaseGPUTransferBuffer(device, m_PendingTransferBuffer);
		m_PendingTransferBuffer = nullptr;
	}

	void QuadIndexBuffer::Bind(::SDL_GPURenderPass* renderPass) const
	{
		SDL_GPUBufferBinding binding{
			.buffer{ m_Buffer },
			.offset{ 0 },
		};
		SDL_BindGPUIndexBuffer(renderPass, &binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
	}

	bool QuadIndexBuffer::CreateBuffers(::SDL_GPUDevice* device, std::uint32_t newCapacity)
	{
		const std::uint32_t indexBytes{ CalculateIndexBytes(newCapacity) };

		SDL_GPUBufferCreateInfo bufferInfo{
			.usage{ SDL_GPU_BUFFERUSAGE_INDEX },
			.size{ indexBytes },
		};
		SDL_GPUBuffer* buffer{ SDL_CreateGPUBuffer(device, &bufferInfo) };

		SDL_GPUTransferBufferCreateInfo transferInfo{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ indexBytes },
		};
		SDL_GPUTransferBuffer* transferBuffer{ SDL_CreateGPUTransferBuffer(device, &transferInfo) };

		if (!buffer || !transferBuffer || !WriteQuadIndices(device, transferBuffer, newCapacity))
		{
			CORE_ERROR("[Renderer] Failed to create the quad index buffer for {} quads: {}", newCapacity, SDL_GetError());

			if (transferBuffer)
			{
				SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			}

			if (buffer)
			{
				SDL_ReleaseGPUBuffer(device, buffer);
			}

			return false;
		}

		Release(device);

		m_Buffer = buffer;
		m_PendingTransferBuffer = transferBuffer;
		m_QuadCapacity = newCapacity;
		return true;
	}

}
