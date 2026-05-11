#include "pch.h"
#include "BinaryEngine/Application.h"

namespace BinaryEngine {

	Application::Application(const WindowSpecification specification)
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			//TODO: Log failed initialization
			return;
		}

		m_Window.emplace(specification);
		m_Renderer.emplace(*m_Window);
	}

	Application::~Application()
	{
		SDL_Quit();
	}

	void Application::Run()
	{
		m_IsRunning = true;

		while (m_IsRunning)
		{
			ProcessEvents();
			m_Renderer->SetDrawColor(Color::White);
			m_Renderer->Clear();
			m_Renderer->Present();
		}

	}

	void Application::ProcessEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
			{
				m_IsRunning = false;
				return;
			}
		}
	}

}
