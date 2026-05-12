#include "pch.h"
#include "BinaryEngine/Application.h"

#include "BinaryEngine/Event/EventTypes.h"

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
		m_Window->SetEventCallback([this](Event& event) { this->OnEvent(event); });
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
			m_Window->ProcessEvents();
			m_Renderer->SetDrawColor(Color::White);
			m_Renderer->Clear();
			m_Renderer->Present();
		}

	}

	void Application::OnEvent(Event& event)
	{
		switch (event.GetEventType())
		{
			case EventType::WindowClosed:
			{
				m_IsRunning = false;
				return;
			}

			case EventType::WindowResized:
			{
				CORE_INFO("Window resized to {0}x{1}", m_Window->GetWidth(), m_Window->GetHeight());
				return;
			}
		}
	}
}