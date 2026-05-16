#include "pch.h"
#include "BinaryEngine/Window/Window.h"

#include "BinaryEngine/Event/EventTypes.h"

namespace BinaryEngine {

	Window::Window(const WindowSpecification& specification) :
		m_WindowSpecification(specification)
	{
		m_Window = SDL_CreateWindow(m_WindowSpecification.title.c_str(), m_WindowSpecification.resolution.x, m_WindowSpecification.resolution.y, SDL_WINDOW_RESIZABLE);

		if (!m_Window)
		{
			//TODO: Log that window has not been created
		}

	}

	Window::~Window()
	{
		if (m_Window)
		{
			SDL_DestroyWindow(m_Window);
		}

	}

	int Window::GetWidth() const
	{
		int width{};
		int height{};
		SDL_GetWindowSize(m_Window, &width, &height);
		return width;
	}

	int Window::GetHeight() const
	{
		int width{};
		int height{};
		SDL_GetWindowSize(m_Window, &width, &height);
		return height;
	}

	Vector2i Window::GetResolution() const
	{
		int width{};
		int height{};
		SDL_GetWindowSize(m_Window, &width, &height);
		return Vector2i{ width, height };
	}

	void Window::SetTitle(const std::string& title)
	{
		m_WindowSpecification.title = title;
		SDL_SetWindowTitle(m_Window, m_WindowSpecification.title.c_str());
	}

	void Window::SetSize(const Vector2i& size)
	{
		SDL_SetWindowSize(m_Window, size.x, size.y);
	}

	void Window::SetFullscreen(bool fullscreen)
	{
		SDL_SetWindowFullscreen(m_Window, fullscreen);
	}

	void Window::StartTextInput()
	{
		SDL_StartTextInput(m_Window);
	}

	void Window::StopTextInput()
	{
		SDL_StopTextInput(m_Window);
	}

	bool Window::IsTextInputActive() const
	{
		return SDL_TextInputActive(m_Window);
	}

	void Window::ProcessEvents()
	{
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			if (!m_EventCallback)
			{
				continue;
			}

			switch (sdlEvent.type)
			{
				case SDL_EVENT_QUIT:
				{
					WindowClosedEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_RESIZED:
				{
					WindowResizedEvent event{
						{ sdlEvent.window.data1, sdlEvent.window.data2 }
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_FOCUS_GAINED:
				{
					WindowFocusedGainedEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_FOCUS_LOST:
				{
					WindowFocusedGainedEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_MINIMIZED:
				{
					m_IsMinimized = true;
					m_IsMaximized = false;
					WindowMinimizedEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_MAXIMIZED:
				{
					m_IsMinimized = false;
					m_IsMaximized = true;
					WindowMaximizedEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_WINDOW_RESTORED:
				{
					m_IsMinimized = false;
					m_IsMaximized = false;
					WindowRestoredEvent event;
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_KEY_DOWN:
				{
					KeyPressedEvent event{
						static_cast<KeyCode>(sdlEvent.key.key),
						static_cast<ScanCode>(sdlEvent.key.scancode),
						static_cast<KeyModifier>(sdlEvent.key.mod),
						sdlEvent.key.repeat != 0
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_KEY_UP:
				{
					KeyReleasedEvent event{
						static_cast<KeyCode>(sdlEvent.key.key),
						static_cast<ScanCode>(sdlEvent.key.scancode),
						static_cast<KeyModifier>(sdlEvent.key.mod),
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					MouseButtonPressedEvent event{
						static_cast<MouseCode>(sdlEvent.button.button),
						{ sdlEvent.button.x, sdlEvent.button.y }
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					MouseButtonReleasedEvent event{
						static_cast<MouseCode>(sdlEvent.button.button),
						{ sdlEvent.button.x, sdlEvent.button.y }
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_MOUSE_MOTION:
				{
					MouseMovedEvent event{
						{ sdlEvent.motion.x, sdlEvent.motion.y },
						{ sdlEvent.motion.xrel, sdlEvent.motion.yrel }
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_MOUSE_WHEEL:
				{
					MouseScrolledEvent event{
						{ sdlEvent.wheel.x, sdlEvent.wheel.y }
					};
					m_EventCallback(event);
					break;
				}

				case SDL_EVENT_TEXT_INPUT:
				{
					TextInputEvent event{ sdlEvent.text.text };
					m_EventCallback(event);
					break;
				}

			}

		}
	}

}