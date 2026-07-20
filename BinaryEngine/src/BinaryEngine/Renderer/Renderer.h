#pragma once

#include <cstdint>
#include <glm/fwd.hpp>

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"
#include "BinaryEngine/Renderer/Texture2D.h"
#include "BinaryEngine/Window/Window.h"

namespace BinaryEngine {

	class Renderer {
	public:
		Renderer(const Window& window, const RendererSpecification& specification = RendererSpecification());
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		struct Statistics {
			std::uint32_t quadCount{ 0 };
			std::uint32_t drawCalls{ 0 };
			std::uint32_t culledSprites{ 0 };
			std::uint32_t quadCapacity{ 0 };
			std::uint32_t maxQuads{ 0 };
		};

		void* GetNativeDevice() const { return m_Device; }
		bool IsValid() const { return m_Device != nullptr; }
		void SetViewport(Vector2i position, Vector2i size);
		void SetPresentMode(PresentMode mode);
		PresentMode GetPresentMode() const { return m_Specification.presentMode; };
		void SetDefaultAlpha(std::uint8_t alpha);
		std::uint8_t GetDefaultAlpha() const { return m_Specification.defaultAlpha; };
		const Statistics& GetStatistics() const { return m_Statistics; }
		void Clear();
		void Present();
		void SetClearColor(const Color& color);
		void BeginScene(const OrthographicCamera& camera);
		void EndScene();
		void DrawSprite(const Texture2D& texture, const Transform& transform);

	private:
		void InitialiseSpriteRenderer();
		bool UploadQuadIndices(struct SDL_GPUBuffer* target, std::uint32_t quadCapacity);
		bool EnsureQuadCapacity(std::uint32_t newCapacity);
		void CalculateCullBounds();
		bool IsSpriteCulled(const Transform& transform, const Vector2f& textureSize) const;

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix{ 1.0f };
		};

		struct SpriteVertex {
			float position[3]{};
			float textureCoordinates[2]{};
			float color[4]{};
		};

		struct RenderBatch {
			struct SDL_GPUTexture* texture{ nullptr };
			std::uint32_t firstQuad{ 0 };
			std::uint32_t quadCount{ 0 };
		};

		struct SpriteDraw {
			struct SDL_GPUTexture* texture{ nullptr };
			float sortZ{ 0.0f };
			SpriteVertex vertices[4]{};
		};

		struct CullBounds {
			float minX{ 0.0f };
			float minY{ 0.0f };
			float maxX{ 0.0f };
			float maxY{ 0.0f };
			bool valid{ false };
		};

		SceneData m_SceneData;

		struct SDL_GPUDevice* m_Device{ nullptr };
		struct SDL_Window* m_Window{ nullptr };

		struct SDL_GPUGraphicsPipeline* m_Pipeline{ nullptr };
		struct SDL_GPUSampler* m_Sampler{ nullptr };
		struct SDL_GPUBuffer* m_VertexBuffer{ nullptr };
		struct SDL_GPUBuffer* m_IndexBuffer{ nullptr };
		struct SDL_GPUTransferBuffer* m_VertexTransferBuffer{ nullptr };

		std::vector<SpriteDraw> m_Draws;
		std::vector<SpriteVertex> m_VertexStaging;
		std::vector<RenderBatch> m_Batches;
		std::uint32_t m_QuadCount{ 0 };
		std::uint32_t m_QuadCapacity{ 0 };
		bool m_SceneActive{ false };

		CullBounds m_CullBounds;
		Statistics m_Statistics;

		struct SDL_GPUCommandBuffer* m_CommandBuffer{ nullptr };
		struct SDL_GPUTexture* m_SwapchainTexture{ nullptr };
		std::uint32_t m_SwapchainWidth{ 0 };
		std::uint32_t m_SwapchainHeight{ 0 };
		bool m_FrameCleared{ false };

		Vector2i m_ViewportPosition{ 0, 0 };
		Vector2i m_ViewportSize{ 0, 0 };

		Color m_ClearColor{ Color::Black };
		RendererSpecification m_Specification;
	};

}