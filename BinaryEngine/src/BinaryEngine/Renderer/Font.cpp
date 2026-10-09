#include "pch.h"
#include "BinaryEngine/Renderer/Font.h"

#include <stb_truetype.h>

#include "BinaryEngine/Renderer/Renderer.h"

namespace {

	constexpr int s_AtlasPadding{ 1 };

	std::vector<std::uint8_t> ExpandCoverageToRGBA(const std::vector<std::uint8_t>& coverage)
	{
		std::vector<std::uint8_t> pixels(coverage.size() * 4);

		for (std::size_t pixelIndex{}; pixelIndex < coverage.size(); pixelIndex++)
		{
			pixels[pixelIndex * 4 + 0] = 0xFF;
			pixels[pixelIndex * 4 + 1] = 0xFF;
			pixels[pixelIndex * 4 + 2] = 0xFF;
			pixels[pixelIndex * 4 + 3] = coverage[pixelIndex];
		}

		return pixels;
	}

}

namespace BinaryEngine {

	void Font::Load(const std::filesystem::path& filePath, const Renderer& renderer, const FontSpecification& specification)
	{
		const std::string pathString{ filePath.string() };

		if (specification.PixelSize <= 0.0f || specification.AtlasSize.x <= 0 || specification.AtlasSize.y <= 0 || specification.CodepointCount == 0)
		{
			CORE_ERROR("[Font] Invalid FontSpecification for {}: pixel size {}, atlas {}x{}, {} codepoints", pathString, specification.PixelSize, specification.AtlasSize.x, specification.AtlasSize.y, specification.CodepointCount);
			return;
		}

		std::ifstream file(pathString, std::ios::binary | std::ios::ate);
		if (!file)
		{
			CORE_ERROR("[Font] Failed to open font file: {}", pathString);
			return;
		}

		const std::streamsize fontFileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<std::uint8_t> fontData(static_cast<std::size_t>(fontFileSize));
		if (!file.read(reinterpret_cast<char*>(fontData.data()), fontFileSize))
		{
			CORE_ERROR("[Font] Failed to read font file: {}", pathString);
			return;
		}

		stbtt_fontinfo fontInfo{};
		if (!stbtt_InitFont(&fontInfo, fontData.data(), 0))
		{
			CORE_ERROR("[Font] Failed to parse font file: {}", pathString);
			return;
		}

		int ascent{};
		int descent{};
		int lineGap{};
		stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

		const float pixelScale{ stbtt_ScaleForPixelHeight(&fontInfo, specification.PixelSize) };

		const std::size_t atlasWidth{ static_cast<std::size_t>(specification.AtlasSize.x) };
		const std::size_t atlasHeight{ static_cast<std::size_t>(specification.AtlasSize.y) };
		std::vector<std::uint8_t> coverage(atlasWidth * atlasHeight, 0);

		stbtt_pack_context packContext{};
		if (!stbtt_PackBegin(&packContext, coverage.data(), specification.AtlasSize.x, specification.AtlasSize.y, 0, s_AtlasPadding, nullptr))
		{
			CORE_ERROR("[Font] Failed to begin atlas packing: {}", pathString);
			return;
		}

		std::vector<stbtt_packedchar> packedCharacters(specification.CodepointCount);

		const int packSucceeded{ stbtt_PackFontRange(&packContext, fontData.data(), 0, specification.PixelSize,static_cast<int>(specification.FirstCodepoint), static_cast<int>(specification.CodepointCount), packedCharacters.data()) };

		stbtt_PackEnd(&packContext);

		if (!packSucceeded)
		{
			CORE_ERROR("[Font] {} glyphs at {} pixels do not fit a {}x{} atlas: {}. Raise FontSpecification::AtlasSize or lower PixelSize", specification.CodepointCount, specification.PixelSize, specification.AtlasSize.x, specification.AtlasSize.y, pathString);
			return;
		}

		const float atlasWidthFloat{ static_cast<float>(specification.AtlasSize.x) };
		const float atlasHeightFloat{ static_cast<float>(specification.AtlasSize.y) };

		std::vector<GlyphMetrics> glyphs(specification.CodepointCount);
		for (std::size_t glyphIndex{}; glyphIndex < packedCharacters.size(); glyphIndex++)
		{
			const stbtt_packedchar& packedCharacter{ packedCharacters[glyphIndex] };
			GlyphMetrics& glyph{ glyphs[glyphIndex] };

			glyph.UVMin = { static_cast<float>(packedCharacter.x0) / atlasWidthFloat, static_cast<float>(packedCharacter.y0) / atlasHeightFloat };
			glyph.UVMax = { static_cast<float>(packedCharacter.x1) / atlasWidthFloat, static_cast<float>(packedCharacter.y1) / atlasHeightFloat };
			glyph.Offset = { packedCharacter.xoff, packedCharacter.yoff };
			glyph.Size = { static_cast<float>(packedCharacter.x1 - packedCharacter.x0), static_cast<float>(packedCharacter.y1 - packedCharacter.y0) };
			glyph.Advance = packedCharacter.xadvance;
		}

		const std::vector<std::uint8_t> atlasPixels{ ExpandCoverageToRGBA(coverage) };

		if (!m_Atlas.Create(renderer, atlasPixels.data(), specification.AtlasSize, pathString))
		{
			CORE_ERROR("[Font] Failed to upload font atlas: {}", pathString);
			return;
		}

		m_Specification = specification;
		m_Glyphs = std::move(glyphs);
		m_Ascent = static_cast<float>(ascent) * pixelScale;
		m_Descent = static_cast<float>(descent) * pixelScale;
		m_LineHeight = static_cast<float>(ascent - descent + lineGap) * pixelScale;

		CORE_TRACE("[Font] Loaded Font successfully: {} ({} glyphs at {} pixels, {}x{} atlas)", pathString, m_Glyphs.size(), m_Specification.PixelSize, m_Specification.AtlasSize.x, m_Specification.AtlasSize.y);
	}

	const GlyphMetrics* Font::GetGlyph(std::uint32_t codepoint) const
	{
		if (codepoint < m_Specification.FirstCodepoint)
		{
			return nullptr;
		}

		const std::uint32_t glyphIndex{ codepoint - m_Specification.FirstCodepoint };
		if (glyphIndex >= m_Glyphs.size())
		{
			return nullptr;
		}

		return &m_Glyphs[glyphIndex];
	}

}
