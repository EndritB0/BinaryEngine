#pragma once

#include <cstdint>
#include <glm/fwd.hpp>

#include "BinaryEngine/Core/Color.h"
#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/BlendMode.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"
#include "BinaryEngine/Renderer/RendererSpecification.h"
#include "BinaryEngine/Renderer/Texture2D.h"
#include "BinaryEngine/Window/Window.h"

namespace BinaryEngine {

	class Renderer {
	public:
		Renderer(const Window& window, const RendererSpecification& specification = RendererSpecification());
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void* GetNativeRenderer() const { return m_Renderer; }
		void SetViewport(Vector2i position, Vector2i size);
		void SetBlendMode(BlendMode mode);
		BlendMode GetBlendMode() const { return m_Specification.blendMode; }
		void SetDefaultAlpha(std::uint8_t alpha) { m_Specification.defaultAlpha = alpha; }
		std::uint8_t GetDefaultAlpha() const { return m_Specification.defaultAlpha; }
		void SetRenderingSettings(BlendMode, std::uint8_t alpha = 255);
		void Clear();
		void Present();
		void SetDrawColor(Color color);
		void BeginScene(const OrthographicCamera& camera);
		void EndScene();
		void DrawTexture(const Texture2D& texture, const Transform& transform);

	private:
		void ApplyRenderSettings();

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix{ 1.0f };
		};

		SceneData m_SceneData;
		struct SDL_Renderer* m_Renderer{ nullptr };
		RendererSpecification m_Specification;
	};

}