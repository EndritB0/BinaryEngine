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

		const std::string pathString{ filePath.string() };

		unsigned char* textureData = stbi_load(pathString.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!textureData)
		{
			CORE_ERROR("[Texture2D] Failed to load texture image data: {}", pathString);
			return;
		}

		const bool textureCreated{ Create(renderer, textureData, { width, height }, pathString) };
		stbi_image_free(textureData);

		if (textureCreated)
		{
			CORE_TRACE("[Texture2D] Loaded Texture successfully: {} ({}x{})", pathString, m_Size.x, m_Size.y);
		}
	}

	bool Texture2D::Create(const Renderer& renderer, const void* pixels, Vector2i size, std::string_view debugName)
	{
		if (!pixels || size.x <= 0 || size.y <= 0)
		{
			CORE_ERROR("[Texture2D] Invalid pixel data or size for texture: {} ({}x{})", debugName, size.x, size.y);
			return false;
		}

		SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(renderer.GetNativeDevice());
		if (!device)
		{
			CORE_ERROR("[Texture2D] NO GPU Device avaiable to upload texture, {}", debugName);
			return false;
		}

		const std::uint32_t textureWidth{ static_cast<std::uint32_t>(size.x) };
		const std::uint32_t textureHeight{ static_cast<std::uint32_t>(size.y) };
		const std::uint32_t textureByteCount{ textureWidth * textureHeight * 4 };

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
			CORE_ERROR("[Texture2D] Failed to create GPU texture: {}, {}", debugName, SDL_GetError());
			return false;
		}

		SDL_GPUTransferBufferCreateInfo transferInfo{
			.usage {SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD},
			.size {textureByteCount},
		};

		SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &transferInfo);
		if (!transferBuffer)
		{
			CORE_ERROR("[Texture2D] Failed to create transfer buffer: {}, {}", debugName, SDL_GetError());
			SDL_ReleaseGPUTexture(device, texture);
			return false;
		}

		void* mappedPixels = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
		if (!mappedPixels)
		{
			CORE_ERROR("[Texture2D] Failed to map transfer buffer: {}, {}", debugName, SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			SDL_ReleaseGPUTexture(device, texture);
			return false;
		}

		SDL_memcpy(mappedPixels, pixels, textureByteCount);
		SDL_UnmapGPUTransferBuffer(device, transferBuffer);

		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
		if (!commandBuffer)
		{
			CORE_ERROR("[Texture2D] Failed to acquire upload command buffer: {}, {}", debugName, SDL_GetError());
			SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
			SDL_ReleaseGPUTexture(device, texture);
			return false;
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

		if (m_Texture && m_Device)
		{
			SDL_ReleaseGPUTexture(m_Device, m_Texture);
		}

		m_Device = device;
		m_Texture = texture;
		m_Size = size;
		return true;
	}

}