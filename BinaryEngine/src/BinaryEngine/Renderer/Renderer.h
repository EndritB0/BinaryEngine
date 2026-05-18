#pragma once

#include "BinaryEngine/Window/Window.h"
#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Renderer/Texture2D.h"

namespace BinaryEngine {

	class Renderer {
	public:
		Renderer(const Window& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void* GetNativeRenderer() const { return m_Renderer; }
		void Clear();
		void Present();
		void SetDrawColor(Color color);
		void DrawTexture(const Texture2D& texture, const Vector2f position, const Vector2f size);

	private:
		struct SDL_Renderer* m_Renderer{ nullptr };

	};

}