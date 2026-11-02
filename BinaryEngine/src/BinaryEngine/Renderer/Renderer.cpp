#include "pch.h"
#include "BinaryEngine/Renderer/Renderer.h"

#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/Detail/BatchRenderer.h"
#include "BinaryEngine/Renderer/Detail/BatchRendererType.h"
#include "BinaryEngine/Renderer/Detail/QuadIndexBuffer.h"
#include "BinaryEngine/Renderer/Detail/RenderQueue.h"
#include "BinaryEngine/Renderer/Detail/RenderState.h"
#include "BinaryEngine/Renderer/Detail/SpriteRenderer.h"
#include "BinaryEngine/Renderer/Font.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"
#include "BinaryEngine/Renderer/Shader.h"
#include "BinaryEngine/Renderer/TextLayout.h"
#include "BinaryEngine/Renderer/Texture2D.h"
#include "BinaryEngine/Renderer/TextureRegion.h"
#include "BinaryEngine/Window/Window.h"

namespace {

	constexpr glm::vec4 s_NdcCorners[4]{
		{ -1.0f, -1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
	};

	struct CullBounds {
		float minX{ 0.0f };
		float minY{ 0.0f };
		float maxX{ 0.0f };
		float maxY{ 0.0f };
		bool valid{ false };
	};

	struct AffineTransform2D {
		BinaryEngine::Vector2f AxisX{ 1.0f, 0.0f };
		BinaryEngine::Vector2f AxisY{ 0.0f, 1.0f };
		BinaryEngine::Vector2f Translation{ 0.0f, 0.0f };
	};

	const char* RenderAPIToDriverName(BinaryEngine::RenderAPI api)
	{
		switch (api)
		{
			case BinaryEngine::RenderAPI::Default: return nullptr;
			case BinaryEngine::RenderAPI::Vulkan: return "vulkan";
			case BinaryEngine::RenderAPI::Direct3D12: return "direct3d12";
			case BinaryEngine::RenderAPI::Metal: return "metal";
			default: return nullptr;
		}
	}

	SDL_GPUPresentMode ConvertPresentMode(BinaryEngine::PresentMode mode)
	{
		switch (mode)
		{
			case BinaryEngine::PresentMode::VSync: return SDL_GPU_PRESENTMODE_VSYNC;
			case BinaryEngine::PresentMode::Immediate: return SDL_GPU_PRESENTMODE_IMMEDIATE;
			case BinaryEngine::PresentMode::Mailbox: return SDL_GPU_PRESENTMODE_MAILBOX;
			default: return SDL_GPU_PRESENTMODE_VSYNC;
		}
	}

	std::string_view PresentModeToString(BinaryEngine::PresentMode mode)
	{
		switch (mode)
		{
			case BinaryEngine::PresentMode::VSync: return "VSync";
			case BinaryEngine::PresentMode::Immediate: return "Immediate";
			case BinaryEngine::PresentMode::Mailbox: return "Mailbox";
			default: return "Unknown";
		}
	}

	void NormaliseSpriteCapacitySpecification(BinaryEngine::RendererSpecification& specification)
	{
		if (specification.MaxQuadCapacity > BinaryEngine::Detail::s_AbsoluteMaxQuads)
		{
			CORE_WARN("[Renderer] MaxQuadCapacity ({}) exceeds the absolute limit, clamping to {}", specification.MaxQuadCapacity, BinaryEngine::Detail::s_AbsoluteMaxQuads);
			specification.MaxQuadCapacity = BinaryEngine::Detail::s_AbsoluteMaxQuads;
		}

		if (specification.MaxQuadCapacity == 0)
		{
			CORE_WARN("[Renderer] MaxQuadCapacity is 0, clamping to 1");
			specification.MaxQuadCapacity = 1;
		}

		if (specification.InitialQuadCapacity == 0)
		{
			CORE_WARN("[Renderer] InitialQuadCapacity is 0, clamping to 1");
			specification.InitialQuadCapacity = 1;
		}

		if (specification.InitialQuadCapacity > specification.MaxQuadCapacity)
		{
			CORE_WARN("[Renderer] InitialQuadCapacity ({}) exceeds MaxQuadCapacity ({}), clamping", specification.InitialQuadCapacity, specification.MaxQuadCapacity);
			specification.InitialQuadCapacity = specification.MaxQuadCapacity;
		}
	}

	SDL_FColor ToFloatColor(const BinaryEngine::Color& color)
	{
		return {
			static_cast<float>(color.red) / 255.0f,
			static_cast<float>(color.green) / 255.0f,
			static_cast<float>(color.blue) / 255.0f,
			static_cast<float>(color.alpha) / 255.0f,
		};
	}

	AffineTransform2D ToAffineTransform2DFromMatrix(const BinaryEngine::Transform& transform)
	{
		const glm::mat4 modelMatrix{ transform.GetModelMatrix() };
		return {
			.AxisX{ modelMatrix[0].x, modelMatrix[0].y },
			.AxisY{ modelMatrix[1].x, modelMatrix[1].y },
			.Translation{ modelMatrix[3].x, modelMatrix[3].y },
		};
	}

	AffineTransform2D ToAffineTransform2D(const BinaryEngine::Transform& transform)
	{
		if (transform.Rotation.x != 0.0f || transform.Rotation.y != 0.0f)
		{
			return ToAffineTransform2DFromMatrix(transform);
		}

		const float rotationRadians{ glm::radians(transform.Rotation.z) };
		const float cosine{ std::cos(rotationRadians) };
		const float sine{ std::sin(rotationRadians) };
		return {
			.AxisX{ cosine * transform.Scale.x, sine * transform.Scale.x },
			.AxisY{ -sine * transform.Scale.y, cosine * transform.Scale.y },
			.Translation{ transform.Position.x, transform.Position.y },
		};
	}
}

namespace BinaryEngine {

