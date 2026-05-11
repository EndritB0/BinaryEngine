#include "pch.h"

int main()
{
	BinaryEngine::WindowSpecification specification{
		.title {"SandBox"},
		.resolution {BinaryEngine::Vector2i{ 1280, 720 }},
		.fpsLimit {120},
		.fullscreen {false},
		.vsync {false}
	};

	BinaryEngine::Application app(specification);
	app.Run();

	return 0;
}