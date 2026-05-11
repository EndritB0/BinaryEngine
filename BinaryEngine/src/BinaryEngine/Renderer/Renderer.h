#pragma once

#include "BinaryEngine/Window/Window.h"

namespace BinaryEngine {

	class Renderer {
	public:
		Renderer(const Window& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void Clear();
		void Present();
		void SetDrawColor(Color color);

	private:
		struct SDL_Renderer* m_Renderer{ nullptr };

	};

}