#pragma once

#include <functional>
#include <string>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Window/WindowSpecification.h"
#include "BinaryEngine/Event/Event.h"

struct SDL_Window;

namespace BinaryEngine {

	class Window {
	public:
		using EventCallback = std::function<void(Event&)>;
		using LiveRedrawCallback = std::function<void()>;

		Window(const WindowSpecification& specification);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		bool IsValid() const { return m_Window != nullptr; }
		const WindowSpecification GetSpecification() const { return m_WindowSpecification; }
		int GetWidth() const;
		int GetHeight() const;
		Vector2i GetResolution() const;
		bool IsFullscreen() const { return m_WindowSpecification.Fullscreen; }
		void SetTitle(const std::string& title);
		void SetSize(const Vector2i& size);
		void SetFullscreen(bool fullscreen);
		void StartTextInput();
		void StopTextInput();
		bool IsTextInputActive() const;
		bool IsMinimized() const { return m_IsMinimized; }
		bool IsMaximized() const { return m_IsMaximized; }
		void SetEventCallback(const EventCallback& callback) { m_EventCallback = callback; }
		void SetLiveRedrawCallback(const LiveRedrawCallback& callback) { m_LiveRedrawCallback = callback; }
		void ProcessEvents();

	private:
		friend class Renderer;
		::SDL_Window* GetNativeWindow() const { return m_Window; }

	private:
		WindowSpecification m_WindowSpecification;
		::SDL_Window* m_Window{ nullptr };
		EventCallback m_EventCallback;
		LiveRedrawCallback m_LiveRedrawCallback;
		bool m_IsMinimized{ false };
		bool m_IsMaximized{ false };
	};

}