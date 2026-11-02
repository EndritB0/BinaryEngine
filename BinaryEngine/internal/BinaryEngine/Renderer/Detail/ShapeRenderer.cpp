#include "pch.h"
#include "BinaryEngine/Renderer/Detail/ShapeRenderer.h"

#include "BinaryEngine/Renderer/Detail/QuadIndexBuffer.h"
#include "BinaryEngine/Renderer/Detail/RenderQueue.h"

namespace {

	constexpr std::string_view s_RendererName{ "Shape" };

	struct ShapeVertex {
		BinaryEngine::Vector2f Position{ 0.0f, 0.0f };
		BinaryEngine::Vector2f LocalPosition{ 0.0f, 0.0f };
		BinaryEngine::Vector2f HalfSize{ 0.0f, 0.0f };
		BinaryEngine::Color VertexColor{};
		float OutlineThickness{ 0.0f };
		std::uint32_t Type{ 0 };
	};

	static_assert(sizeof(ShapeVertex) == 36, "The shape pipeline's vertex layout expects 36-byte vertices");

	constexpr std::uint32_t s_QuadVertexBytes{ BinaryEngine::Detail::s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(ShapeVertex)) };

	static_assert(std::uint64_t{ BinaryEngine::Detail::s_AbsoluteMaxQuads } * s_QuadVertexBytes <= std::numeric_limits<std::uint32_t>::max(), "SDL sizes GPU buffers in 32-bit bytes");

	std::array<ShapeVertex, BinaryEngine::Detail::s_VerticesPerQuad> MakeQuadVertices(const BinaryEngine::Detail::ShapeCommand& command)
	{
		const BinaryEngine::Vector2f extent{ command.HalfSize + command.Margin };
		const std::uint32_t shapeType{ static_cast<std::uint32_t>(command.Type) };

		return { {
			{ command.Origin, { -extent.x, -extent.y }, command.HalfSize, command.VertexColor, command.OutlineThickness, shapeType },
			{ command.Origin + command.EdgeX, { extent.x, -extent.y }, command.HalfSize, command.VertexColor, command.OutlineThickness, shapeType },
			{ command.Origin + command.EdgeX + command.EdgeY, { extent.x, extent.y }, command.HalfSize, command.VertexColor, command.OutlineThickness, shapeType },
			{ command.Origin + command.EdgeY, { -extent.x, extent.y }, command.HalfSize, command.VertexColor, command.OutlineThickness, shapeType },
		} };
	}

}

namespace BinaryEngine::Detail {

	ShapeRenderer::ShapeRenderer(QuadIndexBuffer& indexBuffer) :
		QuadRenderer(indexBuffer, s_RendererName, static_cast<std::uint32_t>(sizeof(ShapeVertex)))
	{}

	bool ShapeRenderer::Initialise(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const RendererSpecification& specification)
	{
		SDL_GPUVertexAttribute vertexAttributes[6]{
			{
				.location{ 0 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, Position)) },
			},

			{
				.location{ 1 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, LocalPosition)) },
			},

			{
				.location{ 2 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, HalfSize)) },
			},

			{
				.location{ 3 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, VertexColor)) },
			},

			{
				.location{ 4 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, OutlineThickness)) },
			},

			{
				.location{ 5 },
				.buffer_slot{ 0 },
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_UINT },
				.offset{ static_cast<std::uint32_t>(offsetof(ShapeVertex, Type)) },
			},
		};

		if (!CreateQuadResources(device, window, vertexShader, fragmentShader, vertexAttributes, 6, specification))
		{
			return false;
		}

		m_Commands.reserve(specification.InitialQuadCapacity);
		return true;
	}

	void ShapeRenderer::ResetFrame()
	{
		m_Commands.clear();
	}

	void ShapeRenderer::WriteVertices(void* destination, const std::vector<std::uint32_t>& drawOrder, std::uint32_t quadCount)
	{
		std::byte* writePosition{ static_cast<std::byte*>(destination) };
		for (std::uint32_t quadSlot{ 0 }; quadSlot < quadCount; quadSlot++)
		{
			const std::array<ShapeVertex, s_VerticesPerQuad> quadVertices{ MakeQuadVertices(m_Commands[drawOrder[quadSlot]]) };
			std::memcpy(writePosition + static_cast<std::size_t>(quadSlot) * s_QuadVertexBytes, quadVertices.data(), s_QuadVertexBytes);
		}
	}

}
