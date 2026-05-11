#include <pch.h>
#include "BinaryEngine/Window/Window.h"

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

}