	struct Renderer::RendererData {
		SDL_GPUDevice* m_Device{ nullptr };
		SDL_Window* m_Window{ nullptr };

		Detail::RenderQueue m_Queue;
		Detail::QuadIndexBuffer m_QuadIndexBuffer;
		Detail::SpriteRenderer m_SpriteRenderer{ m_QuadIndexBuffer };
		std::array<Detail::BatchRenderer*, Detail::s_BatchRendererCount> m_Renderers{ &m_SpriteRenderer };

		TextLayoutResult m_TextLayout;

		CullBounds m_CullBounds;
		Statistics m_Statistics;

		SDL_GPUCommandBuffer* m_CommandBuffer{ nullptr };
		SDL_GPUTexture* m_SwapchainTexture{ nullptr };
		std::uint32_t m_SwapchainWidth{ 0 };
		std::uint32_t m_SwapchainHeight{ 0 };

		Vector2i m_ViewportPosition{ 0, 0 };
		Vector2i m_ViewportSize{ 0, 0 };
		bool m_HasCustomViewport{ false };

		Color m_ClearColor{ Color::Black };
		RendererSpecification m_Specification;

		void SubmitSprite(const Texture2D& texture, const Transform& transform, Vector2f quadSize, Vector2f uvMin, Vector2f uvMax);
		void ResetFrame();
		void SetSceneViewport(SDL_GPURenderPass* renderPass, const Detail::SceneRecord& scene);
		void DrawBatches(SDL_GPURenderPass* renderPass, const glm::mat4& viewProjection, std::size_t firstBatch, std::size_t batchCount, const Detail::RenderState*& boundState, Detail::BatchRendererType& boundRendererType);
		void DrawFrame(bool drawItems);
		bool UploadFrame();
		void SubmitFrame();
		void InitialiseRenderers(const Renderer& renderer);
		void CalculateCullBounds(const glm::mat4& inverseViewProjection);
		bool IsSpriteCulled(const Transform& transform, const Vector2f& quadSize) const;
	};

