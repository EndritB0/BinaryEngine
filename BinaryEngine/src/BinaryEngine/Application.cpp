#include "pch.h"
#include "BinaryEngine/Application.h"

#include "BinaryEngine/ApplicationSpecification.h"
#include "BinaryEngine/Core/Core.h"
#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/Event/EventTypes.h"
#include "BinaryEngine/State/Context.h"

namespace BinaryEngine {

	Application::PlatformContext::PlatformContext()
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			CORE_ERROR("[PlatformContext] Failed to initialise Platform Context: {}", SDL_GetError());
			return;
		}

		m_IsInitialised = true;
		CORE_INFO("[PlatformContext] Platform Context Initialised");
	}

	Application::PlatformContext::~PlatformContext()
	{
		SDL_Quit();
		CORE_INFO("[PlatformContext] Platform Context Shutdown");
	}

	Application::Application(const ApplicationSpecification& specification) :
		m_FrameClock(specification.Clock)
	{
		if (!m_PlatformContext.IsValid())
		{
			return;
		}

		m_Window.emplace(specification.Window);
		if (!m_Window->IsValid())
		{
			return;
		}

		m_Renderer.emplace(*m_Window, specification.Renderer);
		if (!m_Renderer->IsValid())
		{
			return;
		}

		m_CameraSpecification = specification.Camera;
		if (!m_CameraSpecification.DesignSize.has_value())
		{
			m_CameraSpecification.DesignSize = Vector2f{
				static_cast<float>(specification.Window.Resolution.x),
				static_cast<float>(specification.Window.Resolution.y)
			};
		}

		m_AudioEngine.emplace(specification.Audio, m_AssetManager);

		m_Window->SetEventCallback([this](Event& event) { this->OnEvent(event); });
		m_Window->SetLiveRedrawCallback([this]() { this->OnLiveRedraw(); });
		m_StateManager.emplace(Context{ *m_Window, *m_Renderer, m_AssetManager, *m_AudioEngine, m_CameraSpecification, m_FrameClock });
		CORE_INFO("[Application] Application Initialised");
	}

	Application::~Application()
	{
		CORE_INFO("[Application] Application Shutdown");
	}

	void Application::Run()
	{
		if (!IsApplicationInitialised())
		{
			CORE_ERROR("[Application] Application has not been initalised correctly");
			return;
		}

		const std::uint16_t fpsLimit{ m_Window->GetSpecification().FpsLimit };
		m_FrameBudgetNanoseconds = fpsLimit > 0 ? static_cast<std::uint64_t>(SDL_NS_PER_SECOND) / fpsLimit : 0;
		CORE_INFO("[Application] Frame rate limit: {}", fpsLimit == 0 ? "uncapped" : (std::to_string(fpsLimit) + " FPS"));
		CORE_INFO("[Application] Fixed update rate: {} Hz, max frame time: {:.0f} ms", m_FrameClock.GetFixedUpdateRate(), m_FrameClock.GetMaxFrameTime().GetMillisecondsDouble());

		m_IsRunning = true;
		m_FrameClock.Start(SDL_GetTicksNS());

		while (m_IsRunning)
		{
			ProcessEvents();
			if (!m_IsRunning)
			{
				break;
			}

			RunFrame();

			if (m_FrameBudgetNanoseconds > 0)
			{
				const std::uint64_t currentTime{ SDL_GetTicksNS() };
				if (currentTime < m_NextFrameDueTime)
				{
					SDL_DelayPrecise(m_NextFrameDueTime - currentTime);
				}
			}
		}

		CORE_INFO("[Application] Application Loop Ended");
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowClosedEvent>(BIND_FUNCTION(OnWindowClosed));

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

	void Application::Update()
	{
		m_AudioEngine->Update();

		while (m_FrameClock.ConsumeFixedStep())
		{
			m_StateManager->ProcessFixedUpdate(m_FrameClock.GetFixedStep());
		}

		m_StateManager->ProcessUpdate(m_FrameClock.GetDeltaTime());
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

	void Application::RunFrame()
	{
		m_IsInFrame = true;

		const std::uint64_t currentFrameTime{ SDL_GetTicksNS() };
		m_FrameClock.BeginFrame(currentFrameTime);

		const bool isMoreThanOneBudgetLate{ currentFrameTime >= m_NextFrameDueTime + m_FrameBudgetNanoseconds };
		m_NextFrameDueTime = (isMoreThanOneBudgetLate ? currentFrameTime : m_NextFrameDueTime) + m_FrameBudgetNanoseconds;

		Update();
		Render();
		PostFrame();

		m_IsInFrame = false;
	}

	bool Application::OnWindowClosed([[maybe_unused]] WindowClosedEvent& event)
	{
		StopApplication();
		return true;
	}

	void Application::OnLiveRedraw()
	{
		if (!m_IsRunning || m_IsInFrame)
		{
			return;
		}

		if (m_FrameBudgetNanoseconds > 0 && SDL_GetTicksNS() < m_NextFrameDueTime)
		{
			return;
		}

		RunFrame();
	}

}