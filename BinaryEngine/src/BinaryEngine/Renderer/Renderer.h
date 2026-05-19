#pragma once

#include "BinaryEngine/Core/Core.h"
#include "BinaryEngine/Core/Transform.h"
#include "BinaryEngine/Renderer/OrthographicCamera.h"
#include "BinaryEngine/Renderer/Texture2D.h"
#include "BinaryEngine/Window/Window.h"

namespace BinaryEngine {

	class Renderer {
	public:
		Renderer(const Window& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void* GetNativeRenderer() const { return m_Renderer; }
		void Clear();
		void Present();
		void SetDrawColor(Color color);
		void BeginScene(const OrthographicCamera& camera);
		void EndScene();
		void DrawTexture(const Texture2D& texture, const Transform& transform);

	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix{ 1.0f };
		};

		SceneData m_SceneData;
		struct SDL_Renderer* m_Renderer{ nullptr };

	};

}