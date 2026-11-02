#pragma once

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Renderer/RenderSpace.h"

namespace BinaryEngine {

	struct ShapeSpecification {
		BinaryEngine::Color Color{ Color::White };
		float OutlineThickness{ 0.0f };
		RenderSpace Space{ RenderSpace::World };
	};

	struct LineSpecification {
		BinaryEngine::Color Color{ Color::White };
		float Thickness{ 1.0f };
		float SortZ{ 0.0f };
		RenderSpace Space{ RenderSpace::World };
	};

}
