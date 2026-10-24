#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"
#include "BinaryEngine/Renderer/TextSpecification.h"

namespace BinaryEngine {

	class Font;
	class OrthographicCamera;
	class Texture2D;
	class Window;
	struct TextureRegion;
	struct Transform;

	class Renderer {
	public:
		Renderer(const Window& window, const RendererSpecification& specification = RendererSpecification());
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		struct Statistics {
			std::uint32_t quadCount{ 0 };
			std::uint32_t drawCalls{ 0 };
			std::uint32_t culledSprites{ 0 };
			std::uint32_t glyphCount{ 0 };
			std::uint32_t quadCapacity{ 0 };
			std::uint32_t maxQuads{ 0 };
		};

		void* GetNativeDevice() const;
		bool IsValid() const;
		void SetViewport(Vector2i position, Vector2i size);
		void ResetViewport();
		Vector2i GetViewportPosition() const;
		Vector2i GetViewportSize() const;
		bool HasCustomViewport() const;
		void SetPresentMode(PresentMode mode);
		PresentMode GetPresentMode() const;
		void SetDefaultAlpha(std::uint8_t alpha);
		std::uint8_t GetDefaultAlpha() const;
		const Statistics& GetStatistics() const;
		void Clear();
		void Present();
		void SetClearColor(const Color& color);
		void BeginScene(OrthographicCamera& camera);
		void EndScene();
		void DrawSprite(const Texture2D& texture, const Transform& transform);
		void DrawSprite(const Texture2D& texture, const Transform& transform, const TextureRegion& region);
		void DrawText(const Font& font, std::string_view text, const Transform& transform, const TextSpecification& specification = TextSpecification());
		Vector2f MeasureText(const Font& font, std::string_view text, const TextSpecification& specification) const;

	private:
		struct RendererData;
		std::unique_ptr<RendererData> m_Data;
	};

}
