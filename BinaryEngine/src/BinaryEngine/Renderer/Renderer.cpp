#include "pch.h"
#include "BinaryEngine/Renderer/Renderer.h"

namespace BinaryEngine {

	Renderer::Renderer(const Window& window)
	{
		m_Renderer = SDL_CreateRenderer(static_cast<SDL_Window*>(window.GetNativeWindow()), nullptr);

		if (!m_Renderer)
		{
			//TODO: Log that Renderer failed to be created
		}

		int vsync = window.GetSpecification().vsync ? 1 : 0;

		SDL_SetRenderVSync(m_Renderer, vsync);
	}

	Renderer::~Renderer()
	{
		if (m_Renderer)
		{
			SDL_DestroyRenderer(m_Renderer);
		}
	}

	void Renderer::SetViewport(Vector2i position, Vector2i size)
	{
		SDL_Rect rect{
			.x{static_cast<int>(position.x)},
			.y{static_cast<int>(position.y)},
			.w{static_cast<int>(size.x)},
			.h{static_cast<int>(size.y)}
		};
		SDL_SetRenderViewport(m_Renderer, &rect);
	}

	void Renderer::Clear()
	{
		SDL_RenderClear(m_Renderer);
	}

	void Renderer::Present()
	{
		SDL_RenderPresent(m_Renderer);
	}

	void Renderer::SetDrawColor(Color color)
	{
		SDL_SetRenderDrawColor(m_Renderer, color.red, color.green, color.blue, color.alpha);
	}

	void Renderer::BeginScene(const OrthographicCamera& camera)
	{
		m_SceneData.ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::DrawTexture(const Texture2D& texture, const Transform& transform)
	{
		Vector2f textureSize{ texture.GetSize() };
		glm::mat4 sizeScale{ glm::scale(glm::mat4(1.0f), glm::vec3(textureSize.x, textureSize.y, 1.0f)) };
		glm::mat4 modelMatrix{ transform.GetModelMatrix() * sizeScale };

		glm::mat4 mvpMatrix{ m_SceneData.ViewProjectionMatrix * modelMatrix };

		constexpr glm::vec4 s_LocalVertices[4]{
			{ -0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 0.0f, 1.0f },
			{  0.5f,  0.5f, 0.0f, 1.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f },
		};

		constexpr glm::vec2 s_TextureCoordinates[4]{
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f },
		};

		constexpr int s_QuadIndices[6]{ 0, 1, 2, 2, 3, 0 };
		constexpr SDL_FColor s_WhiteTint{ 1.0f, 1.0f, 1.0f, 1.0f };

		int renderWidth{};
		int renderHeight{};
		SDL_GetCurrentRenderOutputSize(m_Renderer, &renderWidth, &renderHeight);
		float screenWidth{ static_cast<float>(renderWidth) };
		float screenHeight{ static_cast<float>(renderHeight) };

		SDL_Vertex finalVertices[4];

		for (int i{}; i < 4; i++)
		{
			glm::vec4 ndcPosition{ mvpMatrix * s_LocalVertices[i] };

			float pixelX{ (ndcPosition.x + 1.0f) * 0.5f * screenWidth };
			float pixelY{ (1.0f - ndcPosition.y) * 0.5f * screenHeight };

			finalVertices[i].position.x = pixelX;
			finalVertices[i].position.y = pixelY;
			finalVertices[i].tex_coord.x = s_TextureCoordinates[i].x;
			finalVertices[i].tex_coord.y = s_TextureCoordinates[i].y;
			finalVertices[i].color = s_WhiteTint;
		}

		SDL_RenderGeometry(m_Renderer, static_cast<SDL_Texture*>(texture.GetNativeTexture()), finalVertices, 4, s_QuadIndices, 6);
	}

}