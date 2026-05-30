#pragma once

#include "BinaryEngine/Window/WindowSpecification.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"

namespace BinaryEngine {

	struct ApplicationSpecification {
		WindowSpecification Window{};
		RendererSpecification Renderer{};
	};

}