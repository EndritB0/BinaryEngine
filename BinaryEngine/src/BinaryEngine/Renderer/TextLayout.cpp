#include "pch.h"
#include "BinaryEngine/Renderer/TextLayout.h"

#include "BinaryEngine/Renderer/Font.h"

namespace {

	float MeasureLineWidth(const BinaryEngine::Font& font, std::string_view line)
	{
		float lineWidth{ 0.0f };

		for (const char character : line)
		{
			const BinaryEngine::GlyphMetrics* glyph{ font.GetGlyph(static_cast<std::uint32_t>(static_cast<unsigned char>(character))) };
			if (!glyph)
			{
				continue;
			}

			lineWidth += glyph->Advance;
		}

		return lineWidth;
	}

	float AlignmentOffset(BinaryEngine::TextAlignment alignment, float lineWidth)
	{
		switch (alignment)
		{
			case BinaryEngine::TextAlignment::Center: return -lineWidth / 2.0f;
			case BinaryEngine::TextAlignment::Right: return -lineWidth;
			case BinaryEngine::TextAlignment::Left:
			default: return 0.0f;
		}
	}

}

namespace BinaryEngine {

	void LayoutText(const Font& font, std::string_view text, const TextSpecification& specification, TextLayoutResult& result)
	{
		result.Glyphs.clear();
		result.Bounds = { 0.0f, 0.0f };
		result.LineCount = 0;

		if (!font.IsValid() || text.empty())
		{
			return;
		}

		const float lineAdvance{ font.GetLineHeight() * specification.LineSpacing };

		float penY{ 0.0f };
		float widestLineWidth{ 0.0f };
		std::size_t lineStart{ 0 };

		while (lineStart <= text.size())
		{
			const std::size_t lineBreak{ text.find('\n', lineStart) };
			const std::size_t lineEnd{ lineBreak == std::string_view::npos ? text.size() : lineBreak };
			const std::string_view line{ text.substr(lineStart, lineEnd - lineStart) };

			const float lineWidth{ MeasureLineWidth(font, line) };
			widestLineWidth = std::max(widestLineWidth, lineWidth);

			float penX{ AlignmentOffset(specification.Alignment, lineWidth) };

			for (const char character : line)
			{
				const GlyphMetrics* glyph{ font.GetGlyph(static_cast<std::uint32_t>(static_cast<unsigned char>(character))) };
				if (!glyph)
				{
					continue;
				}

				if (glyph->Size.x > 0.0f && glyph->Size.y > 0.0f)
				{
					result.Glyphs.emplace_back(Vector2f{ penX + glyph->Offset.x, penY + glyph->Offset.y }, glyph->Size, glyph->UVMin, glyph->UVMax);
				}

				penX += glyph->Advance;
			}

			result.LineCount++;
			penY += lineAdvance;

			if (lineBreak == std::string_view::npos)
			{
				break;
			}

			lineStart = lineBreak + 1;
		}

		result.Bounds = { widestLineWidth, static_cast<float>(result.LineCount) * lineAdvance };
	}

}
