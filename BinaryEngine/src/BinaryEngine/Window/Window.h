#pragma once

#include "BinaryEngine/Window/WindowSpecification.h"

namespace BinaryEngine {

	class Window {
	public:
		Window(const WindowSpecification& specification);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		void* GetNativeWindow() const { return static_cast<void*>(m_Window); }
		const WindowSpecification GetSpecification() const { return m_WindowSpecification; }

	private:
		WindowSpecification m_WindowSpecification;
		struct SDL_Window* m_Window{ nullptr };
	};

}