#pragma once

struct SDL_GPUGraphicsPipeline;

namespace BinaryEngine::Detail {

	struct RenderState {
		::SDL_GPUGraphicsPipeline* Pipeline{ nullptr };
	};

}
