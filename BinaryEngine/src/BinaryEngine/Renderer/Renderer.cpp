#include "pch.h"
#include "BinaryEngine/Renderer/Renderer.h"

#include "BinaryEngine/Renderer/Font.h"
#include "BinaryEngine/Renderer/Shader.h"

namespace {

	constexpr std::uint32_t s_VerticesPerQuad{ 4 };
	constexpr std::uint32_t s_IndicesPerQuad{ 6 };
	constexpr std::uint32_t s_AbsoluteMaxQuads{ 16000000 };
	constexpr std::uint32_t s_InitialScreenDrawCapacity{ 1024 };

	static const char* RenderAPIToDriverName(BinaryEngine::RenderAPI api)
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

	static SDL_GPUPresentMode ConvertPresentMode(BinaryEngine::PresentMode mode)
	{
		switch (mode)
		{
			case BinaryEngine::PresentMode::VSync: return SDL_GPU_PRESENTMODE_VSYNC;
			case BinaryEngine::PresentMode::Immediate: return SDL_GPU_PRESENTMODE_IMMEDIATE;
			case BinaryEngine::PresentMode::Mailbox: return SDL_GPU_PRESENTMODE_MAILBOX;
			default: return SDL_GPU_PRESENTMODE_VSYNC;
		}
	}

	static std::string_view PresentModeToString(BinaryEngine::PresentMode mode)
	{
		switch (mode)
		{
			case BinaryEngine::PresentMode::VSync: return "VSync";
			case BinaryEngine::PresentMode::Immediate: return "Immediate";
			case BinaryEngine::PresentMode::Mailbox: return "Mailbox";
			default: return "Unknown";
		}
	}

	static void NormaliseSpriteCapacitySpecification(BinaryEngine::RendererSpecification& specification)
	{
		if (specification.maxQuadCapacity > s_AbsoluteMaxQuads)
		{
			CORE_WARN("[Renderer] maxQuadCapacity ({}) exceeds the absolute limit, clamping to {}", specification.maxQuadCapacity, s_AbsoluteMaxQuads);
			specification.maxQuadCapacity = s_AbsoluteMaxQuads;
		}

		if (specification.maxQuadCapacity == 0)
		{
			CORE_WARN("[Renderer] maxQuadCapacity is 0, clamping to 1");
			specification.maxQuadCapacity = 1;
		}

		if (specification.initialQuadCapacity == 0)
		{
			CORE_WARN("[Renderer] initialQuadCapacity is 0, clamping to 1");
			specification.initialQuadCapacity = 1;
		}

		if (specification.initialQuadCapacity > specification.maxQuadCapacity)
		{
			CORE_WARN("[Renderer] initialQuadCapacity ({}) exceeds maxQuadCapacity ({}), clamping", specification.initialQuadCapacity, specification.maxQuadCapacity);
			specification.initialQuadCapacity = specification.maxQuadCapacity;
		}

	}

}

namespace BinaryEngine {

	Renderer::Renderer(const Window& window, const RendererSpecification& specification)
		: m_Specification(specification)
	{
		m_Window = static_cast<SDL_Window*>(window.GetNativeWindow());

		NormaliseSpriteCapacitySpecification(m_Specification);
		m_Statistics.maxQuads = m_Specification.maxQuadCapacity;

		constexpr SDL_GPUShaderFormat shaderFormats{ SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL };
		const char* requestedDriver{ RenderAPIToDriverName(m_Specification.renderAPI) };
		const bool debugMode{ m_Specification.validationMode == ValidationMode::Enabled };

		m_Device = SDL_CreateGPUDevice(shaderFormats, debugMode, requestedDriver);

		if (!m_Device)
		{
			CORE_ERROR("[Renderer] Failed to create GPU Device: {}", SDL_GetError());
			return;
		}

		if (!SDL_ClaimWindowForGPUDevice(m_Device, m_Window))
		{
			CORE_ERROR("[Renderer] Failed to claim window for GPU device: {}", SDL_GetError());
			SDL_DestroyGPUDevice(m_Device);
			m_Device = nullptr;
			return;
		}

		SetPresentMode(m_Specification.presentMode);

		CORE_INFO("[Renderer] Renderer Initialized with API: {}", SDL_GetGPUDeviceDriver(m_Device));

		InitialiseSpriteRenderer();
	}

