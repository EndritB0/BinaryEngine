#include "pch.h"
#include "Sandbox/States/IntroState.h"

int main()
{
	BinaryEngine::ApplicationSpecification specification{
		.Window = {
			.title {"SandBox"},
			.resolution {BinaryEngine::Vector2i{ 1280, 720 }},
			.fpsLimit {120},
			.fullscreen {false},
		},

		.Renderer = {
			.vSync {false},
			.blendMode {BinaryEngine::BlendMode::Blend},
		}
	};

	BinaryEngine::Log::Init(specification.Window.title);

	{
		BinaryEngine::Application app(specification);
		app.Add<Sandbox::IntroState>();
		app.Run();
	}

	return 0;
}