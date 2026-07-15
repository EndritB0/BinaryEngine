#include "pch.h"
#include "BinaryEngine/Renderer/Texture2D.h"

#include "BinaryEngine/Renderer/Renderer.h"

namespace BinaryEngine {

	Texture2D::~Texture2D()
	{
		if (m_Texture && m_Device)
		{
			SDL_ReleaseGPUTexture(m_Device, m_Texture);
			m_Texture = nullptr;
			CORE_INFO("[Texture2D] Texture2D Destroyed");
		}
	}

	Texture2D::Texture2D(Texture2D&& other)
		: m_Device(other.m_Device), m_Texture(other.m_Texture), m_Size(other.m_Size)
	{
		other.m_Device = nullptr;
		other.m_Texture = nullptr;
		other.m_Size = { 0, 0 };
	}

	Texture2D& Texture2D::operator=(Texture2D&& other)
	{
		if (this != &other)
		{
			if (m_Texture && m_Device)
			{
				SDL_ReleaseGPUTexture(m_Device, m_Texture);
			}

			m_Device = other.m_Device;
			m_Texture = other.m_Texture;
			m_Size = other.m_Size;

			other.m_Device = nullptr;
			other.m_Texture = nullptr;
			other.m_Size = { 0, 0 };
		}

		return *this;
	}

	void Texture2D::Load(const std::filesystem::path& filePath, const Renderer& renderer)
	{
		int width{};
		int height{};
		int channels{};

		stbi_set_flip_vertically_on_load(0);

		unsigned char* textureData = stbi_load(filePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!textureData)
		{
			CORE_ERROR("[Texture2D] Failed to load texture image data: {}", filePath.string());
			return;
		}

		m_Size = { width, height };

		SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(renderer.GetNativeDevice());
		if (!device)
		{
			CORE_ERROR("[Texture2D] NO GPU Device avaiable to upload texture, {}", filePath.string());
			stbi_image_free(textureData);
			return;
		}

		const std::uint32_t textureWidth{ static_cast<std::uint32_t>(m_Size.x) };
		const std::uint32_t textureHeight{ static_cast<std::uint32_t>(m_Size.y) };
		const std::uint32_t textureSize{ textureWidth * textureHeight * 4 };

		SDL_GPUTextureCreateInfo textureInfo{
			.type {SDL_GPU_TEXTURETYPE_2D},
			.format {SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM},
			.usage {SDL_GPU_TEXTUREUSAGE_SAMPLER},
			.width {textureWidth},
			.height {textureHeight},
			.layer_count_or_depth {1},
			.num_levels {1},
			.sample_count {SDL_GPU_SAMPLECOUNT_1},
		};

		SDL_GPUTexture* texture = SDL_CreateGPUTexture(device, &textureInfo);
		if (!texture)
		{
			CORE_ERROR("[Texture2D] Failed to create GPU texture: {}, {}", filePath.string(), SDL_GetError());
			stbi_image_free(textureData);
			return;
		}

		SDL_GPUTransferBufferCreateInfo transferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {textureSize},
		};

		SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
		if (!transferBuffer)
		{
			CORE_ERROR("[Texture2D] Failed to create transfer buffer: {}, {}", filePath.string(), SDL_GetError());
			SDL_ReleaseGPUTexture(device, texture);
			stbi_image_free(textureData);
			return;
		}

		void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
		if (!mapped)
		{
			CORE_ERROR("[Texture2D] Failed to map transfer buffer: {}, {}", filePath.string(), SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			SDL_ReleaseGPUTexture(device, texture);
			stbi_image_free(textureData);
			return;
		}

		SDL_memcpy(mapped, textureData, textureSize);
		SDL_UnmapGPUTransferBuffer(device, transferBuffer);
		stbi_image_free(textureData);

		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
		if (!commandBuffer)
		{
			CORE_ERROR("[Texture2D] Failed to acquire upload command buffer: {}, {}", filePath.string(), SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			SDL_ReleaseGPUTexture(device, texture);
			return;
		}

		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

		SDL_GPUTextureTransferInfo source{
			.transfer_buffer {transferBuffer},
			.offset {0},
			.pixels_per_row {textureWidth},
			.rows_per_layer {textureHeight},
		};

		SDL_GPUTextureRegion destination{
			.texture {texture},
			.w {textureWidth},
			.h {textureHeight},
			.d {1},
		};

		SDL_UploadToGPUTexture(copyPass, &source, &destination, false);
		SDL_EndGPUCopyPass(copyPass);
		SDL_SubmitGPUCommandBuffer(commandBuffer);
		SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

		m_Device = device;
		m_Texture = texture;
		CORE_TRACE("[Texture2D] Loaded Texture successfully: {} ({}x{})", filePath.string(), m_Size.x, m_Size.y);
	}

}