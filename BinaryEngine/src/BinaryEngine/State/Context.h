#pragma once

namespace BinaryEngine {

	class Window;
	class Renderer;
	class AssetManager;

	struct Context {
		Window& window;
		Renderer& renderer;
		AssetManager& assetManager;
	};

}