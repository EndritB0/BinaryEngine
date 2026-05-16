#pragma once

#include <functional>
#include <string>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Window/WindowSpecification.h"
#include "BinaryEngine/Event/Event.h"

namespace BinaryEngine {

	class Window {
	public:
		using EventCallback = std::function<void(Event&)>;

		Window(const WindowSpecification& specification);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		void* GetNativeWindow() const { return static_cast<void*>(m_Window); }
		const WindowSpecification GetSpecification() const { return m_WindowSpecification; }
		int GetWidth() const;
		int GetHeight() const;
		Vector2i GetResolution() const;
		bool IsFullscreen() const { return m_WindowSpecification.fullscreen; }
		void SetTitle(const std::string& title);
		void SetSize(const Vector2i& size);
		void SetFullscreen(bool fullscreen);
		void StartTextInput();
		void StopTextInput();
		bool IsTextInputActive() const;
		bool IsMinimized() const { return m_IsMinimized; }
		bool IsMaximized() const { return m_IsMaximized; }
		void SetEventCallback(const EventCallback& callback) { m_EventCallback = callback; }
		void ProcessEvents();

	private:
		WindowSpecification m_WindowSpecification;
		struct SDL_Window* m_Window{ nullptr };
		EventCallback m_EventCallback;
		bool m_IsMinimized{ false };
		bool m_IsMaximized{ false };
	};

}