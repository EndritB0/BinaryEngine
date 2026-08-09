#pragma once

namespace BinaryEngine {

	class Window;
	class Renderer;
	class AssetManager;
	struct CameraSpecification;

	struct Context {
		Window& window;
		Renderer& renderer;
		AssetManager& assetManager;
		const CameraSpecification& camera;
	};

}