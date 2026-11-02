#pragma once

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Renderer/RenderSpace.h"

namespace BinaryEngine {

	enum class TextAlignment {
		Left = 0,
		Center,
		Right
	};

	struct TextSpecification {
		float Size{ 16.0f };
		Color FillColor{ Color::White };
		RenderSpace Space{ RenderSpace::World };
		TextAlignment Alignment{ TextAlignment::Left };
		float LineSpacing{ 1.0f };
	};

}
