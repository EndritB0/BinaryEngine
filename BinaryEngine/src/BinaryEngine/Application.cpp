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
		m_StateManager.emplace(Context{ *m_Window, *m_Renderer });
	}

	Application::~Application()
	{
		SDL_Quit();
	}

	void Application::Run()
	{
		m_IsRunning = true;
		m_LastFrameTime = SDL_GetTicksNS();

		while (m_IsRunning)
		{
			std::uint64_t currentFrameTime{ SDL_GetTicksNS() };
			TimeStep dt{ currentFrameTime - m_LastFrameTime };
			m_LastFrameTime = currentFrameTime;

			ProcessEvents();
			Update(dt);
			Render();
			PostFrame();
		}

	}

	void Application::OnEvent(Event& event)
	{
		switch (event.GetEventType())
		{
			case EventType::WindowClosed:
			{
				StopApplication();
				event.handled = true;
				return;
			}

			case EventType::WindowResized:
			{
				CORE_INFO("Window resized to {0}x{1}", m_Window->GetWidth(), m_Window->GetHeight());
				break;
			}

			default:
			{
				break;
			}
		}

		if (!event.handled)
		{
			m_StateManager->ProcessEvent(event);
		}
	}

	void Application::StopApplication()
	{
		m_IsRunning = false;
	}

	void Application::ProcessEvents()
	{
		m_Window->ProcessEvents();
	}

	void Application::Update(TimeStep dt)
	{
		m_StateManager->ProcessUpdate(dt);
	}

	void Application::Render()
	{
		m_Renderer->Clear();
		m_StateManager->ProcessRender();
		m_Renderer->Present();
	}

	void Application::PostFrame()
	{
		m_StateManager->ApplyPendingChanges();
		if (!m_StateManager->HasState())
		{
			StopApplication();
		}
	}
}