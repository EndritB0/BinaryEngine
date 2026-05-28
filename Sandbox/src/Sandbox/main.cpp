#include "pch.h"
#include "Sandbox/States/IntroState.h"

int main()
{
	BinaryEngine::WindowSpecification specification{
		.title {"SandBox"},
		.resolution {BinaryEngine::Vector2i{ 1280, 720 }},
		.fpsLimit {120},
		.fullscreen {false},
	};

	BinaryEngine::Log::Init(specification.title);

	{
		BinaryEngine::Application app(specification);
		app.Add<Sandbox::IntroState>();
		app.Run();
	}

	return 0;
}