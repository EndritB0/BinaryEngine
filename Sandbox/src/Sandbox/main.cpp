#include "pch.h"

#include "Sandbox/SandboxOptions.h"
#include "Sandbox/States/SandboxShellState.h"

int main(int argc, char* argv[])
{
	BinaryEngine::ApplicationSpecification specification{
		.Window = {
			.Title{ "SandBox" },
			.Resolution{ BinaryEngine::Vector2i{ 1280, 720 } },
			.FpsLimit{ 120 },
			.Fullscreen{ false },
		},

		.Renderer = {
			.RenderAPI{ BinaryEngine::RenderAPI::Default },
			.PresentMode{ BinaryEngine::PresentMode::Immediate },
			.TextureFilter{ BinaryEngine::TextureFilter::Nearest },
			.ValidationMode{ BinaryEngine::ValidationMode::Enabled },
		},

		.Camera = {
			.DesignSize{ BinaryEngine::Vector2f{ 640.0f, 360.0f } },
			.ViewportMode{ BinaryEngine::CameraViewportMode::FixedHeight },
			.SnapMode{ BinaryEngine::CameraSnapMode::PixelPerfect },
		},

		.Audio = {
			.Buses = {
				{ .Name{ "Music" } },
				{ .Name{ "SFX" } },
				{ .Name{ "Ambience" }, .Parent{ "SFX" } },
			},
		},
	};

	BinaryEngine::Log::Init(specification.Window.Title);

	const std::optional<Sandbox::SandboxOptions> options{ Sandbox::ParseSandboxOptions(argc, argv) };
	if (!options.has_value())
	{
		return 1;
	}

	{
		BinaryEngine::Application app(specification);
		app.Add<Sandbox::SandboxShellState>(*options);
		app.Run();
	}

	return 0;
}
