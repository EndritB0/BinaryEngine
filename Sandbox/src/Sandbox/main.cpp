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
			.renderAPI {BinaryEngine::RenderAPI::Default},
			.presentMode {BinaryEngine::PresentMode::Immediate},
			.textureFilter {BinaryEngine::TextureFilter::Nearest},
			.validationMode {BinaryEngine::ValidationMode::Enabled},
		},

		.Camera = {
			.DesignSize {BinaryEngine::Vector2f{ 640.0f, 360.0f }},
			.ViewportMode {BinaryEngine::CameraViewportMode::FixedHeight},
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