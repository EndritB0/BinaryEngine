#pragma once

#include <cstdint>
#include <optional>
#include <type_traits>

#include "BinaryEngine/ApplicationSpecification.h"
#include "BinaryEngine/Asset/AssetManager.h"
#include "BinaryEngine/Core/Timestep.h"
#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/Event/EventTypes.h"
#include "BinaryEngine/Renderer/Renderer.h"
#include "BinaryEngine/State/State.h"
#include "BinaryEngine/State/StateManager.h"
#include "BinaryEngine/Window/Window.h"

namespace BinaryEngine {

	class Application {
	public:
		Application(const ApplicationSpecification specification);
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		template<typename T, typename... Args>
			requires(std::is_base_of_v<State, T>)
		void Add(Args&&... args)
		{
			m_StateManager->RequestPushState<T>(std::forward<Args>(args)...);
			m_StateManager->ApplyPendingChanges();
		}

		void Run();

	private:
		void OnEvent(Event& event);
		void StopApplication();
		void ProcessEvents();
		void Update(TimeStep dt);
		void Render();
		void PostFrame();
		bool OnWindowClosed(WindowClosedEvent& event);
		bool OnWindowResized(WindowResizedEvent& event);

	private:
		std::optional<Window> m_Window;
		std::optional<Renderer> m_Renderer;
		std::optional<StateManager> m_StateManager;
		AssetManager m_AssetManager;
		std::uint64_t m_LastFrameTime{};
		bool m_IsRunning{ false };

	};

}