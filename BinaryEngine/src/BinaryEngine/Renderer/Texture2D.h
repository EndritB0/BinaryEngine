#pragma once

#include <filesystem>

#include "BinaryEngine/Asset/Asset.h"
#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class Renderer;

	class Texture2D : public Asset {
	public:
		Texture2D() = default;
		virtual ~Texture2D() override;

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&);
		Texture2D& operator=(Texture2D&&);

		static AssetType GetStaticAssetType() { return AssetType::Texture2D; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		void Load(const std::filesystem::path& filePath, const Renderer& renderer);
		int GetWidth() const { return m_Size.x; }
		int GetHeight() const { return m_Size.y; }
		Vector2i GetSize() const { return m_Size; }
		void* GetNativeTexture() const { return m_Texture; }

	private:
		struct SDL_GPUDevice* m_Device{ nullptr };
		struct SDL_GPUTexture* m_Texture{ nullptr };
		Vector2i m_Size{ 0, 0 };
	};

}