	Renderer::Renderer(const Window& window, const RendererSpecification& specification) :
		m_Data(std::make_unique<RendererData>())
	{
		m_Data->m_Specification = specification;
		m_Data->m_Window = window.GetNativeWindow();

		NormaliseSpriteCapacitySpecification(m_Data->m_Specification);
		m_Data->m_Statistics.maxQuads = m_Data->m_Specification.MaxQuadCapacity;

		constexpr SDL_GPUShaderFormat shaderFormats{ SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL };
		const char* requestedDriver{ RenderAPIToDriverName(m_Data->m_Specification.RenderAPI) };
		const bool debugMode{ m_Data->m_Specification.ValidationMode == ValidationMode::Enabled };

		m_Data->m_Device = SDL_CreateGPUDevice(shaderFormats, debugMode, requestedDriver);

		if (!m_Data->m_Device)
		{
			CORE_ERROR("[Renderer] Failed to create GPU Device: {}", SDL_GetError());
			return;
		}

		if (!SDL_ClaimWindowForGPUDevice(m_Data->m_Device, m_Data->m_Window))
		{
			CORE_ERROR("[Renderer] Failed to claim window for GPU device: {}", SDL_GetError());
			SDL_DestroyGPUDevice(m_Data->m_Device);
			m_Data->m_Device = nullptr;
			return;
		}

		SetPresentMode(m_Data->m_Specification.PresentMode);

		CORE_INFO("[Renderer] Renderer Initialized with API: {}", SDL_GetGPUDeviceDriver(m_Data->m_Device));

		m_Data->InitialiseRenderers(*this);
	}

	Renderer::~Renderer()
	{
		if (m_Data->m_Device)
		{
			for (Detail::BatchRenderer* renderer : m_Data->m_Renderers)
			{
				renderer->Release(m_Data->m_Device);
			}

			m_Data->m_QuadIndexBuffer.Release(m_Data->m_Device);

			if (m_Data->m_Window)
			{
				SDL_ReleaseWindowFromGPUDevice(m_Data->m_Device, m_Data->m_Window);
			}

			SDL_DestroyGPUDevice(m_Data->m_Device);
			CORE_INFO("[Renderer] Renderer Shutdown");
		}
	}

	SDL_GPUDevice* Renderer::GetNativeDevice() const
	{
		return m_Data->m_Device;
	}

	bool Renderer::IsValid() const
	{
		if (!m_Data->m_Device || !m_Data->m_QuadIndexBuffer.IsValid())
		{
			return false;
		}

		return std::ranges::all_of(m_Data->m_Renderers, [](const Detail::BatchRenderer* renderer) { return renderer->IsValid(); });
	}

	Vector2i Renderer::GetViewportPosition() const
	{
		return m_Data->m_ViewportPosition;
	}

	Vector2i Renderer::GetViewportSize() const
	{
		return m_Data->m_ViewportSize;
	}

	bool Renderer::HasCustomViewport() const
	{
		return m_Data->m_HasCustomViewport;
	}

	PresentMode Renderer::GetPresentMode() const
	{
		return m_Data->m_Specification.PresentMode;
	}

	std::uint8_t Renderer::GetDefaultAlpha() const
	{
		return m_Data->m_Specification.DefaultAlpha;
	}

	const Renderer::Statistics& Renderer::GetStatistics() const
	{
		return m_Data->m_Statistics;
	}

	void Renderer::SetViewport(Vector2i position, Vector2i size)
	{
		m_Data->m_ViewportPosition = position;
		m_Data->m_ViewportSize = size;
		m_Data->m_HasCustomViewport = true;
	}

	void Renderer::ResetViewport()
	{
		m_Data->m_ViewportPosition = { 0, 0 };
		m_Data->m_ViewportSize = { 0, 0 };
		m_Data->m_HasCustomViewport = false;
	}

	void Renderer::SetPresentMode(PresentMode mode)
	{
		SDL_GPUPresentMode sdlMode{ ConvertPresentMode(mode) };

		if (mode != PresentMode::VSync && !SDL_WindowSupportsGPUPresentMode(m_Data->m_Device, m_Data->m_Window, sdlMode))
		{
			CORE_WARN("[Renderer] Requested present mode '{}' is not supported by the window, falling back to VSync", PresentModeToString(mode));
			sdlMode = SDL_GPU_PRESENTMODE_VSYNC;
			mode = PresentMode::VSync;
		}

		if (!SDL_SetGPUSwapchainParameters(m_Data->m_Device, m_Data->m_Window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, sdlMode))
		{
			CORE_ERROR("[Renderer] Failed to set GPU Swapchain Parameters: {}", SDL_GetError());
			return;
		}

		m_Data->m_Specification.PresentMode = mode;
		CORE_TRACE("[Renderer] Present mode set to '{}'", PresentModeToString(mode));
	}

