#include "pch.h"
#include "BinaryEngine/Application.h"

#include "BinaryEngine/ApplicationSpecification.h"
#include "BinaryEngine/Core/Core.h"
#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/Event/EventTypes.h"
#include "BinaryEngine/State/Context.h"

namespace BinaryEngine {

	Application::Application(const ApplicationSpecification specification)
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			CORE_ERROR("[Application] Failed to initialize Application: {}", SDL_GetError());
			return;
		}

		m_Window.emplace(specification.Window);
		m_Renderer.emplace(*m_Window, specification.Renderer);
		m_Window->SetEventCallback([this](Event& event) { this->OnEvent(event); });
		m_StateManager.emplace(Context{ *m_Window, *m_Renderer, m_AssetManager });
		CORE_INFO("[Application] Application Initialised");
	}

	Application::~Application()
	{
		SDL_Quit();
		CORE_INFO("[Application] Application Shutdown");
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

		CORE_INFO("[Application] Application Loop Ended");
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowClosedEvent>(BIND_FUNCTION(OnWindowClosed));
		dispatcher.Dispatch<WindowResizedEvent>(BIND_FUNCTION(OnWindowResized));

		if (!event.handled)
		{
			m_StateManager->ProcessEvent(event);
		}

	}

	void Application::StopApplication()
	{
		m_IsRunning = false;
		CORE_TRACE("[Application] Stopping Application requested");
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

	bool Application::OnWindowClosed([[maybe_unused]] WindowClosedEvent& event)
	{
		StopApplication();
		return true;
	}

	bool Application::OnWindowResized(WindowResizedEvent& event)
	{
		m_Renderer->SetViewport({ 0, 0 }, { event.GetWidth(), event.GetHeight() });
		return false;
	}

}