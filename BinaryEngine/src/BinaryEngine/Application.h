#pragma once
#include <optional>

#include "BinaryEngine/Window/Window.h"
#include "BinaryEngine/Renderer/Renderer.h"

namespace BinaryEngine {

	class Application {
	public:
		Application(const WindowSpecification specification);
		~Application();

		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;

		void Run();

	private:
		void ProcessEvents();

	private:
		std::optional<Window> m_Window;
		std::optional<Renderer> m_Renderer;
		bool m_IsRunning{ false };

	};

}