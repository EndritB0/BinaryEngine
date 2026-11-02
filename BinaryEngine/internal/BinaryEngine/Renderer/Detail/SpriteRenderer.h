#pragma once

#include <cstdint>
#include <vector>

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/Detail/QuadRenderer.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"

struct SDL_GPUDevice;
struct SDL_GPURenderPass;
struct SDL_GPUSampler;
struct SDL_GPUShader;
struct SDL_Window;

namespace BinaryEngine::Detail {

	struct SpriteCommand {
		Vector2f Origin{ 0.0f, 0.0f };
		Vector2f EdgeX{ 0.0f, 0.0f };
		Vector2f EdgeY{ 0.0f, 0.0f };
		Vector2f UVStart{ 0.0f, 0.0f };
		Vector2f UVEnd{ 0.0f, 0.0f };
		Color VertexColor{};
	};

	class SpriteRenderer : public QuadRenderer {
	public:
		explicit SpriteRenderer(QuadIndexBuffer& indexBuffer);

		bool Initialise(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const RendererSpecification& specification);

		std::uint32_t AddCommand(Vector2f origin, Vector2f edgeX, Vector2f edgeY, Vector2f uvStart, Vector2f uvEnd, Color vertexColor)
		{
			const std::uint32_t commandIndex{ static_cast<std::uint32_t>(m_Commands.size()) };
			m_Commands.emplace_back(origin, edgeX, edgeY, uvStart, uvEnd, vertexColor);
			return commandIndex;
		}

		virtual bool IsValid() const override;
		virtual void Release(::SDL_GPUDevice* device) override;
		virtual void ResetFrame() override;
		virtual void DrawBatch(::SDL_GPURenderPass* renderPass, const RenderBatch& batch) override;

	protected:
		virtual void WriteVertices(void* destination, const std::vector<std::uint32_t>& drawOrder, std::uint32_t quadCount) override;

	private:
		std::vector<SpriteCommand> m_Commands;
		::SDL_GPUSampler* m_Sampler{ nullptr };
	};

}