	Renderer::~Renderer()
	{
		if (m_Device)
		{
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
			if (m_Sampler)
			{
				SDL_ReleaseGPUSampler(m_Device, m_Sampler);
			}
			if (m_Pipeline)
			{
				SDL_ReleaseGPUGraphicsPipeline(m_Device, m_Pipeline);
			}

			if (m_Window)
			{
				SDL_ReleaseWindowFromGPUDevice(m_Device, m_Window);
			}

			SDL_DestroyGPUDevice(m_Device);
			CORE_INFO("[Renderer] Renderer Shutdown");
		}
	}

	void Renderer::SetViewport(Vector2i position, Vector2i size)
	{
		m_ViewportPosition = position;
		m_ViewportSize = size;
		m_HasCustomViewport = true;
	}

	void Renderer::ResetViewport()
	{
		m_ViewportPosition = { 0, 0 };
		m_ViewportSize = { 0, 0 };
		m_HasCustomViewport = false;
	}

	void Renderer::SetPresentMode(PresentMode mode)
	{
		SDL_GPUPresentMode sdlMode{ ConvertPresentMode(mode) };

		if (mode != PresentMode::VSync && !SDL_WindowSupportsGPUPresentMode(m_Device, m_Window, sdlMode))
		{
			CORE_WARN("[Renderer] Requested present mode '{}' is not supported by the window, falling back to VSync", static_cast<int>(mode));
			sdlMode = SDL_GPU_PRESENTMODE_VSYNC;
			mode = PresentMode::VSync;
		}

		if (!SDL_SetGPUSwapchainParameters(m_Device, m_Window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, sdlMode))
		{
			CORE_ERROR("[Renderer] Failed to set GPU Swapchain Parameters: {}", SDL_GetError());
			return;
		}

		m_Specification.presentMode = mode;
		CORE_TRACE("[Renderer] Present mode set to '{}'", PresentModeToString(mode));
	}

	void Renderer::SetDefaultAlpha(std::uint8_t alpha)
	{
		m_Specification.defaultAlpha = alpha;
		CORE_TRACE("[Renderer] Default Alpha set to {}", alpha);
	}

	void Renderer::Clear()
	{
		m_FrameCleared = false;
		m_SwapchainTexture = nullptr;

		m_CommandBuffer = SDL_AcquireGPUCommandBuffer(m_Device);
		if (!m_CommandBuffer)
		{
			CORE_ERROR("[Renderer] Failed to acquire GPU command buffer: {}", SDL_GetError());
			return;
		}

		if (!SDL_WaitAndAcquireGPUSwapchainTexture(m_CommandBuffer, m_Window, &m_SwapchainTexture, &m_SwapchainWidth, &m_SwapchainHeight))
		{
			CORE_ERROR("[Renderer] Failed to acquire swapchain texture: {}", SDL_GetError());
			m_SwapchainTexture = nullptr;
		}
	}

	void Renderer::Present()
	{
		if (!m_CommandBuffer)
		{
			return;
		}

		if (!m_FrameCleared && m_SwapchainTexture)
		{
			SDL_FColor clearColor{
				static_cast<float>(m_ClearColor.red) / 255.0f,
				static_cast<float>(m_ClearColor.green) / 255.0f,
				static_cast<float>(m_ClearColor.blue) / 255.0f,
				static_cast<float>(m_ClearColor.alpha) / 255.0f
			};

			SDL_GPUColorTargetInfo colorTarget{
				.texture {m_SwapchainTexture},
				.clear_color {clearColor},
				.load_op {SDL_GPU_LOADOP_CLEAR},
				.store_op {SDL_GPU_STOREOP_STORE},
			};

			SDL_GPURenderPass* renderPass{ SDL_BeginGPURenderPass(m_CommandBuffer, &colorTarget, 1, nullptr) };
			if (renderPass)
			{
				SDL_EndGPURenderPass(renderPass);
			}
			m_FrameCleared = true;
		}

		SDL_SubmitGPUCommandBuffer(m_CommandBuffer);
		m_CommandBuffer = nullptr;
		m_SwapchainTexture = nullptr;
	}

