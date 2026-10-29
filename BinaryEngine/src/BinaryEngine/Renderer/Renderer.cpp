#include "pch.h"
#include "BinaryEngine/Renderer/Renderer.h"

#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/Font.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"
#include "BinaryEngine/Renderer/Shader.h"
#include "BinaryEngine/Renderer/TextLayout.h"
#include "BinaryEngine/Renderer/Texture2D.h"
#include "BinaryEngine/Renderer/TextureRegion.h"
#include "BinaryEngine/Window/Window.h"

namespace {

	constexpr std::uint32_t s_VerticesPerQuad{ 4 };
	constexpr std::uint32_t s_IndicesPerQuad{ 6 };
	constexpr std::uint32_t s_AbsoluteMaxQuads{ 16000000 };

	constexpr std::uint64_t s_ScreenSpaceKeyBit{ std::uint64_t{ 1 } << 63 };
	constexpr std::uint32_t s_SortKeyIndexBits{ 31 };
	constexpr std::uint64_t s_SortKeyIndexMask{ (std::uint64_t{ 1 } << s_SortKeyIndexBits) - 1 };

	enum class QuadSpace : std::uint8_t {
		World = 0,
		Screen
	};

	struct SpriteVertex {
		BinaryEngine::Vector2f Position{ 0.0f, 0.0f };
		BinaryEngine::Vector2f TextureCoordinates{ 0.0f, 0.0f };
		BinaryEngine::Color VertexColor{};
	};

	static_assert(sizeof(SpriteVertex) == 20, "The sprite pipeline's vertex layout expects 20-byte vertices");

