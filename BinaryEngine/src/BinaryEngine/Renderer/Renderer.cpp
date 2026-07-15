#include "pch.h"
#include "BinaryEngine/Renderer/Renderer.h"

#include "BinaryEngine/Renderer/Shader.h"

namespace {

	constexpr std::uint32_t s_MaxQuads = 10000;
	constexpr std::uint32_t s_VerticesPerQuad = 4;
	constexpr std::uint32_t s_IndicesPerQuad = 6;
	constexpr std::uint32_t s_MaxVertices = s_MaxQuads * s_VerticesPerQuad;
	constexpr std::uint32_t s_MaxIndices = s_MaxQuads * s_IndicesPerQuad;

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

}

namespace BinaryEngine {

	Renderer::Renderer(const Window& window, const RendererSpecification& specification)
		: m_Specification(specification)
	{
		m_Window = static_cast<SDL_Window*>(window.GetNativeWindow());

		constexpr SDL_GPUShaderFormat shaderFormats = SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;
		const char* requestedDriver = RenderAPIToDriverName(m_Specification.renderAPI);
		const bool debugMode = m_Specification.validationMode == ValidationMode::Enabled;

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
	}

	void Renderer::SetPresentMode(PresentMode mode)
	{
		SDL_GPUPresentMode sdlMode = ConvertPresentMode(mode);

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

			SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(m_CommandBuffer, &colorTarget, 1, nullptr);
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

	void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		m_SceneData.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
		m_Draws.clear();
		m_QuadCount = 0;
		m_SceneActive = true;
	}

