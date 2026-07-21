#pragma once

#include "BinaryEngine/Renderer/RendererSpecification.h"
#include "BinaryEngine/Window/WindowSpecification.h"

namespace BinaryEngine {

	struct ApplicationSpecification {
		WindowSpecification Window{};
		RendererSpecification Renderer{};
	};

}