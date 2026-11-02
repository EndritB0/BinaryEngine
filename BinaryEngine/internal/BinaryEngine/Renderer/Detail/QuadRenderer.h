#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include "BinaryEngine/Renderer/Detail/BatchRenderer.h"
#include "BinaryEngine/Renderer/Detail/RenderState.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"

struct SDL_GPUBuffer;
struct SDL_GPUCopyPass;
struct SDL_GPUDevice;
struct SDL_GPURenderPass;
struct SDL_GPUShader;
struct SDL_GPUTransferBuffer;
struct SDL_GPUVertexAttribute;
struct SDL_Window;

namespace BinaryEngine::Detail {

	class QuadIndexBuffer;

	class QuadRenderer : public BatchRenderer {
	public:
		virtual bool IsValid() const override;
		virtual void Release(::SDL_GPUDevice* device) override;
		virtual void PrepareFrame(::SDL_GPUDevice* device, const std::vector<std::uint32_t>& drawOrder) override;
		virtual void Upload(::SDL_GPUDevice* device, ::SDL_GPUCopyPass* copyPass) override;
		virtual void Bind(::SDL_GPURenderPass* renderPass) override;
		virtual void DrawBatch(::SDL_GPURenderPass* renderPass, const RenderBatch& batch) override;

		const RenderState* GetRenderState() const { return &m_RenderState; }

	protected:
		QuadRenderer(QuadIndexBuffer& indexBuffer, std::string_view name, std::uint32_t vertexSize);

		bool CreateQuadResources(::SDL_GPUDevice* device, ::SDL_Window* window, ::SDL_GPUShader* vertexShader, ::SDL_GPUShader* fragmentShader, const ::SDL_GPUVertexAttribute* vertexAttributes, std::uint32_t vertexAttributeCount, const RendererSpecification& specification);

		virtual void WriteVertices(void* destination, const std::vector<std::uint32_t>& drawOrder, std::uint32_t quadCount) = 0;

	private:
		bool CreateVertexBuffers(::SDL_GPUDevice* device, std::uint32_t newCapacity);
		void GrowVertexBuffersIfNeeded(::SDL_GPUDevice* device, std::uint32_t neededQuadCount);

	private:
		QuadIndexBuffer& m_IndexBuffer;
		std::string_view m_Name;
		std::uint32_t m_VertexSize{ 0 };
		RenderState m_RenderState;
		::SDL_GPUBuffer* m_VertexBuffer{ nullptr };
		::SDL_GPUTransferBuffer* m_TransferBuffer{ nullptr };
		std::uint32_t m_QuadCapacity{ 0 };
		std::uint32_t m_MaxQuadCapacity{ 0 };
		std::uint32_t m_DrawableQuadCount{ 0 };
		bool m_GrowthFailed{ false };
	};

}
