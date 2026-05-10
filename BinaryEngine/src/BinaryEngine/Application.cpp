#include "pch.h"
#include "BinaryEngine/Application.h"

namespace BinaryEngine {

	void Application::Run()
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			SDL_Log("SDL failed to initialize: %s\n", SDL_GetError());
			return;
		}

		SDL_Log("Hello World!");

		SDL_Quit();
	}

}
