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
		BinaryEngine::RenderAPI RenderAPI{ RenderAPI::Default };
		BinaryEngine::PresentMode PresentMode{ PresentMode::VSync };
		BinaryEngine::TextureFilter TextureFilter{ TextureFilter::Nearest };
		BinaryEngine::ValidationMode ValidationMode{ ValidationMode::Disabled };
		std::uint8_t DefaultAlpha{ 255 };
		std::uint32_t InitialQuadCapacity{ 10000 };
		std::uint32_t MaxQuadCapacity{ 1000000 };
		bool CullSprites{ true };
	};

}