	void Renderer::SetClearColor(const Color& color)
	{
		m_ClearColor = color;
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		const Vector2i swapchainSize{ static_cast<int>(m_SwapchainWidth), static_cast<int>(m_SwapchainHeight) };
		if (swapchainSize.x > 0 && swapchainSize.y > 0 && camera.GetWindowSize() != swapchainSize)
		{
			camera.OnResize(swapchainSize);
		}

		m_SceneData.ViewProjectionMatrix = camera.GetViewProjectionMatrix();

		const Vector2f screenViewSize{ camera.GetScreenViewSize() };
		m_SceneData.ScreenViewProjectionMatrix = glm::ortho(0.0f, screenViewSize.x, screenViewSize.y, 0.0f, -1.0f, 1.0f);

		m_Draws.clear();
		m_ScreenDraws.clear();
		m_QuadCount = 0;
		m_SceneActive = true;

		m_Statistics.quadCount = 0;
		m_Statistics.drawCalls = 0;
		m_Statistics.culledSprites = 0;
		m_Statistics.glyphCount = 0;


		if (m_HasCustomViewport)
		{
			m_SceneViewport = { m_ViewportPosition, m_ViewportSize };
		}
		else
		{
			m_SceneViewport = camera.CalculateViewport({ static_cast<int>(m_SwapchainWidth), static_cast<int>(m_SwapchainHeight) });
		}

		CalculateCullBounds(camera.GetInverseViewProjectionMatrix());
	}

	void Renderer::EndScene()
	{
		m_SceneActive = false;

		if (!m_CommandBuffer || !m_SwapchainTexture)
		{
			m_Draws.clear();
			m_ScreenDraws.clear();
			return;
		}

		std::uint32_t neededCapacity{ static_cast<std::uint32_t>(m_Draws.size() + m_ScreenDraws.size()) };
		if (neededCapacity > m_Specification.maxQuadCapacity)
		{
			CORE_WARN("[Renderer] Scene requested {} quads, ceiling is {}, dropping {}", neededCapacity, m_Specification.maxQuadCapacity, neededCapacity - m_Specification.maxQuadCapacity);
			neededCapacity = m_Specification.maxQuadCapacity;
		}

		if (neededCapacity > m_QuadCapacity)
		{
			EnsureQuadCapacity(neededCapacity);
		}

		const std::uint32_t drawableQuadCount{ std::min(neededCapacity, m_QuadCapacity) };

		const std::uint32_t worldDrawableQuadCount{ std::min(static_cast<std::uint32_t>(m_Draws.size()), drawableQuadCount) };
		const std::uint32_t screenDrawableQuadCount{ std::min(static_cast<std::uint32_t>(m_ScreenDraws.size()), drawableQuadCount - worldDrawableQuadCount) };

		const auto sortByDepth = [](const SpriteDraw& left, const SpriteDraw& right) { return left.sortZ < right.sortZ; };
		std::stable_sort(m_Draws.begin(), m_Draws.end(), sortByDepth);
		std::stable_sort(m_ScreenDraws.begin(), m_ScreenDraws.end(), sortByDepth);

		m_VertexStaging.clear();
		m_Batches.clear();

		AppendBatches(m_Draws, worldDrawableQuadCount, false);
		const std::size_t worldBatchCount{ m_Batches.size() };
		AppendBatches(m_ScreenDraws, screenDrawableQuadCount, true);

		m_Statistics.quadCount = m_QuadCount;
		m_Statistics.drawCalls = static_cast<std::uint32_t>(m_Batches.size());

		bool drawSprites{ m_Pipeline && m_Sampler && m_QuadCount > 0 };
		if (drawSprites)
		{
			const std::uint32_t vertexBytes{ m_QuadCount * s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(SpriteVertex)) };

			void* mappedVertices{ SDL_MapGPUTransferBuffer(m_Device, m_VertexTransferBuffer, true) };
			if (mappedVertices)
			{
				SDL_memcpy(mappedVertices, m_VertexStaging.data(), vertexBytes);
				SDL_UnmapGPUTransferBuffer(m_Device, m_VertexTransferBuffer);

				SDL_GPUCopyPass* copyPass{ SDL_BeginGPUCopyPass(m_CommandBuffer) };

				SDL_GPUTransferBufferLocation vertexSource{
					.transfer_buffer {m_VertexTransferBuffer},
					.offset {0},
				};

				SDL_GPUBufferRegion vertexRegion{
					.buffer {m_VertexBuffer},
					.offset {0},
					.size {vertexBytes},
				};

				SDL_UploadToGPUBuffer(copyPass, &vertexSource, &vertexRegion, true);
				SDL_EndGPUCopyPass(copyPass);
			}
			else
			{
				drawSprites = false;
				CORE_ERROR("[Renderer] Failed to map vertex transfer buffer: {}", SDL_GetError());
			}
		}