	constexpr std::uint32_t s_QuadVertexBytes{ s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(SpriteVertex)) };

	static_assert(std::uint64_t{ s_AbsoluteMaxQuads } * s_QuadVertexBytes <= std::numeric_limits<std::uint32_t>::max(), "SDL sizes GPU buffers in 32-bit bytes");

	struct QuadCommand {
		SDL_GPUTexture* Texture{ nullptr };
		BinaryEngine::Vector2f Origin{ 0.0f, 0.0f };
		BinaryEngine::Vector2f EdgeX{ 0.0f, 0.0f };
		BinaryEngine::Vector2f EdgeY{ 0.0f, 0.0f };
		BinaryEngine::Vector2f UVStart{ 0.0f, 0.0f };
		BinaryEngine::Vector2f UVEnd{ 0.0f, 0.0f };
		BinaryEngine::Color VertexColor{};
		float SortZ{ 0.0f };
		QuadSpace Space{ QuadSpace::World };
	};

	struct RenderBatch {
		SDL_GPUTexture* texture{ nullptr };
		std::uint32_t firstQuad{ 0 };
		std::uint32_t quadCount{ 0 };
	};

	struct SceneRecord {
		glm::mat4 ViewProjectionMatrix{ 1.0f };
		glm::mat4 ScreenViewProjectionMatrix{ 1.0f };
		BinaryEngine::CameraViewport Viewport{};
		std::size_t FirstCommand{ 0 };
		std::size_t FirstBatch{ 0 };
		std::size_t WorldBatchCount{ 0 };
		std::size_t ScreenBatchCount{ 0 };
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
		if (specification.MaxQuadCapacity > s_AbsoluteMaxQuads)
		{
			CORE_WARN("[Renderer] MaxQuadCapacity ({}) exceeds the absolute limit, clamping to {}", specification.MaxQuadCapacity, s_AbsoluteMaxQuads);
			specification.MaxQuadCapacity = s_AbsoluteMaxQuads;
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

	std::uint32_t ToOrderableBits(float value)
	{
		const float normalisedValue{ value == 0.0f ? 0.0f : value };
		const std::uint32_t bits{ std::bit_cast<std::uint32_t>(normalisedValue) };
		return (bits & 0x80000000u) != 0 ? ~bits : (bits | 0x80000000u);
	}

	std::uint64_t MakeSortKey(QuadSpace space, float sortZ, std::uint32_t sceneCommandIndex)
	{
		const std::uint64_t spaceBit{ space == QuadSpace::Screen ? s_ScreenSpaceKeyBit : 0 };
		return spaceBit | (static_cast<std::uint64_t>(ToOrderableBits(sortZ)) << s_SortKeyIndexBits) | sceneCommandIndex;
	}

	std::array<SpriteVertex, s_VerticesPerQuad> MakeQuadVertices(const QuadCommand& command)
	{
		return { {
			{ command.Origin, command.UVStart, command.VertexColor },
			{ command.Origin + command.EdgeX, { command.UVEnd.x, command.UVStart.y }, command.VertexColor },
			{ command.Origin + command.EdgeX + command.EdgeY, command.UVEnd, command.VertexColor },
			{ command.Origin + command.EdgeY, { command.UVStart.x, command.UVEnd.y }, command.VertexColor },
		} };
	}
}

namespace BinaryEngine {

	struct Renderer::RendererData {
		SDL_GPUDevice* m_Device{ nullptr };
		SDL_Window* m_Window{ nullptr };

		SDL_GPUGraphicsPipeline* m_Pipeline{ nullptr };
		SDL_GPUSampler* m_Sampler{ nullptr };
		SDL_GPUBuffer* m_VertexBuffer{ nullptr };
		SDL_GPUBuffer* m_IndexBuffer{ nullptr };
		SDL_GPUTransferBuffer* m_VertexTransferBuffer{ nullptr };
		SDL_GPUTransferBuffer* m_PendingIndexTransferBuffer{ nullptr };

		std::vector<QuadCommand> m_Commands;
		std::vector<std::uint64_t> m_SortKeys;
		std::vector<std::uint32_t> m_DrawOrder;
		std::vector<RenderBatch> m_Batches;
		std::vector<SceneRecord> m_Scenes;
		TextLayoutResult m_TextLayout;
		std::uint32_t m_QuadCapacity{ 0 };
		bool m_GrowthFailed{ false };
		bool m_SceneActive{ false };
		std::uint32_t m_FrameDroppedQuads{ 0 };
		bool m_QuadLimitWarned{ false };

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
		bool HasRoomForCommand();
		void ResetQueue();
		void ResetFrame();
		void DiscardOpenScene();
		void CloseScene();
		std::size_t AppendBatches(const SceneRecord& scene, std::uint32_t firstKeyIndex, std::uint32_t keyCount);
		bool UploadQuads(std::uint32_t drawableQuadCount);
		void BindQuadPipeline(SDL_GPURenderPass* renderPass);
		void SetSceneViewport(SDL_GPURenderPass* renderPass, const SceneRecord& scene);
		void DrawBatches(SDL_GPURenderPass* renderPass, const glm::mat4& viewProjection, std::size_t firstBatch, std::size_t batchCount, std::uint32_t drawableQuadCount);
		void DrawFrame(std::uint32_t drawableQuadCount);
		void SubmitFrame();
		void InitialiseSpriteRenderer(const Shader& vertexShader, const Shader& fragmentShader);
		bool WriteQuadIndices(SDL_GPUTransferBuffer* indexTransfer, std::uint32_t quadCapacity);
		bool CreateQuadBuffers(std::uint32_t newCapacity);
		void GrowQuadBuffersIfNeeded(std::uint32_t neededQuadCount);
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

		Shader vertexShader;
		vertexShader.Load("./resources/shader/sprite/sprite.vert", *this, { ShaderStage::Vertex, 0, 1 });

		Shader fragmentShader;
		fragmentShader.Load("./resources/shader/sprite/sprite.frag", *this, { ShaderStage::Fragment, 1, 0 });

		m_Data->InitialiseSpriteRenderer(vertexShader, fragmentShader);
	}

	Renderer::~Renderer()
	{
		if (m_Data->m_Device)
		{
			if (m_Data->m_PendingIndexTransferBuffer)
			{
				SDL_ReleaseGPUTransferBuffer(m_Data->m_Device, m_Data->m_PendingIndexTransferBuffer);
			}
			if (m_Data->m_VertexTransferBuffer)
			{
				SDL_ReleaseGPUTransferBuffer(m_Data->m_Device, m_Data->m_VertexTransferBuffer);
			}
			if (m_Data->m_IndexBuffer)
			{
				SDL_ReleaseGPUBuffer(m_Data->m_Device, m_Data->m_IndexBuffer);
			}
			if (m_Data->m_VertexBuffer)
			{
				SDL_ReleaseGPUBuffer(m_Data->m_Device, m_Data->m_VertexBuffer);
			}
			if (m_Data->m_Sampler)
			{
				SDL_ReleaseGPUSampler(m_Data->m_Device, m_Data->m_Sampler);
			}
			if (m_Data->m_Pipeline)
			{
				SDL_ReleaseGPUGraphicsPipeline(m_Data->m_Device, m_Data->m_Pipeline);
			}

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
		return m_Data->m_Device && m_Data->m_Pipeline && m_Data->m_Sampler && m_Data->m_QuadCapacity > 0;
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
		if (m_Data->m_SceneActive)
		{
			m_Data->DiscardOpenScene();
		}

		const Vector2i swapchainSize{ static_cast<int>(m_Data->m_SwapchainWidth), static_cast<int>(m_Data->m_SwapchainHeight) };
		if (swapchainSize.x > 0 && swapchainSize.y > 0 && camera.GetWindowSize() != swapchainSize)
		{
			camera.OnResize(swapchainSize);
		}

		SceneRecord& scene{ m_Data->m_Scenes.emplace_back() };
		scene.ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		const Vector2f screenViewSize{ camera.GetScreenViewSize() };
		scene.ScreenViewProjectionMatrix = glm::ortho(0.0f, screenViewSize.x, screenViewSize.y, 0.0f, -1.0f, 1.0f);

		scene.Viewport = m_Data->m_HasCustomViewport ? CameraViewport{ m_Data->m_ViewportPosition, m_Data->m_ViewportSize } : camera.CalculateViewport(swapchainSize);
		scene.FirstCommand = m_Data->m_Commands.size();
		m_Data->m_SceneActive = true;

		m_Data->m_Statistics.quadCount = 0;
		m_Data->m_Statistics.drawCalls = 0;
		m_Data->m_Statistics.culledSprites = 0;
		m_Data->m_Statistics.glyphCount = 0;

		m_Data->CalculateCullBounds(camera.GetInverseViewProjectionMatrix());
	}

	void Renderer::EndScene()
	{
		if (!m_Data->m_SceneActive)
		{
			return;
		}

		if (!m_Data->m_CommandBuffer || !m_Data->m_SwapchainTexture)
		{
			m_Data->DiscardOpenScene();
			return;
		}

		m_Data->CloseScene();
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
		if (!m_Data->m_SceneActive || !font.IsValid())
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

		const bool isWorldSpace{ specification.Space == TextSpace::World };
		const float verticalSign{ isWorldSpace ? -1.0f : 1.0f };
		const QuadSpace space{ isWorldSpace ? QuadSpace::World : QuadSpace::Screen };

		const AffineTransform2D basis{ ToAffineTransform2D(transform) };

		for (const LaidOutGlyph& glyph : m_Data->m_TextLayout.Glyphs)
		{
			if (!m_Data->HasRoomForCommand())
			{
				return;
			}

			const float left{ glyph.Position.x * glyphScale };
			const float right{ (glyph.Position.x + glyph.Size.x) * glyphScale };
			const float top{ glyph.Position.y * glyphScale * verticalSign };
			const float bottom{ (glyph.Position.y + glyph.Size.y) * glyphScale * verticalSign };

			QuadCommand& command{ m_Data->m_Commands.emplace_back() };
			command.Texture = nativeTexture;
			command.Origin = basis.Translation + basis.AxisX * left + basis.AxisY * top;
			command.EdgeX = basis.AxisX * (right - left);
			command.EdgeY = basis.AxisY * (bottom - top);
			command.UVStart = glyph.UVMin;
			command.UVEnd = glyph.UVMax;
			command.VertexColor = specification.FillColor;
			command.SortZ = transform.Position.z;
			command.Space = space;

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
		if (!m_SceneActive)
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

		if (!HasRoomForCommand())
		{
			return;
		}

		const AffineTransform2D basis{ ToAffineTransform2D(transform) };

		const Vector2f edgeX{ basis.AxisX * quadSize.x };
		const Vector2f edgeY{ basis.AxisY * quadSize.y };

		QuadCommand& command{ m_Commands.emplace_back() };
		command.Texture = nativeTexture;
		command.Origin = basis.Translation - (edgeX + edgeY) * 0.5f;
		command.EdgeX = edgeX;
		command.EdgeY = edgeY;
		command.UVStart = { uvMin.x, uvMax.y };
		command.UVEnd = { uvMax.x, uvMin.y };
		command.VertexColor = Color{ 0xFF, 0xFF, 0xFF, m_Specification.DefaultAlpha };
		command.SortZ = transform.Position.z;
		command.Space = QuadSpace::World;
	}

	bool Renderer::RendererData::HasRoomForCommand()
	{
		if (m_Commands.size() - m_Scenes.back().FirstCommand < s_AbsoluteMaxQuads)
		{
			return true;
		}

		m_FrameDroppedQuads++;
		return false;
	}

	void Renderer::RendererData::ResetQueue()
	{
		m_Commands.clear();
		m_DrawOrder.clear();
		m_Batches.clear();
		m_Scenes.clear();
		m_SceneActive = false;
	}

	void Renderer::RendererData::ResetFrame()
	{
		ResetQueue();

		if (m_FrameDroppedQuads == 0)
		{
			m_QuadLimitWarned = false;
		}
		m_FrameDroppedQuads = 0;
	}

	void Renderer::RendererData::DiscardOpenScene()
	{
		m_Commands.resize(m_Scenes.back().FirstCommand);
		m_Scenes.pop_back();
		m_SceneActive = false;
	}

	void Renderer::RendererData::CloseScene()
	{
		m_SceneActive = false;

		SceneRecord& scene{ m_Scenes.back() };
		const std::uint32_t sceneCommandCount{ static_cast<std::uint32_t>(m_Commands.size() - scene.FirstCommand) };

		m_SortKeys.clear();
		for (std::uint32_t sceneCommandIndex{ 0 }; sceneCommandIndex < sceneCommandCount; sceneCommandIndex++)
		{
			const QuadCommand& command{ m_Commands[scene.FirstCommand + sceneCommandIndex] };
			m_SortKeys.push_back(MakeSortKey(command.Space, command.SortZ, sceneCommandIndex));
		}
		std::sort(m_SortKeys.begin(), m_SortKeys.end());

		const auto firstScreenKey{ std::partition_point(m_SortKeys.begin(), m_SortKeys.end(), [](std::uint64_t sortKey) { return (sortKey & s_ScreenSpaceKeyBit) == 0; }) };
		const std::uint32_t worldQuadCount{ static_cast<std::uint32_t>(firstScreenKey - m_SortKeys.begin()) };
		const std::uint32_t screenQuadCount{ sceneCommandCount - worldQuadCount };

		const std::uint32_t quadBudget{ m_Specification.MaxQuadCapacity - static_cast<std::uint32_t>(m_DrawOrder.size()) };
		const std::uint32_t keptScreenQuadCount{ std::min(screenQuadCount, quadBudget) };
		const std::uint32_t keptWorldQuadCount{ std::min(worldQuadCount, quadBudget - keptScreenQuadCount) };
		m_FrameDroppedQuads += sceneCommandCount - keptWorldQuadCount - keptScreenQuadCount;

		scene.FirstBatch = m_Batches.size();
		scene.WorldBatchCount = AppendBatches(scene, 0, keptWorldQuadCount);
		scene.ScreenBatchCount = AppendBatches(scene, worldQuadCount, keptScreenQuadCount);

		m_Statistics.quadCount = keptWorldQuadCount + keptScreenQuadCount;
		m_Statistics.drawCalls = static_cast<std::uint32_t>(scene.WorldBatchCount + scene.ScreenBatchCount);
	}

	std::size_t Renderer::RendererData::AppendBatches(const SceneRecord& scene, std::uint32_t firstKeyIndex, std::uint32_t keyCount)
	{
		const std::size_t firstBatchIndex{ m_Batches.size() };

		for (std::uint32_t keyIndex{ firstKeyIndex }; keyIndex < firstKeyIndex + keyCount; keyIndex++)
		{
			const std::uint32_t commandIndex{ static_cast<std::uint32_t>(scene.FirstCommand + (m_SortKeys[keyIndex] & s_SortKeyIndexMask)) };
			SDL_GPUTexture* texture{ m_Commands[commandIndex].Texture };

			if (m_Batches.size() == firstBatchIndex || m_Batches.back().texture != texture)
			{
				m_Batches.emplace_back(texture, static_cast<std::uint32_t>(m_DrawOrder.size()), 0);
			}

			m_DrawOrder.push_back(commandIndex);
			m_Batches.back().quadCount++;
		}

		return m_Batches.size() - firstBatchIndex;
	}

	bool Renderer::RendererData::UploadQuads(std::uint32_t drawableQuadCount)
	{
		void* mappedVertices{ SDL_MapGPUTransferBuffer(m_Device, m_VertexTransferBuffer, true) };
		if (!mappedVertices)
		{
			CORE_ERROR("[Renderer] Failed to map vertex transfer buffer: {}", SDL_GetError());
			return false;
		}

		std::byte* destination{ static_cast<std::byte*>(mappedVertices) };
		for (std::uint32_t quadSlot{ 0 }; quadSlot < drawableQuadCount; quadSlot++)
		{
			const std::array<SpriteVertex, s_VerticesPerQuad> quadVertices{ MakeQuadVertices(m_Commands[m_DrawOrder[quadSlot]]) };
			std::memcpy(destination + static_cast<std::size_t>(quadSlot) * s_QuadVertexBytes, quadVertices.data(), s_QuadVertexBytes);
		}

		SDL_UnmapGPUTransferBuffer(m_Device, m_VertexTransferBuffer);

		SDL_GPUCopyPass* copyPass{ SDL_BeginGPUCopyPass(m_CommandBuffer) };
		if (!copyPass)
		{
			CORE_ERROR("[Renderer] Failed to begin copy pass: {}", SDL_GetError());
			return false;
		}

		if (m_PendingIndexTransferBuffer)
		{
			SDL_GPUTransferBufferLocation indexSource{
				.transfer_buffer{ m_PendingIndexTransferBuffer },
				.offset{ 0 },
			};

			SDL_GPUBufferRegion indexRegion{
				.buffer{ m_IndexBuffer },
				.offset{ 0 },
				.size{ m_QuadCapacity * s_IndicesPerQuad * static_cast<std::uint32_t>(sizeof(std::uint32_t)) },
			};

			SDL_UploadToGPUBuffer(copyPass, &indexSource, &indexRegion, false);
			SDL_ReleaseGPUTransferBuffer(m_Device, m_PendingIndexTransferBuffer);
			m_PendingIndexTransferBuffer = nullptr;
		}

		SDL_GPUTransferBufferLocation vertexSource{
			.transfer_buffer{ m_VertexTransferBuffer },
			.offset{ 0 },
		};

		SDL_GPUBufferRegion vertexRegion{
			.buffer{ m_VertexBuffer },
			.offset{ 0 },
			.size{ drawableQuadCount * s_QuadVertexBytes },
		};

		SDL_UploadToGPUBuffer(copyPass, &vertexSource, &vertexRegion, true);
		SDL_EndGPUCopyPass(copyPass);
		return true;
	}

	void Renderer::RendererData::BindQuadPipeline(SDL_GPURenderPass* renderPass)
	{
		SDL_BindGPUGraphicsPipeline(renderPass, m_Pipeline);

		SDL_GPUBufferBinding vertexBinding{
			.buffer{ m_VertexBuffer },
			.offset{ 0 },
		};
		SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);

		SDL_GPUBufferBinding indexBinding{
			.buffer{ m_IndexBuffer },
			.offset{ 0 },
		};
		SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
	}

	void Renderer::RendererData::SetSceneViewport(SDL_GPURenderPass* renderPass, const SceneRecord& scene)
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

	void Renderer::RendererData::DrawBatches(SDL_GPURenderPass* renderPass, const glm::mat4& viewProjection, std::size_t firstBatch, std::size_t batchCount, std::uint32_t drawableQuadCount)
	{
		if (batchCount == 0)
		{
			return;
		}

		SDL_PushGPUVertexUniformData(m_CommandBuffer, 0, &viewProjection, static_cast<std::uint32_t>(sizeof(viewProjection)));

		for (std::size_t batchIndex{ firstBatch }; batchIndex < firstBatch + batchCount; batchIndex++)
		{
			const RenderBatch& batch{ m_Batches[batchIndex] };
			if (batch.firstQuad >= drawableQuadCount)
			{
				return;
			}

			const std::uint32_t quadCount{ std::min(batch.quadCount, drawableQuadCount - batch.firstQuad) };

			SDL_GPUTextureSamplerBinding textureBinding{
				.texture{ batch.texture },
				.sampler{ m_Sampler },
			};
			SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);
			SDL_DrawGPUIndexedPrimitives(renderPass, quadCount * s_IndicesPerQuad, 1, batch.firstQuad * s_IndicesPerQuad, 0, 0);
		}
	}

	void Renderer::RendererData::DrawFrame(std::uint32_t drawableQuadCount)
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

		if (drawableQuadCount > 0)
		{
			BindQuadPipeline(renderPass);

			for (const SceneRecord& scene : m_Scenes)
			{
				SetSceneViewport(renderPass, scene);
				DrawBatches(renderPass, scene.ViewProjectionMatrix, scene.FirstBatch, scene.WorldBatchCount, drawableQuadCount);
				DrawBatches(renderPass, scene.ScreenViewProjectionMatrix, scene.FirstBatch + scene.WorldBatchCount, scene.ScreenBatchCount, drawableQuadCount);
			}
		}

		SDL_EndGPURenderPass(renderPass);
	}

	void Renderer::RendererData::SubmitFrame()
	{
		if (m_FrameDroppedQuads > 0 && !m_QuadLimitWarned)
		{
			CORE_WARN("[Renderer] The frame went over the quad limit of {} and dropped {} quads. Further drops aren't logged until a frame fits.", m_Specification.MaxQuadCapacity, m_FrameDroppedQuads);
			m_QuadLimitWarned = true;
		}

		const std::uint32_t frameQuadCount{ static_cast<std::uint32_t>(m_DrawOrder.size()) };
		GrowQuadBuffersIfNeeded(frameQuadCount);

		const std::uint32_t drawableQuadCount{ std::min(frameQuadCount, m_QuadCapacity) };
		const bool drawQuads{ m_Pipeline && m_Sampler && drawableQuadCount > 0 && UploadQuads(drawableQuadCount) };
		DrawFrame(drawQuads ? drawableQuadCount : 0);
	}

	void Renderer::RendererData::InitialiseSpriteRenderer(const Shader& vertexShader, const Shader& fragmentShader)
	{
		if (!vertexShader.IsValid() || !fragmentShader.IsValid())
		{
			CORE_ERROR("[Renderer] Failed to initialize sprite renderer");
			return;
		}

		const std::uint32_t vertexSize{ static_cast<std::uint32_t>(sizeof(SpriteVertex)) };

		SDL_GPUVertexBufferDescription vertexBufferDescription{
			.slot{ 0 },
			.pitch{ vertexSize },
			.input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX },
			.instance_step_rate{ 0 },
		};

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
			.format{ SDL_GetGPUSwapchainTextureFormat(m_Device, m_Window) },
			.blend_state{ blendState },
		};

		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{
			.vertex_shader{ vertexShader.GetNativeShader() },
			.fragment_shader{ fragmentShader.GetNativeShader() },
			.vertex_input_state{
				.vertex_buffer_descriptions{ &vertexBufferDescription },
				.num_vertex_buffers{ 1 },
				.vertex_attributes{ vertexAttributes },
				.num_vertex_attributes{ 3 },
			},
			.primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST },
			.target_info{
				.color_target_descriptions{ &colorTargetDescription },
				.num_color_targets{ 1 },
			},
		};

		m_Pipeline = SDL_CreateGPUGraphicsPipeline(m_Device, &pipelineInfo);
		if (!m_Pipeline)
		{
			CORE_ERROR("[Renderer] Failed to create Graphics Pipeline: {}", SDL_GetError());
			return;
		}

		const bool useLinearFiltering{ m_Specification.TextureFilter == TextureFilter::Linear };
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

		m_Sampler = SDL_CreateGPUSampler(m_Device, &samplerInfo);
		if (!m_Sampler)
		{
			CORE_ERROR("[Renderer] Failed to create sampler: {}", SDL_GetError());
			return;
		}

		if (!CreateQuadBuffers(m_Specification.InitialQuadCapacity))
		{
			return;
		}

		m_Commands.reserve(m_Specification.InitialQuadCapacity);
		m_SortKeys.reserve(m_Specification.InitialQuadCapacity);
		m_DrawOrder.reserve(m_Specification.InitialQuadCapacity);
		m_Batches.reserve(64);
		m_Scenes.reserve(4);
		CORE_INFO("[Renderer] Sprite Renderer Initialised ({} quad capacity, {} max)", m_QuadCapacity, m_Specification.MaxQuadCapacity);
	}

	bool Renderer::RendererData::WriteQuadIndices(SDL_GPUTransferBuffer* indexTransfer, std::uint32_t quadCapacity)
	{
		void* mappedIndices{ SDL_MapGPUTransferBuffer(m_Device, indexTransfer, false) };
		if (!mappedIndices)
		{
			return false;
		}

		std::uint32_t* indices{ static_cast<std::uint32_t*>(mappedIndices) };
		for (std::uint32_t quadIndex{}; quadIndex < quadCapacity; quadIndex++)
		{
			const std::uint32_t firstVertex{ quadIndex * s_VerticesPerQuad };
			const std::uint32_t firstIndex{ quadIndex * s_IndicesPerQuad };
			indices[firstIndex + 0] = firstVertex + 0;
			indices[firstIndex + 1] = firstVertex + 1;
			indices[firstIndex + 2] = firstVertex + 2;
			indices[firstIndex + 3] = firstVertex + 2;
			indices[firstIndex + 4] = firstVertex + 3;
			indices[firstIndex + 5] = firstVertex + 0;
		}

		SDL_UnmapGPUTransferBuffer(m_Device, indexTransfer);
		return true;
	}

	bool Renderer::RendererData::CreateQuadBuffers(std::uint32_t newCapacity)
	{
		const std::uint32_t vertexBytes{ newCapacity * s_QuadVertexBytes };
		const std::uint32_t indexBytes{ newCapacity * s_IndicesPerQuad * static_cast<std::uint32_t>(sizeof(std::uint32_t)) };

		SDL_GPUBufferCreateInfo vertexBufferInfo{
			.usage{ SDL_GPU_BUFFERUSAGE_VERTEX },
			.size{ vertexBytes },
		};
		SDL_GPUBuffer* vertexBuffer{ SDL_CreateGPUBuffer(m_Device, &vertexBufferInfo) };

		SDL_GPUBufferCreateInfo indexBufferInfo{
			.usage{ SDL_GPU_BUFFERUSAGE_INDEX },
			.size{ indexBytes },
		};
		SDL_GPUBuffer* indexBuffer{ SDL_CreateGPUBuffer(m_Device, &indexBufferInfo) };

		SDL_GPUTransferBufferCreateInfo vertexTransferInfo{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ vertexBytes },
		};
		SDL_GPUTransferBuffer* vertexTransfer{ SDL_CreateGPUTransferBuffer(m_Device, &vertexTransferInfo) };

		SDL_GPUTransferBufferCreateInfo indexTransferInfo{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ indexBytes },
		};
		SDL_GPUTransferBuffer* indexTransfer{ SDL_CreateGPUTransferBuffer(m_Device, &indexTransferInfo) };

		const bool buffersCreated{ vertexBuffer && indexBuffer && vertexTransfer && indexTransfer && WriteQuadIndices(indexTransfer, newCapacity) };
		if (!buffersCreated)
		{
			CORE_ERROR("[Renderer] Failed to create sprite buffers for {} quads: {}", newCapacity, SDL_GetError());

			if (indexTransfer)
			{
				SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
			}

			if (vertexTransfer)
			{
				SDL_ReleaseGPUTransferBuffer(m_Device, vertexTransfer);
			}

			if (indexBuffer)
			{
				SDL_ReleaseGPUBuffer(m_Device, indexBuffer);
			}

			if (vertexBuffer)
			{
				SDL_ReleaseGPUBuffer(m_Device, vertexBuffer);
			}

			return false;
		}

		if (m_PendingIndexTransferBuffer)
		{
			SDL_ReleaseGPUTransferBuffer(m_Device, m_PendingIndexTransferBuffer);
		}

		if (m_VertexTransferBuffer)
		{
			SDL_ReleaseGPUTransferBuffer(m_Device, m_VertexTransferBuffer);
		}

		if (m_IndexBuffer)
		{
			SDL_ReleaseGPUBuffer(m_Device, m_IndexBuffer);
		}

		if (m_VertexBuffer)
		{
			SDL_ReleaseGPUBuffer(m_Device, m_VertexBuffer);
		}

		if (m_QuadCapacity > 0)
		{
			CORE_INFO("[Renderer] Quad buffers grew from {} to {} quads", m_QuadCapacity, newCapacity);
		}

		m_VertexBuffer = vertexBuffer;
		m_IndexBuffer = indexBuffer;
		m_VertexTransferBuffer = vertexTransfer;
		m_PendingIndexTransferBuffer = indexTransfer;
		m_QuadCapacity = newCapacity;
		m_Statistics.quadCapacity = newCapacity;
		return true;
	}

	void Renderer::RendererData::GrowQuadBuffersIfNeeded(std::uint32_t neededQuadCount)
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

		const std::uint32_t grownCapacity{ std::min(std::max(neededQuadCount, m_QuadCapacity * 2), m_Specification.MaxQuadCapacity) };
		if (!CreateQuadBuffers(grownCapacity))
		{
			m_GrowthFailed = true;
		}
	}

	void Renderer::RendererData::CalculateCullBounds(const glm::mat4& inverseViewProjection)
	{
		constexpr glm::vec4 ndcCorners[4]{
			{ -1.0f, -1.0f, 0.0f, 1.0f },
			{ 1.0f, -1.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f, 0.0f, 1.0f },
			{ -1.0f, 1.0f, 0.0f, 1.0f },
		};

		glm::vec4 firstCorner{ inverseViewProjection * ndcCorners[0] };
		firstCorner /= firstCorner.w;

		float minX{ firstCorner.x };
		float minY{ firstCorner.y };
		float maxX{ firstCorner.x };
		float maxY{ firstCorner.y };

		for (int cornerIndex{ 1 }; cornerIndex < 4; cornerIndex++)
		{
			glm::vec4 worldCorner{ inverseViewProjection * ndcCorners[cornerIndex] };
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
