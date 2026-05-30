#include "pch.h"
#include "BinaryEngine/Renderer/Texture2D.h"

#include "BinaryEngine/Renderer/Renderer.h"

namespace BinaryEngine {

	Texture2D::~Texture2D()
	{
		if (m_Texture)
		{
			SDL_DestroyTexture(m_Texture);
			m_Texture = nullptr;
			CORE_INFO("[Texture2D] Texture2D Destroyed");
		}
	}

	Texture2D::Texture2D(Texture2D&& other)
		: m_Texture(other.m_Texture), m_Size(other.m_Size)
	{
		other.m_Texture = nullptr;
		other.m_Size = { 0, 0 };
	}

	Texture2D& Texture2D::operator=(Texture2D&& other)
	{
		if (this != &other)
		{
			if (m_Texture)
			{
				SDL_DestroyTexture(m_Texture);
			}

			m_Texture = other.m_Texture;
			m_Size = other.m_Size;

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
		SDL_Renderer* nativeRenderer = static_cast<SDL_Renderer*>(renderer.GetNativeRenderer());

		m_Texture = SDL_CreateTexture(nativeRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, m_Size.x, m_Size.y);

		if (!m_Texture)
		{
			CORE_ERROR("[Texture2D] Failed to create texture: {}, {}", filePath.string(), SDL_GetError());
		}
		else
		{
			int pitch{ m_Size.x * 4 };
			SDL_UpdateTexture(m_Texture, nullptr, textureData, pitch);
			SDL_SetTextureScaleMode(m_Texture, SDL_SCALEMODE_NEAREST);
			CORE_TRACE("[Texture2D] Loaded Texture successfully: {} ({}x{})", filePath.string(), m_Size.x, m_Size.y);
		}

		stbi_image_free(textureData);
	}

}