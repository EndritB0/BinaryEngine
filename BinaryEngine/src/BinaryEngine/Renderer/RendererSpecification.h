#pragma once

#include <cstdint>

namespace BinaryEngine {

	enum class RenderAPI {
		Default = 0,
		Vulkan,
		Direct3D12,
		Metal,
	};

	enum class PresentMode {
		VSync = 0,
		Immediate,
		Mailbox,
	};

	enum class TextureFilter {
		Nearest = 0,
		Linear,
	};

	enum class ValidationMode {
		Disabled = 0,
		Enabled,
	};

	struct RendererSpecification {
		RenderAPI renderAPI{ RenderAPI::Default };
		PresentMode presentMode{ PresentMode::VSync };
		TextureFilter textureFilter{ TextureFilter::Nearest };
		ValidationMode validationMode{ ValidationMode::Disabled };
		std::uint8_t defaultAlpha{ 255 };
	};

}