		SDL_FColor clearColor{
			static_cast<float>(m_ClearColor.red) / 255.0f,
			static_cast<float>(m_ClearColor.green) / 255.0f,
			static_cast<float>(m_ClearColor.blue) / 255.0f,
			static_cast<float>(m_ClearColor.alpha) / 255.0f
		};

		SDL_GPUColorTargetInfo colorTarget{
			.texture = m_SwapchainTexture,
			.clear_color = clearColor,
			.load_op {m_FrameCleared ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR},
			.store_op {SDL_GPU_STOREOP_STORE},
		};

		SDL_GPURenderPass* renderPass{ SDL_BeginGPURenderPass(m_CommandBuffer, &colorTarget, 1, nullptr) };
		if (renderPass)
		{
			const bool useSceneViewport{ m_SceneViewport.Size.x > 0 && m_SceneViewport.Size.y > 0 };

			SDL_GPUViewport viewport{
				useSceneViewport ? static_cast<float>(m_SceneViewport.Position.x) : 0.0f,
				useSceneViewport ? static_cast<float>(m_SceneViewport.Position.y) : 0.0f,
				useSceneViewport ? static_cast<float>(m_SceneViewport.Size.x) : static_cast<float>(m_SwapchainWidth),
				useSceneViewport ? static_cast<float>(m_SceneViewport.Size.y) : static_cast<float>(m_SwapchainHeight),
				0.0f, 1.0f
			};
			SDL_SetGPUViewport(renderPass, &viewport);

			if (drawSprites)
			{
				SDL_BindGPUGraphicsPipeline(renderPass, m_Pipeline);

				SDL_GPUBufferBinding vertexBinding{
					.buffer {m_VertexBuffer},
					.offset {0},
				};
				SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);

				SDL_GPUBufferBinding indexBinding{
					.buffer {m_IndexBuffer},
					.offset {0},
				};
				SDL_BindGPUIndexBuffer(renderPass, &indexBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

				const auto drawBatchRange = [&](std::size_t firstBatchIndex, std::size_t endBatchIndex) {
					for (std::size_t batchIndex{ firstBatchIndex }; batchIndex < endBatchIndex; batchIndex++)
					{
						const RenderBatch& batch{ m_Batches[batchIndex] };

						SDL_GPUTextureSamplerBinding textureBinding{
							.texture {batch.texture},
							.sampler {m_Sampler},
						};
						SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);
						SDL_DrawGPUIndexedPrimitives(renderPass, batch.quadCount * s_IndicesPerQuad, 1, batch.firstQuad * s_IndicesPerQuad, 0, 0);
					}
					};

				SDL_PushGPUVertexUniformData(m_CommandBuffer, 0, &m_SceneData.ViewProjectionMatrix,
											 static_cast<std::uint32_t>(sizeof(m_SceneData.ViewProjectionMatrix)));
				drawBatchRange(0, worldBatchCount);

				if (worldBatchCount < m_Batches.size())
				{
					SDL_PushGPUVertexUniformData(m_CommandBuffer, 0, &m_SceneData.ScreenViewProjectionMatrix,
												 static_cast<std::uint32_t>(sizeof(m_SceneData.ScreenViewProjectionMatrix)));
					drawBatchRange(worldBatchCount, m_Batches.size());
				}
			}

