#include "pch.h"
#include "BinaryEngine/Renderer/Detail/QuadRenderer.h"

#include "BinaryEngine/Renderer/Detail/QuadIndexBuffer.h"
#include "BinaryEngine/Renderer/Detail/RenderQueue.h"

namespace BinaryEngine::Detail {

	QuadRenderer::QuadRenderer(QuadIndexBuffer& indexBuffer, std::string_view name, std::uint32_t vertexSize) :
		m_IndexBuffer(indexBuffer), m_Name(name), m_VertexSize(vertexSize)
	{}

	bool QuadRenderer::IsValid() const
	{
		return m_RenderState.Pipeline && m_VertexBuffer && m_TransferBuffer;
	}

	void QuadRenderer::Release(::SDL_GPUDevice* device)
	{
		if (m_TransferBuffer)
		{
			SDL_ReleaseGPUTransferBuffer(device, m_TransferBuffer);
			m_TransferBuffer = nullptr;
		}

		if (m_VertexBuffer)
		{
			SDL_ReleaseGPUBuffer(device, m_VertexBuffer);
			m_VertexBuffer = nullptr;
		}

		if (m_RenderState.Pipeline)
		{
			SDL_ReleaseGPUGraphicsPipeline(device, m_RenderState.Pipeline);
			m_RenderState.Pipeline = nullptr;
		}
	}

	void QuadRenderer::PrepareFrame(::SDL_GPUDevice* device, const std::vector<std::uint32_t>& drawOrder)
	{
		m_DrawableQuadCount = 0;

		const std::uint32_t frameQuadCount{ static_cast<std::uint32_t>(drawOrder.size()) };
		GrowVertexBuffersIfNeeded(device, frameQuadCount);

		if (frameQuadCount == 0)
		{
			return;
		}

		m_IndexBuffer.EnsureCapacity(device, frameQuadCount);

		const std::uint32_t drawableQuadCount{ std::min({ frameQuadCount, m_QuadCapacity, m_IndexBuffer.GetCapacity() }) };

		void* mappedVertices{ SDL_MapGPUTransferBuffer(device, m_TransferBuffer, true) };
		if (!mappedVertices)
		{
			CORE_ERROR("[Renderer] Failed to map the {} vertex transfer buffer: {}", m_Name, SDL_GetError());
			return;
		}

		WriteVertices(mappedVertices, drawOrder, drawableQuadCount);
		SDL_UnmapGPUTransferBuffer(device, m_TransferBuffer);

		m_DrawableQuadCount = drawableQuadCount;
	}

	void QuadRenderer::Upload(::SDL_GPUDevice* device, ::SDL_GPUCopyPass* copyPass)
	{
		if (m_DrawableQuadCount == 0)
		{
			return;
		}

		m_IndexBuffer.UploadPending(device, copyPass);

		SDL_GPUTransferBufferLocation source{
			.transfer_buffer{ m_TransferBuffer },
			.offset{ 0 },
		};

		SDL_GPUBufferRegion region{
			.buffer{ m_VertexBuffer },
			.offset{ 0 },
			.size{ m_DrawableQuadCount * s_VerticesPerQuad * m_VertexSize },
		};

		SDL_UploadToGPUBuffer(copyPass, &source, &region, true);
	}

	void QuadRenderer::Bind(::SDL_GPURenderPass* renderPass)
	{
		SDL_GPUBufferBinding vertexBinding{
			.buffer{ m_VertexBuffer },
			.offset{ 0 },
		};
		SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);

