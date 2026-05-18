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

	void Renderer::DrawTexture(const Texture2D& texture, const Vector2f position, const Vector2f size)
	{
		SDL_FRect dstRect{
			.x {position.x},
			.y {position.y},
			.w {size.x},
			.h {size.y}
		};

		SDL_RenderTexture(m_Renderer, static_cast<SDL_Texture*>(texture.GetNativeTexture()), nullptr, &dstRect);
	}

}