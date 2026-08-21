#pragma once

#include "BinaryEngine/Core/Color.h"

namespace BinaryEngine {

	enum class TextSpace {
		World = 0,
		Screen
	};

	enum class TextAlignment {
		Left = 0,
		Center,
		Right
	};

	struct TextSpecification {
		float Size{ 16.0f };
		Color FillColor{ Color::White };
		TextSpace Space{ TextSpace::World };
		TextAlignment Alignment{ TextAlignment::Left };
		float LineSpacing{ 1.0f };
	};

}
