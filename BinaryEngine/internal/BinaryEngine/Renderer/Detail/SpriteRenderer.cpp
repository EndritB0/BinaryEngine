#include "pch.h"
#include "BinaryEngine/Renderer/Detail/SpriteRenderer.h"

#include "BinaryEngine/Renderer/Detail/QuadIndexBuffer.h"
#include "BinaryEngine/Renderer/Detail/RenderQueue.h"

namespace {

	constexpr std::string_view s_RendererName{ "Sprite" };

	struct SpriteVertex {
		BinaryEngine::Vector2f Position{ 0.0f, 0.0f };
		BinaryEngine::Vector2f TextureCoordinates{ 0.0f, 0.0f };
		BinaryEngine::Color VertexColor{};
	};

	static_assert(sizeof(SpriteVertex) == 20, "The sprite pipeline's vertex layout expects 20-byte vertices");

	constexpr std::uint32_t s_QuadVertexBytes{ BinaryEngine::Detail::s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(SpriteVertex)) };

	static_assert(std::uint64_t{ BinaryEngine::Detail::s_AbsoluteMaxQuads } * s_QuadVertexBytes <= std::numeric_limits<std::uint32_t>::max(), "SDL sizes GPU buffers in 32-bit bytes");

	std::array<SpriteVertex, BinaryEngine::Detail::s_VerticesPerQuad> MakeQuadVertices(const BinaryEngine::Detail::SpriteCommand& command)
	{
		return { {
			{ command.Origin, command.UVStart, command.VertexColor },
			{ command.Origin + command.EdgeX, { command.UVEnd.x, command.UVStart.y }, command.VertexColor },
			{ command.Origin + command.EdgeX + command.EdgeY, command.UVEnd, command.VertexColor },
			{ command.Origin + command.EdgeY, { command.UVStart.x, command.UVEnd.y }, command.VertexColor },
		} };
	}

}

namespace BinaryEngine::Detail {

	SpriteRenderer::SpriteRenderer(QuadIndexBuffer& indexBuffer) :
		QuadRenderer(indexBuffer, s_RendererName, static_cast<std::uint32_t>(sizeof(SpriteVertex)))
	{}

	bool SpriteRenderer::Initialise(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const RendererSpecification& specification)
	{
		SDL_GPUVertexAttribute vertexAttributes[3]{
			{
				.location{ 0 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
				.offset{ static_cast<std::uint32_t>(offsetof(SpriteVertex, Position)) },
			},

			{
				.location{ 1 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
				.offset{ static_cast<std::uint32_t>(offsetof(SpriteVertex, TextureCoordinates)) },
			},

			{
				.location{ 2 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM },
				.offset{ static_cast<std::uint32_t>(offsetof(SpriteVertex, VertexColor)) },
			},
		};

		if (!CreateQuadResources(device, window, vertexShader, fragmentShader, vertexAttributes, 3, specification))
		{
			return false;
		}

		const bool useLinearFiltering{ specification.TextureFilter == TextureFilter::Linear };
		const SDL_GPUFilter filter{ useLinearFiltering ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST };
		const SDL_GPUSamplerMipmapMode mipmapMode{ useLinearFiltering ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST };

		SDL_GPUSamplerCreateInfo samplerInfo{
			.min_filter{ filter },
			.mag_filter{ filter },
			.mipmap_mode{ mipmapMode },
			.address_mode_u{ SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE },
			.address_mode_v{ SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE },
			.address_mode_w{ SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE },
		};

		m_Sampler = SDL_CreateGPUSampler(device, &samplerInfo);
		if (!m_Sampler)
		{
			CORE_ERROR("[Renderer] Failed to create sampler: {}", SDL_GetError());
			return false;
		}

		m_Commands.reserve(specification.InitialQuadCapacity);
		return true;
	}

	bool SpriteRenderer::IsValid() const
	{
		return QuadRenderer::IsValid() && m_Sampler;
	}

	void SpriteRenderer::Release(::SDL_GPUDevice* device)
	{
		if (m_Sampler)
		{
			SDL_ReleaseGPUSampler(device, m_Sampler);
			m_Sampler = nullptr;
		}

		QuadRenderer::Release(device);
	}

	void SpriteRenderer::ResetFrame()
	{
		m_Commands.clear();
	}

	void SpriteRenderer::DrawBatch(::SDL_GPURenderPass* renderPass, const RenderBatch& batch)
	{
		SDL_GPUTextureSamplerBinding textureBinding{
			.texture{ batch.Texture },
			.sampler{ m_Sampler },
		};
		SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);

		QuadRenderer::DrawBatch(renderPass, batch);
	}

	void SpriteRenderer::WriteVertices(void* destination, const std::vector<std::uint32_t>& drawOrder, std::uint32_t quadCount)
	{
		std::byte* writePosition{ static_cast<std::byte*>(destination) };
		for (std::uint32_t quadSlot{ 0 }; quadSlot < quadCount; quadSlot++)
		{
			const std::array<SpriteVertex, s_VerticesPerQuad> quadVertices{ MakeQuadVertices(m_Commands[drawOrder[quadSlot]]) };
			std::memcpy(writePosition + static_cast<std::size_t>(quadSlot) * s_QuadVertexBytes, quadVertices.data(), s_QuadVertexBytes);
		}
	}

}