			SDL_EndGPURenderPass(renderPass);
		}

		m_FrameCleared = true;
	}

	void Renderer::DrawSprite(const Texture2D& texture, const Transform& transform)
	{
		Vector2f textureSize{ texture.GetSize() };
		SubmitSprite(texture, transform, textureSize, { 0.0f, 0.0f }, { 1.0f, 1.0f });
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

		SubmitSprite(texture, transform, regionSize, uvMin, uvMax);
	}

	void Renderer::SubmitSprite(const Texture2D& texture, const Transform& transform, Vector2f quadSize, Vector2f uvMin, Vector2f uvMax)
	{
		if (!m_SceneActive)
		{
			return;
		}

		SDL_GPUTexture* nativeTexture{ static_cast<SDL_GPUTexture*>(texture.GetNativeTexture()) };
		if (!nativeTexture)
		{
			return;
		}

		if (m_Specification.cullSprites && m_CullBounds.valid && IsSpriteCulled(transform, quadSize))
		{
			m_Statistics.culledSprites++;
			return;
		}

		glm::mat4 modelMatrix{ transform.GetModelMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(quadSize.x, quadSize.y, 1.0f)) };

		constexpr glm::vec3 localVertices[4]{
			{ -0.5f, -0.5f, 0.0f },
			{  0.5f, -0.5f, 0.0f },
			{  0.5f,  0.5f, 0.0f },
			{ -0.5f,  0.5f, 0.0f },
		};

		const glm::vec2 quadTextureCoordinates[4]{
			{ uvMin.x, uvMax.y },
			{ uvMax.x, uvMax.y },
			{ uvMax.x, uvMin.y },
			{ uvMin.x, uvMin.y },
		};

		const float alpha{ static_cast<float>(m_Specification.defaultAlpha) / 255.0f };

		SpriteDraw& draw{ m_Draws.emplace_back() };
		draw.texture = nativeTexture;
		draw.sortZ = transform.Position.z;

		for (int cornerIndex{}; cornerIndex < 4; cornerIndex++)
		{
			glm::vec4 worldPosition{ modelMatrix * glm::vec4(localVertices[cornerIndex], 1.0f) };

			SpriteVertex& vertex{ draw.vertices[cornerIndex] };
			vertex.position[0] = worldPosition.x;
			vertex.position[1] = worldPosition.y;
			vertex.position[2] = worldPosition.z;
			vertex.textureCoordinates[0] = quadTextureCoordinates[cornerIndex].x;
			vertex.textureCoordinates[1] = quadTextureCoordinates[cornerIndex].y;
			vertex.color[0] = 1.0f;
			vertex.color[1] = 1.0f;
			vertex.color[2] = 1.0f;
			vertex.color[3] = alpha;
		}
	}

	void Renderer::AppendBatches(const std::vector<SpriteDraw>& draws, std::uint32_t drawableQuadCount, bool forceNewBatch)
	{
		for (std::uint32_t drawIndex{}; drawIndex < drawableQuadCount; drawIndex++)
		{
			const SpriteDraw& draw{ draws[drawIndex] };

			const bool startNewBatch{ m_Batches.empty() || m_Batches.back().texture != draw.texture || (forceNewBatch && drawIndex == 0) };

			if (startNewBatch)
			{
				m_Batches.emplace_back(draw.texture, m_QuadCount, 0);
			}

			for (const SpriteVertex& vertex : draw.vertices)
			{
				m_VertexStaging.push_back(vertex);
			}

			m_Batches.back().quadCount++;
			m_QuadCount++;
		}
	}

	void Renderer::DrawText(const Font& font, std::string_view text, const Transform& transform, const TextSpecification& specification)
	{
		if (!m_SceneActive || !font.IsValid())
		{
			return;
		}

		SDL_GPUTexture* nativeTexture{ static_cast<SDL_GPUTexture*>(font.GetTexture().GetNativeTexture()) };
		if (!nativeTexture)
		{
			return;
		}

		LayoutText(font, text, specification, m_TextLayout);
		if (m_TextLayout.Glyphs.empty())
		{
			return;
		}

		const float glyphScale{ specification.Size / font.GetPixelSize() };

		const bool isWorldSpace{ specification.Space == TextSpace::World };
		const float verticalSign{ isWorldSpace ? -1.0f : 1.0f };

		const glm::mat4 modelMatrix{ transform.GetModelMatrix() };

		const float red{ static_cast<float>(specification.FillColor.red) / 255.0f };
		const float green{ static_cast<float>(specification.FillColor.green) / 255.0f };
		const float blue{ static_cast<float>(specification.FillColor.blue) / 255.0f };
		const float alpha{ static_cast<float>(specification.FillColor.alpha) / 255.0f };

		std::vector<SpriteDraw>& targetDrawList{ isWorldSpace ? m_Draws : m_ScreenDraws };

		for (const LaidOutGlyph& glyph : m_TextLayout.Glyphs)
		{
			const float left{ glyph.Position.x * glyphScale };
			const float right{ (glyph.Position.x + glyph.Size.x) * glyphScale };
			const float top{ glyph.Position.y * glyphScale * verticalSign };
			const float bottom{ (glyph.Position.y + glyph.Size.y) * glyphScale * verticalSign };

			const glm::vec2 glyphCorners[4]{
				{ left, top },
				{ right, top },
				{ right, bottom },
				{ left, bottom },
			};

			const glm::vec2 glyphTextureCoordinates[4]{
				{ glyph.UVMin.x, glyph.UVMin.y },
				{ glyph.UVMax.x, glyph.UVMin.y },
				{ glyph.UVMax.x, glyph.UVMax.y },
				{ glyph.UVMin.x, glyph.UVMax.y },
			};

			SpriteDraw& draw{ targetDrawList.emplace_back() };
			draw.texture = nativeTexture;
			draw.sortZ = transform.Position.z;

			for (int cornerIndex{}; cornerIndex < 4; cornerIndex++)
			{
				const glm::vec4 transformedPosition{ modelMatrix * glm::vec4(glyphCorners[cornerIndex], 0.0f, 1.0f) };

				SpriteVertex& vertex{ draw.vertices[cornerIndex] };
				vertex.position[0] = transformedPosition.x;
				vertex.position[1] = transformedPosition.y;
				vertex.position[2] = transformedPosition.z;
				vertex.textureCoordinates[0] = glyphTextureCoordinates[cornerIndex].x;
				vertex.textureCoordinates[1] = glyphTextureCoordinates[cornerIndex].y;
				vertex.color[0] = red;
				vertex.color[1] = green;
				vertex.color[2] = blue;
				vertex.color[3] = alpha;
			}

			m_Statistics.glyphCount++;
		}
	}

	Vector2f Renderer::MeasureText(const Font& font, std::string_view text, const TextSpecification& specification)
	{
		if (!font.IsValid())
		{
			return { 0.0f, 0.0f };
		}

		LayoutText(font, text, specification, m_TextLayout);

		return m_TextLayout.Bounds * (specification.Size / font.GetPixelSize());
	}

	void Renderer::InitialiseSpriteRenderer()
	{
		Shader vertexShader;
		vertexShader.Load("./resources/shader/sprite/sprite.vert", *this, { ShaderStage::Vertex, 0, 1 });

		Shader fragmentShader;
		fragmentShader.Load("./resources/shader/sprite/sprite.frag", *this, { ShaderStage::Fragment, 1, 0 });

		if (!vertexShader.IsValid() || !fragmentShader.IsValid())
		{
			CORE_ERROR("[Renderer] Failed to initialize sprite renderer");
			return;
		}

		const std::uint32_t vertexSize{ static_cast<std::uint32_t>(sizeof(SpriteVertex)) };

		SDL_GPUVertexBufferDescription vertexBufferDescription{
			.slot {0},
			.pitch {vertexSize},
			.input_rate {SDL_GPU_VERTEXINPUTRATE_VERTEX},
			.instance_step_rate {0},
		};

		SDL_GPUVertexAttribute vertexAttributes[3]{
			{
				.location {0},
				.buffer_slot {0},
				.format {SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3},
				.offset {0},
			},

			{
				.location {1},
				.buffer_slot {0},
				.format {SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2},
				.offset {static_cast<std::uint32_t>(sizeof(float) * 3)},
			},

			{
				.location {2},
				.buffer_slot {0},
				.format {SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4},
				.offset {static_cast<std::uint32_t>(sizeof(float) * 5)},
			},
		};

		SDL_GPUColorTargetBlendState blendState{
			.src_color_blendfactor {SDL_GPU_BLENDFACTOR_SRC_ALPHA},
			.dst_color_blendfactor {SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA},
			.color_blend_op {SDL_GPU_BLENDOP_ADD},
			.src_alpha_blendfactor {SDL_GPU_BLENDFACTOR_ONE},
			.dst_alpha_blendfactor {SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA},
			.alpha_blend_op {SDL_GPU_BLENDOP_ADD},
			.enable_blend {true},
		};

		SDL_GPUColorTargetDescription colorTargetDescription{
			.format {SDL_GetGPUSwapchainTextureFormat(m_Device, m_Window)},
			.blend_state {blendState},
		};

		SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{
			.vertex_shader {static_cast<SDL_GPUShader*>(vertexShader.GetNativeShader())},
			.fragment_shader {static_cast<SDL_GPUShader*>(fragmentShader.GetNativeShader())},
			.vertex_input_state {
				.vertex_buffer_descriptions {&vertexBufferDescription},
				.num_vertex_buffers {1},
				.vertex_attributes {vertexAttributes},
				.num_vertex_attributes {3},
			},
			.primitive_type {SDL_GPU_PRIMITIVETYPE_TRIANGLELIST},
			.target_info {
				.color_target_descriptions {&colorTargetDescription},
				.num_color_targets {1},
			},
		};

		m_Pipeline = SDL_CreateGPUGraphicsPipeline(m_Device, &pipelineInfo);
		if (!m_Pipeline)
		{
			CORE_ERROR("[Renderer] Failed to create Graphics Pipeline {}", SDL_GetError());
			return;
		}

		const bool useLinearFiltering{ m_Specification.textureFilter == TextureFilter::Linear };
		const SDL_GPUFilter filter{ useLinearFiltering ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST };
		const SDL_GPUSamplerMipmapMode mipmapMode{ useLinearFiltering ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST };

		SDL_GPUSamplerCreateInfo samplerInfo{
			.min_filter {filter},
			.mag_filter {filter},
			.mipmap_mode {mipmapMode},
			.address_mode_u {SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE},
			.address_mode_v {SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE},
			.address_mode_w {SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE},
		};

		m_Sampler = SDL_CreateGPUSampler(m_Device, &samplerInfo);
		if (!m_Sampler)
		{
			CORE_ERROR("[Renderer] Failed to create sampler: {}", SDL_GetError());
			return;
		}

		if (!EnsureQuadCapacity(m_Specification.initialQuadCapacity))
		{
			return;
		}

		m_Draws.reserve(m_Specification.maxQuadCapacity);
		m_ScreenDraws.reserve(s_InitialScreenDrawCapacity);
		m_VertexStaging.reserve(m_Specification.maxQuadCapacity * s_VerticesPerQuad);
		m_Batches.reserve(64);
		CORE_INFO("[Renderer] Sprite Renderer Initialised ({} quad capacity, {} max)", m_QuadCapacity, m_Specification.maxQuadCapacity);
	}

	bool Renderer::UploadQuadIndices(struct SDL_GPUBuffer* target, std::uint32_t quadCapacity)
	{
		const std::uint32_t indexCount{ quadCapacity * s_IndicesPerQuad };
		const std::uint32_t indexBytes{ indexCount * static_cast<std::uint32_t>(sizeof(std::uint32_t)) };

		std::vector<std::uint32_t> indices(indexCount);
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

		SDL_GPUTransferBufferCreateInfo indexTransferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {indexBytes},
		};
		SDL_GPUTransferBuffer* indexTransfer{ SDL_CreateGPUTransferBuffer(m_Device, &indexTransferInfo) };
		if (!indexTransfer)
		{
			CORE_ERROR("[Renderer] Failed to create index transfer buffer: {}", SDL_GetError());
			return false;
		}

		void* mappedIndices{ SDL_MapGPUTransferBuffer(m_Device, indexTransfer, false) };
		if (!mappedIndices)
		{
			CORE_ERROR("[Renderer] Failed to map index transfer buffer: {}", SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
			return false;
		}
		SDL_memcpy(mappedIndices, indices.data(), indexBytes);
		SDL_UnmapGPUTransferBuffer(m_Device, indexTransfer);

		SDL_GPUCommandBuffer* uploadCommands{ SDL_AcquireGPUCommandBuffer(m_Device) };
		if (!uploadCommands)
		{
			CORE_ERROR("[Renderer] Failed to acquire command buffer for index upload: {}", SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
			return false;
		}

		SDL_GPUCopyPass* copyPass{ SDL_BeginGPUCopyPass(uploadCommands) };
		if (!copyPass)
		{
			CORE_ERROR("[Renderer] Failed to begin copy pass for index upload: {}", SDL_GetError());
			SDL_SubmitGPUCommandBuffer(uploadCommands);
			SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
			return false;
		}

		SDL_GPUTransferBufferLocation indexSource{
			.transfer_buffer {indexTransfer},
			.offset {0},
		};

		SDL_GPUBufferRegion indexRegion{
			.buffer {target},
			.offset {0},
			.size {indexBytes},
		};

		SDL_UploadToGPUBuffer(copyPass, &indexSource, &indexRegion, false);
		SDL_EndGPUCopyPass(copyPass);

		const bool uploadSubmitted{ SDL_SubmitGPUCommandBuffer(uploadCommands) };
		if (!uploadSubmitted)
		{
			CORE_ERROR("[Renderer] Failed to submit index upload: {}", SDL_GetError());
		}

		SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
		return uploadSubmitted;
	}

	bool Renderer::EnsureQuadCapacity(std::uint32_t newCapacity)
	{
		const std::uint32_t vertexBytes{ newCapacity * s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(SpriteVertex)) };
		const std::uint32_t indexBytes{ newCapacity * s_IndicesPerQuad * static_cast<std::uint32_t>(sizeof(std::uint32_t)) };

		SDL_GPUBufferCreateInfo vertexBufferInfo{
			.usage {SDL_GPU_BUFFERUSAGE_VERTEX},
			.size {vertexBytes},
		};
		SDL_GPUBuffer* vertexBuffer{ SDL_CreateGPUBuffer(m_Device, &vertexBufferInfo) };

		SDL_GPUBufferCreateInfo indexBufferInfo{
			.usage {SDL_GPU_BUFFERUSAGE_INDEX},
			.size {indexBytes},
		};
		SDL_GPUBuffer* indexBuffer{ SDL_CreateGPUBuffer(m_Device, &indexBufferInfo) };

		SDL_GPUTransferBufferCreateInfo vertexTransferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {vertexBytes},
		};
		SDL_GPUTransferBuffer* vertexTransfer{ SDL_CreateGPUTransferBuffer(m_Device, &vertexTransferInfo) };

		const bool buffersCreated{ vertexBuffer && indexBuffer && vertexTransfer };
		if (!buffersCreated)
		{
			CORE_ERROR("[Renderer] Failed to create sprite buffers for {} quads: {}", newCapacity, SDL_GetError());
		}

		if (!buffersCreated || !UploadQuadIndices(indexBuffer, newCapacity))
		{
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

		m_VertexBuffer = vertexBuffer;
		m_IndexBuffer = indexBuffer;
		m_VertexTransferBuffer = vertexTransfer;
		m_QuadCapacity = newCapacity;
		m_Statistics.quadCapacity = newCapacity;
		return true;
	}

	void Renderer::CalculateCullBounds(const glm::mat4& inverseViewProjection)
	{
		constexpr glm::vec4 ndcCorners[4]{
			{ -1.0f, -1.0f, 0.0f, 1.0f },
			{  1.0f, -1.0f, 0.0f, 1.0f },
			{  1.0f,  1.0f, 0.0f, 1.0f },
			{ -1.0f,  1.0f, 0.0f, 1.0f },
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

	bool Renderer::IsSpriteCulled(const Transform& transform, const Vector2f& textureSize) const
	{
		const float halfWidth{ textureSize.x * transform.Scale.x * 0.5f };
		const float halfHeight{ textureSize.y * transform.Scale.y * 0.5f };
		const float radius{ std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight) };

		return transform.Position.x + radius < m_CullBounds.minX
			|| transform.Position.x - radius > m_CullBounds.maxX
			|| transform.Position.y + radius < m_CullBounds.minY
			|| transform.Position.y - radius > m_CullBounds.maxY;
	}

}