		m_IndexBuffer.Bind(renderPass);
	}

	void QuadRenderer::DrawBatch(::SDL_GPURenderPass* renderPass, const RenderBatch& batch)
	{
		if (batch.FirstItem >= m_DrawableQuadCount)
		{
			return;
		}

		const std::uint32_t quadCount{ std::min(batch.ItemCount, m_DrawableQuadCount - batch.FirstItem) };
		SDL_DrawGPUIndexedPrimitives(renderPass, quadCount * s_IndicesPerQuad, 1, batch.FirstItem * s_IndicesPerQuad, 0, 0);
	}

	bool QuadRenderer::CreateQuadResources(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const ::SDL_GPUVertexAttribute* vertexAttributes, std::uint32_t vertexAttributeCount, const RendererSpecification& specification)
	{
		m_MaxQuadCapacity = specification.MaxQuadCapacity;

		SDL_GPUVertexBufferDescription vertexBufferDescription{
			.slot{ 0 },
			.pitch{ m_VertexSize },
			.input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX },
			.instance_step_rate{ 0 },
		};

		SDL_GPUColorTargetBlendState blendState{
			.src_color_blendfactor{ SDL_GPU_BLENDFACTOR_SRC_ALPHA },
			.dst_color_blendfactor{ SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA },
			.color_blend_op{ SDL_GPU_BLENDOP_ADD },
			.src_alpha_blendfactor{ SDL_GPU_BLENDFACTOR_ONE },
			.dst_alpha_blendfactor{ SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA },
			.alpha_blend_op{ SDL_GPU_BLENDOP_ADD },
			.enable_blend{ true },
		};

		SDL_GPUColorTargetDescription colorTargetDescription{
			.format{ SDL_GetGPUSwapchainTextureFormat(device, window) },
			.blend_state{ blendState },
		};

		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{
			.vertex_shader{ vertexShader },
			.fragment_shader{ fragmentShader },
			.vertex_input_state{
				.vertex_buffer_descriptions{ &vertexBufferDescription },
				.num_vertex_buffers{ 1 },
				.vertex_attributes{ vertexAttributes },
				.num_vertex_attributes{ vertexAttributeCount },
			},
			.primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST },
			.target_info{
				.color_target_descriptions{ &colorTargetDescription },
				.num_color_targets{ 1 },
			},
		};

		m_RenderState.Pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);
		if (!m_RenderState.Pipeline)
		{
			CORE_ERROR("[Renderer] Failed to create the {} graphics pipeline: {}", m_Name, SDL_GetError());
			return false;
		}

		return CreateVertexBuffers(device, specification.InitialQuadCapacity);
	}

	bool QuadRenderer::CreateVertexBuffers(::SDL_GPUDevice* device, std::uint32_t newCapacity)
	{
		const std::uint32_t vertexBytes{ newCapacity * s_VerticesPerQuad * m_VertexSize };

		SDL_GPUBufferCreateInfo vertexBufferInfo{
			.usage{ SDL_GPU_BUFFERUSAGE_VERTEX },
			.size{ vertexBytes },
		};
		SDL_GPUBuffer* vertexBuffer{ SDL_CreateGPUBuffer(device, &vertexBufferInfo) };

		SDL_GPUTransferBufferCreateInfo transferInfo{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ vertexBytes },
		};
		SDL_GPUTransferBuffer* transferBuffer{ SDL_CreateGPUTransferBuffer(device, &transferInfo) };

		if (!vertexBuffer || !transferBuffer)
		{
			CORE_ERROR("[Renderer] Failed to create the {} vertex buffers for {} quads: {}", m_Name, newCapacity, SDL_GetError());

			if (transferBuffer)
			{
				SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			}

			if (vertexBuffer)
			{
				SDL_ReleaseGPUBuffer(device, vertexBuffer);
			}

			return false;
		}

		if (m_TransferBuffer)
		{
			SDL_ReleaseGPUTransferBuffer(device, m_TransferBuffer);
		}

		if (m_VertexBuffer)
		{
			SDL_ReleaseGPUBuffer(device, m_VertexBuffer);
		}

		if (m_QuadCapacity > 0)
		{
			CORE_INFO("[Renderer] {} vertex buffer grew from {} to {} quads", m_Name, m_QuadCapacity, newCapacity);
		}

		m_VertexBuffer = vertexBuffer;
		m_TransferBuffer = transferBuffer;
		m_QuadCapacity = newCapacity;
		return true;
	}

	void QuadRenderer::GrowVertexBuffersIfNeeded(::SDL_GPUDevice* device, std::uint32_t neededQuadCount)
	{
		if (neededQuadCount <= m_QuadCapacity)
		{
			m_GrowthFailed = false;
			return;
		}

		if (m_GrowthFailed)
		{
			return;
		}

		const std::uint32_t grownCapacity{ std::min(std::max(neededQuadCount, m_QuadCapacity * 2), m_MaxQuadCapacity) };
		if (!CreateVertexBuffers(device, grownCapacity))
		{
			m_GrowthFailed = true;
		}
	}

}