	void Renderer::EndScene()
	{
		m_SceneActive = false;

		if (!m_CommandBuffer || !m_SwapchainTexture)
		{
			return;
		}

		std::stable_sort(m_Draws.begin(), m_Draws.end(), [](const SpriteDraw& a, const SpriteDraw& b) { return a.sortZ < b.sortZ; });

		m_VertexStaging.clear();
		m_Batches.clear();
		for (const SpriteDraw& draw : m_Draws)
		{
			if (m_Batches.empty() || m_Batches.back().texture != draw.texture)
			{
				m_Batches.push_back(RenderBatch{ draw.texture, m_QuadCount, 0 });
			}

			for (const SpriteVertex& vertex : draw.vertices)
			{
				m_VertexStaging.push_back(vertex);
			}

			m_Batches.back().quadCount++;
			m_QuadCount++;
		}

		bool drawSprites = m_Pipeline && m_Sampler && m_QuadCount > 0;
		if (drawSprites)
		{
			const std::uint32_t usedBytes = m_QuadCount * s_VerticesPerQuad * static_cast<std::uint32_t>(sizeof(SpriteVertex));

			void* mapped = SDL_MapGPUTransferBuffer(m_Device, m_VertexTransferBuffer, true);
			if (mapped)
			{
				SDL_memcpy(mapped, m_VertexStaging.data(), usedBytes);
				SDL_UnmapGPUTransferBuffer(m_Device, m_VertexTransferBuffer);

				SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(m_CommandBuffer);

				SDL_GPUTransferBufferLocation source{
					.transfer_buffer {m_VertexTransferBuffer},
					.offset {0},
				};

				SDL_GPUBufferRegion region{
					.buffer {m_VertexBuffer},
					.offset {0},
					.size {usedBytes},
				};

				SDL_UploadToGPUBuffer(copyPass, &source, &region, true);
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

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(m_CommandBuffer, &colorTarget, 1, nullptr);
		if (renderPass)
		{
			if (drawSprites)
			{
				SDL_GPUViewport viewport{
					0.0f, 0.0f,
					static_cast<float>(m_SwapchainWidth), static_cast<float>(m_SwapchainHeight),
					0.0f, 1.0f
				};
				SDL_SetGPUViewport(renderPass, &viewport);

				SDL_BindGPUGraphicsPipeline(renderPass, m_Pipeline);
				SDL_PushGPUVertexUniformData(m_CommandBuffer, 0, &m_SceneData.ViewProjectionMatrix,
											 static_cast<std::uint32_t>(sizeof(m_SceneData.ViewProjectionMatrix)));

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

				for (const RenderBatch& batch : m_Batches)
				{
					SDL_GPUTextureSamplerBinding textureBinding{
						.texture {batch.texture},
						.sampler {m_Sampler},
					};
					SDL_BindGPUFragmentSamplers(renderPass, 0, &textureBinding, 1);
					SDL_DrawGPUIndexedPrimitives(renderPass, batch.quadCount * s_IndicesPerQuad, 1, batch.firstQuad * s_IndicesPerQuad, 0, 0);
				}
			}

			SDL_EndGPURenderPass(renderPass);
		}

		m_FrameCleared = true;
	}

	void Renderer::DrawTexture(const Texture2D& texture, const Transform& transform)
	{
		if (!m_SceneActive)
		{
			return;
		}

		SDL_GPUTexture* nativeTexture = static_cast<SDL_GPUTexture*>(texture.GetNativeTexture());
		if (!nativeTexture)
		{
			return;
		}

		if (m_QuadCount >= s_MaxQuads)
		{
			CORE_WARN("[Renderer] Sprite batch full ({} quads); dropping draw", s_MaxQuads);
			return;
		}

		Vector2f textureSize{ texture.GetSize() };
		glm::mat4 model{ transform.GetModelMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(textureSize.x, textureSize.y, 1.0f)) };

		constexpr glm::vec3 localVertices[4]{
			{ -0.5f, -0.5f, 0.0f },
			{  0.5f, -0.5f, 0.0f },
			{  0.5f,  0.5f, 0.0f },
			{ -0.5f,  0.5f, 0.0f },
		};

		constexpr glm::vec2 texCoords[4]{
			{ 0.0f, 1.0f },
			{ 1.0f, 1.0f },
			{ 1.0f, 0.0f },
			{ 0.0f, 0.0f },
		};

		const float alpha = static_cast<float>(m_Specification.defaultAlpha) / 255.0f;

		SpriteDraw& draw = m_Draws.emplace_back();
		draw.texture = nativeTexture;
		draw.sortZ = transform.Position.z;

		for (int corner = 0; corner < 4; corner++)
		{
			glm::vec4 worldPosition{ model * glm::vec4(localVertices[corner], 1.0f) };

			SpriteVertex& vertex = draw.vertices[corner];
			vertex.position[0] = worldPosition.x;
			vertex.position[1] = worldPosition.y;
			vertex.position[2] = worldPosition.z;
			vertex.textureCoordinates[0] = texCoords[corner].x;
			vertex.textureCoordinates[1] = texCoords[corner].y;
			vertex.color[0] = 1.0f;
			vertex.color[1] = 1.0f;
			vertex.color[2] = 1.0f;
			vertex.color[3] = alpha;
		}
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

		const std::uint32_t vertexSize{ static_cast<Uint32>(sizeof(SpriteVertex)) };

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

		SDL_GPUBufferCreateInfo vertexBufferInfo{
			.usage {SDL_GPU_BUFFERUSAGE_VERTEX},
			.size {s_MaxVertices * vertexSize},
		};
		m_VertexBuffer = SDL_CreateGPUBuffer(m_Device, &vertexBufferInfo);

		SDL_GPUBufferCreateInfo indexBufferInfo{
			.usage{SDL_GPU_BUFFERUSAGE_INDEX},
			.size{s_MaxIndices * static_cast<std::uint32_t>(sizeof(std::uint32_t))},
		};
		m_IndexBuffer = SDL_CreateGPUBuffer(m_Device, &indexBufferInfo);

		SDL_GPUTransferBufferCreateInfo vertexTransferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {s_MaxVertices * vertexSize},
		};
		m_VertexTransferBuffer = SDL_CreateGPUTransferBuffer(m_Device, &vertexTransferInfo);

		if (!m_VertexBuffer || !m_IndexBuffer || !m_VertexTransferBuffer)
		{
			CORE_ERROR("[Renderer] Failed to create Sprite Buffers: {}", SDL_GetError());
			return;
		}

		std::vector<std::uint32_t> indices(s_MaxIndices);
		for (std::uint32_t quad{}; quad < s_MaxQuads; quad++)
		{
			const std::uint32_t vertex{ quad * s_VerticesPerQuad };
			const std::uint32_t index{ quad * s_IndicesPerQuad };
			indices[index + 0] = vertex + 0;
			indices[index + 1] = vertex + 1;
			indices[index + 2] = vertex + 2;
			indices[index + 3] = vertex + 2;
			indices[index + 4] = vertex + 3;
			indices[index + 5] = vertex + 0;
		}

		const std::uint32_t indexBytes{ s_MaxIndices * static_cast<uint32_t>(sizeof(std::uint32_t)) };

		SDL_GPUTransferBufferCreateInfo indexTransferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {indexBytes},
		};
		SDL_GPUTransferBuffer* indexTransfer = SDL_CreateGPUTransferBuffer(m_Device, &indexTransferInfo);
		if (!indexTransfer)
		{
			CORE_ERROR("[Renderer] Failed to create index transfer buffer: {}", SDL_GetError());
			return;
		}

		void* mapped = SDL_MapGPUTransferBuffer(m_Device, indexTransfer, false);
		if (!mapped)
		{
			CORE_ERROR("[Renderer] Failed to map index transfer buffer: {}", SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);
			return;
		}
		SDL_memcpy(mapped, indices.data(), indexBytes);
		SDL_UnmapGPUTransferBuffer(m_Device, indexTransfer);

		SDL_GPUCommandBuffer* uploadCommands = SDL_AcquireGPUCommandBuffer(m_Device);
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCommands);

		SDL_GPUTransferBufferLocation indexSource{
			.transfer_buffer = indexTransfer,
			.offset = 0,
		};

		SDL_GPUBufferRegion indexRegion{
			.buffer = m_IndexBuffer,
			.offset = 0,
			.size = indexBytes,
		};

		SDL_UploadToGPUBuffer(copyPass, &indexSource, &indexRegion, false);
		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(uploadCommands);
		SDL_ReleaseGPUTransferBuffer(m_Device, indexTransfer);

		m_Draws.reserve(s_MaxQuads);
		m_VertexStaging.reserve(s_MaxVertices);
		m_Batches.reserve(64);
		CORE_INFO("[Renderer] Sprite Renderer Initialised");
	};
}