	void Renderer::SetDefaultAlpha(std::uint8_t alpha)
	{
		m_Data->m_Specification.DefaultAlpha = alpha;
		CORE_TRACE("[Renderer] Default Alpha set to {}", alpha);
	}

	void Renderer::Clear()
	{
		m_Data->ResetFrame();
		m_Data->m_SwapchainTexture = nullptr;

		m_Data->m_CommandBuffer = SDL_AcquireGPUCommandBuffer(m_Data->m_Device);
		if (!m_Data->m_CommandBuffer)
		{
			CORE_ERROR("[Renderer] Failed to acquire GPU command buffer: {}", SDL_GetError());
			return;
		}

		if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_Data->m_CommandBuffer, m_Data->m_Window, &m_Data->m_SwapchainTexture, &m_Data->m_SwapchainWidth, &m_Data->m_SwapchainHeight))
		{
			CORE_ERROR("[Renderer] Failed to acquire swapchain texture: {}", SDL_GetError());
			m_Data->m_SwapchainTexture = nullptr;
		}
	}

	void Renderer::Present()
	{
		if (!m_Data->m_CommandBuffer)
		{
			return;
		}

		if (m_Data->m_SwapchainTexture)
		{
			m_Data->SubmitFrame();
		}

		if (!SDL_SubmitGPUCommandBuffer(m_Data->m_CommandBuffer))
		{
			CORE_ERROR("[Renderer] Failed to submit command buffer: {}", SDL_GetError());
		}

		m_Data->m_CommandBuffer = nullptr;
		m_Data->m_SwapchainTexture = nullptr;
	}

	void Renderer::SetClearColor(const Color& color)
	{
		m_Data->m_ClearColor = color;
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		if (m_Data->m_Queue.IsSceneOpen())
		{
			m_Data->m_Queue.DiscardOpenScene();
		}

		const Vector2i swapchainSize{ static_cast<int>(m_Data->m_SwapchainWidth), static_cast<int>(m_Data->m_SwapchainHeight) };
		if (swapchainSize.x > 0 && swapchainSize.y > 0 && camera.GetWindowSize() != swapchainSize)
		{
			camera.OnResize(swapchainSize);
		}

		const Vector2f screenViewSize{ camera.GetScreenViewSize() };
		const CameraViewport viewport{ m_Data->m_HasCustomViewport ? CameraViewport{ m_Data->m_ViewportPosition, m_Data->m_ViewportSize } : camera.CalculateViewport(swapchainSize) };
		m_Data->m_Queue.OpenScene(camera.GetViewProjectionMatrix(), glm::ortho(0.0f, screenViewSize.x, screenViewSize.y, 0.0f, -1.0f, 1.0f), viewport);

		m_Data->m_Statistics.quadCount = 0;
		m_Data->m_Statistics.drawCalls = 0;
		m_Data->m_Statistics.culledSprites = 0;
		m_Data->m_Statistics.glyphCount = 0;

		m_Data->CalculateCullBounds(camera.GetInverseViewProjectionMatrix());
	}

	void Renderer::EndScene()
	{
		if (!m_Data->m_Queue.IsSceneOpen())
		{
			return;
		}

		if (!m_Data->m_CommandBuffer || !m_Data->m_SwapchainTexture)
		{
			m_Data->m_Queue.DiscardOpenScene();
			return;
		}

		const Detail::SceneRecord& scene{ m_Data->m_Queue.CloseScene() };
		m_Data->m_Statistics.quadCount = scene.KeptEntryCount;
		m_Data->m_Statistics.drawCalls = static_cast<std::uint32_t>(scene.WorldBatchCount + scene.ScreenBatchCount);
	}

	void Renderer::DrawSprite(const Texture2D& texture, const Transform& transform)
	{
		Vector2f textureSize{ texture.GetSize() };
		m_Data->SubmitSprite(texture, transform, textureSize, { 0.0f, 0.0f }, { 1.0f, 1.0f });
	}

	void Renderer::DrawSprite(const Texture2D& texture, const Transform& transform, const TextureRegion& region)
	{
		if (region.Size.x <= 0 || region.Size.y <= 0)
		{
			return;
		}

		Vector2f textureSize{ texture.GetSize() };
		if (textureSize.x <= 0.0f || textureSize.y <= 0.0f)
		{
			return;
		}

		Vector2f regionPosition{ region.Position };
		Vector2f regionSize{ region.Size };

		Vector2f uvMin{ regionPosition.x / textureSize.x, regionPosition.y / textureSize.y };
		Vector2f uvMax{ (regionPosition.x + regionSize.x) / textureSize.x, (regionPosition.y + regionSize.y) / textureSize.y };

		m_Data->SubmitSprite(texture, transform, regionSize, uvMin, uvMax);
	}

	void Renderer::DrawText(const Font& font, std::string_view text, const Transform& transform, const TextSpecification& specification)
	{
		if (!m_Data->m_Queue.IsSceneOpen() || !font.IsValid())
		{
			return;
		}

		SDL_GPUTexture* nativeTexture{ font.GetTexture().GetNativeTexture() };
		if (!nativeTexture)
		{
			return;
		}

		LayoutText(font, text, specification, m_Data->m_TextLayout);
		if (m_Data->m_TextLayout.Glyphs.empty())
		{
			return;
		}

		const float glyphScale{ specification.Size / font.GetPixelSize() };

		const float verticalSign{ specification.Space == RenderSpace::World ? -1.0f : 1.0f };

		const AffineTransform2D basis{ ToAffineTransform2D(transform) };

		for (const LaidOutGlyph& glyph : m_Data->m_TextLayout.Glyphs)
		{
			if (!m_Data->m_Queue.HasRoomForEntry())
			{
				return;
			}

			const float left{ glyph.Position.x * glyphScale };
			const float right{ (glyph.Position.x + glyph.Size.x) * glyphScale };
			const float top{ glyph.Position.y * glyphScale * verticalSign };
			const float bottom{ (glyph.Position.y + glyph.Size.y) * glyphScale * verticalSign };

			const std::uint32_t commandIndex{ m_Data->m_SpriteRenderer.AddCommand(basis.Translation + basis.AxisX * left + basis.AxisY * top, basis.AxisX * (right - left), basis.AxisY * (bottom - top), glyph.UVMin, glyph.UVMax, specification.FillColor) };
			m_Data->m_Queue.AddEntry(m_Data->m_SpriteRenderer.GetRenderState(), nativeTexture, commandIndex, transform.Position.z, Detail::BatchRendererType::Sprite, specification.Space);

			m_Data->m_Statistics.glyphCount++;
		}
	}

	Vector2f Renderer::MeasureText(const Font& font, std::string_view text, const TextSpecification& specification) const
	{
		if (!font.IsValid())
		{
			return { 0.0f, 0.0f };
		}

		LayoutText(font, text, specification, m_Data->m_TextLayout);

		return m_Data->m_TextLayout.Bounds * (specification.Size / font.GetPixelSize());
	}

	void Renderer::RendererData::SubmitSprite(const Texture2D& texture, const Transform& transform, Vector2f quadSize, Vector2f uvMin, Vector2f uvMax)
	{
		if (!m_Queue.IsSceneOpen())
		{
			return;
		}

		SDL_GPUTexture* nativeTexture{ texture.GetNativeTexture() };
		if (!nativeTexture)
		{
			return;
		}

		if (m_Specification.CullSprites && m_CullBounds.valid && IsSpriteCulled(transform, quadSize))
		{
			m_Statistics.culledSprites++;
			return;
		}

		if (!m_Queue.HasRoomForEntry())
		{
			return;
		}

		const AffineTransform2D basis{ ToAffineTransform2D(transform) };

		const Vector2f edgeX{ basis.AxisX * quadSize.x };
		const Vector2f edgeY{ basis.AxisY * quadSize.y };

		const std::uint32_t commandIndex{ m_SpriteRenderer.AddCommand(basis.Translation - (edgeX + edgeY) * 0.5f, edgeX, edgeY, { uvMin.x, uvMax.y }, { uvMax.x, uvMin.y }, Color{ 0xFF, 0xFF, 0xFF, m_Specification.DefaultAlpha }) };
		m_Queue.AddEntry(m_SpriteRenderer.GetRenderState(), nativeTexture, commandIndex, transform.Position.z, Detail::BatchRendererType::Sprite, RenderSpace::World);
	}

	void Renderer::RendererData::ResetFrame()
	{
		m_Queue.ResetFrame();
		m_QuadIndexBuffer.ResetFrame();

		for (Detail::BatchRenderer* renderer : m_Renderers)
		{
			renderer->ResetFrame();
		}
	}

	void Renderer::RendererData::SetSceneViewport(SDL_GPURenderPass* renderPass, const Detail::SceneRecord& scene)
	{
		const bool useSceneViewport{ scene.Viewport.Size.x > 0 && scene.Viewport.Size.y > 0 };

		SDL_GPUViewport viewport{
			useSceneViewport ? static_cast<float>(scene.Viewport.Position.x) : 0.0f,
			useSceneViewport ? static_cast<float>(scene.Viewport.Position.y) : 0.0f,
			useSceneViewport ? static_cast<float>(scene.Viewport.Size.x) : static_cast<float>(m_SwapchainWidth),
			useSceneViewport ? static_cast<float>(scene.Viewport.Size.y) : static_cast<float>(m_SwapchainHeight),
			0.0f, 1.0f
		};
		SDL_SetGPUViewport(renderPass, &viewport);
	}

	void Renderer::RendererData::DrawBatches(SDL_GPURenderPass* renderPass, const glm::mat4& viewProjection, std::size_t firstBatch, std::size_t batchCount, const Detail::RenderState*& boundState, Detail::BatchRendererType& boundRendererType)
	{
		if (batchCount == 0)
		{
			return;
		}

		SDL_PushGPUVertexUniformData(m_CommandBuffer, 0, &viewProjection, static_cast<std::uint32_t>(sizeof(viewProjection)));

		for (std::size_t batchIndex{ firstBatch }; batchIndex < firstBatch + batchCount; batchIndex++)
		{
			const Detail::RenderBatch& batch{ m_Queue.GetBatch(batchIndex) };

			if (batch.State != boundState)
			{
				SDL_BindGPUGraphicsPipeline(renderPass, batch.State->Pipeline);
				boundState = batch.State;
			}

			Detail::BatchRenderer& renderer{ *m_Renderers[static_cast<std::size_t>(batch.RendererType)] };
			if (batch.RendererType != boundRendererType)
			{
				renderer.Bind(renderPass);
				boundRendererType = batch.RendererType;
			}

			renderer.DrawBatch(renderPass, batch);
		}
	}

	void Renderer::RendererData::DrawFrame(bool drawItems)
	{
		SDL_GPUColorTargetInfo colorTarget{
			.texture{ m_SwapchainTexture },
			.clear_color{ ToFloatColor(m_ClearColor) },
			.load_op{ SDL_GPU_LOADOP_CLEAR },
			.store_op{ SDL_GPU_STOREOP_STORE },
		};

		SDL_GPURenderPass* renderPass{ SDL_BeginGPURenderPass(m_CommandBuffer, &colorTarget, 1, nullptr) };
		if (!renderPass)
		{
			return;
		}

		if (drawItems)
		{
			const Detail::RenderState* boundState{ nullptr };
			Detail::BatchRendererType boundRendererType{ Detail::BatchRendererType::Count };

			for (const Detail::SceneRecord& scene : m_Queue.GetScenes())
			{
				SetSceneViewport(renderPass, scene);
				DrawBatches(renderPass, scene.ViewProjectionMatrix, scene.FirstBatch, scene.WorldBatchCount, boundState, boundRendererType);
				DrawBatches(renderPass, scene.ScreenViewProjectionMatrix, scene.FirstBatch + scene.WorldBatchCount, scene.ScreenBatchCount, boundState, boundRendererType);
			}
		}

		SDL_EndGPURenderPass(renderPass);
	}

	bool Renderer::RendererData::UploadFrame()
	{
		SDL_GPUCopyPass* copyPass{ SDL_BeginGPUCopyPass(m_CommandBuffer) };
		if (!copyPass)
		{
			CORE_ERROR("[Renderer] Failed to begin copy pass: {}", SDL_GetError());
			return false;
		}

		for (Detail::BatchRenderer* renderer : m_Renderers)
		{
			renderer->Upload(m_Device, copyPass);
		}

		SDL_EndGPUCopyPass(copyPass);
		return true;
	}

	void Renderer::RendererData::SubmitFrame()
	{
		if (m_Queue.ConsumeDropWarning())
		{
			CORE_WARN("[Renderer] The frame went over the quad limit of {} and dropped {} quads. Further drops aren't logged until a frame fits.", m_Specification.MaxQuadCapacity, m_Queue.GetFrameDroppedEntryCount());
		}

		for (std::size_t rendererIndex{ 0 }; rendererIndex < Detail::s_BatchRendererCount; rendererIndex++)
		{
			m_Renderers[rendererIndex]->PrepareFrame(m_Device, m_Queue.GetDrawOrder(rendererIndex));
		}

		m_Statistics.quadCapacity = m_QuadIndexBuffer.GetCapacity();

		const bool hasItems{ m_Queue.GetKeptEntryCount() > 0 };
		DrawFrame(hasItems && UploadFrame());
	}

	void Renderer::RendererData::InitialiseRenderers(const Renderer& renderer)
	{
		Shader spriteVertexShader;
		spriteVertexShader.Load("./resources/shader/sprite/sprite.vert", renderer, { ShaderStage::Vertex, 0, 1 });

		Shader spriteFragmentShader;
		spriteFragmentShader.Load("./resources/shader/sprite/sprite.frag", renderer, { ShaderStage::Fragment, 1, 0 });

		if (!spriteVertexShader.IsValid() || !spriteFragmentShader.IsValid())
		{
			CORE_ERROR("[Renderer] Failed to initialize sprite renderer");
			return;
		}

		if (!m_QuadIndexBuffer.Initialise(m_Device, m_Specification.InitialQuadCapacity, m_Specification.MaxQuadCapacity))
		{
			return;
		}

		m_Statistics.quadCapacity = m_QuadIndexBuffer.GetCapacity();

		if (!m_SpriteRenderer.Initialise(m_Device, m_Window, spriteVertexShader.GetNativeShader(), spriteFragmentShader.GetNativeShader(), m_Specification))
		{
			return;
		}

		m_Queue.Initialise(m_Specification.InitialQuadCapacity, m_Specification.MaxQuadCapacity);
		CORE_INFO("[Renderer] Renderers Initialised ({} quad capacity, {} max)", m_QuadIndexBuffer.GetCapacity(), m_Specification.MaxQuadCapacity);
	}

	void Renderer::RendererData::CalculateCullBounds(const glm::mat4& inverseViewProjection)
	{
		glm::vec4 firstCorner{ inverseViewProjection * s_NdcCorners[0] };
		firstCorner /= firstCorner.w;

		float minX{ firstCorner.x };
		float minY{ firstCorner.y };
		float maxX{ firstCorner.x };
		float maxY{ firstCorner.y };

		for (int cornerIndex{ 1 }; cornerIndex < 4; cornerIndex++)
		{
			glm::vec4 worldCorner{ inverseViewProjection * s_NdcCorners[cornerIndex] };
			worldCorner /= worldCorner.w;

			minX = std::min(minX, worldCorner.x);
			minY = std::min(minY, worldCorner.y);
			maxX = std::max(maxX, worldCorner.x);
			maxY = std::max(maxY, worldCorner.y);
		}

		m_CullBounds.valid = std::isfinite(minX)
			&& std::isfinite(maxX)
			&& std::isfinite(minY)
			&& std::isfinite(maxY)
			&& minX <= maxX && minY <= maxY;

		m_CullBounds.minX = minX;
		m_CullBounds.maxX = maxX;
		m_CullBounds.minY = minY;
		m_CullBounds.maxY = maxY;
	}

	bool Renderer::RendererData::IsSpriteCulled(const Transform& transform, const Vector2f& quadSize) const
	{
		const float halfWidth{ quadSize.x * transform.Scale.x * 0.5f };
		const float halfHeight{ quadSize.y * transform.Scale.y * 0.5f };
		const float radius{ std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight) };

		return transform.Position.x + radius < m_CullBounds.minX
			|| transform.Position.x - radius > m_CullBounds.maxX
			|| transform.Position.y + radius < m_CullBounds.minY
			|| transform.Position.y - radius > m_CullBounds.maxY;
	}

}
