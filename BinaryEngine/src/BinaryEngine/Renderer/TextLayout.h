#pragma once

#include <string_view>
#include <vector>

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/TextSpecification.h"

namespace BinaryEngine {

	class Font;

	struct LaidOutGlyph {
		Vector2f Position{ 0.0f, 0.0f };
		Vector2f Size{ 0.0f, 0.0f };
		Vector2f UVMin{ 0.0f, 0.0f };
		Vector2f UVMax{ 0.0f, 0.0f };
	};

	struct TextLayoutResult {
		std::vector<LaidOutGlyph> Glyphs;
		Vector2f Bounds{ 0.0f, 0.0f };
		int LineCount{ 0 };
	};

	void LayoutText(const Font& font, std::string_view text, const TextSpecification& specification, TextLayoutResult& result);

}
