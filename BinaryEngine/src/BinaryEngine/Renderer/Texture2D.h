#pragma once

#include <filesystem>

namespace BinaryEngine {

	class Renderer;

	class Texture2D {
	public:
		Texture2D() = default;
		~Texture2D();

		Texture2D(const Texture2D&) = delete;
		Texture2D& operator=(const Texture2D&) = delete;
		Texture2D(Texture2D&&);
		Texture2D& operator=(Texture2D&&);

		void LoadFromFile(const Renderer& renderer, const std::filesystem::path& filePath);
		int GetWidth() const { return m_Size.x; }
		int GetHeight() const { return m_Size.y; }
		Vector2i GetSize() const { return m_Size; }
		void* GetNativeTexture() const { return m_Texture; }

	private:
		struct SDL_Texture* m_Texture{ nullptr };
		Vector2i m_Size{ 0, 0 };
	};

}