#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "BinaryEngine/Asset/Asset.h"
#include "BinaryEngine/Asset/AssetTypes.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/Texture2D.h"

namespace BinaryEngine {

	class Renderer;

	struct FontSpecification {
		float PixelSize{ 32.0f };
		Vector2i AtlasSize{ 1024, 1024 };
		std::uint32_t FirstCodepoint{ 32 };
		std::uint32_t CodepointCount{ 95 };
	};

	struct GlyphMetrics {
		Vector2f UVMin{ 0.0f, 0.0f };
		Vector2f UVMax{ 0.0f, 0.0f };
		Vector2f Offset{ 0.0f, 0.0f };
		Vector2f Size{ 0.0f, 0.0f };
		float Advance{ 0.0f };
	};

	class Font : public Asset {
	public:
		Font() = default;
		virtual ~Font() override = default;

		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;
		Font(Font&&) = delete;
		Font& operator=(Font&&) = delete;

		static AssetType GetStaticAssetType() { return AssetType::Font; }
		virtual AssetType GetAssetType() const override { return GetStaticAssetType(); }
		void Load(const std::filesystem::path& filePath, const Renderer& renderer, const FontSpecification& specification = FontSpecification());
		bool IsValid() const { return m_Atlas.IsValid() && !m_Glyphs.empty(); }
		const FontSpecification& GetSpecification() const { return m_Specification; }
		const Texture2D& GetTexture() const { return m_Atlas; }
		float GetPixelSize() const { return m_Specification.PixelSize; }
		float GetAscent() const { return m_Ascent; }
		float GetDescent() const { return m_Descent; }
		float GetLineHeight() const { return m_LineHeight; }
		const GlyphMetrics* GetGlyph(std::uint32_t codepoint) const;

	private:
		FontSpecification m_Specification{};
		Texture2D m_Atlas;
		std::vector<GlyphMetrics> m_Glyphs;
		float m_Ascent{ 0.0f };
		float m_Descent{ 0.0f };
		float m_LineHeight{ 0.0f };
	};

}
